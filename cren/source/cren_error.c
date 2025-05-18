#include "cren_error.h"

#include "cren_platform.h"

static CRenError gLastError = Success;

const char* cren_error_cstr(CRenError error) {

	switch (error)
	{
	// context
	case ContextIntializationFailed: return "CRen context could not be initialized";
	case RendererInitializationFailed: return "Cren renderer could not be initialized";
	// renderer
	case Vulkan_InstanceCreationFailed: return "Vulkan instance creation has failed";
	case Vulkan_DebuggerCreationFailed: return "Vulkan debugger creation has failed";
	case Vulkan_SurfaceCreationFailed: return "Vulkan surface creation has failed";
	case Vulkan_PhysicalDeviceUnfit: return "Vulkan choosen physical device is unfit for the application";
	case Vulkan_DeviceCreationFailed: return "Vulkan logical device creation has failed";
	case Vulkan_SemaphoreCreationFailed: return "Vulkan semaphore creation has failed";
	case Vulkan_FenceCreationFailed: return "Vulkan fence creation has failed";
	case Vulkan_SwapchainCreationFailed: return "Vulkan swapchain creation has failed";
	case Vulkan_CommandPoolCreationFailed: return "Vulkan command pool creation has failed";
	case Vulkan_CommandBufferCreationFailed: return "Vulkan command buffer creation has failed";
	case Vulkan_CommandBufferAllocationFailed: return "Vulkan command buffer allocation has failed";
	case Vulkan_FramebufferCreationFailed: return "Vulkan framebuffer creation has failed";


	case Success: return "No errors";
	default: return "Undefined error";
	}

	return NULL;
}

const char* cren_last_error_desc() {
    return cren_error_cstr(gLastError);
}

void cren_set_error(CRenError error) {
	gLastError = error;
}
