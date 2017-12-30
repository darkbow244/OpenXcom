/*
 * An interface to the rendering infrastructure.
 * This should not be instanced.
 */


#ifndef OPENXCOM_RENDERER_H
#define OPENXCOM_RENDERER_H

#include <SDL.h>
#include <string>
#include <vector>

namespace OpenXcom
{

enum RendererType
{
	RENDERER_SDL2,
	RENDERER_OPENGL
};

class Surface;
class Screen;

/***
 * Base class for Renderers.
 *
 * Renderers are responsible for presenting the final frame
 * (stored in a screenSurface) to the user, possibly upscaling
 * it in the process.
 */
class Renderer
{
public:
	/// Creates a renderer and binds it to the screen surface
	Renderer(Screen& gameScreen, const SDL_Window *window) { };
	virtual ~Renderer() {};
	/// Returns a human-readable list of upscalers
	virtual std::vector<std::string> getUpscalers() = 0;
	/// Sets the desired upscaler
	virtual void setUpscaler(int upscalerId) = 0;
	virtual void setUpscalerByName(const std::string &scalerName) = 0;
	/// Sets the desired output rectangle.
	virtual void setOutputRect(SDL_Rect *dstRect) = 0;
	/// Blits the contents of the SDL_Surface to the screen.
	virtual void flip() = 0;
	/// Saves a screenshot to filename.
	virtual void screenshot(const std::string &filename) const = 0;
	/// Returns the renderer type.
	virtual RendererType getRendererType() = 0;
    /// Returns the renderer name
    virtual std::string getRendererName() = 0;

};

/// Register an upscaler
void registerUpscaler(std::string rendererName, std::string upscalerName);
/// Get all available upscalers
std::vector<std::pair<std::string, std::string>> getRegisteredUpscalers();
}
#endif
