These patches are needed to build the current OpenXcom for android.

The SDL2* patches are needed for the corresponding libraries to be linked
statically to the main library. The SDL_gfx-SDL2.patch file enables the
SDL_gfx library to be built with SDL2. sdl_gfx_Android.mk should replace the
Android.mk file in SDL_gfx dir (again, it simply enables the library to be
linked statically).

SDL_gfx-SDL2.patch is not made by me, it's from the r102 revision of SDL_gfx
and is available here: http://sourceforge.net/p/sdlgfx/code/HEAD/tree/Other%20Builds/SDL_gfx-SDL2.patch

