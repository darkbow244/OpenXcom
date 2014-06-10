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
#ifndef OPENXCOM_TEXTURE_H
#define OPENXCOM_TEXTURE_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>

namespace OpenXcom
{

  
class Screen;
class Surface;
  
 /**
  * An encapsulation of the SDL_Texture struct from SDL2.
  * Used to draw transparent overlays when needed.
  * Designed to use mostly the same principles as the Surface class.
  */ 
class Texture
{
protected:
	/// The texture itself
	SDL_Texture *_texture;
	/// Texture size (if we ever need it)
	int _height, _width, _x, _y;
	/// Currently attached renderer
	SDL_Renderer *_renderer;
	/// Pointer to a Screen object (to remove ourselves from the rendering queue
	Screen *_screen;
	/// Source and destination rectangles (which part of texture to blit, where to blit the texture)
	SDL_Rect _srcRect, _dstRect;
	bool _visible, _hidden, _redraw;
	bool _srcSet, _dstSet;
	/// Additional alpha modulator
	Uint8 _alphaMod;
	
	/// Texture format; maybe should be set by screen options?
	Uint32 _format;
	/// Texture access mode
	int _access;
	Uint8 _alpha;
	
public:
	/// Creates a new texture with the specified size and position; texture access will be STREAMING.
	Texture(Screen *screen, int width, int height, int x=0, int y=0);
	/// Creates a new texture from an existing one; texture access will be TARGET.
	Texture(const Texture& other);
	/// Creates a new texture from an existing surface; texture access will be STATIC.
	Texture(Screen *screen, const Surface& surface);
	/// Cleans up the texture.
	virtual ~Texture();
	/// Loads a general image file.
	void loadImage(const std::string &filename);
	/// Draws this texture to the current rendering target.
	virtual void draw();
	/// Sets alpha modulation for this texture.
	void setAlphaMod(Uint8 alpha);
	/// Sets source rectangle (crop)
	void setSrcRect(int x, int y, int width, int height);
	/// Sets destination rectangle.
	void setDstRect(int x, int y, int width, int height);
	/// Clears source rectangle
	void clearSrcRect();
	/// Clears destination rectangle
	void clearDstRect();
};

}


#endif