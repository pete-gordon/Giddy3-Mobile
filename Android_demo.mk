LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := maindemo

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

LOCAL_CFLAGS += -O2 -D__android__ -D__demo__ -DBASEPATH=\"/sdcard/Android/data/com.emtronics.giddy3demo/files/giddy3/\" -DBASEPATH_ALT=\"/data/data/com.emtronics.giddy3demo/files/giddy3/\"

LOCAL_SRC_FILES :=  += $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	enemies.c \
	giddy3.c \
	ptplay.c \
	render.c \
	samples.c \
	specials.c \
	tables.c \
	titles.c \
	

LOCAL_SHARED_LIBRARIES := SDL

LOCAL_LDLIBS := -lGLESv1_CM -llog -lz

include $(BUILD_SHARED_LIBRARY)
