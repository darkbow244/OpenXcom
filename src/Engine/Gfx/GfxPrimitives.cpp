/* GfxPrimitives.cpp - functions from SDL_gfx required by OpenXcom */

/* Original copyright follows: */

/*

SDL_gfxPrimitives.c: graphics primitives for SDL surfaces

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


#include "GfxPrimitives.h"

#include "GfxFontData.h"

#include <math.h>
#include <cstdlib>
#include <string.h>

#define CLIP_LEFT_EDGE   0x1
#define CLIP_RIGHT_EDGE  0x2
#define CLIP_BOTTOM_EDGE 0x4
#define CLIP_TOP_EDGE    0x8
#define CLIP_INSIDE(a)   (!a)
#define CLIP_REJECT(a,b) (a&b)
#define CLIP_ACCEPT(a,b) (!(a|b))

#define DEFAULT_ALPHA_PIXEL_ROUTINE
#undef EXPERIMENTAL_ALPHA_PIXEL_ROUTINE
#define ALPHA_PIXEL_ADDITIVE_BLEND

#define clip_xmin(surface) surface->clip_rect.x
#define clip_xmax(surface) surface->clip_rect.x+surface->clip_rect.w-1
#define clip_ymin(surface) surface->clip_rect.y
#define clip_ymax(surface) surface->clip_rect.y+surface->clip_rect.h-1

#define ABS(a) (((a)<0) ? -(a) : (a))

namespace OpenXcom
{

	namespace GfxPrimitives
	{
		/*!
		\brief Alpha adjustment table for custom blitter.

		The table provides values for a modified, non-linear
		transfer function which maintain brightness.

		*/
		const unsigned int GFX_ALPHA_ADJUST_ARRAY[256] = {
			0,  /* 0 */
			15,  /* 1 */
			22,  /* 2 */
			27,  /* 3 */
			31,  /* 4 */
			35,  /* 5 */
			39,  /* 6 */
			42,  /* 7 */
			45,  /* 8 */
			47,  /* 9 */
			50,  /* 10 */
			52,  /* 11 */
			55,  /* 12 */
			57,  /* 13 */
			59,  /* 14 */
			61,  /* 15 */
			63,  /* 16 */
			65,  /* 17 */
			67,  /* 18 */
			69,  /* 19 */
			71,  /* 20 */
			73,  /* 21 */
			74,  /* 22 */
			76,  /* 23 */
			78,  /* 24 */
			79,  /* 25 */
			81,  /* 26 */
			82,  /* 27 */
			84,  /* 28 */
			85,  /* 29 */
			87,  /* 30 */
			88,  /* 31 */
			90,  /* 32 */
			91,  /* 33 */
			93,  /* 34 */
			94,  /* 35 */
			95,  /* 36 */
			97,  /* 37 */
			98,  /* 38 */
			99,  /* 39 */
			100,  /* 40 */
			102,  /* 41 */
			103,  /* 42 */
			104,  /* 43 */
			105,  /* 44 */
			107,  /* 45 */
			108,  /* 46 */
			109,  /* 47 */
			110,  /* 48 */
			111,  /* 49 */
			112,  /* 50 */
			114,  /* 51 */
			115,  /* 52 */
			116,  /* 53 */
			117,  /* 54 */
			118,  /* 55 */
			119,  /* 56 */
			120,  /* 57 */
			121,  /* 58 */
			122,  /* 59 */
			123,  /* 60 */
			124,  /* 61 */
			125,  /* 62 */
			126,  /* 63 */
			127,  /* 64 */
			128,  /* 65 */
			129,  /* 66 */
			130,  /* 67 */
			131,  /* 68 */
			132,  /* 69 */
			133,  /* 70 */
			134,  /* 71 */
			135,  /* 72 */
			136,  /* 73 */
			137,  /* 74 */
			138,  /* 75 */
			139,  /* 76 */
			140,  /* 77 */
			141,  /* 78 */
			141,  /* 79 */
			142,  /* 80 */
			143,  /* 81 */
			144,  /* 82 */
			145,  /* 83 */
			146,  /* 84 */
			147,  /* 85 */
			148,  /* 86 */
			148,  /* 87 */
			149,  /* 88 */
			150,  /* 89 */
			151,  /* 90 */
			152,  /* 91 */
			153,  /* 92 */
			153,  /* 93 */
			154,  /* 94 */
			155,  /* 95 */
			156,  /* 96 */
			157,  /* 97 */
			158,  /* 98 */
			158,  /* 99 */
			159,  /* 100 */
			160,  /* 101 */
			161,  /* 102 */
			162,  /* 103 */
			162,  /* 104 */
			163,  /* 105 */
			164,  /* 106 */
			165,  /* 107 */
			165,  /* 108 */
			166,  /* 109 */
			167,  /* 110 */
			168,  /* 111 */
			168,  /* 112 */
			169,  /* 113 */
			170,  /* 114 */
			171,  /* 115 */
			171,  /* 116 */
			172,  /* 117 */
			173,  /* 118 */
			174,  /* 119 */
			174,  /* 120 */
			175,  /* 121 */
			176,  /* 122 */
			177,  /* 123 */
			177,  /* 124 */
			178,  /* 125 */
			179,  /* 126 */
			179,  /* 127 */
			180,  /* 128 */
			181,  /* 129 */
			182,  /* 130 */
			182,  /* 131 */
			183,  /* 132 */
			184,  /* 133 */
			184,  /* 134 */
			185,  /* 135 */
			186,  /* 136 */
			186,  /* 137 */
			187,  /* 138 */
			188,  /* 139 */
			188,  /* 140 */
			189,  /* 141 */
			190,  /* 142 */
			190,  /* 143 */
			191,  /* 144 */
			192,  /* 145 */
			192,  /* 146 */
			193,  /* 147 */
			194,  /* 148 */
			194,  /* 149 */
			195,  /* 150 */
			196,  /* 151 */
			196,  /* 152 */
			197,  /* 153 */
			198,  /* 154 */
			198,  /* 155 */
			199,  /* 156 */
			200,  /* 157 */
			200,  /* 158 */
			201,  /* 159 */
			201,  /* 160 */
			202,  /* 161 */
			203,  /* 162 */
			203,  /* 163 */
			204,  /* 164 */
			205,  /* 165 */
			205,  /* 166 */
			206,  /* 167 */
			206,  /* 168 */
			207,  /* 169 */
			208,  /* 170 */
			208,  /* 171 */
			209,  /* 172 */
			210,  /* 173 */
			210,  /* 174 */
			211,  /* 175 */
			211,  /* 176 */
			212,  /* 177 */
			213,  /* 178 */
			213,  /* 179 */
			214,  /* 180 */
			214,  /* 181 */
			215,  /* 182 */
			216,  /* 183 */
			216,  /* 184 */
			217,  /* 185 */
			217,  /* 186 */
			218,  /* 187 */
			218,  /* 188 */
			219,  /* 189 */
			220,  /* 190 */
			220,  /* 191 */
			221,  /* 192 */
			221,  /* 193 */
			222,  /* 194 */
			222,  /* 195 */
			223,  /* 196 */
			224,  /* 197 */
			224,  /* 198 */
			225,  /* 199 */
			225,  /* 200 */
			226,  /* 201 */
			226,  /* 202 */
			227,  /* 203 */
			228,  /* 204 */
			228,  /* 205 */
			229,  /* 206 */
			229,  /* 207 */
			230,  /* 208 */
			230,  /* 209 */
			231,  /* 210 */
			231,  /* 211 */
			232,  /* 212 */
			233,  /* 213 */
			233,  /* 214 */
			234,  /* 215 */
			234,  /* 216 */
			235,  /* 217 */
			235,  /* 218 */
			236,  /* 219 */
			236,  /* 220 */
			237,  /* 221 */
			237,  /* 222 */
			238,  /* 223 */
			238,  /* 224 */
			239,  /* 225 */
			240,  /* 226 */
			240,  /* 227 */
			241,  /* 228 */
			241,  /* 229 */
			242,  /* 230 */
			242,  /* 231 */
			243,  /* 232 */
			243,  /* 233 */
			244,  /* 234 */
			244,  /* 235 */
			245,  /* 236 */
			245,  /* 237 */
			246,  /* 238 */
			246,  /* 239 */
			247,  /* 240 */
			247,  /* 241 */
			248,  /* 242 */
			248,  /* 243 */
			249,  /* 244 */
			249,  /* 245 */
			250,  /* 246 */
			250,  /* 247 */
			251,  /* 248 */
			251,  /* 249 */
			252,  /* 250 */
			252,  /* 251 */
			253,  /* 252 */
			253,  /* 253 */
			254,  /* 254 */
			255   /* 255 */
		};

		/*!
		\brief Internal pixel drawing function with alpha blending where input color in in destination format.

		Contains two alternative 32 bit alpha blending routines which can be enabled at the source
		level with the defines DEFAULT_ALPHA_PIXEL_ROUTINE or EXPERIMENTAL_ALPHA_PIXEL_ROUTINE.
		Only the bits up to the surface depth are significant in the color value.

		\param dst The surface to draw on.
		\param x The horizontal coordinate of the pixel.
		\param y The vertical position of the pixel.
		\param color The color value of the pixel to draw.
		\param alpha The blend factor to apply while drawing.

		\returns Returns 0 on success, -1 on failure.
		*/
		int _putPixelAlpha(SDL_Surface *dst, Sint16 x, Sint16 y, Uint32 color, Uint8 alpha)
		{
			SDL_PixelFormat *format;
			Uint32 Rmask, Gmask, Bmask, Amask;
			Uint32 Rshift, Gshift, Bshift, Ashift;
			Uint32 sR, sG, sB;
			Uint32 dR, dG, dB, dA;

			if (dst == NULL)
			{
				return (-1);
			}

			if (x >= clip_xmin(dst) && x <= clip_xmax(dst) &&
				y >= clip_ymin(dst) && y <= clip_ymax(dst))
			{

				format = dst->format;

				switch (format->BytesPerPixel) {
				case 1:
				{		/* Assuming 8-bpp */
						  Uint8 *pixel = (Uint8 *)dst->pixels + y * dst->pitch + x;
						  if (alpha == 255) {
							  *pixel = color;
						  }
						  else {
							  Uint8 R, G, B;
							  SDL_Palette *palette = format->palette;
							  SDL_Color *colors = palette->colors;
							  SDL_Color dColor = colors[*pixel];
							  SDL_Color sColor = colors[color];
							  dR = dColor.r;
							  dG = dColor.g;
							  dB = dColor.b;
							  sR = sColor.r;
							  sG = sColor.g;
							  sB = sColor.b;

							  R = dR + ((sR - dR) * alpha >> 8);
							  G = dG + ((sG - dG) * alpha >> 8);
							  B = dB + ((sB - dB) * alpha >> 8);

							  *pixel = SDL_MapRGB(format, R, G, B);
						  }
				}
					break;

				case 2:
				{		/* Probably 15-bpp or 16-bpp */
						  Uint16 *pixel = (Uint16 *)dst->pixels + y * dst->pitch / 2 + x;
						  if (alpha == 255) {
							  *pixel = color;
						  }
						  else {
							  Uint16 R, G, B, A;
							  Uint16 dc = *pixel;

							  Rmask = format->Rmask;
							  Gmask = format->Gmask;
							  Bmask = format->Bmask;
							  Amask = format->Amask;

							  dR = (dc & Rmask);
							  dG = (dc & Gmask);
							  dB = (dc & Bmask);

							  R = (dR + (((color & Rmask) - dR) * alpha >> 8)) & Rmask;
							  G = (dG + (((color & Gmask) - dG) * alpha >> 8)) & Gmask;
							  B = (dB + (((color & Bmask) - dB) * alpha >> 8)) & Bmask;
							  *pixel = R | G | B;
							  if (Amask != 0) {
								  dA = (dc & Amask);
								  A = (dA + (((color & Amask) - dA) * alpha >> 8)) & Amask;
								  *pixel |= A;
							  }
						  }
				}
					break;

				case 3:
				{		/* Slow 24-bpp mode, usually not used */
						  Uint8 R, G, B;
						  Uint8 Rshift8, Gshift8, Bshift8;
						  Uint8 *pixel = (Uint8 *)dst->pixels + y * dst->pitch + x * 3;

						  Rshift = format->Rshift;
						  Gshift = format->Gshift;
						  Bshift = format->Bshift;

						  Rshift8 = Rshift >> 3;
						  Gshift8 = Gshift >> 3;
						  Bshift8 = Bshift >> 3;

						  sR = (color >> Rshift) & 0xFF;
						  sG = (color >> Gshift) & 0xFF;
						  sB = (color >> Bshift) & 0xFF;

						  if (alpha == 255) {
							  *(pixel + Rshift8) = sR;
							  *(pixel + Gshift8) = sG;
							  *(pixel + Bshift8) = sB;
						  }
						  else {
							  dR = *((pixel)+Rshift8);
							  dG = *((pixel)+Gshift8);
							  dB = *((pixel)+Bshift8);

							  R = dR + ((sR - dR) * alpha >> 8);
							  G = dG + ((sG - dG) * alpha >> 8);
							  B = dB + ((sB - dB) * alpha >> 8);

							  *((pixel)+Rshift8) = R;
							  *((pixel)+Gshift8) = G;
							  *((pixel)+Bshift8) = B;
						  }
				}
					break;

#ifdef DEFAULT_ALPHA_PIXEL_ROUTINE

				case 4:
				{		/* Probably :-) 32-bpp */
						  Uint32 R, G, B, A;
						  Uint32 *pixel = (Uint32 *)dst->pixels + y * dst->pitch / 4 + x;
						  if (alpha == 255) {
							  *pixel = color;
						  }
						  else {
							  Uint32 dc = *pixel;

							  Rmask = format->Rmask;
							  Gmask = format->Gmask;
							  Bmask = format->Bmask;
							  Amask = format->Amask;

							  Rshift = format->Rshift;
							  Gshift = format->Gshift;
							  Bshift = format->Bshift;
							  Ashift = format->Ashift;

							  dR = (dc & Rmask) >> Rshift;
							  dG = (dc & Gmask) >> Gshift;
							  dB = (dc & Bmask) >> Bshift;


							  R = ((dR + ((((color & Rmask) >> Rshift) - dR) * alpha >> 8)) << Rshift) & Rmask;
							  G = ((dG + ((((color & Gmask) >> Gshift) - dG) * alpha >> 8)) << Gshift) & Gmask;
							  B = ((dB + ((((color & Bmask) >> Bshift) - dB) * alpha >> 8)) << Bshift) & Bmask;
							  *pixel = R | G | B;
							  if (Amask != 0) {
								  dA = (dc & Amask) >> Ashift;

#ifdef ALPHA_PIXEL_ADDITIVE_BLEND
								  A = (dA | GFX_ALPHA_ADJUST_ARRAY[alpha & 255]) << Ashift; // make destination less transparent...
#else
								  A = ((dA + ((((color & Amask) >> Ashift) - dA) * alpha >> 8)) << Ashift) & Amask;
#endif
								  *pixel |= A;
							  }
						  }
				}
					break;
#endif

#ifdef EXPERIMENTAL_ALPHA_PIXEL_ROUTINE

				case 4:{		/* Probably :-) 32-bpp */
						   if (alpha == 255) {
							   *((Uint32 *)dst->pixels + y * dst->pitch / 4 + x) = color;
						   }
						   else {
							   Uint32 *pixel = (Uint32 *)dst->pixels + y * dst->pitch / 4 + x;
							   Uint32 dR, dG, dB, dA;
							   Uint32 dc = *pixel;

							   Uint32 surfaceAlpha, preMultR, preMultG, preMultB;
							   Uint32 aTmp;

							   Rmask = format->Rmask;
							   Gmask = format->Gmask;
							   Bmask = format->Bmask;
							   Amask = format->Amask;

							   dR = (color & Rmask);
							   dG = (color & Gmask);
							   dB = (color & Bmask);
							   dA = (color & Amask);

							   Rshift = format->Rshift;
							   Gshift = format->Gshift;
							   Bshift = format->Bshift;
							   Ashift = format->Ashift;

							   preMultR = (alpha * (dR >> Rshift));
							   preMultG = (alpha * (dG >> Gshift));
							   preMultB = (alpha * (dB >> Bshift));

							   surfaceAlpha = ((dc & Amask) >> Ashift);
							   aTmp = (255 - alpha);
							   if (A = 255 - ((aTmp * (255 - surfaceAlpha)) >> 8)) {
								   aTmp *= surfaceAlpha;
								   R = (preMultR + ((aTmp * ((dc & Rmask) >> Rshift)) >> 8)) / A << Rshift & Rmask;
								   G = (preMultG + ((aTmp * ((dc & Gmask) >> Gshift)) >> 8)) / A << Gshift & Gmask;
								   B = (preMultB + ((aTmp * ((dc & Bmask) >> Bshift)) >> 8)) / A << Bshift & Bmask;
							   }
							   *pixel = R | G | B | (A << Ashift & Amask);

						   }
				}
					break;
#endif
				}
			}

			return (0);
		}


		/*!
		\brief Pixel draw with blending enabled if a<255.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the pixel.
		\param y Y (vertical) coordinate of the pixel.
		\param color The color value of the pixel to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int pixelColor(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color)
		{
			Uint8 alpha;
			Uint32 mcolor;
			int result = 0;

			/*
			* Lock the surface
			*/
			if (SDL_MUSTLOCK(dst)) {
				if (SDL_LockSurface(dst) < 0) {
					return (-1);
				}
			}

			/*
			* Setup color
			*/
			alpha = color & 0x000000ff;
			mcolor =
				SDL_MapRGBA(dst->format, (color & 0xff000000) >> 24,
				(color & 0x00ff0000) >> 16, (color & 0x0000ff00) >> 8, alpha);

			/*
			* Draw
			*/
			result = _putPixelAlpha(dst, x, y, mcolor, alpha);

			/*
			* Unlock the surface
			*/
			if (SDL_MUSTLOCK(dst)) {
				SDL_UnlockSurface(dst);
			}

			return (result);
		}

		/*!
		\brief Internal function to draw filled rectangle with alpha blending.

		Assumes color is in destination format.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first corner (upper left) of the rectangle.
		\param y1 Y coordinate of the first corner (upper left) of the rectangle.
		\param x2 X coordinate of the second corner (lower right) of the rectangle.
		\param y2 Y coordinate of the second corner (lower right) of the rectangle.
		\param color The color value of the rectangle to draw (0xRRGGBBAA).
		\param alpha Alpha blending amount for pixels.

		\returns Returns 0 on success, -1 on failure.
		*/
		int _filledRectAlpha(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color, Uint8 alpha)
		{
			SDL_PixelFormat *format;
			Uint32 Rmask, Gmask, Bmask, Amask;
			Uint32 Rshift, Gshift, Bshift, Ashift;
			Uint32 sR, sG, sB, sA;
			Uint32 dR, dG, dB, dA;
			Sint16 x, y;

			if (dst == NULL) {
				return (-1);
			}

			format = dst->format;
			switch (format->BytesPerPixel) {
			case 1:
			{			/* Assuming 8-bpp */
					  Uint8 *row, *pixel;
					  Uint8 R, G, B;
					  SDL_Color *colors = format->palette->colors;
					  SDL_Color dColor;
					  SDL_Color sColor = colors[color];
					  sR = sColor.r;
					  sG = sColor.g;
					  sB = sColor.b;

					  for (y = y1; y <= y2; y++) {
						  row = (Uint8 *)dst->pixels + y * dst->pitch;
						  for (x = x1; x <= x2; x++) {
							  if (alpha == 255) {
								  *(row + x) = color;
							  }
							  else {
								  pixel = row + x;

								  dColor = colors[*pixel];
								  dR = dColor.r;
								  dG = dColor.g;
								  dB = dColor.b;

								  R = dR + ((sR - dR) * alpha >> 8);
								  G = dG + ((sG - dG) * alpha >> 8);
								  B = dB + ((sB - dB) * alpha >> 8);

								  *pixel = SDL_MapRGB(format, R, G, B);
							  }
						  }
					  }
			}
				break;

			case 2:
			{			/* Probably 15-bpp or 16-bpp */
					  Uint16 *row, *pixel;
					  Uint16 R, G, B, A;
					  Uint16 dc;
					  Rmask = format->Rmask;
					  Gmask = format->Gmask;
					  Bmask = format->Bmask;
					  Amask = format->Amask;

					  sR = (color & Rmask);
					  sG = (color & Gmask);
					  sB = (color & Bmask);
					  sA = (color & Amask);

					  for (y = y1; y <= y2; y++) {
						  row = (Uint16 *)dst->pixels + y * dst->pitch / 2;
						  for (x = x1; x <= x2; x++) {
							  if (alpha == 255) {
								  *(row + x) = color;
							  }
							  else {
								  pixel = row + x;
								  dc = *pixel;

								  dR = (dc & Rmask);
								  dG = (dc & Gmask);
								  dB = (dc & Bmask);

								  R = (dR + ((sR - dR) * alpha >> 8)) & Rmask;
								  G = (dG + ((sG - dG) * alpha >> 8)) & Gmask;
								  B = (dB + ((sB - dB) * alpha >> 8)) & Bmask;
								  *pixel = R | G | B;
								  if (Amask != 0) {
									  dA = (dc & Amask);
									  A = (dA + ((sA - dA) * alpha >> 8)) & Amask;
									  *pixel |= A;
								  }
							  }
						  }
					  }
			}
				break;

			case 3:
			{			/* Slow 24-bpp mode, usually not used */
					  Uint8 *row, *pixel;
					  Uint8 R, G, B;
					  Uint8 Rshift8, Gshift8, Bshift8;

					  Rshift = format->Rshift;
					  Gshift = format->Gshift;
					  Bshift = format->Bshift;

					  Rshift8 = Rshift >> 3;
					  Gshift8 = Gshift >> 3;
					  Bshift8 = Bshift >> 3;

					  sR = (color >> Rshift) & 0xff;
					  sG = (color >> Gshift) & 0xff;
					  sB = (color >> Bshift) & 0xff;

					  for (y = y1; y <= y2; y++) {
						  row = (Uint8 *)dst->pixels + y * dst->pitch;
						  for (x = x1; x <= x2; x++) {
							  pixel = row + x * 3;

							  if (alpha == 255) {
								  *(pixel + Rshift8) = sR;
								  *(pixel + Gshift8) = sG;
								  *(pixel + Bshift8) = sB;
							  }
							  else {
								  dR = *((pixel)+Rshift8);
								  dG = *((pixel)+Gshift8);
								  dB = *((pixel)+Bshift8);

								  R = dR + ((sR - dR) * alpha >> 8);
								  G = dG + ((sG - dG) * alpha >> 8);
								  B = dB + ((sB - dB) * alpha >> 8);

								  *((pixel)+Rshift8) = R;
								  *((pixel)+Gshift8) = G;
								  *((pixel)+Bshift8) = B;
							  }
						  }
					  }
			}
				break;

#ifdef DEFAULT_ALPHA_PIXEL_ROUTINE
			case 4:
			{			/* Probably :-) 32-bpp */
					  Uint32 *row, *pixel;
					  Uint32 R, G, B, A;
					  Uint32 dc;
					  Rmask = format->Rmask;
					  Gmask = format->Gmask;
					  Bmask = format->Bmask;
					  Amask = format->Amask;

					  Rshift = format->Rshift;
					  Gshift = format->Gshift;
					  Bshift = format->Bshift;
					  Ashift = format->Ashift;

					  sR = (color & Rmask) >> Rshift;
					  sG = (color & Gmask) >> Gshift;
					  sB = (color & Bmask) >> Bshift;
					  sA = (color & Amask) >> Ashift;

					  for (y = y1; y <= y2; y++) {
						  row = (Uint32 *)dst->pixels + y * dst->pitch / 4;
						  for (x = x1; x <= x2; x++) {
							  if (alpha == 255) {
								  *(row + x) = color;
							  }
							  else {
								  pixel = row + x;
								  dc = *pixel;

								  dR = (dc & Rmask) >> Rshift;
								  dG = (dc & Gmask) >> Gshift;
								  dB = (dc & Bmask) >> Bshift;

								  R = ((dR + ((sR - dR) * alpha >> 8)) << Rshift) & Rmask;
								  G = ((dG + ((sG - dG) * alpha >> 8)) << Gshift) & Gmask;
								  B = ((dB + ((sB - dB) * alpha >> 8)) << Bshift) & Bmask;
								  *pixel = R | G | B;
								  if (Amask != 0) {
									  dA = (dc & Amask) >> Ashift;
#ifdef ALPHA_PIXEL_ADDITIVE_BLEND
									  A = (dA | GFX_ALPHA_ADJUST_ARRAY[sA & 255]) << Ashift; // make destination less transparent...
#else
									  A = ((dA + ((sA - dA) * alpha >> 8)) << Ashift) & Amask;
#endif
									  *pixel |= A;
								  }
							  }
						  }
					  }
			}
				break;
#endif

#ifdef EXPERIMENTAL_ALPHA_PIXEL_ROUTINE
			case 4:{			/* Probably :-) 32-bpp */
					   Uint32 *row, *pixel;
					   Uint32 dR, dG, dB, dA;
					   Uint32 dc;
					   Uint32 surfaceAlpha, preMultR, preMultG, preMultB;
					   Uint32 aTmp;

					   Rmask = format->Rmask;
					   Gmask = format->Gmask;
					   Bmask = format->Bmask;
					   Amask = format->Amask;

					   dR = (color & Rmask);
					   dG = (color & Gmask);
					   dB = (color & Bmask);
					   dA = (color & Amask);

					   Rshift = format->Rshift;
					   Gshift = format->Gshift;
					   Bshift = format->Bshift;
					   Ashift = format->Ashift;

					   preMultR = (alpha * (dR >> Rshift));
					   preMultG = (alpha * (dG >> Gshift));
					   preMultB = (alpha * (dB >> Bshift));

					   for (y = y1; y <= y2; y++) {
						   row = (Uint32 *)dst->pixels + y * dst->pitch / 4;
						   for (x = x1; x <= x2; x++) {
							   if (alpha == 255) {
								   *(row + x) = color;
							   }
							   else {
								   pixel = row + x;
								   dc = *pixel;

								   surfaceAlpha = ((dc & Amask) >> Ashift);
								   aTmp = (255 - alpha);
								   if (A = 255 - ((aTmp * (255 - surfaceAlpha)) >> 8)) {
									   aTmp *= surfaceAlpha;
									   R = (preMultR + ((aTmp * ((dc & Rmask) >> Rshift)) >> 8)) / A << Rshift & Rmask;
									   G = (preMultG + ((aTmp * ((dc & Gmask) >> Gshift)) >> 8)) / A << Gshift & Gmask;
									   B = (preMultB + ((aTmp * ((dc & Bmask) >> Bshift)) >> 8)) / A << Bshift & Bmask;
								   }
								   *pixel = R | G | B | (A << Ashift & Amask);
							   }
						   }
					   }
			}
				break;
#endif

			}

			return (0);
		}

		/*!
		\brief Draw filled rectangle of RGBA color with alpha blending.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first corner (upper left) of the rectangle.
		\param y1 Y coordinate of the first corner (upper left) of the rectangle.
		\param x2 X coordinate of the second corner (lower right) of the rectangle.
		\param y2 Y coordinate of the second corner (lower right) of the rectangle.
		\param color The color value of the rectangle to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int filledRectAlpha(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color)
		{
			Uint8 alpha;
			Uint32 mcolor;
			int result = 0;

			/*
			* Lock the surface
			*/
			if (SDL_MUSTLOCK(dst)) {
				if (SDL_LockSurface(dst) < 0) {
					return (-1);
				}
			}

			/*
			* Setup color
			*/
			alpha = color & 0x000000ff;
			mcolor =
				SDL_MapRGBA(dst->format, (color & 0xff000000) >> 24,
				(color & 0x00ff0000) >> 16, (color & 0x0000ff00) >> 8, alpha);

			/*
			* Draw
			*/
			result = _filledRectAlpha(dst, x1, y1, x2, y2, mcolor, alpha);

			/*
			* Unlock the surface
			*/
			if (SDL_MUSTLOCK(dst)) {
				SDL_UnlockSurface(dst);
			}

			return (result);
		}

		/*!
		\brief Internal function to draw horizontal line of RGBA color with alpha blending.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first point (i.e. left) of the line.
		\param x2 X coordinate of the second point (i.e. right) of the line.
		\param y Y coordinate of the points of the line.
		\param color The color value of the line to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int _HLineAlpha(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color)
		{
			return (filledRectAlpha(dst, x1, y, x2, y, color));
		}

		/*!
		\brief Internal function to draw vertical line of RGBA color with alpha blending.

		\param dst The surface to draw on.
		\param x X coordinate of the points of the line.
		\param y1 Y coordinate of the first point (top) of the line.
		\param y2 Y coordinate of the second point (bottom) of the line.
		\param color The color value of the line to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int _VLineAlpha(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color)
		{
			return (filledRectAlpha(dst, x, y1, x, y2, color));
		}


		/*!
		\brief Internal clip-encoding routine.

		Calculates a segement-based clipping encoding for a point against a rectangle.

		\param x X coordinate of point.
		\param y Y coordinate of point.
		\param left X coordinate of left edge of the rectangle.
		\param top Y coordinate of top edge of the rectangle.
		\param right X coordinate of right edge of the rectangle.
		\param bottom Y coordinate of bottom edge of the rectangle.
		*/
		static int _clipEncode(Sint16 x, Sint16 y, Sint16 left, Sint16 top, Sint16 right, Sint16 bottom)
		{
			int code = 0;

			if (x < left) {
				code |= CLIP_LEFT_EDGE;
			}
			else if (x > right) {
				code |= CLIP_RIGHT_EDGE;
			}
			if (y < top) {
				code |= CLIP_TOP_EDGE;
			}
			else if (y > bottom) {
				code |= CLIP_BOTTOM_EDGE;
			}
			return code;
		}


		/*!
		\brief Clip line to a the clipping rectangle of a surface.

		\param dst Target surface to draw on.
		\param x1 Pointer to X coordinate of first point of line.
		\param y1 Pointer to Y coordinate of first point of line.
		\param x2 Pointer to X coordinate of second point of line.
		\param y2 Pointer to Y coordinate of second point of line.
		*/
		static int _clipLine(SDL_Surface * dst, Sint16 * x1, Sint16 * y1, Sint16 * x2, Sint16 * y2)
		{
			Sint16 left, right, top, bottom;
			int code1, code2;
			int draw = 0;
			Sint16 swaptmp;
			float m;

			/*
			* Get clipping boundary
			*/
			left = dst->clip_rect.x;
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			top = dst->clip_rect.y;
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;

			while (1) {
				code1 = _clipEncode(*x1, *y1, left, top, right, bottom);
				code2 = _clipEncode(*x2, *y2, left, top, right, bottom);
				if (CLIP_ACCEPT(code1, code2)) {
					draw = 1;
					break;
				}
				else if (CLIP_REJECT(code1, code2))
					break;
				else {
					if (CLIP_INSIDE(code1)) {
						swaptmp = *x2;
						*x2 = *x1;
						*x1 = swaptmp;
						swaptmp = *y2;
						*y2 = *y1;
						*y1 = swaptmp;
						swaptmp = code2;
						code2 = code1;
						code1 = swaptmp;
					}
					if (*x2 != *x1) {
						m = (float)(*y2 - *y1) / (float)(*x2 - *x1);
					}
					else {
						m = 1.0f;
					}
					if (code1 & CLIP_LEFT_EDGE) {
						*y1 += (Sint16)((left - *x1) * m);
						*x1 = left;
					}
					else if (code1 & CLIP_RIGHT_EDGE) {
						*y1 += (Sint16)((right - *x1) * m);
						*x1 = right;
					}
					else if (code1 & CLIP_BOTTOM_EDGE) {
						if (*x2 != *x1) {
							*x1 += (Sint16)((bottom - *y1) / m);
						}
						*y1 = bottom;
					}
					else if (code1 & CLIP_TOP_EDGE) {
						if (*x2 != *x1) {
							*x1 += (Sint16)((top - *y1) / m);
						}
						*y1 = top;
					}
				}
			}

			return draw;
		}

		/*!
		\brief Draw vertical line with blending.

		\param dst The surface to draw on.
		\param x X coordinate of the points of the line.
		\param y1 Y coordinate of the first point (i.e. top) of the line.
		\param y2 Y coordinate of the second point (i.e. bottom) of the line.
		\param color The color value of the line to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int vlineColor(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color)
		{
			Sint16 left, right, top, bottom;
			Uint8 *pixel, *pixellast;
			int dy;
			int pixx, pixy;
			Sint16 h;
			Sint16 ytmp;
			int result = -1;
			Uint8 *colorptr;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Swap y1, y2 if required to ensure y1<=y2
			*/
			if (y1 > y2) {
				ytmp = y1;
				y1 = y2;
				y2 = ytmp;
			}

			/*
			* Get clipping boundary and
			* check visibility of vline
			*/
			left = dst->clip_rect.x;
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			if ((x<left) || (x>right)) {
				return (0);
			}
			top = dst->clip_rect.y;
			if (y2<top) {
				return(0);
			}
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
			if (y1>bottom) {
				return(0);
			}

			/*
			* Clip x
			*/
			if (y1 < top) {
				y1 = top;
			}
			if (y2 > bottom) {
				y2 = bottom;
			}

			/*
			* Calculate height
			*/
			h = y2 - y1;

			/*
			* Alpha check
			*/
			if ((color & 255) == 255) {

				/*
				* No alpha-blending required
				*/

				/*
				* Setup color
				*/
				colorptr = (Uint8 *)& color;
				if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
					color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
				}
				else {
					color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
				}

				/*
				* Lock the surface
				*/
				if (SDL_MUSTLOCK(dst)) {
					if (SDL_LockSurface(dst) < 0) {
						return (-1);
					}
				}

				/*
				* More variable setup
				*/
				dy = h;
				pixx = dst->format->BytesPerPixel;
				pixy = dst->pitch;
				pixel = ((Uint8 *)dst->pixels) + pixx * (int)x + pixy * (int)y1;
				pixellast = pixel + pixy * dy;

				/*
				* Draw
				*/
				switch (dst->format->BytesPerPixel) {
				case 1:
					for (; pixel <= pixellast; pixel += pixy) {
						*(Uint8 *)pixel = color;
					}
					break;
				case 2:
					for (; pixel <= pixellast; pixel += pixy) {
						*(Uint16 *)pixel = color;
					}
					break;
				case 3:
					for (; pixel <= pixellast; pixel += pixy) {
						if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
							pixel[0] = (color >> 16) & 0xff;
							pixel[1] = (color >> 8) & 0xff;
							pixel[2] = color & 0xff;
						}
						else {
							pixel[0] = color & 0xff;
							pixel[1] = (color >> 8) & 0xff;
							pixel[2] = (color >> 16) & 0xff;
						}
					}
					break;
				default:		/* case 4 */
					for (; pixel <= pixellast; pixel += pixy) {
						*(Uint32 *)pixel = color;
					}
					break;
				}

				/* Unlock surface */
				if (SDL_MUSTLOCK(dst)) {
					SDL_UnlockSurface(dst);
				}

				/*
				* Set result code
				*/
				result = 0;

			}
			else {

				/*
				* Alpha blending blit
				*/

				result = _VLineAlpha(dst, x, y1, y1 + h, color);

			}

			return (result);
		}

		/*!
		\brief Draw horizontal line with blending.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first point (i.e. left) of the line.
		\param x2 X coordinate of the second point (i.e. right) of the line.
		\param y Y coordinate of the points of the line.
		\param color The color value of the line to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int hlineColor(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color)
		{
			Sint16 left, right, top, bottom;
			Uint8 *pixel, *pixellast;
			int dx;
			int pixx, pixy;
			Sint16 xtmp;
			int result = -1;
			Uint8 *colorptr;
			Uint8 color3[3];

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Swap x1, x2 if required to ensure x1<=x2
			*/
			if (x1 > x2) {
				xtmp = x1;
				x1 = x2;
				x2 = xtmp;
			}

			/*
			* Get clipping boundary and
			* check visibility of hline
			*/
			left = dst->clip_rect.x;
			if (x2<left) {
				return(0);
			}
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			if (x1>right) {
				return(0);
			}
			top = dst->clip_rect.y;
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
			if ((y<top) || (y>bottom)) {
				return (0);
			}

			/*
			* Clip x
			*/
			if (x1 < left) {
				x1 = left;
			}
			if (x2 > right) {
				x2 = right;
			}

			/*
			* Calculate width difference
			*/
			dx = x2 - x1;

			/*
			* Alpha check
			*/
			if ((color & 255) == 255) {

				/*
				* No alpha-blending required
				*/

				/*
				* Setup color
				*/
				colorptr = (Uint8 *)& color;
				if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
					color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
				}
				else {
					color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
				}

				/*
				* Lock the surface
				*/
				if (SDL_MUSTLOCK(dst)) {
					if (SDL_LockSurface(dst) < 0) {
						return (-1);
					}
				}

				/*
				* More variable setup
				*/
				pixx = dst->format->BytesPerPixel;
				pixy = dst->pitch;
				pixel = ((Uint8 *)dst->pixels) + pixx * (int)x1 + pixy * (int)y;

				/*
				* Draw
				*/
				switch (dst->format->BytesPerPixel) {
				case 1:
					memset(pixel, color, dx + 1);
					break;
				case 2:
					pixellast = pixel + dx + dx;
					for (; pixel <= pixellast; pixel += pixx) {
						*(Uint16 *)pixel = color;
					}
					break;
				case 3:
					pixellast = pixel + dx + dx + dx;
					if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
						color3[0] = (color >> 16) & 0xff;
						color3[1] = (color >> 8) & 0xff;
						color3[2] = color & 0xff;
					}
					else {
						color3[0] = color & 0xff;
						color3[1] = (color >> 8) & 0xff;
						color3[2] = (color >> 16) & 0xff;
					}
					for (; pixel <= pixellast; pixel += pixx) {
						memcpy(pixel, color3, 3);
					}
					break;
				default:		/* case 4 */
					dx = dx + dx;
					pixellast = pixel + dx + dx;
					for (; pixel <= pixellast; pixel += pixx) {
						*(Uint32 *)pixel = color;
					}
					break;
				}

				/*
				* Unlock surface
				*/
				if (SDL_MUSTLOCK(dst)) {
					SDL_UnlockSurface(dst);
				}

				/*
				* Set result code
				*/
				result = 0;

			}
			else {

				/*
				* Alpha blending blit
				*/
				result = _HLineAlpha(dst, x1, x1 + dx, y, color);
			}

			return (result);
		}

		/*!
		\brief Pixel draw with blending enabled if a<255 - no surface locking.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the pixel.
		\param y Y (vertical) coordinate of the pixel.
		\param color The color value of the pixel to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int pixelColorNolock(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color)
		{
			Uint8 alpha;
			Uint32 mcolor;
			int result = 0;

			/*
			* Setup color
			*/
			alpha = color & 0x000000ff;
			mcolor =
				SDL_MapRGBA(dst->format, (color & 0xff000000) >> 24,
				(color & 0x00ff0000) >> 16, (color & 0x0000ff00) >> 8, alpha);

			/*
			* Draw
			*/
			result = _putPixelAlpha(dst, x, y, mcolor, alpha);

			return (result);
		}

		/*!
		\brief Draw line with alpha blending.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first point of the line.
		\param y1 Y coordinate of the first point of the line.
		\param x2 X coordinate of the second point of the line.
		\param y2 Y coordinate of the second point of the line.
		\param color The color value of the line to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int lineColor(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color)
		{
			int pixx, pixy;
			int x, y;
			int dx, dy;
			int ax, ay;
			int sx, sy;
			int swaptmp;
			Uint8 *pixel;
			Uint8 *colorptr;

			/*
			* Clip line and test if we have to draw
			*/
			if (!(_clipLine(dst, &x1, &y1, &x2, &y2))) {
				return (0);
			}

			/*
			* Test for special cases of straight lines or single point
			*/
			if (x1 == x2) {
				if (y1 < y2) {
					return (vlineColor(dst, x1, y1, y2, color));
				}
				else if (y1 > y2) {
					return (vlineColor(dst, x1, y2, y1, color));
				}
				else {
					return (pixelColor(dst, x1, y1, color));
				}
			}
			if (y1 == y2) {
				if (x1 < x2) {
					return (hlineColor(dst, x1, x2, y1, color));
				}
				else if (x1 > x2) {
					return (hlineColor(dst, x2, x1, y1, color));
				}
			}

			/*
			* Variable setup
			*/
			dx = x2 - x1;
			dy = y2 - y1;
			sx = (dx >= 0) ? 1 : -1;
			sy = (dy >= 0) ? 1 : -1;

			/* Lock surface */
			if (SDL_MUSTLOCK(dst)) {
				if (SDL_LockSurface(dst) < 0) {
					return (-1);
				}
			}

			/*
			* Check for alpha blending
			*/
			if ((color & 255) == 255) {

				/*
				* No alpha blending - use fast pixel routines
				*/

				/*
				* Setup color
				*/
				colorptr = (Uint8 *)& color;
				if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
					color = SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
				}
				else {
					color = SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
				}

				/*
				* More variable setup
				*/
				dx = sx * dx + 1;
				dy = sy * dy + 1;
				pixx = dst->format->BytesPerPixel;
				pixy = dst->pitch;
				pixel = ((Uint8 *)dst->pixels) + pixx * (int)x1 + pixy * (int)y1;
				pixx *= sx;
				pixy *= sy;
				if (dx < dy) {
					swaptmp = dx;
					dx = dy;
					dy = swaptmp;
					swaptmp = pixx;
					pixx = pixy;
					pixy = swaptmp;
				}

				/*
				* Draw
				*/
				x = 0;
				y = 0;
				switch (dst->format->BytesPerPixel) {
				case 1:
					for (; x < dx; x++, pixel += pixx) {
						*pixel = color;
						y += dy;
						if (y >= dx) {
							y -= dx;
							pixel += pixy;
						}
					}
					break;
				case 2:
					for (; x < dx; x++, pixel += pixx) {
						*(Uint16 *)pixel = color;
						y += dy;
						if (y >= dx) {
							y -= dx;
							pixel += pixy;
						}
					}
					break;
				case 3:
					for (; x < dx; x++, pixel += pixx) {
						if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
							pixel[0] = (color >> 16) & 0xff;
							pixel[1] = (color >> 8) & 0xff;
							pixel[2] = color & 0xff;
						}
						else {
							pixel[0] = color & 0xff;
							pixel[1] = (color >> 8) & 0xff;
							pixel[2] = (color >> 16) & 0xff;
						}
						y += dy;
						if (y >= dx) {
							y -= dx;
							pixel += pixy;
						}
					}
					break;
				default:		/* case 4 */
					for (; x < dx; x++, pixel += pixx) {
						*(Uint32 *)pixel = color;
						y += dy;
						if (y >= dx) {
							y -= dx;
							pixel += pixy;
						}
					}
					break;
				}

			}
			else {

				/*
				* Alpha blending required - use single-pixel blits
				*/

				ax = ABS(dx) << 1;
				ay = ABS(dy) << 1;
				x = x1;
				y = y1;
				if (ax > ay) {
					int d = ay - (ax >> 1);

					while (x != x2) {
						pixelColorNolock(dst, x, y, color);
						if (d > 0 || (d == 0 && sx == 1)) {
							y += sy;
							d -= ax;
						}
						x += sx;
						d += ay;
					}
				}
				else {
					int d = ax - (ay >> 1);

					while (y != y2) {
						pixelColorNolock(dst, x, y, color);
						if (d > 0 || ((d == 0) && (sy == 1))) {
							x += sx;
							d -= ay;
						}
						y += sy;
						d += ax;
					}
				}
				pixelColorNolock(dst, x, y, color);

			}

			/* Unlock surface */
			if (SDL_MUSTLOCK(dst)) {
				SDL_UnlockSurface(dst);
			}

			return (0);
		}

		/*!
		\brief Draw line with alpha blending.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first point of the line.
		\param y1 Y coordinate of the first point of the line.
		\param x2 X coordinate of the second point of the line.
		\param y2 Y coordinate of the second point of the line.
		\param r The red value of the line to draw.
		\param g The green value of the line to draw.
		\param b The blue value of the line to draw.
		\param a The alpha value of the line to draw.

		\returns Returns 0 on success, -1 on failure.
		*/
		int lineRGBA(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			/*
			* Draw
			*/
			return (lineColor(dst, x1, y1, x2, y2, ((Uint32)r << 24) | ((Uint32)g << 16) | ((Uint32)b << 8) | (Uint32)a));
		}

		/*!
		\brief Draw filled circle with blending.

		Note: Based on algorithms from sge library with modifications by A. Schiffler for
		multiple-hline draw removal and other minor speedup changes.

		\param dst The surface to draw on.
		\param x X coordinate of the center of the filled circle.
		\param y Y coordinate of the center of the filled circle.
		\param rad Radius in pixels of the filled circle.
		\param color The color value of the filled circle to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int filledCircleColor(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint32 color)
		{
			Sint16 left, right, top, bottom;
			int result;
			Sint16 x1, y1, x2, y2;
			Sint16 cx = 0;
			Sint16 cy = rad;
			Sint16 ocx = (Sint16)0xffff;
			Sint16 ocy = (Sint16)0xffff;
			Sint16 df = 1 - rad;
			Sint16 d_e = 3;
			Sint16 d_se = -2 * rad + 5;
			Sint16 xpcx, xmcx, xpcy, xmcy;
			Sint16 ypcy, ymcy, ypcx, ymcx;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Sanity check radius
			*/
			if (rad < 0) {
				return (-1);
			}

			/*
			* Special case for rad=0 - draw a point
			*/
			if (rad == 0) {
				return (pixelColor(dst, x, y, color));
			}

			/*
			* Get circle and clipping boundary and
			* test if bounding box of circle is visible
			*/
			x2 = x + rad;
			left = dst->clip_rect.x;
			if (x2<left) {
				return(0);
			}
			x1 = x - rad;
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			if (x1>right) {
				return(0);
			}
			y2 = y + rad;
			top = dst->clip_rect.y;
			if (y2<top) {
				return(0);
			}
			y1 = y - rad;
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
			if (y1>bottom) {
				return(0);
			}

			/*
			* Draw
			*/
			result = 0;
			do {
				xpcx = x + cx;
				xmcx = x - cx;
				xpcy = x + cy;
				xmcy = x - cy;
				if (ocy != cy) {
					if (cy > 0) {
						ypcy = y + cy;
						ymcy = y - cy;
						result |= hlineColor(dst, xmcx, xpcx, ypcy, color);
						result |= hlineColor(dst, xmcx, xpcx, ymcy, color);
					}
					else {
						result |= hlineColor(dst, xmcx, xpcx, y, color);
					}
					ocy = cy;
				}
				if (ocx != cx) {
					if (cx != cy) {
						if (cx > 0) {
							ypcx = y + cx;
							ymcx = y - cx;
							result |= hlineColor(dst, xmcy, xpcy, ymcx, color);
							result |= hlineColor(dst, xmcy, xpcy, ypcx, color);
						}
						else {
							result |= hlineColor(dst, xmcy, xpcy, y, color);
						}
					}
					ocx = cx;
				}
				/*
				* Update
				*/
				if (df < 0) {
					df += d_e;
					d_e += 2;
					d_se += 2;
				}
				else {
					df += d_se;
					d_e += 2;
					d_se += 4;
					cy--;
				}
				cx++;
			} while (cx <= cy);

			return (result);
		}

		/*!
		\brief Internal helper qsort callback functions used in filled polygon drawing.

		\param a The surface to draw on.
		\param b Vertex array containing X coordinates of the points of the polygon.

		\returns Returns 0 if a==b, a negative number if a<b or a positive number if a>b.
		*/
		int _gfxPrimitivesCompareInt(const void *a, const void *b)
		{
			return (*(const int *)a) - (*(const int *)b);
		}

		/*!
		\brief Global vertex array to use if optional parameters are not given in filledPolygonMT calls.

		Note: Used for non-multithreaded (default) operation of filledPolygonMT.
		*/
		static int *gfxPrimitivesPolyIntsGlobal = NULL;

		/*!
		\brief Flag indicating if global vertex array was already allocated.

		Note: Used for non-multithreaded (default) operation of filledPolygonMT.
		*/
		static int gfxPrimitivesPolyAllocatedGlobal = 0;

		/*!
		\brief Draw filled polygon with alpha blending (multi-threaded capable).

		Note: The last two parameters are optional; but are required for multithreaded operation.

		\param dst The surface to draw on.
		\param vx Vertex array containing X coordinates of the points of the filled polygon.
		\param vy Vertex array containing Y coordinates of the points of the filled polygon.
		\param n Number of points in the vertex array. Minimum number is 3.
		\param color The color value of the filled polygon to draw (0xRRGGBBAA).
		\param polyInts Preallocated, temporary vertex array used for sorting vertices. Required for multithreaded operation; set to NULL otherwise.
		\param polyAllocated Flag indicating if temporary vertex array was allocated. Required for multithreaded operation; set to NULL otherwise.

		\returns Returns 0 on success, -1 on failure.
		*/
		int filledPolygonColorMT(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color, int **polyInts, int *polyAllocated)
		{
			int result;
			int i;
			int y, xa, xb;
			int miny, maxy;
			int x1, y1;
			int x2, y2;
			int ind1, ind2;
			int ints;
			int *gfxPrimitivesPolyInts = NULL;
			int *gfxPrimitivesPolyIntsNew = NULL;
			int gfxPrimitivesPolyAllocated = 0;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Vertex array NULL check
			*/
			if (vx == NULL) {
				return (-1);
			}
			if (vy == NULL) {
				return (-1);
			}

			/*
			* Sanity check number of edges
			*/
			if (n < 3) {
				return -1;
			}

			/*
			* Map polygon cache
			*/
			if ((polyInts == NULL) || (polyAllocated == NULL)) {
				/* Use global cache */
				gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsGlobal;
				gfxPrimitivesPolyAllocated = gfxPrimitivesPolyAllocatedGlobal;
			}
			else {
				/* Use local cache */
				gfxPrimitivesPolyInts = *polyInts;
				gfxPrimitivesPolyAllocated = *polyAllocated;
			}

			/*
			* Allocate temp array, only grow array
			*/
			if (!gfxPrimitivesPolyAllocated) {
				gfxPrimitivesPolyInts = (int *)malloc(sizeof(int)* n);
				gfxPrimitivesPolyAllocated = n;
			}
			else {
				if (gfxPrimitivesPolyAllocated < n) {
					gfxPrimitivesPolyIntsNew = (int *)realloc(gfxPrimitivesPolyInts, sizeof(int)* n);
					if (!gfxPrimitivesPolyIntsNew) {
						if (!gfxPrimitivesPolyInts) {
							free(gfxPrimitivesPolyInts);
							gfxPrimitivesPolyInts = NULL;
						}
						gfxPrimitivesPolyAllocated = 0;
					}
					else {
						gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsNew;
						gfxPrimitivesPolyAllocated = n;
					}
				}
			}

			/*
			* Check temp array
			*/
			if (gfxPrimitivesPolyInts == NULL) {
				gfxPrimitivesPolyAllocated = 0;
			}

			/*
			* Update cache variables
			*/
			if ((polyInts == NULL) || (polyAllocated == NULL)) {
				gfxPrimitivesPolyIntsGlobal = gfxPrimitivesPolyInts;
				gfxPrimitivesPolyAllocatedGlobal = gfxPrimitivesPolyAllocated;
			}
			else {
				*polyInts = gfxPrimitivesPolyInts;
				*polyAllocated = gfxPrimitivesPolyAllocated;
			}

			/*
			* Check temp array again
			*/
			if (gfxPrimitivesPolyInts == NULL) {
				return(-1);
			}

			/*
			* Determine Y maxima
			*/
			miny = vy[0];
			maxy = vy[0];
			for (i = 1; (i < n); i++) {
				if (vy[i] < miny) {
					miny = vy[i];
				}
				else if (vy[i] > maxy) {
					maxy = vy[i];
				}
			}

			/*
			* Draw, scanning y
			*/
			result = 0;
			for (y = miny; (y <= maxy); y++) {
				ints = 0;
				for (i = 0; (i < n); i++) {
					if (!i) {
						ind1 = n - 1;
						ind2 = 0;
					}
					else {
						ind1 = i - 1;
						ind2 = i;
					}
					y1 = vy[ind1];
					y2 = vy[ind2];
					if (y1 < y2) {
						x1 = vx[ind1];
						x2 = vx[ind2];
					}
					else if (y1 > y2) {
						y2 = vy[ind1];
						y1 = vy[ind2];
						x2 = vx[ind1];
						x1 = vx[ind2];
					}
					else {
						continue;
					}
					if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2))) {
						gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
					}
				}

				qsort(gfxPrimitivesPolyInts, ints, sizeof(int), _gfxPrimitivesCompareInt);

				for (i = 0; (i < ints); i += 2) {
					xa = gfxPrimitivesPolyInts[i] + 1;
					xa = (xa >> 16) + ((xa & 32768) >> 15);
					xb = gfxPrimitivesPolyInts[i + 1] - 1;
					xb = (xb >> 16) + ((xb & 32768) >> 15);
					result |= hlineColor(dst, xa, xb, y, color);
				}
			}

			return (result);
		}

		/*!
		\brief Draw filled polygon with alpha blending.

		Note: Standard filledPolygon function is calling multithreaded version with NULL parameters
		to use the global vertex cache.

		\param dst The surface to draw on.
		\param vx Vertex array containing X coordinates of the points of the filled polygon.
		\param vy Vertex array containing Y coordinates of the points of the filled polygon.
		\param n Number of points in the vertex array. Minimum number is 3.
		\param color The color value of the filled polygon to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int filledPolygonColor(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, Uint32 color)
		{
			/*
			* Draw
			*/
			return (filledPolygonColorMT(dst, vx, vy, n, color, NULL, NULL));
		}

		/*!
		\brief Internal function to draw a textured horizontal line.

		\param dst The surface to draw on.
		\param x1 X coordinate of the first point (i.e. left) of the line.
		\param x2 X coordinate of the second point (i.e. right) of the line.
		\param y Y coordinate of the points of the line.
		\param texture The texture surface to retrieve color information from.
		\param texture_dx The X offset for the texture lookup.
		\param texture_dy The Y offset for the textured lookup.

		\returns Returns 0 on success, -1 on failure.
		*/
		int _HLineTextured(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, SDL_Surface *texture, int texture_dx, int texture_dy)
		{
			Sint16 left, right, top, bottom;
			Sint16 w;
			Sint16 xtmp;
			int result = 0;
			int texture_x_walker;
			int texture_y_start;
			SDL_Rect source_rect, dst_rect;
			int pixels_written, write_width;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Swap x1, x2 if required to ensure x1<=x2
			*/
			if (x1 > x2) {
				xtmp = x1;
				x1 = x2;
				x2 = xtmp;
			}

			/*
			* Get clipping boundary and
			* check visibility of hline
			*/
			left = dst->clip_rect.x;
			if (x2<left) {
				return(0);
			}
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			if (x1>right) {
				return(0);
			}
			top = dst->clip_rect.y;
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
			if ((y<top) || (y>bottom)) {
				return (0);
			}

			/*
			* Clip x
			*/
			if (x1 < left) {
				x1 = left;
			}
			if (x2 > right) {
				x2 = right;
			}

			/*
			* Calculate width to draw
			*/
			w = x2 - x1 + 1;

			/*
			* Determine where in the texture we start drawing
			*/
			texture_x_walker = (x1 - texture_dx) % texture->w;
			if (texture_x_walker < 0){
				texture_x_walker = texture->w + texture_x_walker;
			}

			texture_y_start = (y + texture_dy) % texture->h;
			if (texture_y_start < 0){
				texture_y_start = texture->h + texture_y_start;
			}

			// setup the source rectangle; we are only drawing one horizontal line
			source_rect.y = texture_y_start;
			source_rect.x = texture_x_walker;
			source_rect.h = 1;

			// we will draw to the current y
			dst_rect.y = y;

			// if there are enough pixels left in the current row of the texture
			// draw it all at once
			if (w <= texture->w - texture_x_walker){
				source_rect.w = w;
				source_rect.x = texture_x_walker;
				dst_rect.x = x1;
				result = (SDL_BlitSurface(texture, &source_rect, dst, &dst_rect) == 0);
			}
			else { // we need to draw multiple times
				// draw the first segment
				pixels_written = texture->w - texture_x_walker;
				source_rect.w = pixels_written;
				source_rect.x = texture_x_walker;
				dst_rect.x = x1;
				result |= (SDL_BlitSurface(texture, &source_rect, dst, &dst_rect) == 0);
				write_width = texture->w;

				// now draw the rest
				// set the source x to 0
				source_rect.x = 0;
				while (pixels_written < w){
					if (write_width >= w - pixels_written) {
						write_width = w - pixels_written;
					}
					source_rect.w = write_width;
					dst_rect.x = x1 + pixels_written;
					result |= (SDL_BlitSurface(texture, &source_rect, dst, &dst_rect) == 0);
					pixels_written += write_width;
				}
			}

			return result;
		}

		/*!
		\brief Draws a polygon filled with the given texture (Multi-Threading Capable).

		This operation use internally SDL_BlitSurface for lines of the source texture. It supports
		alpha drawing.

		To get the best performance of this operation you need to make sure the texture and the dst surface have the same format
		(see  http://docs.mandragor.org/files/Common_libs_documentation/SDL/SDL_Documentation_project_en/sdlblitsurface.html).
		The last two parameters are optional, but required for multithreaded operation. When set to NULL, uses global static temp array.

		\param dst the destination surface,
		\param vx array of x vector components
		\param vy array of x vector components
		\param n the amount of vectors in the vx and vy array
		\param texture the sdl surface to use to fill the polygon
		\param texture_dx the offset of the texture relative to the screeen. if you move the polygon 10 pixels
		to the left and want the texture to apear the same you need to increase the texture_dx value
		\param texture_dy see texture_dx
		\param polyInts preallocated temp array storage for vertex sorting (used for multi-threaded operation)
		\param polyAllocated flag indicating oif the temp array was allocated (used for multi-threaded operation)

		\returns Returns 0 on success, -1 on failure.
		*/
		int texturedPolygonMT(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n,
			SDL_Surface * texture, int texture_dx, int texture_dy, int **polyInts, int *polyAllocated)
		{
			int result;
			int i;
			int y, xa, xb;
			int minx, maxx, miny, maxy;
			int x1, y1;
			int x2, y2;
			int ind1, ind2;
			int ints;
			int *gfxPrimitivesPolyInts = NULL;
			int gfxPrimitivesPolyAllocated = 0;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Sanity check number of edges
			*/
			if (n < 3) {
				return -1;
			}

			/*
			* Map polygon cache
			*/
			if ((polyInts == NULL) || (polyAllocated == NULL)) {
				/* Use global cache */
				gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsGlobal;
				gfxPrimitivesPolyAllocated = gfxPrimitivesPolyAllocatedGlobal;
			}
			else {
				/* Use local cache */
				gfxPrimitivesPolyInts = *polyInts;
				gfxPrimitivesPolyAllocated = *polyAllocated;
			}

			/*
			* Allocate temp array, only grow array
			*/
			if (!gfxPrimitivesPolyAllocated) {
				gfxPrimitivesPolyInts = (int *)malloc(sizeof(int)* n);
				gfxPrimitivesPolyAllocated = n;
			}
			else {
				if (gfxPrimitivesPolyAllocated < n) {
					gfxPrimitivesPolyInts = (int *)realloc(gfxPrimitivesPolyInts, sizeof(int)* n);
					gfxPrimitivesPolyAllocated = n;
				}
			}

			/*
			* Check temp array
			*/
			if (gfxPrimitivesPolyInts == NULL) {
				gfxPrimitivesPolyAllocated = 0;
			}

			/*
			* Update cache variables
			*/
			if ((polyInts == NULL) || (polyAllocated == NULL)) {
				gfxPrimitivesPolyIntsGlobal = gfxPrimitivesPolyInts;
				gfxPrimitivesPolyAllocatedGlobal = gfxPrimitivesPolyAllocated;
			}
			else {
				*polyInts = gfxPrimitivesPolyInts;
				*polyAllocated = gfxPrimitivesPolyAllocated;
			}

			/*
			* Check temp array again
			*/
			if (gfxPrimitivesPolyInts == NULL) {
				return(-1);
			}

			/*
			* Determine X,Y minima,maxima
			*/
			miny = vy[0];
			maxy = vy[0];
			minx = vx[0];
			maxx = vx[0];
			for (i = 1; (i < n); i++) {
				if (vy[i] < miny) {
					miny = vy[i];
				}
				else if (vy[i] > maxy) {
					maxy = vy[i];
				}
				if (vx[i] < minx) {
					minx = vx[i];
				}
				else if (vx[i] > maxx) {
					maxx = vx[i];
				}
			}
			if (maxx <0 || minx > dst->w){
				return -1;
			}
			if (maxy <0 || miny > dst->h){
				return -1;
			}

			/*
			* Draw, scanning y
			*/
			result = 0;
			for (y = miny; (y <= maxy); y++) {
				ints = 0;
				for (i = 0; (i < n); i++) {
					if (!i) {
						ind1 = n - 1;
						ind2 = 0;
					}
					else {
						ind1 = i - 1;
						ind2 = i;
					}
					y1 = vy[ind1];
					y2 = vy[ind2];
					if (y1 < y2) {
						x1 = vx[ind1];
						x2 = vx[ind2];
					}
					else if (y1 > y2) {
						y2 = vy[ind1];
						y1 = vy[ind2];
						x2 = vx[ind1];
						x1 = vx[ind2];
					}
					else {
						continue;
					}
					if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2))) {
						gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
					}
				}

				qsort(gfxPrimitivesPolyInts, ints, sizeof(int), _gfxPrimitivesCompareInt);

				for (i = 0; (i < ints); i += 2) {
					xa = gfxPrimitivesPolyInts[i] + 1;
					xa = (xa >> 16) + ((xa & 32768) >> 15);
					xb = gfxPrimitivesPolyInts[i + 1] - 1;
					xb = (xb >> 16) + ((xb & 32768) >> 15);
					result |= _HLineTextured(dst, xa, xb, y, texture, texture_dx, texture_dy);
				}
			}

			return (result);
		}

		/*!
		\brief Draws a polygon filled with the given texture.

		This standard version is calling multithreaded versions with NULL cache parameters.

		\param dst the destination surface,
		\param vx array of x vector components
		\param vy array of x vector components
		\param n the amount of vectors in the vx and vy array
		\param texture the sdl surface to use to fill the polygon
		\param texture_dx the offset of the texture relative to the screeen. if you move the polygon 10 pixels
		to the left and want the texture to apear the same you need to increase the texture_dx value
		\param texture_dy see texture_dx

		\returns Returns 0 on success, -1 on failure.
		*/
		int texturedPolygon(SDL_Surface * dst, const Sint16 * vx, const Sint16 * vy, int n, SDL_Surface *texture, int texture_dx, int texture_dy)
		{
			/*
			* Draw
			*/
			return (texturedPolygonMT(dst, vx, vy, n, texture, texture_dx, texture_dy, NULL, NULL));
		}

		/*!
		\brief Rotates a 32 bit surface in increments of 90 degrees.

		Specialized 90 degree rotator which rotates a 'src' surface in 90 degree
		increments clockwise returning a new surface. Faster than rotozoomer since
		not scanning or interpolation takes place. Input surface must be 32 bit.
		(code contributed by J. Schiller, improved by C. Allport and A. Schiffler)

		\param src Source surface to rotate.
		\param numClockwiseTurns Number of clockwise 90 degree turns to apply to the source.

		\returns The new, rotated surface; or NULL for surfaces with incorrect input format.
		*/
		SDL_Surface* rotateSurface90Degrees(SDL_Surface* src, int numClockwiseTurns)
		{
			int row, col, newWidth, newHeight;
			int bpp, src_ipr, dst_ipr;
			SDL_Surface* dst;
			Uint32* srcBuf;
			Uint32* dstBuf;

			/* Has to be a valid surface pointer and only 32-bit surfaces (for now) */
			if (!src || src->format->BitsPerPixel != 32) { return NULL; }

			/* normalize numClockwiseTurns */
			while (numClockwiseTurns < 0) { numClockwiseTurns += 4; }
			numClockwiseTurns = (numClockwiseTurns % 4);

			/* if it's even, our new width will be the same as the source surface */
			newWidth = (numClockwiseTurns % 2) ? (src->h) : (src->w);
			newHeight = (numClockwiseTurns % 2) ? (src->w) : (src->h);
			dst = SDL_CreateRGBSurface(src->flags, newWidth, newHeight, src->format->BitsPerPixel,
				src->format->Rmask,
				src->format->Gmask,
				src->format->Bmask,
				src->format->Amask);
			if (!dst) {
				return NULL;
			}

			if (SDL_MUSTLOCK(dst)) {
				SDL_LockSurface(dst);
			}
			if (SDL_MUSTLOCK(dst)) {
				SDL_LockSurface(dst);
			}

			/* Calculate int-per-row */
			bpp = src->format->BitsPerPixel / 8;
			src_ipr = src->pitch / bpp;
			dst_ipr = dst->pitch / bpp;

			switch (numClockwiseTurns) {
			case 0: /* Make a copy of the surface */
			{
						/* Unfortunately SDL_BlitSurface cannot be used to make a copy of the surface
						since it does not preserve alpha. */

						if (src->pitch == dst->pitch) {
							/* If the pitch is the same for both surfaces, the memory can be copied all at once. */
							memcpy(dst->pixels, src->pixels, (src->h * src->pitch));
						}
						else
						{
							/* If the pitch differs, copy each row separately */
							srcBuf = (Uint32*)(src->pixels);
							dstBuf = (Uint32*)(dst->pixels);
							for (row = 0; row < src->h; row++) {
								memcpy(dstBuf, srcBuf, dst->w * bpp);
								srcBuf += src_ipr;
								dstBuf += dst_ipr;
							} /* end for(col) */
						} /* end for(row) */
			}
				break;

				/* rotate clockwise */
			case 1: /* rotated 90 degrees clockwise */
			{
						for (row = 0; row < src->h; ++row) {
							srcBuf = (Uint32*)(src->pixels) + (row * src_ipr);
							dstBuf = (Uint32*)(dst->pixels) + (dst->w - row - 1);
							for (col = 0; col < src->w; ++col) {
								*dstBuf = *srcBuf;
								++srcBuf;
								dstBuf += dst_ipr;
							}
							/* end for(col) */
						}
						/* end for(row) */
			}
				break;

			case 2: /* rotated 180 degrees clockwise */
			{
						for (row = 0; row < src->h; ++row) {
							srcBuf = (Uint32*)(src->pixels) + (row * src_ipr);
							dstBuf = (Uint32*)(dst->pixels) + ((dst->h - row - 1) * dst_ipr) + (dst->w - 1);
							for (col = 0; col < src->w; ++col) {
								*dstBuf = *srcBuf;
								++srcBuf;
								--dstBuf;
							}
						}
			}
				break;

			case 3:
			{
					  for (row = 0; row < src->h; ++row) {
						  srcBuf = (Uint32*)(src->pixels) + (row * src_ipr);
						  dstBuf = (Uint32*)(dst->pixels) + row + ((dst->h - 1) * dst_ipr);
						  for (col = 0; col < src->w; ++col) {
							  *dstBuf = *srcBuf;
							  ++srcBuf;
							  dstBuf -= dst_ipr;
						  }
					  }
			}
				break;
			}
			/* end switch */

			if (SDL_MUSTLOCK(src)) {
				SDL_UnlockSurface(src);
			}
			if (SDL_MUSTLOCK(dst)) {
				SDL_UnlockSurface(dst);
			}

			return dst;
		}


		/*!
		\brief Global cache for NxM pixel font surfaces created at runtime.
		*/
		static SDL_Surface *gfxPrimitivesFont[256];

		/*!
		\brief Global cache of the color used for the font surfaces created at runtime.
		*/
		static Uint32 gfxPrimitivesFontColor[256];

		/*!
		\brief Pointer to the current font data. Default is a 8x8 pixel internal font.
		*/
		static const unsigned char *currentFontdata = gfxPrimitivesFontdata;

		/*!
		\brief Width of the current font. Default is 8.
		*/
		static Uint32 charWidth = 8;

		/*!
		\brief Height of the current font. Default is 8.
		*/
		static Uint32 charHeight = 8;

		/*!
		\brief Width for rendering. Autocalculated.
		*/
		static Uint32 charWidthLocal = 8;

		/*!
		\brief Height for rendering. Autocalculated.
		*/
		static Uint32 charHeightLocal = 8;

		/*!
		\brief Pitch of the current font in bytes. Default is 1.
		*/
		static Uint32 charPitch = 1;

		/*!
		\brief Characters 90deg clockwise rotations. Default is 0. Max is 3.
		*/
		static Uint32 charRotation = 0;

		/*!
		\brief Character data size in bytes of the current font. Default is 8.
		*/
		static Uint32 charSize = 8;

		/*!
		\brief Sets or resets the current global font data.

		The font data array is organized in follows:
		[fontdata] = [character 0][character 1]...[character 255] where
		[character n] = [byte 1 row 1][byte 2 row 1]...[byte {pitch} row 1][byte 1 row 2] ...[byte {pitch} row height] where
		[byte n] = [bit 0]...[bit 7] where
		[bit n] = [0 for transparent pixel|1 for colored pixel]

		\param fontdata Pointer to array of font data. Set to NULL, to reset global font to the default 8x8 font.
		\param cw Width of character in bytes. Ignored if fontdata==NULL.
		\param ch Height of character in bytes. Ignored if fontdata==NULL.
		*/
		void gfxPrimitivesSetFont(const void *fontdata, Uint32 cw, Uint32 ch)
		{
			int i;

			if ((fontdata) && (cw) && (ch)) {
				currentFontdata = (const unsigned char*) fontdata;
				charWidth = cw;
				charHeight = ch;
			}
			else {
				currentFontdata = gfxPrimitivesFontdata;
				charWidth = 8;
				charHeight = 8;
			}

			charPitch = (charWidth + 7) / 8;
			charSize = charPitch * charHeight;

			/* Maybe flip width/height for rendering */
			if ((charRotation == 1) || (charRotation == 3))
			{
				charWidthLocal = charHeight;
				charHeightLocal = charWidth;
			}
			else
			{
				charWidthLocal = charWidth;
				charHeightLocal = charHeight;
			}

			/* Clear character cache */
			for (i = 0; i < 256; i++) {
				if (gfxPrimitivesFont[i]) {
					SDL_FreeSurface(gfxPrimitivesFont[i]);
					gfxPrimitivesFont[i] = NULL;
				}
			}
		}

		/*!
		\brief Sets current global font character rotation steps.

		Default is 0 (no rotation). 1 = 90deg clockwise. 2 = 180deg clockwise. 3 = 270deg clockwise.
		Changing the rotation, will reset the character cache.

		\param rotation Number of 90deg clockwise steps to rotate
		*/
		void gfxPrimitivesSetFontRotation(Uint32 rotation)
		{
			int i;

			rotation = rotation & 3;
			if (charRotation != rotation)
			{
				/* Store rotation */
				charRotation = rotation;

				/* Maybe flip width/height for rendering */
				if ((charRotation == 1) || (charRotation == 3))
				{
					charWidthLocal = charHeight;
					charHeightLocal = charWidth;
				}
				else
				{
					charWidthLocal = charWidth;
					charHeightLocal = charHeight;
				}

				/* Clear character cache */
				for (i = 0; i < 256; i++) {
					if (gfxPrimitivesFont[i]) {
						SDL_FreeSurface(gfxPrimitivesFont[i]);
						gfxPrimitivesFont[i] = NULL;
					}
				}
			}
		}

		/*!
		\brief Draw a character of the currently set font.

		On first call for a particular character and color combination, the function needs to
		generate the character surface (slower. Subsequent calls blit a cached surface (fast).
		Uses alpha blending if A<255 in color.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the upper left corner of the character.
		\param y Y (vertical) coordinate of the upper left corner of the character.
		\param c The character to draw.
		\param color The color value of the character to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int characterColor(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color)
		{
			Sint16 left, right, top, bottom;
			Sint16 x1, y1, x2, y2;
			SDL_Rect srect;
			SDL_Rect drect;
			int result;
			Uint32 ix, iy;
			const unsigned char *charpos;
			Uint8 *curpos;
			int forced_redraw;
			Uint8 patt, mask;
			Uint8 *linepos;
			Uint32 pitch;
			SDL_Surface *rotatedCharacter;
			Uint32 ci;

			/*
			* Check visibility of clipping rectangle
			*/
			if ((dst->clip_rect.w == 0) || (dst->clip_rect.h == 0)) {
				return(0);
			}

			/*
			* Get text and clipping boundary and
			* test if bounding box of character is visible
			*/

			left = dst->clip_rect.x;
			x2 = x + charWidthLocal;
			if (x2<left) {
				return(0);
			}
			right = dst->clip_rect.x + dst->clip_rect.w - 1;
			x1 = x;
			if (x1>right) {
				return(0);
			}
			top = dst->clip_rect.y;
			y2 = y + charHeightLocal;
			if (y2<top) {
				return(0);
			}
			bottom = dst->clip_rect.y + dst->clip_rect.h - 1;
			y1 = y;
			if (y1>bottom) {
				return(0);
			}

			/*
			* Setup source rectangle
			*/
			srect.x = 0;
			srect.y = 0;
			srect.w = charWidthLocal;
			srect.h = charHeightLocal;

			/*
			* Setup destination rectangle
			*/
			drect.x = x;
			drect.y = y;
			drect.w = charWidthLocal;
			drect.h = charHeightLocal;

			/* Character index in cache */
			ci = (unsigned char)c;

			/*
			* Create new charWidth x charHeight bitmap surface if not already present.
			* Might get rotated later.
			*/
			if (gfxPrimitivesFont[ci] == NULL) {
				gfxPrimitivesFont[ci] =
					SDL_CreateRGBSurface(0,
					charWidth, charHeight, 32,
					0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
				/*
				* Check pointer
				*/
				if (gfxPrimitivesFont[ci] == NULL) {
					return (-1);
				}
				/*
				* Definitely redraw
				*/
				forced_redraw = 1;
			}
			else {
				forced_redraw = 0;
			}

			/*
			* Check if color has changed
			*/
			if ((gfxPrimitivesFontColor[ci] != color) || (forced_redraw)) {
				/*
				* Redraw character
				*/
				SDL_SetSurfaceAlphaMod(gfxPrimitivesFont[ci], 255);
				gfxPrimitivesFontColor[ci] = color;

				/* Lock font-surface */
				if (SDL_LockSurface(gfxPrimitivesFont[ci]) != 0)
					return (-1);

				/*
				* Variable setup
				*/
				charpos = currentFontdata + ci * charSize;
				linepos = (Uint8 *)gfxPrimitivesFont[ci]->pixels;
				pitch = gfxPrimitivesFont[ci]->pitch;

				/*
				* Drawing loop
				*/
				patt = 0;
				for (iy = 0; iy < charHeight; iy++) {
					mask = 0x00;
					curpos = linepos;
					for (ix = 0; ix < charWidth; ix++) {
						if (!(mask >>= 1)) {
							patt = *charpos++;
							mask = 0x80;
						}

						if (patt & mask)
							*(Uint32 *)curpos = color;
						else
							*(Uint32 *)curpos = 0;
						curpos += 4;
					}
					linepos += pitch;
				}

				/* Unlock font-surface */
				SDL_UnlockSurface(gfxPrimitivesFont[ci]);

				/* Maybe rotate and replace cached image */
				if (charRotation>0)
				{
					rotatedCharacter = rotateSurface90Degrees(gfxPrimitivesFont[ci], charRotation);
					SDL_FreeSurface(gfxPrimitivesFont[ci]);
					gfxPrimitivesFont[ci] = rotatedCharacter;
				}
			}

			/*
			* Draw bitmap onto destination surface
			*/
			result = SDL_BlitSurface(gfxPrimitivesFont[ci], &srect, dst, &drect);

			return (result);
		}

		/*!
		\brief Draw a character of the currently set font.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the upper left corner of the character.
		\param y Y (vertical) coordinate of the upper left corner of the character.
		\param c The character to draw.
		\param r The red value of the character to draw.
		\param g The green value of the character to draw.
		\param b The blue value of the character to draw.
		\param a The alpha value of the character to draw.

		\returns Returns 0 on success, -1 on failure.
		*/
		int characterRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			/*
			* Draw
			*/
			return (characterColor(dst, x, y, c, ((Uint32)r << 24) | ((Uint32)g << 16) | ((Uint32)b << 8) | (Uint32)a));
		}

		/*!
		\brief Draw a string in the currently set font.

		The spacing between consequtive characters in the string is the fixed number of pixels
		of the character width of the current global font.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the upper left corner of the string.
		\param y Y (vertical) coordinate of the upper left corner of the string.
		\param s The string to draw.
		\param color The color value of the string to draw (0xRRGGBBAA).

		\returns Returns 0 on success, -1 on failure.
		*/
		int stringColor(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint32 color)
		{
			int result = 0;
			Sint16 curx = x;
			Sint16 cury = y;
			const char *curchar = s;

			while (*curchar && !result) {
				result |= characterColor(dst, curx, cury, *curchar, color);
				switch (charRotation)
				{
				case 0:
					curx += charWidthLocal;
					break;
				case 2:
					curx -= charWidthLocal;
					break;
				case 1:
					cury += charHeightLocal;
					break;
				case 3:
					cury -= charHeightLocal;
					break;
				}
				curchar++;
			}

			return (result);
		}

		/*!
		\brief Draw a string in the currently set font.

		\param dst The surface to draw on.
		\param x X (horizontal) coordinate of the upper left corner of the string.
		\param y Y (vertical) coordinate of the upper left corner of the string.
		\param s The string to draw.
		\param r The red value of the string to draw.
		\param g The green value of the string to draw.
		\param b The blue value of the string to draw.
		\param a The alpha value of the string to draw.

		\returns Returns 0 on success, -1 on failure.
		*/
		int stringRGBA(SDL_Surface * dst, Sint16 x, Sint16 y, const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			/*
			* Draw
			*/
			return (stringColor(dst, x, y, s, ((Uint32)r << 24) | ((Uint32)g << 16) | ((Uint32)b << 8) | (Uint32)a));
		}


	}



}