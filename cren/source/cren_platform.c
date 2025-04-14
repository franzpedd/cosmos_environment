#include "cren_platform.h"

#include <threads.h>

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
    PFN_vkCreateMetalSurfaceEXT fn = (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");
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

void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, size_t outputSize) {
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
