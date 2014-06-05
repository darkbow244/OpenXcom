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
#include "Surface.h"
#include "Screen.h"
#include "Texture.h"
#include "ShaderDraw.h"
#include <vector>
#include <fstream>
#include <SDL_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_endian.h>
#include "Palette.h"
#include "Exception.h"
#include "ShaderMove.h"
#include <stdlib.h>
#ifdef _WIN32
#include <malloc.h>
#endif
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc __mingw_aligned_malloc
#define _aligned_free   __mingw_aligned_free
#endif //MINGW
#include "Language.h"
#ifdef __MORPHOS__
#include <ppcinline/exec.h>
#endif

#include "Options.h"

namespace OpenXcom
{
/**
 * Creates a 32bpp surface with no prior data. Texture access will be set to SDL_TEXTUREACCESS_STREAMING.
 * @note Texture will be filled with random data at the start. You should use a Surface object to
 * load a graphic before using it.
 * @param screen Game's Screen object.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
Texture::Texture(Screen *screen, int width, int height, int x, int y): _screen(screen), _srcSet(false), _dstSet(false), 
		_height(height), _width(width), _x(x), _y(y), _visible(false), _hidden(true), _redraw(false), _alphaMod(255), 
		_format(SDL_PIXELFORMAT_ARGB8888), _access(SDL_TEXTUREACCESS_STREAMING)
{
	_renderer = _screen->getRenderer();
	_texture = SDL_CreateTexture(_renderer, _format, _access, _height, _width);
	if (_x || _y) {
		_dstSet = true;
		_dstRect.x = _x;
		_dstRect.y = _y;
		_dstRect.w = _width;
		_dstRect.h = _height;
	}
	// Set blending mode to alpha; maybe change that later?
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

/**
 * Creates a full copy of the texture; due to the way it's done, the resulting
 * texture access mode will be SDL_TEXTUREACCESS_TARGET.
 * @param other Texture to copy from.
 */


Texture::Texture(const Texture& other)
{
	_renderer = other._renderer;
	_width = other._width;
	_height = other._height;
	_srcSet = other._srcSet;
	_dstSet = other._dstSet;
	_srcRect = other._srcRect;
	_dstRect = other._dstRect;
	_alphaMod = other._alphaMod;
	_format = other._format;
	_access = SDL_TEXTUREACCESS_TARGET;
	_texture = SDL_CreateTexture(_renderer, _format, _access, _height, _width);
	// Save the previous renderer target.
	SDL_Texture *prevTarget = SDL_GetRenderTarget(_renderer);
	SDL_SetRenderTarget(_renderer, _texture);
	SDL_RenderCopy(_renderer, other._texture, NULL, NULL);
	SDL_SetRenderTarget(_renderer, prevTarget);
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

/**
 * Creates a new texture by converting it from a Surface to a texture.
 * Access mode will be set to SDL_TEXTUREACCESS_STATIC, so you should use
 * other methods if you want streaming textures.
 * @param screen Game's Screen object.
 * @param surface The surface to copy from.
 */
Texture::Texture(Screen *screen, const Surface& surface)
{
	_srcSet = false;
	_dstSet = false;
	_screen = screen;
	_renderer = _screen->getRenderer();
	_texture = SDL_CreateTextureFromSurface(_renderer, surface.getSurface());
	_height = surface.getHeight();
	_width = surface.getWidth();
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

/**
 * Performs cleanup on texture destruction.
 * Basically a call to SDL_DestroyTexture
 */
Texture::~Texture()
{
	_screen->removeOverlay(this);
	SDL_DestroyTexture(_texture);
}

void Texture::loadImage(const std::string &filename)
{
	// Basically a copy-paste of Surface's loadImage call
	std::string utf8 = Language::wstrToUtf8(Language::fsToWstr(filename));
	SDL_Surface *img = IMG_Load(utf8.c_str());
	if (!img)
	{
		std::string err = filename + ":" + IMG_GetError();
		throw Exception(err);
	}
	/*SDL_Surface *temp = SDL_CreateRGBSurfaceFrom(img->pixels, img->w, img->h,
						     img->format->BitsPerPixel,
						     img->pitch, 0x00FF0000,
						     0x0000FF00, 0x000000FF,
						     0xFF000000);*/
	SDL_Surface *temp = SDL_ConvertSurface(img, _screen->getSurface()->getSurface()->format, 0);
	SDL_UpdateTexture(_texture, NULL, temp->pixels, temp->pitch);
	SDL_FreeSurface(temp);
	SDL_FreeSurface(img);
}

void Texture::draw()
{
	SDL_Rect *src = NULL;
	SDL_Rect *dst = NULL;
	if(_srcSet) 
	{
		src = &_srcRect;
	}
	if(_dstSet)
	{
		dst = &_dstRect;
	}
	int prevW, prevH;
	SDL_RenderGetLogicalSize(_renderer, &prevW, &prevH);
	SDL_RenderSetLogicalSize(_renderer, Options::displayWidth, Options::displayHeight);
	SDL_RenderCopy(_renderer, _texture, src, dst);
	SDL_RenderSetLogicalSize(_renderer, prevW, prevH);

}

void Texture::setAlphaMod(Uint8 alpha)
{
	_alpha = alpha;
	SDL_SetTextureAlphaMod(_texture, _alpha);
}

void Texture::setSrcRect(int x, int y, int width, int height)
{
	_srcSet = true;
	_srcRect.x = x;
	_srcRect.y = y;
	_srcRect.w = width;
	_srcRect.h = height;
}

void Texture::setDstRect(int x, int y, int width, int height)
{
  	_dstSet = true;
	_dstRect.x = x;
	_dstRect.y = y;
	_dstRect.w = width;
	_dstRect.h = height;
}

void Texture::clearSrcRect()
{
	_srcSet = false;
}

void Texture::clearDstRect()
{
	_dstSet = false;
}

  
  
}