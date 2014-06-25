LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions

LOCAL_MODULE := main

SDL_PATH := ../SDL

SDL_GFX_PATH := ../SDL_gfx

SDL_IMAGE_PATH := ../SDL2_image

SDL_MIXER_PATH := ../SDL2_mixer

YAMLCPP_PATH := ../yamlcpp

BOOST_PATH := ../boost

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(SDL_GFX_PATH) \
	$(LOCAL_PATH)/$(SDL_IMAGE_PATH) \
	$(LOCAL_PATH)/$(SDL_MIXER_PATH)

LOCAL_ARM_MODE := arm

OPENXCOM_VERSION := $(shell git -C $(LOCAL_PATH) describe | sed 's/.*-/-/' | sed 's/.*/\\\"&\\\"/')

LOCAL_CFLAGS += -DOPENXCOM_VERSION_GIT="$(OPENXCOM_VERSION)"

#LOCAL_ARM_NEON := true

# Add your application source files here...
# deleted: $(SDL_PATH)/src/main/android/SDL_android_main.c
LOCAL_SRC_FILES := SDL_android_main.c \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/../../../src/main.cpp \
	$(LOCAL_PATH)/../../../src/lodepng.cpp \
	$(wildcard $(LOCAL_PATH)/../../../src/Basescape/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Battlescape/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Engine/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Engine/Adlib/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Engine/Scalers/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Geoscape/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Interface/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Menu/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Resource/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Ruleset/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Savegame/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../src/Ufopaedia/*.cpp))

LOCAL_STATIC_LIBRARIES := SDL2_static SDL2_image SDL2_mixer SDL_gfx

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

LOCAL_EXPORT_LDLIBS := -lSDL2

# yaml-cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(YAMLCPP_PATH)/include \
					$(LOCAL_PATH)/$(BOOST_PATH)/include/boost-1_53
LOCAL_CPP_INCLUDES += $(LOCAL_PATH)/$(BOOST_PATH)/include/boost-1_53

LOCAL_SRC_FILES += \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/$(YAMLCPP_PATH)/src/*.cpp))

include $(BUILD_SHARED_LIBRARY)
