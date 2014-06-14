/*
    SDL_android_main.c, placed in the public domain by Sam Lantinga  3/13/14
*/

/**
 * Modified version to accept parameters from Java, needed for argc and argv trickery
 */
#include "../SDL/src/SDL_internal.h"

#ifdef __ANDROID__

/* Include the SDL main definition header */
#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

/* Start up the SDL app */
int Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jstring locale)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);

    SDL_SetMainReady();

    /* Run the application code! */
    int status;
    char *argv[4];
    
    /* Process the locale string first */
    
    const char *localeString = (*env)->GetStringUTFChars(env, locale, 0);
    argv[0] = SDL_strdup("SDL_app");
    argv[1] = SDL_strdup("--locale");
    argv[2] = SDL_strdup(localeString);
    argv[3] = NULL;
    
    (*env)->ReleaseStringUTFChars(env, locale, localeString);
    
    status = SDL_main(3, argv);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */

    return status;
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
