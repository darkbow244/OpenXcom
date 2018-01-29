#include "SDLRenderer.h"
#include "Exception.h"
#include "Logger.h"
#include "../lodepng.h"

#include "Screen.h"
#include "Options.h"

namespace OpenXcom
{

std::string SDLRenderer::upscalerHintById(int id)
{
	static std::string hints[] = {"nearest", "linear"};
	if (id > 1) return "";
	return hints[id];
}

SDLRenderer::SDLRenderer(Screen &gameScreen) : Renderer(gameScreen, nullptr), _gameScreen(gameScreen), _window(nullptr),
											   _texture(nullptr), _renderer(nullptr)
{
	_upscalers = {"nearest", "linear"};
}

SDLRenderer::SDLRenderer(Screen& gameScreen, SDL_Window *window) : Renderer(gameScreen, window), _gameScreen(gameScreen), _window(window),
																   _renderer(NULL), _texture(NULL)
{
	_upscalers = {"nearest", "linear"};

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	_srcRect.x = _srcRect.y = _srcRect.w = _srcRect.h = 0;
	_dstRect.x = _dstRect.y = _dstRect.w = _dstRect.h = 0;
	if (_renderer == NULL)
	{
		Log(LOG_ERROR) << "[SDLRenderer] Couldn't create renderer; error message: " << SDL_GetError();
		throw Exception(SDL_GetError());
	}
	Log(LOG_INFO) << "[SDLRenderer] Renderer created";

	setUpscalerByName(Options::scalerName);
	SDL_RendererInfo info;
	SDL_GetRendererInfo(_renderer, &info);
	Log(LOG_INFO) << "[SDLRenderer] Created new SDLRenderer, using " << info.name;

	_srcRect.w = _gameScreen.getSurface()->getWidth();
	_srcRect.h = _gameScreen.getSurface()->getHeight();

	_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888,
								 SDL_TEXTUREACCESS_STREAMING,
								 _srcRect.w,
								 _srcRect.h);
}

void SDLRenderer::setUpscaler(int upscalerId)
{
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, upscalerHintById(upscalerId).c_str());
	if (_texture) SDL_DestroyTexture(_texture);
	if (_renderer) SDL_DestroyRenderer(_renderer);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _srcRect.w, _srcRect.h);
}

void SDLRenderer::setUpscalerByName(const std::string &scalerName)
{
	if (scalerName == "linear")
	{
		setUpscaler(1);
	}
	else
	{
		setUpscaler(0);
	}
}

void SDLRenderer::setOutputRect(SDL_Rect *dstRect)
{
	_dstRect.x = dstRect->x;
	_dstRect.y = dstRect->y;
	_dstRect.w = dstRect->w;
	_dstRect.h = dstRect->h;
	Log(LOG_INFO) << "[SDLRenderer] Output resolution: " << _dstRect.w << "x" << _dstRect.h;
	Log(LOG_INFO) << "[SDLRenderer] Offset: " << _dstRect.x << "x" << _dstRect.y;
}

SDLRenderer::~SDLRenderer()
{
	if (_texture) SDL_DestroyTexture(_texture);
	if (_renderer) SDL_DestroyRenderer(_renderer);
}

void SDLRenderer::flip()
{
	if (_srcRect.w != _gameScreen.getSurface()->getWidth() ||
            _srcRect.h != _gameScreen.getSurface()->getHeight())
	{
		SDL_DestroyTexture(_texture);
		_srcRect.w = _gameScreen.getSurface()->getWidth();
		_srcRect.h = _gameScreen.getSurface()->getHeight();
		_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888,
									 SDL_TEXTUREACCESS_STREAMING,
									 _srcRect.w, _srcRect.h);
	}
	void *pixels;
	int pitch;
	/*SDL_LockTexture(_texture, &_srcRect, &pixels, &pitch);
    char *pixBytes = (char*)pixels;
	for(int y = 0; y < _gameScreen.getSurface()->getSurface()->pitch; ++y)
	{
		SDL_memcpy(pixBytes + y * pitch, (char*)_gameScreen.getSurface()->getSurface()->pixels + y * _gameScreen.getSurface()->getSurface()->pitch,
				   _gameScreen.getSurface()->getSurface()->pitch);
	}
	SDL_UnlockTexture(_texture);*/
    SDL_UpdateTexture(_texture, &_srcRect, _gameScreen.getSurface()->getSurface()->pixels, _gameScreen.getSurface()->getSurface()->pitch);

	SDL_RenderClear(_renderer);
	SDL_RenderCopy(_renderer, _texture, &_srcRect, &_dstRect);
	SDL_RenderPresent(_renderer); //TODO: check error?
}

void SDLRenderer::screenshot(const std::string &filename) const
{
	int width, height;
	SDL_GetWindowSize(_window, &width, &height);
	unsigned char *pixels = new unsigned char[width * height * 4];
	int error = SDL_RenderReadPixels(_renderer, NULL, SDL_PIXELFORMAT_ABGR8888, (void*)pixels, width * 4);
	if (error)
	{
		Log(LOG_ERROR) << "Acquiring pixels failed while trying to save screenshot: " << SDL_GetError();
		delete[] pixels;
		return;
	}
	error = lodepng::encode(filename, (const unsigned char*) pixels, width, height, LCT_RGBA);
	if (error)
	{
		Log(LOG_ERROR) << "Saving to PNG failed: " << lodepng_error_text(error);
	}
	delete[] pixels;
}

}
