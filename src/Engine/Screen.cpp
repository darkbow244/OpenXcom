/*
 * Copyright 2010-2014 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Screen.h"
#include <sstream>
#include <cmath>
#include <iomanip>
#include <cassert>
#include <limits.h>
#include "../lodepng.h"
#include "Exception.h"
#include "Surface.h"
#include "Logger.h"
#include "Action.h"
#include "Options.h"
#include "CrossPlatform.h"
#include "Zoom.h"
#include "Timer.h"
#include <SDL.h>
#include "Renderer.h"
#include "SDLRenderer.h"
#include "OpenGLRenderer.h"

namespace OpenXcom
{

const int Screen::ORIGINAL_WIDTH = 320;
const int Screen::ORIGINAL_HEIGHT = 200;

/**
 * Sets up all the internal display flags depending on
 * the current video settings.
 */
void Screen::makeVideoFlags()
{
	_flags = SDL_WINDOW_OPENGL;
	if (Options::allowResize)
	{
		_flags |= SDL_WINDOW_RESIZABLE;
	}

#if 0
	if (Options::asyncBlit)
	{
		_flags |= SDL_ASYNCBLIT;
	}
	if (isOpenGLEnabled())
	{
		_flags = SDL_OPENGL;
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	}
#endif

	// Handle display mode
	if (Options::fullscreen)
	{
		_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (Options::borderless)
	{
		_flags |= SDL_WINDOW_BORDERLESS;
	}

	// Try fixing the Samsung devices - see https://bugzilla.libsdl.org/show_bug.cgi?id=2291
#ifdef __ANDROID__
	if (Options::forceGLMode) 
	{
		Log(LOG_INFO) << "Setting GL format to RGB565";
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	}
#endif

	//_bpp = (isHQXEnabled() || isOpenGLEnabled()) ? 32 : 8;
	_bpp = 32;
	_baseWidth = Options::baseXResolution;
	_baseHeight = Options::baseYResolution;
}


/**
 * Initializes a new display screen for the game to render contents to.
 * The screen is set up based on the current options.
 */
Screen::Screen() : _baseWidth(ORIGINAL_WIDTH), _baseHeight(ORIGINAL_HEIGHT), _scaleX(1.0), _scaleY(1.0), _numColors(0), _firstColor(0), _pushPalette(false), _surface(0), _window(NULL), _renderer(NULL), _texture(NULL)
	, _prevWidth(0), _prevHeight(0)
{
	// The default values for _window and _renderer are set to NULL so that we can check if there's a window already
	resetDisplay();	
	memset(deferredPalette, 0, 256*sizeof(SDL_Color));
}

/**
 * Deletes the buffer from memory. The display screen itself
 * is automatically freed once SDL shuts down.
 */
Screen::~Screen()
{
	delete _renderer;	
	delete _surface;
}

/**
 * Returns the screen's internal buffer surface. Any
 * contents that need to be shown will be blitted to this.
 * @return Pointer to the buffer surface.
 */
Surface *Screen::getSurface()
{
	_pushPalette = true;
	return _surface;
}

/**
 * Handles screen key shortcuts.
 * @param action Pointer to an action.
 */
void Screen::handle(Action *action)
{
	if (Options::debug)
	{
		if (action->getDetails()->type == SDL_KEYDOWN && action->getDetails()->key.keysym.sym == SDLK_F8)
		{
			switch(Timer::gameSlowSpeed)
			{
				case 1: Timer::gameSlowSpeed = 5; break;
				case 5: Timer::gameSlowSpeed = 15; break;
				default: Timer::gameSlowSpeed = 1; break;
			}				
		}
	}
	
	if (action->getDetails()->type == SDL_KEYDOWN && action->getDetails()->key.keysym.sym == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT) != 0)
	{
		Options::fullscreen = !Options::fullscreen;
		resetDisplay();
	}
	else if (action->getDetails()->type == SDL_KEYDOWN && action->getDetails()->key.keysym.sym == Options::keyScreenshot)
	{
		std::ostringstream ss;
		int i = 0;
		do
		{
			ss.str("");
			ss << Options::getUserFolder() << "screen" << std::setfill('0') << std::setw(3) << i << ".png";
			i++;
		}
		while (CrossPlatform::fileExists(ss.str()));
		screenshot(ss.str());
		return;
	}
}


/**
 * Renders the buffer's contents onto the screen, applying
 * any necessary filters or conversions in the process.
 * If the scaling factor is bigger than 1, the entire contents
 * of the buffer are resized by that factor (eg. 2 = doubled)
 * before being put on screen.
 */
void Screen::flip()
{
	_renderer->flip(_surface->getSurface());
}

/**
 * Clears all the contents out of the internal buffer.
 */
void Screen::clear()
{
	_surface->clear();

#if 0
	if (_screen->flags & SDL_SWSURFACE) memset(_screen->pixels, 0, _screen->h*_screen->pitch);
	else SDL_FillRect(_screen, &_clear, 0);
#endif
}

/**
 * Changes the 8bpp palette used to render the screen's contents.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 * @param immediately Apply palette changes immediately, otherwise wait for next blit.
 */
void Screen::setPalette(SDL_Color* colors, int firstcolor, int ncolors, bool immediately)
{
	if (_numColors && (_numColors != ncolors) && (_firstColor != firstcolor))
	{
		// an initial palette setup has not been comitted to the screen yet
		// just update it with whatever colors are being sent now
		memmove(&(deferredPalette[firstcolor]), colors, sizeof(SDL_Color)*ncolors);
		_numColors = 256; // all the use cases are just a full palette with 16-color follow-ups
		_firstColor = 0;
	} else
	{
		memmove(&(deferredPalette[firstcolor]), colors, sizeof(SDL_Color) * ncolors);
		_numColors = ncolors;
		_firstColor = firstcolor;
	}

	_surface->setPalette(colors, firstcolor, ncolors);

#if 0
	// defer actual update of screen until SDL_Flip()
	if (immediately && _screen->format->BitsPerPixel == 8 && SDL_SetColors(_screen, colors, firstcolor, ncolors) == 0)
	{
		Log(LOG_DEBUG) << "Display palette doesn't match requested palette";
	}
#endif

	// Sanity check
	/*
	SDL_Color *newcolors = _screen->format->palette->colors;
	for (int i = firstcolor, j = 0; i < firstcolor + ncolors; i++, j++)
	{
		Log(LOG_DEBUG) << (int)newcolors[i].r << " - " << (int)newcolors[i].g << " - " << (int)newcolors[i].b;
		Log(LOG_DEBUG) << (int)colors[j].r << " + " << (int)colors[j].g << " + " << (int)colors[j].b;
		if (newcolors[i].r != colors[j].r ||
			newcolors[i].g != colors[j].g ||
			newcolors[i].b != colors[j].b)
		{
			Log(LOG_ERROR) << "Display palette doesn't match requested palette";
			break;
		}
	}
	*/
}

/**
 * Returns the screen's 8bpp palette.
 * @return Pointer to the palette's colors.
 */
SDL_Color *Screen::getPalette() const
{
	return (SDL_Color*)deferredPalette;
}

/**
 * Returns the width of the screen.
 * @return Width in pixels.
 */
int Screen::getWidth() const
{
	int w, h;
	SDL_GetWindowSize(_window, &w, &h);
	return w;
}

/**
 * Returns the height of the screen.
 * @return Height in pixels
 */
int Screen::getHeight() const
{
	int w, h;
	SDL_GetWindowSize(_window, &w, &h);
	return h;
}

/**
 * Resets the screen surfaces based on the current display options,
 * as they don't automatically take effect.
 * @param resetVideo Reset display surface.
 */
void Screen::resetDisplay(bool resetVideo)
{
	int width = Options::displayWidth;
	int height = Options::displayHeight;
	bool switchRenderer = false;
	if (!_renderer)
	{
		switchRenderer = true;
	}
	else
	{
		if ( (Options::useOpenGL && _renderer->getRendererType() == RENDERER_SDL2)
			|| (!Options::useOpenGL && _renderer->getRendererType() == RENDERER_OPENGL) )
		{
			switchRenderer = true;
		}
	}
#ifdef __linux__
	Uint32 oldFlags = _flags;
#endif
	makeVideoFlags();
	// A kludge to make video resolution changing work
	resetVideo = ( (_prevWidth != _baseWidth) || (_prevHeight != _baseHeight) ) || resetVideo;

	Log(LOG_INFO) << "Current _baseWidth x _baseHeight: " << _baseWidth << "x" << _baseHeight;

	if (!_surface || (_surface && 
		(_surface->getSurface()->format->BitsPerPixel != _bpp || 
		_surface->getSurface()->w != _baseWidth ||
		_surface->getSurface()->h != _baseHeight))) // don't reallocate _surface if not necessary, it's a waste of CPU cycles
	{
		if (_surface) delete _surface;
		//_surface = new Surface(_baseWidth, _baseHeight, 0, 0, Screen::isHQXEnabled() ? 32 : 8); // only HQX needs 32bpp for this surface; the OpenGL class has its own 32bpp buffer
		/* So why exactly does the new surface have fixed size? */
		_surface = new Surface(_baseWidth, _baseHeight, 0, 0, 32);
		if (_surface->getSurface()->format->BitsPerPixel == 8) _surface->setPalette(deferredPalette);
	}
	SDL_SetColorKey(_surface->getSurface(), 0, 0); // turn off color key! 

	if (resetVideo /*|| _screen->format->BitsPerPixel != _bpp */)
	{
		/* FIXME: leak? */
		Log(LOG_INFO) << "Attempting to set display to " << width << "x" << height << "x" << _bpp << "...";
		/* Attempt to set scaling */
		if (Options::useNearestScaler)
		{
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		}
		else if (Options::useLinearScaler)
		{
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		}
		else if (Options::useAnisotropicScaler)
		{
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
		}
		// Hack: you have to destroy and recreate a window to make it resizable.
		if (_window)
		{
			if ((SDL_GetWindowFlags(_window) & SDL_WINDOW_RESIZABLE) != (_flags & SDL_WINDOW_RESIZABLE))
			{
				if (_renderer)
				{
					delete _renderer;
					_renderer = NULL;
				}
				SDL_DestroyWindow(_window);
				_window = NULL;
			}
		}
		/* Now, we only need to create a window AND a renderer when we have none*/
		if (_window == NULL)
		{
			Log(LOG_INFO) << "Attempting to create a new window since we have none yet";
			int winX, winY;
			if (Options::borderless)
			{
				winX = SDL_WINDOWPOS_CENTERED;
				winY = SDL_WINDOWPOS_CENTERED;
			}
			else
			{
				if ((Options::windowedModePositionX != -1) && (Options::windowedModePositionY != -1))
				{
					winX = Options::windowedModePositionX;
					winY = Options::windowedModePositionY;
				}
				else
				{
					winX = SDL_WINDOWPOS_UNDEFINED;
					winY = SDL_WINDOWPOS_UNDEFINED;
				}
			}
			_window = SDL_CreateWindow("OpenXcom",
						   winX,
						   winY,
						   width, height, _flags);
			/* In case something went horribly wrong */
			if (_window == NULL)
			{
				Log(LOG_ERROR) << SDL_GetError();
				throw Exception(SDL_GetError());
			}
			Log(LOG_INFO) << "Created a window, size is: " << width << "x" << height;
		}
		else
		{
#ifndef __ANDROID__
			SDL_SetWindowSize(_window, width, height);
#endif
			SDL_SetWindowBordered(_window, Options::borderless? SDL_FALSE : SDL_TRUE);
			SDL_SetWindowFullscreen(_window, Options::fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		}

		/* By this point we have a window and no renderer, so it's time to make one! */
		if (switchRenderer)
		{
			delete _renderer;
			_renderer = NULL;
		}

		if (!_renderer)
		{
#ifndef __NO_OPENGL
			if (Options::useOpenGL)
			{
				_renderer = new OpenGLRenderer(_window);
			}
			else
			{
				_renderer = new SDLRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
			}
#else
			_renderer = new SDLRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
#endif
			//_renderer = new OpenGLRenderer(_window/*, -1, SDL_RENDERER_ACCELERATED */);
			_renderer->setPixelFormat(_surface->getSurface()->format->format);
		}
		SDL_Rect baseRect;
		baseRect.x = baseRect.y = 0;
		baseRect.w = _baseWidth;
		baseRect.h = _baseHeight;
		_renderer->setInternalRect(&baseRect);
		Log(LOG_INFO) << "Display set to " << getWidth() << "x" << getHeight() << "x32";
		
		/* Save new baseWidth and baseHeight */
		_prevWidth = _baseWidth;
		_prevHeight = _baseHeight;
	}
	else
	{
		clear();
	}
	assert (_window != 0 && _renderer != 0);

	Options::displayWidth = getWidth();
	Options::displayHeight = getHeight();
	_scaleX = getWidth() / (double)_baseWidth;
	_scaleY = getHeight() / (double)_baseHeight;
	Log(LOG_INFO) << "Pre-bar scales: _scaleX = " << _scaleX << ", _scaleY = " << _scaleY;
	//_scale = std::min(getWidth() / (double) _baseWidth, getHeight() / (double) _baseHeight);
	//_scaleX = _scale;
	//_scaleY = _scale;
#if 1
	_clear.x = 0;
	_clear.y = 0;
	/* _clear.w = getWidth();
	_clear.h = getHeight(); */
	_clear.w = _baseWidth;
	_clear.h = _baseHeight;
#endif

	double pixelRatioY = 1.0;
	if (Options::nonSquarePixelRatio && !Options::allowResize)
	{
		pixelRatioY = 1.2;
	}
	bool cursorInBlackBands;
	if (!Options::keepAspectRatio)
	{
		cursorInBlackBands = false;
	}
#ifndef __ANDROID__
	else if (Options::fullscreen)
	{
		cursorInBlackBands = Options::cursorInBlackBandsInFullscreen;
	}
	else if (!Options::borderless)
	{
		cursorInBlackBands = Options::cursorInBlackBandsInWindow;
	}
	else
	{
		cursorInBlackBands = Options::cursorInBlackBandsInBorderlessWindow;
	}
#else
	else {
	// Force the scales to be equal?
		cursorInBlackBands = true;
	}
#endif

	if (_scaleX > _scaleY && Options::keepAspectRatio)
	{
		int targetWidth = (int)floor(_scaleY * (double)_baseWidth);
		_topBlackBand = _bottomBlackBand = 0;
		_leftBlackBand = (getWidth() - targetWidth) / 2;
		if (_leftBlackBand < 0)
		{
			_leftBlackBand = 0;
		}
		_rightBlackBand = getWidth() - targetWidth - _leftBlackBand;
		_cursorTopBlackBand = 0;

		if (cursorInBlackBands)
		{
			_scaleX = _scaleY;
			_cursorLeftBlackBand = _leftBlackBand;
		}
		else
		{
			_cursorLeftBlackBand = 0;
		}		
	}
	else if (_scaleY > _scaleX && Options::keepAspectRatio)
	{
		int targetHeight = (int)floor(_scaleX * (double)_baseHeight * pixelRatioY);
		_topBlackBand = (getHeight() - targetHeight) / 2;
		if (_topBlackBand < 0)
		{
			_topBlackBand = 0;
		}
        _bottomBlackBand = getHeight() - targetHeight - _topBlackBand;
		if (_bottomBlackBand < 0)
		{
			_bottomBlackBand = 0;
		}
		_leftBlackBand = _rightBlackBand = 0;
		_cursorLeftBlackBand = 0;

		if (cursorInBlackBands)
		{
			_scaleY = _scaleX;
			_cursorTopBlackBand = _topBlackBand;
		}
		else
		{
			_cursorTopBlackBand = 0;
		}		
	}
	else
	{
		_topBlackBand = _bottomBlackBand = _leftBlackBand = _rightBlackBand = _cursorTopBlackBand = _cursorLeftBlackBand = 0;
	}
	Log(LOG_INFO) << "Scale (post-bar): scaleX = " << _scaleX << ", scaleY = " << _scaleY;
	Log(LOG_INFO) << "Black bars: top: " << _topBlackBand << ", left: " << _leftBlackBand;
	SDL_Rect outRect;
	outRect.x = _leftBlackBand;
	outRect.y = _topBlackBand;
	outRect.w = getWidth() - _leftBlackBand - _rightBlackBand;
	outRect.h = getHeight() - _topBlackBand - _bottomBlackBand;
	_renderer->setOutputRect(&outRect);
}

/**
 * Returns the screen's X scale.
 * @return Scale factor.
 */
double Screen::getXScale() const
{
	return _scaleX;
}

/**
 * Returns the screen's Y scale.
 * @return Scale factor.
 */
double Screen::getYScale() const
{
	return _scaleY;
}

double Screen::getScale() const
{
	return _scale;
}

/**
 * Returns the screen's top black forbidden to cursor band's height.
 * @return Height in pixel.
 */
int Screen::getCursorTopBlackBand() const
{
	return _cursorTopBlackBand;
}

/**
 * Returns the screen's left black forbidden to cursor band's width.
 * @return Width in pixel.
 */
int Screen::getCursorLeftBlackBand() const
{
	return _cursorLeftBlackBand;
}

/**
 * Saves a screenshot of the screen's contents.
 * @param filename Filename of the PNG file.
 */
void Screen::screenshot(const std::string &filename) const
{
	assert (0 && "FIXME: no time for screenshots");
#if 0
	SDL_Surface *screenshot = SDL_CreateRGBSurface(0, getWidth(), getHeight(), 24, 0xff, 0xff00, 0xff0000, 0);
	SDL_Surface *screenshot = SDL_AllocSurface(0, getWidth() - getWidth()%4, getHeight(), 24, 0xff, 0xff00, 0xff0000, 0);
	
	if (isOpenGLEnabled())
	{
#ifndef __NO_OPENGL
		GLenum format = GL_RGB;

		for (int y = 0; y < getHeight(); ++y)
		{
			glReadPixels(0, getHeight()-(y+1), getWidth() - getWidth()%4, 1, format, GL_UNSIGNED_BYTE, ((Uint8*)screenshot->pixels) + y*screenshot->pitch);
		}
		glErrorCheck();
#endif
	}
	else
	{
		SDL_BlitSurface(_screen, 0, screenshot, 0);
	}

	unsigned error = lodepng::encode(filename, (const unsigned char *)(screenshot->pixels), getWidth() - getWidth()%4, getHeight(), LCT_RGB);
	if (error)
	{
		Log(LOG_ERROR) << "Saving to PNG failed: " << lodepng_error_text(error);
	}

	SDL_FreeSurface(screenshot);
#endif
}


/** 
 * Check whether a 32bpp scaler has been selected.
 * @return if it is enabled with a compatible resolution.
 */
bool Screen::is32bitEnabled()
{
	int w = Options::displayWidth;
	int h = Options::displayHeight;
	int baseW = Options::baseXResolution;
	int baseH = Options::baseYResolution;

	return ((Options::useHQXFilter || Options::useXBRZFilter) && (
			(w == baseW * 2 && h == baseH * 2) ||
			(w == baseW * 3 && h == baseH * 3) ||
			(w == baseW * 4 && h == baseH * 4) ||
			(w == baseW * 5 && h == baseH * 5 && Options::useXBRZFilter)));
}

/**
 * Check if OpenGL is enabled.
 * @return if it is enabled.
 */
bool Screen::isOpenGLEnabled()
{
#ifdef __NO_OPENGL
	return false;
#else
	return Options::useOpenGL;
#endif
}

/**
 * Gets the Horizontal offset from the mid-point of the screen, in pixels.
 * @return the horizontal offset.
 */
int Screen::getDX()
{
	return (_baseWidth - ORIGINAL_WIDTH) / 2;
}

/**
 * Gets the Vertical offset from the mid-point of the screen, in pixels.
 * @return the vertical offset.
 */
int Screen::getDY()
{
	return (_baseHeight - ORIGINAL_HEIGHT) / 2;
}

/**
* Changes a given scale, and if necessary, switch the current base resolution.
* @param type reference to which scale option we are using, battlescape or geoscape.
* @param selection the new scale level.
* @param width reference to which x scale to adjust.
* @param height reference to which y scale to adjust.
* @param change should we change the current scale.
*/
void Screen::updateScale(int &type, int selection, int &width, int &height, bool change)
{
	double pixelRatioY = 1.0;

	if (Options::nonSquarePixelRatio)
	{
		pixelRatioY = 1.2;
	}

	type = selection;
	switch (type)
	{
	case SCALE_15X:
		width = Screen::ORIGINAL_WIDTH * 1.5;
		height = Screen::ORIGINAL_HEIGHT * 1.5;
		break;
	case SCALE_2X:
		width = Screen::ORIGINAL_WIDTH * 2;
		height = Screen::ORIGINAL_HEIGHT * 2;
		break;
	case SCALE_SCREEN_DIV_3:
		width = Options::displayWidth / 3;
		height = Options::displayHeight / pixelRatioY / 3;
		break;
	case SCALE_SCREEN_DIV_2:
		width = Options::displayWidth / 2;
		height = Options::displayHeight / pixelRatioY  / 2.0;
		break;
	case SCALE_SCREEN:
		width = Options::displayWidth;
		height = Options::displayHeight / pixelRatioY;
		break;
	case SCALE_ORIGINAL:
	default:
		width = Screen::ORIGINAL_WIDTH;
		height = Screen::ORIGINAL_HEIGHT;
		break;
	}

	// don't go under minimum resolution... it's bad, mmkay?
	width = std::max(width, Screen::ORIGINAL_WIDTH);
	height = std::max(height, Screen::ORIGINAL_HEIGHT);

	if (change && (Options::baseXResolution != width || Options::baseYResolution != height))
	{
		Options::baseXResolution = width;
		Options::baseYResolution = height;
	}
}

SDL_Window * Screen::getWindow() const
{
	return _window;
}
}
