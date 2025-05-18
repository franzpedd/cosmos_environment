#ifndef CREN_ERROR_INCLUDED
#define CREN_ERROR_INCLUDED

#include "cren_platform.h"

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief macro for "asserting" and simple logging
#ifdef NDEBUG
#define CREN_LOG(...)
#define CREN_ASSERT(condition, msg) ((void)0)
#else
	#if defined PLATFORM_WINDOWS || defined PLATFORM_WAYLAND || defined PLATFORM_X11
		#include <stdio.h>
		#define CREN_LOG(...) fprintf(stdout, __VA_ARGS__);
		#define CREN_ASSERT(condition, msg) do { if (!(condition)) { fprintf(stderr, "[Line: %d - File:%s] Assertion: %s : Message: %s\n", __LINE__, __FILE__ , #condition, msg); } } while (0);
	#elif defined PLATFORM_ANDROID
		#include <android/log.h>
		#define CREN_LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "CRen", __VA_ARGS__)
		#define CREN_ASSERT(condition, msg) do { if (!(condition)) {  __android_log_print(ANDROID_LOG_ERROR, "CRen",  "[Line: %d - File:%s] Assertion: %s : Message: %s\n" ,__LINE__, __FILE__, #condition, msg); } } while (0);
	#endif
#endif

typedef enum {
	ContextIntializationFailed = -65536,
	RendererInitializationFailed,
	
	Vulkan_InstanceCreationFailed,
	Vulkan_DebuggerCreationFailed,
	Vulkan_SurfaceCreationFailed,
	Vulkan_PhysicalDeviceUnfit,
	Vulkan_DeviceCreationFailed,
	Vulkan_SemaphoreCreationFailed,
	Vulkan_FenceCreationFailed,
	Vulkan_SwapchainCreationFailed,
	Vulkan_CommandPoolCreationFailed,
	Vulkan_CommandBufferCreationFailed,
	Vulkan_CommandBufferAllocationFailed,
	Vulkan_FramebufferCreationFailed,

	Success = 1
} CRenError;

/// @brief converts an error identifier to it's string description
/// @param error the enum error
/// @return the error description
CREN_API const char* cren_error_cstr(CRenError error);

/// @brief returns the last error that has happend on the code
/// @return the error description
CREN_API const char* cren_last_error_desc();

/// @ flags the last error to a predefined one, normally used internally
CREN_API void cren_set_error(CRenError error);

#ifdef __cplusplus 
}
#endif

#endif // CREN_ERROR_INCLUDED