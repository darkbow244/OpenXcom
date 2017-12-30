/*
 * An implementation of a renderer using the SDL2 renderer infrastructure
 */
#ifndef OPENXCOM_SDLRENDERER_H
#define OPENXCOM_SDLRENDERER_H

#include <SDL.h>
#include "Renderer.h"
#include "Surface.h"
#include <string>

namespace OpenXcom
{

class SDLRenderer :
	public Renderer
{
private:
	static const RendererType _rendererType = RENDERER_SDL2;
	Screen &_gameScreen;

    std::vector<std::string> _upscalers;
	SDL_Window *_window;
	SDL_Renderer *_renderer;
	SDL_Texture *_texture;

	SDL_Rect _srcRect, _dstRect;
	/// Gets upscaler hint name by its ID
	std::string upscalerHintById(int id);

public:
	/// Creates a renderer and binds it to the screen surface
	SDLRenderer(Screen &gameScreen, SDL_Window* window);
	~SDLRenderer() override;
	/// Returns a human-readable list of upscalers
	std::vector<std::string> getUpscalers() override {return _upscalers;}
	/// Sets the desired upscaler
	void setUpscaler(int upscalerId) override;
	void setUpscalerByName(const std::string &scalerName) override;
	/// Sets the desired output rectangle.
	void setOutputRect(SDL_Rect *dstRect) override;
	/// Blits the contents of the SDL_Surface to the screen.
	void flip() override;
	/// Saves a screenshot to filename.
	void screenshot(const std::string &filename) const override;
	/// Returns the renderer type.
	RendererType getRendererType() override { return RendererType::RENDERER_SDL2; }
    /// Returns the renderer name
    std::string getRendererName() override { return "SDL"; }
};

}
#endif