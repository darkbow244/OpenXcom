/* GfxPrimitives.h - required functions from SDL_gfxPrimitives */

/* Original copyrignt notice follows : */

/*

SDL_gfxPrimitives.h: graphics primitives for SDL

Copyright (C) 2001-2012  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net

*/


#ifndef GFX_PRIMITIVES_H_
#define GFX_PRIMITIVES_H_

#include <SDL.h>

namespace OpenXcom
{

	namespace GfxPrimitives
	{
		int pixelColor(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);

		int hlineColor(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
		int vlineColor(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
		int lineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
		int lineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1,
			Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		int filledCircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);

		int filledPolygonColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color);

		int texturedPolygon(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, SDL_Surface * texture, int texture_dx, int texture_dy);

		void gfxPrimitivesSetFont(const void *fontdata, Uint32 cw, Uint32 ch);
		void gfxPrimitivesSetFontRotation(Uint32 rotation);
		int characterColor(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color);
		int characterRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		int stringColor(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint32 color);
		int stringRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	}

}



#endif /*GFX_PRIMITIVES_H_*/