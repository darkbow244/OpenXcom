/*
    SDL_android_main.c, placed in the public domain by Sam Lantinga  3/13/14
*/

/**
 * This file is a modified version of SDL_android_main.c from SDL2 source code distribution.
 * It is modified to feed locale string into the main application.
 */
#include "../SDL/src/SDL_internal.h"

#ifdef __ANDROID__

/* Include the SDL main definition header */
#include "SDL_main.h"

/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>
#include <android/log.h>

/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern void SDL_Android_Init(JNIEnv* env, jclass cls);

/* Start up the SDL app */
int Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jstring locale, jstring gamePath, jstring dataPath)
{
    /* This interface could expand with ABI negotiation, calbacks, etc. */
    SDL_Android_Init(env, cls);

    SDL_SetMainReady();

    /* Run the application code! */
    int status;
    char *argv[10];
    
    /* Process the locale string first */
    const char *localeString = (*env)->GetStringUTFChars(env, locale, 0);
    const char *gamePathString = (*env)->GetStringUTFChars(env, gamePath, 0);
    const char *gameDataString = (*env)->GetStringUTFChars(env, dataPath, 0);
    __android_log_print(ANDROID_LOG_INFO, "OpenXcom", "  Game path is: %s", gamePathString);
    __android_log_print(ANDROID_LOG_INFO, "OpenXcom", "  Data path is: %s", gameDataString);
    argv[0] = SDL_strdup("SDL_app");
    argv[1] = SDL_strdup("-locale");
    argv[2] = SDL_strdup(localeString);
    argv[3] = SDL_strdup("-user");
    argv[4] = SDL_strdup(gamePathString);
    argv[5] = SDL_strdup("-data");
    argv[6] = SDL_strdup(gameDataString);
    argv[7] = SDL_strdup("-cfg");
    argv[8] = SDL_strdup(gamePathString);
    argv[9] = NULL;
    
    (*env)->ReleaseStringUTFChars(env, locale, localeString);
    (*env)->ReleaseStringUTFChars(env, gamePath, gamePathString);
    (*env)->ReleaseStringUTFChars(env, dataPath, gameDataString);
    
    status = SDL_main(10, argv);

    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */

    return status;
}

#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
