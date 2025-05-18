#ifndef CREN_PLATFORM_INCLUDED
#define CREN_PLATFORM_INCLUDED

#include "cren_defines.h"
#include "cren_utils.h"

/// @brief convenient macro around thread_local, since it's been renamed on C23
//#define CRenThreadLocal _Thread_local

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef PLATFORM_ANDROID

#include <android/asset_manager.h>

/// @brief called from Java-Side of things, sets up the assets manager, making it persistent accross shared libraries
/// @return the address of the assets manager
CREN_API AAssetManager* cren_android_assets_manager_get();

/// @brief the android assets manager must be called from the Java-side, therefore this must be initialized from there, using this function
/// @param manager the android asset manager used
CREN_API void cren_android_assets_manager_init(void* manager);

#endif

/// @brief loads a file from a given path, cross-platform
/// @param path the file's path on disk/archive
/// @param outSize the file's content size in bytes
/// @return the u8* data loaded from file or NULL if an error occurred
CREN_API unsigned int* cren_load_file(const char* path, unsigned long long* outSize);

/// @brief creates a window surface for the underneath window
/// @param instance vulkan instance object
/// @param surface output vulkan surface khr
/// @param nativeWindow raw-ptr to the native window, like HWND
/// @return 1 on success, 0 on failure
CREN_API int cren_surface_create(void* instance, void* surface, void* nativeWindow);

/// @brief formats a const char* with a disk address of a file, constructing it's path
/// @param subpath the desired file path, like "Texture/Sky/left.png"
/// @param assetsRoot the data root location on disk
/// @param removeExtension if equals 1, removes the file extension
/// @param output output string the filepath will be formated to
/// @param outputSize the maximum characters output has, so it won't overflow
CREN_API void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, unsigned long long outputSize);

/// @brief locks the in-context thread
CREN_API void cren_thread_lock();

/// @brief unlocks the in-context thread
CREN_API void cren_thread_unlock();

/// @brief loads an image given a disk path using stb's library
/// @param path image's disk path
/// @param desiredChannels how many channels are desired to be loaded (3: RGB, 4: RGBA)
/// @param outWidth image's read width size
/// @param outHeight image's read height size
/// @param outChannels image's read channels count
/// @return the loaded image or NULL if an error has occured
CREN_API unsigned char* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels);

/// @brief release the stb image previously created
/// @param ptr image's address
CREN_API void cren_stbimage_destroy(unsigned char* ptr);

/// @brief returns the last error from attempting to load image
// @return the brief error message
CREN_API const char* cren_stbimage_get_error();

#ifdef __cplusplus 
}
#endif

#endif // CREN_PLATFORM_INCLUDED