#ifndef CREN_PLATFORM_INCLUDED
#define CREN_PLATFORM_INCLUDED

#include "cren_utils.h"

#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Various defines
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_APPLE
    #define VK_USE_PLATFORM_METAL_EXT
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
	#if defined(__WAYLAND__)
	    #define PLATFORM_WAYLAND
        #define VK_USE_PLATFORM_WAYLAND_KHR
	#elif defined(__X11__)
	    #define PLATFORM_X11
        #define VK_USE_PLATFORM_XLIB_KHR
	#endif
#else
    #error "Unsupported platform"
#endif

#include <volk.h>

/// @brief  align-as per compiler
#if defined(_MSC_VER)
    #define align_as(X) __declspec(align(X))
#elif defined(__GNUC__) || defined(__clang__)
    #define align_as(X) __attribute__((aligned(X)))
#else
    #include <stdalign.h>
    #define align_as(X) _Alignas(X)
#endif

/// @brief returns an unsigned integer of a version, 0.1.2.325 as an example
#define CREN_MAKE_VERSION(variant, major, minor, patch) ((((unsigned int)(variant)) << 29U) | (((unsigned int)(major)) << 22U) | (((unsigned int)(minor)) << 12U) | ((unsigned int)(patch)))

/// @brief size of a static C-style array. don't use on pointers
#define CREN_ARRAYSIZE(ARR) ((int)(sizeof(ARR) / sizeof(*(ARR))))     

/// @brief how many frames are simultaneosly rendered (multi-buffering)
#define CREN_CONCURRENTLY_RENDERED_FRAMES 2

/// @brief how many characters a path may have
#define CREN_PATH_MAX_SIZE 128

/// @brief macro for "asserting"
#ifdef NDEBUG
#define CREN_ASSERT(condition, msg) ((void)0)
#else
#define CREN_ASSERT(condition, msg) do { if (!(condition)) { fprintf(stderr, "[Line: %d - File:%s] Assertion: %s : Message: %s\n", __LINE__, __FILE__ , #condition, msg); } } while (0);
#endif

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates a window surface for the underneath window
/// @param instance vulkan instance object
/// @param surface output vulkan surface khr
/// @param nativeWindow raw-ptr to the native window, like HWND
/// @return 1 on success, 0 on failure
int cren_surface_create(void* instance, void* surface, void* nativeWindow);

/// @brief formats a const char* with a disk address of a file, constructing it's path
/// @param subpath the desired file path, like "Texture/Sky/left.png"
/// @param assetsRoot the data root location on disk
/// @param removeExtension if equals 1, removes the file extension
/// @param output output string the filepath will be formated to
/// @param outputSize the maximum characters output has, so it won't overflow
void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, size_t outputSize);

/// @brief locks the in-context thread
void cren_thread_lock();

/// @brief unlocks the in-context thread
void cren_thread_unlock();

/// @brief loads an image given a disk path using stb's library
/// @param path image's disk path
/// @param desiredChannels how many channels are desired to be loaded (3: RGB, 4: RGBA)
/// @param outWidth image's read width size
/// @param outHeight image's read height size
/// @param outChannels image's read channels count
/// @return the loaded image or NULL if an error has occured
unsigned char* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels);

/// @brief release the stb image previously created
/// @param ptr image's address
void cren_stbimage_destroy(unsigned char* ptr);

#ifdef __cplusplus 
}
#endif

#endif // CREN_PLATFORM_INCLUDED