#include "cren_platform.h"
#include "cren_error.h"

#include <threads.h>
#include <string.h>

/// @brief vulkan support detection
#ifdef PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(PLATFORM_APPLE)
    #define VK_USE_PLATFORM_METAL_EXT
#elif defined(PLATFORM_ANDROID)
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(PLATFORM_WAYLAND)
    #define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(PLATFORM_X11)
    #define VK_USE_PLATFORM_XLIB_KHR
#else
    #error "Unsupported platform"
#endif

#include <vulkan/vulkan.h>

#ifdef PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <sal.h>
#elif defined(PLATFORM_APPLE)
    #include <Metal/Metal.h>
#elif defined(PLATFORM_ANDROID)
    #include <android/native_window.h>
#elif defined(PLATFORM_WAYLAND)
    #include <wayland-client.h>
#elif defined(PLATFORM_X11)
    #include <X11/Xlib.h>
#endif

#include <stb_image.h>

#if defined PLATFORM_WINDOWS || defined PLATFORM_WAYLAND || defined PLATFORM_X11

/// @brief only deals with loading desktop files
/// @param path the file's path on disk, hopefully all figured it out beforehand by the user (me/you)
/// @param size the file's content size in bytes
/// @return the file's content
static unsigned int* internal_cren_dekstop_load_file(const char* path, unsigned long long* outSize) {
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    const long file_size_long = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size_long <= 0 || file_size_long % sizeof(unsigned int) != 0) {
        fclose(file);
        return NULL;
    }

    const size_t file_size = (size_t)file_size_long;
    unsigned int* spirv_code = (unsigned int*)crenmemory_allocate(file_size, 1);
    if (!spirv_code) {
        fclose(file);
        return NULL;
    }

    const size_t words_read = fread(spirv_code, sizeof(unsigned int),
        file_size / sizeof(unsigned int), file);
    fclose(file);

    if (words_read != file_size / sizeof(unsigned int)) {
        free(spirv_code);
        return NULL;
    }

    if (outSize) *outSize = file_size;
    return spirv_code;
}

#elif defined PLATFORM_ANDROID

static AAssetManager* g_AssetManager = NULL;

AAssetManager* cren_android_assets_manager_get() {
    if (!g_AssetManager) {
        CREN_LOG("[Android]: AssetManager accessed before init!");
    }
    return g_AssetManager;
}

void cren_android_assets_manager_init(void* manager) {
    CREN_LOG("[Android]: AssetManager initializing with address %p", manager);
    g_AssetManager = manager;
}

/// @brief only deals with loading android files
/// @param path the file's path on disk, hopefully all figured it out beforehand by the user (me/you)
/// @param size the file's content size in bytes
/// @return the file's content
static unsigned int* internal_cren_android_load_file(const char* path, unsigned long long* outSize) {
    if (!g_AssetManager) {
        CREN_LOG("[Android]: AssetManager is NULL, make sure CRen is compiled as a Shared Library and don't forget to call cren_android_assets_manager_init from Java-Side");
        return NULL;
    }

    AAsset* asset = AAssetManager_open(g_AssetManager, path, AASSET_MODE_BUFFER);
    if (!asset) {
        CREN_LOG("[Android]: The desired Asset with path %s does not exists", path);
        return NULL;
    }

    const size_t file_size = AAsset_getLength(asset);
    if (file_size == 0 || file_size % sizeof(unsigned int) != 0) {
        AAsset_close(asset);
        return NULL;
    }

    unsigned int* data = (unsigned int*)crenmemory_allocate(file_size, 1);
    if (!data) {
        AAsset_close(asset);
        return NULL;
    }

    if (AAsset_read(asset, data, file_size) != file_size) {
        free(data);
        AAsset_close(asset);
        return NULL;
    }

    AAsset_close(asset);

    if (outSize) *outSize = file_size;
    return data;
}

#endif

unsigned int* cren_load_file(const char* path, unsigned long long* outSize) {
    #ifdef PLATFORM_ANDROID
    return internal_cren_android_load_file(path, outSize);
    #else
    return internal_cren_dekstop_load_file(path, outSize);
    #endif
}

int cren_surface_create(void* instance, void* surface, void* nativeWindow) {
    VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;
    VkInstance vkInstance = (VkInstance)instance;
    VkSurfaceKHR* vkSurface = (VkSurfaceKHR*)surface;

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(NULL);
    createInfo.hwnd = (HWND)nativeWindow;
    PFN_vkCreateWin32SurfaceKHR fn = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
#elif defined(PLATFORM_APPLE)
    VkMetalSurfaceCreateInfoEXT createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = (CAMetalLayer*)nativeWindow;
    PFN_vkCreateMetalSurfaceEXT fn = (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateMetalSurfaceEXT");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
#elif defined(PLATFORM_ANDROID)
    VkAndroidSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = (ANativeWindow*)nativeWindow;
    PFN_vkCreateAndroidSurfaceKHR fn = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateAndroidSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
#elif defined(__WAYLAND__)
    VkWaylandSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = ((struct wl_display*)nativeWindow);
    createInfo.surface = ((struct wl_surface*)nativeWindow);
    PFN_vkCreateWaylandSurfaceKHR fn = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWaylandSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
#elif defined(__X11__)
    VkXlibSurfaceCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = XOpenDisplay(NULL);
    createInfo.window = (Window)nativeWindow;
    PFN_vkCreateXlibSurfaceKHR fn = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateXlibSurfaceKHR");
    result = fn(vkInstance, &createInfo, NULL, vkSurface);
#endif

    return result == VK_SUCCESS;
}

void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, unsigned long long outputSize) {
    snprintf(output, outputSize, "%s/%s", assetsRoot, subpath);
    
    if (removeExtension == 1) {
        char* lastDot = strrchr(output, '.'); // find the last '.' in the string
        if (lastDot) {
            *lastDot = '\0'; // truncate the string at the last '.'
        }
    }
}

/// 
static mtx_t g_mutex;
static once_flag g_once_flag = ONCE_FLAG_INIT;

#if defined(PLATFORM_WINDOWS)
#define ACQUIRES_LOCK _Acquires_lock_(&g_mutex)
#define RELEASES_LOCK _Releases_lock_(&g_mutex)
#else
// Empty macros for other platforms
#define ACQUIRES_LOCK
#define RELEASES_LOCK
#endif

/// @brief initializes the mutex
static void init_mutex(void) {
    mtx_init(&g_mutex, mtx_plain);
}

static void internal_lock() ACQUIRES_LOCK {
    call_once(&g_once_flag, init_mutex);
    mtx_lock(&g_mutex);
}

static void internal_unlock() RELEASES_LOCK {
    mtx_unlock(&g_mutex);
}

void cren_thread_lock() {
    internal_lock();
}

void cren_thread_unlock() {
    internal_unlock();
}

unsigned char* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels) {

    int x, y, channels = 0;
    stbi_uc* pixels = stbi_load(path, &x, &y, &channels, desiredChannels);

    *outWidth = x;
    *outHeight = y;
    *outChannels = channels;
    return pixels;
}

void cren_stbimage_destroy(unsigned char* ptr) {
    stbi_image_free(ptr);
}

const char* cren_stbimage_get_error()
{
    return stbi_failure_reason();
}
