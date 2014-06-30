/*
 * An interface to the rendering infrastructure.
 * This should not be instanced.
 */


#ifndef OPENXCOM_RENDERER_H
#define OPENXCOM_RENDERER_H

#include <SDL.h>

namespace OpenXcom
{

enum RendererType
{
	RENDERER_SDL2,
	RENDERER_OPENGL
};

class Renderer
{
public:
	Renderer(void);
	virtual ~Renderer(void);
	/// Sets the surface's pixel format
	virtual void setPixelFormat(Uint32 format) = 0;
	/// Sets the size of the expected SDL_Surface.
	virtual void setInternalRect(SDL_Rect *srcRect) = 0;
	/// Sets the desired output rectangle.
	virtual void setOutputRect(SDL_Rect *dstRect) = 0;
	/// Blits the contents of the SDL_Surface to the screen.
	virtual void flip(SDL_Surface *srcSurface) = 0; 
	virtual RendererType getRendererType() = 0;

};
}
#endif
