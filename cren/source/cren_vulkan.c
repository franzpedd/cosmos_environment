#include "cren_vulkan.h"

#include "cren_callback.h"
#include "cren_context.h"
#include "cren_math.h"
#include "cren_utils.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instance-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief internal function vulkan validation errors callback to
/// @param severity how severe the message is
/// @param type what type of error has occurred
/// @param callback callback info, like message
/// @param userdata not used, vulkan requires it though
/// @return false uppon handled severities, true otherwise
static VKAPI_ATTR VkBool32 VKAPI_CALL internal_crenvk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback, void* userData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        fprintf(stderr, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        fprintf(stderr, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        fprintf(stderr, "%s\n", callback->pMessage);
        return VK_FALSE;
    }

    return VK_TRUE;
}

/// @brief returns a dynamic array containing all instance extensions required by the renderer
/// @param validations includes validation extensions support, used if validations are requested by the application
/// @return the array with all extensions required
static CRenArray* cren_get_required_instance_extensions(int validations) {
    CRenArray* extensions = crenarray_create(6);
    if (!extensions) return NULL;

    crenarray_push_back(extensions, VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(PLATFORM_WINDOWS)
    crenarray_push_back(extensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(PLATFORM_APPLE)
    crenarray_push_back(extensions, VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    crenarray_push_back(extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#elif defined(PLATFORM_ANDROID)
    crenarray_push_back(extensions, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(PLATFORM_WAYLAND)
    crenarray_push_back(extensions, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(PLATFORM_X11)
    crenarray_push_back(extensions, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    crenarray_push_back(extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    if (validations) {
        crenarray_push_back(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        crenarray_push_back(extensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

/// @brief creates a vulkan instance and a vulkan debug utils messenger if validations are requested
/// @param appName application's name
/// @param appVersion application's version
/// @param apiVersion wich vulkan version desired
/// @param validations request the api validaions or not
/// @return 1 on success, 0 on failure
static int internal_crenvk_instance_create(vkInstance* instance, const char* appName, unsigned int appVersion, unsigned int apiVersion, int validations) {
    CRenArray* extensions = cren_get_required_instance_extensions(validations);
    CRenArray* validationLayers = crenarray_create(1);
    crenarray_push_back(validationLayers, "VK_LAYER_KHRONOS_validation");

    // initialize volk
    if(volkInitialize() != VK_SUCCESS) return 0;

    // initialize application info
    VkApplicationInfo appInfo = { 0 };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName = "CRen";
    appInfo.engineVersion = appVersion;
    appInfo.apiVersion = apiVersion;

    // initialize instance create info
    VkInstanceCreateInfo instanceCI = { 0 };
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCI.pApplicationInfo = &appInfo;
    instanceCI.enabledExtensionCount = (unsigned int)crenarray_size(extensions);
    instanceCI.ppEnabledExtensionNames = (const char* const*)crenarray_data(extensions);
#ifdef PLATFORM_APPLE
    instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    // enable validation layers if requested
    VkDebugUtilsMessengerCreateInfoEXT debugCI = { 0 };
    if (validations) {
        debugCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCI.pfnUserCallback = internal_crenvk_debug_callback;
        instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCI;
        instanceCI.enabledLayerCount = (unsigned int)crenarray_size(validationLayers);
        instanceCI.ppEnabledLayerNames = (const char* const*)crenarray_data(validationLayers);
    }

    if(vkCreateInstance(&instanceCI, NULL, &instance->instance) != VK_SUCCESS) return 0;

    // load instance
    volkLoadInstance(instance->instance);

    // create debugger
    if (validations) {
        if(vkCreateDebugUtilsMessengerEXT(instance->instance, &debugCI, NULL, &instance->debugger) != VK_SUCCESS) {
            crenarray_destroy(extensions);
            crenarray_destroy(validationLayers);
            vkDestroyInstance(instance->instance, NULL);
            volkFinalize();

            return 0;
        }
    }

    // free used resources
    crenarray_destroy(extensions);
    crenarray_destroy(validationLayers);

    return 1;
}

/// @brief destroys the vulkan instance and shutsdown volk
/// @param instance address of the vkInstance object
static void internal_crenvk_instance_destroy(vkInstance* instance) {
    if (instance->debugger) vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debugger, NULL);
    if (instance->instance) vkDestroyInstance(instance->instance, NULL);
    volkFinalize();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Device-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief returns what type of memory is necessary given filter an properties
/// @param physicalDevice vulkan phyiscal device
/// @param typeFilter memory filter
/// @param properties memory properties
/// @return the value of the memory type, 0 on not found
static unsigned int internal_crenvk_find_memory_type(VkPhysicalDevice physicalDevice, unsigned int typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (unsigned int i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 0;
}

/// @brief finds the index of each vulkan queue
/// @param device vulkan physical device
/// @param surface vulkan surface
/// @return all queues indice
static vkQueueFamilyIndices internal_crenvk_find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    vkQueueFamilyIndices indices = { 0 };
    indices.graphicFamily = -1;
    indices.presentFamily = -1;
    indices.computeFamily = -1;

    unsigned int queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties* queue_families = crenmemory_allocate(queue_family_count * sizeof(VkQueueFamilyProperties), 1);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    for (unsigned int i = 0; i < queue_family_count; i++) {
        // check for graphics support
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicFamily = i;
            indices.graphicFound = 1;
        }

        // check for compute support
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
            indices.computeFound = 1;
        }

        // check for presentation support
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices.presentFamily = i;
            indices.presentFound = 1;
        }

        if (indices.graphicFamily && indices.presentFound && indices.computeFound) break;
    }

    crenmemory_deallocate(queue_families);
    return indices;
}


/// @brief checks for required device extensions
/// @param device vulkan device
/// @param required_extensions list of required extensions
/// @param extension_count the ammout of extensions
/// @return 1 on success, 0 on failure
static int internal_crenvk_check_device_extension_support(VkPhysicalDevice device, const char** required_extensions, unsigned int extension_count)
{
    unsigned int available_extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, NULL);

    VkExtensionProperties* available_extensions = crenmemory_allocate(available_extension_count * sizeof(VkExtensionProperties), 1);
    vkEnumerateDeviceExtensionProperties(device, NULL, &available_extension_count, available_extensions);

    for (unsigned int i = 0; i < extension_count; i++) {
        int extension_found = 0;
        for (unsigned int j = 0; j < available_extension_count; j++) {
            if (cren_strcmp(required_extensions[i], available_extensions[j].extensionName) == 0) {
                extension_found = 1;
                break;
            }
        }
        if (!extension_found) {
            crenmemory_deallocate(available_extensions);
            return 0;
        }
    }

    crenmemory_deallocate(available_extensions);
    return 1;
}

/// @brief chooses the most-suitable physical device available
/// @param instance vulkan instance
/// @param surface vulkan window surface
/// @return choosen physical device
static VkPhysicalDevice internal_crenvk_choose_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    
    // selecting most suitable physical device
    unsigned int gpus = 0;
    vkEnumeratePhysicalDevices(instance, &gpus, NULL);

    VkPhysicalDevice* devices = crenmemory_allocate(gpus * sizeof(VkPhysicalDevice), 1);
    vkEnumeratePhysicalDevices(instance, &gpus, devices);

    VkPhysicalDevice choosenOne = VK_NULL_HANDLE;
    const char* requiredExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const unsigned int requiredExtensionsCount = 1;
    VkDeviceSize bestScore = 0;

    for (unsigned int i = 0; i < gpus; i++) {

        // checking support
        VkPhysicalDeviceProperties device_props;
        VkPhysicalDeviceFeatures device_features;
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceProperties(devices[i], &device_props);
        vkGetPhysicalDeviceFeatures(devices[i], &device_features);
        vkGetPhysicalDeviceMemoryProperties(devices[i], &mem_props);
        vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(devices[i], surface);
        if (!indices.graphicFound || !indices.presentFound || !indices.computeFound) continue;
        if (!internal_crenvk_check_device_extension_support(devices[i], requiredExtensions, requiredExtensionsCount)) continue;

        // scoring
        VkDeviceSize currentScore = 0;
        if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) currentScore += 1000;  // discrete gpu
        currentScore += device_props.limits.maxImageDimension2D;                                    // max texture size
        for (unsigned int j = 0; j < mem_props.memoryHeapCount; j++) {                                  // prefer devices with dedicated VRAM
            if (mem_props.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                currentScore += mem_props.memoryHeaps[j].size / (VkDeviceSize)(1024 * 1024);        // mb
            }
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            choosenOne = devices[i];
        }
    }

    crenmemory_deallocate(devices);
    return choosenOne;
}

/// @brief creates a vulkan logical device
/// @param physicalDevice choosen vulkan physical device 
/// @param surface vulkan window surface
/// @param device final vulkan device
/// @param graphicsQueue vulkan graphics queue
/// @param presentQueue vulkan presentation queue
/// @param computeQueue vulkan compute queue
/// @param validations signals vulkan validations on/off
/// @return 1 on success, 0 on failure
static int internal_crenvk_create_logical_device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice* device, VkQueue* graphicsQueue, VkQueue* presentQueue, VkQueue* computeQueue, int validations)
{
    const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" }; // must be the same as instance, wich it is
    unsigned int validationLayerCount = 1;

    // find unique queue families
    vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(physicalDevice, surface);
    unsigned int queueFamilyIndices[3]; // store unique queue family indices
    unsigned int queueCount = 0;
    float queuePriority = 1.0f;

    if (indices.graphicFamily != -1) queueFamilyIndices[queueCount++] = indices.graphicFamily;
    if (indices.presentFamily != -1 && indices.presentFamily != indices.graphicFamily)  queueFamilyIndices[queueCount++] = indices.presentFamily;
    if (indices.computeFamily != -1 && indices.computeFamily != indices.graphicFamily && indices.computeFamily != indices.presentFamily) queueFamilyIndices[queueCount++] = indices.computeFamily;

    // create queue create info for each unique queue family
    VkDeviceQueueCreateInfo* queueCreateInfos = (VkDeviceQueueCreateInfo*)crenmemory_allocate(sizeof(VkDeviceQueueCreateInfo) * queueCount, 1);
    for (unsigned int i = 0; i < queueCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].pNext = NULL;
        queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
        queueCreateInfos[i].flags = 0;
    }

    // extensions
    #if defined(PLATFORM_APPLE) && (VK_HEADER_VERSION >= 216)
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME };
    unsigned int extensionCount = 2;
    #else
    const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    unsigned int extensionCount = 1;
    #endif

    // required features
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.shaderInt64 = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // device create info
    VkDeviceCreateInfo deviceCI = { 0 };
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pNext = NULL;
    deviceCI.flags = 0;
    deviceCI.queueCreateInfoCount = queueCount;
    deviceCI.pQueueCreateInfos = queueCreateInfos;
    deviceCI.enabledExtensionCount = extensionCount;
    deviceCI.ppEnabledExtensionNames = extensions;
    deviceCI.pEnabledFeatures = &deviceFeatures;

    // validation layers
    if (validations && validationLayerCount > 0) {
        deviceCI.enabledLayerCount = validationLayerCount;
        deviceCI.ppEnabledLayerNames = validationLayers;
    }
    else {
        deviceCI.enabledLayerCount = 0;
        deviceCI.ppEnabledLayerNames = NULL;
    }

    // create the device
    if(vkCreateDevice(physicalDevice, &deviceCI, NULL, device) != VK_SUCCESS) {
        crenmemory_deallocate(queueCreateInfos);
        return 0;
    }

    // retrieve queues
    vkGetDeviceQueue(*device, indices.graphicFamily, 0, graphicsQueue);
    vkGetDeviceQueue(*device, indices.presentFamily, 0, presentQueue);
    vkGetDeviceQueue(*device, indices.computeFamily, 0, computeQueue);

    crenmemory_deallocate(queueCreateInfos);

    return 1;
}

/// @brief creates the physical and logical device as well of other related-stuff
/// @param backend cren vulkan backend memory address
/// @param nativeWindow raw ptr to the window object
/// @param validations flags the validations are on/off
/// @return 1 on success, 0 on failure
static int internal_crenvk_device_create(CRenVulkanBackend* backend, void* nativeWindow, int validations) {

    if (cren_surface_create(backend->instance.instance, &backend->device.surface, nativeWindow) != 1) return 0;

    backend->device.physicalDevice = internal_crenvk_choose_physical_device(backend->instance.instance, backend->device.surface);

    if (!backend->device.physicalDevice) {
        vkDestroySurfaceKHR(backend->instance.instance, backend->device.surface, NULL);
        return 0;
    }

    vkGetPhysicalDeviceMemoryProperties(backend->device.physicalDevice, &backend->device.physicalDeviceMemoryProperties);
    vkGetPhysicalDeviceProperties(backend->device.physicalDevice, &backend->device.physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(backend->device.physicalDevice, &backend->device.physicalDeviceFeatures);

    // create logical device
    if(internal_crenvk_create_logical_device(backend->device.physicalDevice, backend->device.surface, &backend->device.device, &backend->device.graphicsQueue, &backend->device.presentQueue, &backend->device.computeQueue, validations) != 1) {
        vkDestroySurfaceKHR(backend->instance.instance, backend->device.surface, NULL);
        return 0;
    }

    // syncronization objects
    VkSemaphoreCreateInfo semaphoreCI = { 0 };
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = NULL;
    semaphoreCI.flags = 0;

    VkFenceCreateInfo fenceCI = { 0 };
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = NULL;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    backend->device.imageAvailableSemaphores = crenmemory_allocate(sizeof(VkSemaphore) * CREN_CONCURRENTLY_RENDERED_FRAMES, 1);
    backend->device.finishedRenderingSemaphores = crenmemory_allocate(sizeof(VkSemaphore) * CREN_CONCURRENTLY_RENDERED_FRAMES, 1);
    backend->device.framesInFlightFences = crenmemory_allocate(sizeof(VkFence) * CREN_CONCURRENTLY_RENDERED_FRAMES, 1);

    for (size_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        if (vkCreateSemaphore(backend->device.device, &semaphoreCI, NULL, &backend->device.imageAvailableSemaphores[i]) != VK_SUCCESS) return 0;
        if (vkCreateSemaphore(backend->device.device, &semaphoreCI, NULL, &backend->device.finishedRenderingSemaphores[i]) != VK_SUCCESS) return 0;
        if (vkCreateFence(backend->device.device, &fenceCI, NULL, &backend->device.framesInFlightFences[i]) != VK_SUCCESS) return 0;
    }
    
    return 1;
}

/// @brief destroy device-related objects
/// @param instance cren vulkan instance address
/// @param device cren vulkan device address
static void internal_crenvk_device_destroy(vkInstance* instance, vkDevice* device) {
    if (!instance || !device) return;

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        if (device->imageAvailableSemaphores[i]) vkDestroySemaphore(device->device, device->imageAvailableSemaphores[i], NULL);
    }
    crenmemory_deallocate(device->imageAvailableSemaphores);

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        if (device->finishedRenderingSemaphores[i]) vkDestroySemaphore(device->device, device->finishedRenderingSemaphores[i], NULL);
    }
    crenmemory_deallocate(device->finishedRenderingSemaphores);

    for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
        if (device->framesInFlightFences[i]) vkDestroyFence(device->device, device->framesInFlightFences[i], NULL);
    }
    crenmemory_deallocate(device->framesInFlightFences);

    if (device->device) vkDestroyDevice(device->device, NULL);
    if (device->surface) vkDestroySurfaceKHR(instance->instance, device->surface, NULL);
}

int crenvk_device_create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory *memory, void *data) {
    VkBufferCreateInfo bufferCI = {0};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size = size;
    bufferCI.usage = usage;
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if(vkCreateBuffer(device, &bufferCI, NULL, buffer) != VK_SUCCESS) {
        return 0;
    }

    VkMemoryRequirements memRequirements = {0};
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    // allocate memory for the buffer and bind it
    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = internal_crenvk_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, properties);
    if(vkAllocateMemory(device, &allocInfo, NULL, memory) != VK_SUCCESS) {
        vkDestroyBuffer(device, *buffer, NULL);
        return 0;
    }

    if(vkBindBufferMemory(device, *buffer, *memory, 0) != VK_SUCCESS) {
        vkDestroyBuffer(device, *buffer, NULL);
        return 0;
    }

    // if data is provided, copy it into the buffer
    if (data) {
        void* mapped = NULL;
        if(vkMapMemory(device, *memory, 0, size, 0, &mapped) != VK_SUCCESS) {
            vkDestroyBuffer(device, *buffer, NULL);
            return 0;
        }

        crenmemory_copy(mapped, data, size);

        // flush the memory if it's not host-coherent
        if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange memoryRange = {0};
            memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.memory = *memory;
            memoryRange.offset = 0;
            memoryRange.size = size;
            if(vkFlushMappedMemoryRanges(device, 1, &memoryRange) == VK_SUCCESS, "Failed to flush mapped memory") {
                vkUnmapMemory(device, *memory);
                vkDestroyBuffer(device, *buffer, NULL);
                return 0;
            }
        }

        vkUnmapMemory(device, *memory);
    }

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Swapchain-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief queries swapchain details 
/// @param physicalDevice vulkan physical device
/// @param surface vulkan window surface
/// @return the queried swapchain details
static vkSwapchainDetails internal_crenvk_query_swapchain_details(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    vkSwapchainDetails details = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.surfaceFormatCount, NULL);

    if (details.surfaceFormatCount != 0) {
        details.pSurfaceFormats = crenmemory_allocate(details.surfaceFormatCount * sizeof(VkSurfaceFormatKHR), 1);
        if (details.pSurfaceFormats) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.surfaceFormatCount, details.pSurfaceFormats);
        }
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.pPresentModes = crenmemory_allocate(details.presentModeCount * sizeof(VkPresentModeKHR), 1);
        if (details.pPresentModes) {
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, details.pPresentModes);
        }
    }

    return details;
}

/// @brief returns the most suitable surface format
/// @param formats address to all formats to evaluate
/// @param quantity quantity of formats passed
/// @return the most suitable format
static VkSurfaceFormatKHR internal_crenvk_choose_swapchain_surface_format(VkSurfaceFormatKHR* formats, unsigned int quantity) {
    for (unsigned int i = 0; i < quantity; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return formats[i];
    }

    return formats[0];
}

/// @brief chooses the most suitable presentation mode for the swapchain
/// @param modes address of modes to evaluate
/// @param quantity ammout of modes passed
/// @param vsync vsync is on/off
/// @return the most suitable present mode from the list of modes
static VkPresentModeKHR internal_crenvk_choose_swapchain_present_mode(VkPresentModeKHR* modes, unsigned int quantity, int vsync) {
    // handle edge cases and vsync request
    if (modes == NULL || quantity == 0 || vsync)  return VK_PRESENT_MODE_FIFO_KHR; // Fallback to FIFO

    // search for the best non-VSync mode
    int immediateModeAvailable = 0;
    for (unsigned int i = 0; i < quantity; i++) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)  return VK_PRESENT_MODE_MAILBOX_KHR; // Prefer MAILBOX (multiple buffering) if available
        if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) immediateModeAvailable = 1; // Mark IMMEDIATE mode as available for fallback
    }

    // fallback to IMMEDIATE if available
    if (immediateModeAvailable) return VK_PRESENT_MODE_IMMEDIATE_KHR;

    // fallback to FIFO (always supported)
    return VK_PRESENT_MODE_FIFO_KHR;
}

/// @brief chooses the most suitable extent for the swapchain
/// @param capabilities memory address of the swapchain capabilities
/// @param width desired width
/// @param height desired height
/// @return the window extent
static VkExtent2D internal_crenvk_choose_swapchain_extent(const VkSurfaceCapabilitiesKHR* capabilities, unsigned int width, unsigned int height) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    VkExtent2D actualExtent = { width, height };
    actualExtent.width = uint_clamp(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    actualExtent.height = uint_clamp(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return actualExtent;
}

/// @brief creates a swapchain for image presentation
/// @param swapchain cren swapchain memory address
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param surface vulkan window surface
/// @param width swapchain width
/// @param height swapchain height
/// @param vsync vsync is enabled/disabled
/// @return 1 on success, 0 on failure
static int internal_crenvk_swapchain_create(vkSwapchain* swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, int vsync) {
    
    vkSwapchainDetails details = internal_crenvk_query_swapchain_details(physicalDevice, surface);
    swapchain->swapchainFormat = internal_crenvk_choose_swapchain_surface_format(details.pSurfaceFormats, details.surfaceFormatCount);
    swapchain->swapchainPresentMode = internal_crenvk_choose_swapchain_present_mode(details.pPresentModes, details.presentModeCount, vsync);
    swapchain->swapchainExtent = internal_crenvk_choose_swapchain_extent(&details.capabilities, width, height);

    // images in the swapchain
    swapchain->swapchainImageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && swapchain->swapchainImageCount > details.capabilities.maxImageCount) swapchain->swapchainImageCount = details.capabilities.maxImageCount;

    // create swapchain
    vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(physicalDevice, surface);
    unsigned int queueFamilyIndices[] = { indices.graphicFamily, indices.presentFamily, indices.computeFamily };
    VkSwapchainCreateInfoKHR swapchainCI = { 0 };
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.pNext = NULL;
    swapchainCI.flags = 0;
    swapchainCI.surface = surface;
    swapchainCI.minImageCount = swapchain->swapchainImageCount;
    swapchainCI.imageFormat = swapchain->swapchainFormat.format;
    swapchainCI.imageColorSpace = swapchain->swapchainFormat.colorSpace;
    swapchainCI.imageExtent = swapchain->swapchainExtent;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // allow copying swapchain images
    swapchainCI.preTransform = details.capabilities.currentTransform;
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = swapchain->swapchainPresentMode;
    swapchainCI.clipped = VK_TRUE;

    if (indices.graphicFamily != indices.presentFamily) {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    }

    else {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if(vkCreateSwapchainKHR(device, &swapchainCI, NULL, &swapchain->swapchain) != VK_SUCCESS) {
        crenmemory_deallocate(details.pPresentModes);
        crenmemory_deallocate(details.pSurfaceFormats);
        return 0;
    } 

    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain->swapchainImageCount, NULL);
    swapchain->swapchainImages = (VkImage*)crenmemory_allocate(swapchain->swapchainImageCount * sizeof(VkImage), 1); // dont forget to deallocate at shutdown or resizes
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain->swapchainImageCount, swapchain->swapchainImages);

    // create image views
    swapchain->swapchainImageViews = crenmemory_allocate(sizeof(VkImageView) * swapchain->swapchainImageCount, 1); // dont forget to deallocate at shutdown or resizes
    for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) {
        swapchain->swapchainImageViews[i] = crenvk_image_view_create(device, swapchain->swapchainImages[i], swapchain->swapchainFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);
    }

    // free details
    crenmemory_deallocate(details.pPresentModes);
    crenmemory_deallocate(details.pSurfaceFormats);

    return 1;
}

/// @brief destroys the cre swapchain objects
/// @param swapchain cren vulkan swapchain
/// @param device vulkan device
static void internal_crenvk_swapchain_destroy(vkSwapchain* swapchain, VkDevice device) {
    for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) {
        vkDestroyImageView(device, swapchain->swapchainImageViews[i], NULL);
    }
    crenmemory_deallocate(swapchain->swapchainImageViews);
    
    crenmemory_deallocate(swapchain->swapchainImages); // swapchain images are destroyed by the swapchain
    vkDestroySwapchainKHR(device, swapchain->swapchain, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipeline-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief opens the SPIRV file containing the shader code and returns it
/// @param source path on disk
/// @param outSize spirv code size
/// @return the address to the loaded spirv code or NULL if an error occured
static unsigned int* internal_crenvk_shader_loadspirv(const char* source, size_t* outSize) {
    FILE* file = fopen(source, "rb");
    if (!file) {
        return NULL;
    }

    // get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // ensure the file size is valid
    if (file_size <= 0 || file_size % sizeof(unsigned int) != 0) {
        fclose(file);
        return NULL;
    }

    // allocate memory for the SPIR-V code
    unsigned int* spirv_code = (unsigned int*)crenmemory_allocate(file_size, 0);
    if (!spirv_code) {
        fclose(file);
        return NULL;
    }

    // read the file into the buffer
    size_t word_count = file_size / sizeof(unsigned int);
    size_t read_count = fread(spirv_code, sizeof(unsigned int), word_count, file);
    if (read_count != word_count) {
        crenmemory_deallocate(spirv_code);
        fclose(file);
        return NULL;
    }

    fclose(file);

    if (outSize) {
        *outSize = file_size;
    }

    return spirv_code;
}

/// @brief creates an array of VkVertexInputBindingDescription based on parameters
/// @param passingVertexData flags if vertex data is passed to the shader stages
/// @param bindingCount output binding count
/// @return VkVertexInputBindingDescription's array of NULL if an error occurs
static VkVertexInputBindingDescription* internal_crenvk_pipeline_get_binding_descriptions(int passingVertexData, unsigned int* bindingCount) {
	if (!passingVertexData) return NULL;

	VkVertexInputBindingDescription* bindings = (VkVertexInputBindingDescription*)crenmemory_allocate(sizeof(VkVertexInputBindingDescription), 1);
	bindings[0].binding = 0;
	bindings[0].stride = sizeof(vkVertex);
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	*bindingCount = 1U;
	return bindings;
}

/// @brief creates an array of VkVertexInputAttributeDescription
/// @param vertexComponents array of vertex components
/// @param componentsCount quantity of vertex components on the array
/// @param attributesCount output of attributes quantity
/// @return an array of VkVertexInputAttributeDescription based on parameters or NULL if an error occurs
static VkVertexInputAttributeDescription* internal_crenvk_get_attribute_descriptions(vkVertexComponent* vertexComponents, unsigned int componentsCount, unsigned int* attributesCount) {
	VkVertexInputAttributeDescription* bindings = (VkVertexInputAttributeDescription*)crenmemory_allocate(sizeof(VkVertexInputAttributeDescription) * componentsCount, 1);

	for (unsigned int i = 0; i < componentsCount; i++) {
		
		vkVertexComponent component = vertexComponents[i];
		VkVertexInputAttributeDescription desc = { 0 };
		desc.binding = 0;
		desc.location = (unsigned int)component;

		switch (component)
		{
			case VK_VERTEX_COMPONENT_POSITION:
			{
				desc.format = VK_FORMAT_R32G32B32_SFLOAT;
				desc.offset = offsetof(vkVertex, position);
				break;
			}

			case VK_VERTEX_COMPONENT_NORMAL:
			{
				desc.format = VK_FORMAT_R32G32B32_SFLOAT;
				desc.offset = offsetof(vkVertex, normal);
				break;
			}

			case VK_VERTEX_COMPONENT_UV_0:
			{
				desc.format = VK_FORMAT_R32G32_SFLOAT;
				desc.offset = offsetof(vkVertex, uv_0);
				break;
			}
			
			case VK_VERTEX_COMPONENT_COLOR_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(vkVertex, color_0);
				break;
			}

			case VK_VERTEX_COMPONENT_WEIGHTS_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(vkVertex, weights_0);
				break;
			}

			case VK_VERTEX_COMPONENT_JOINTS_0:
			{
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				desc.offset = offsetof(vkVertex, joints_0);
				break;
			}
		}

		bindings[i] = desc;
	}

	*attributesCount = componentsCount;
	return bindings;
}

/// @brief fills-up a VkPipelineVertexInputStateCreateInfo based on requested components
/// @param pipeline cren vulkan pipeline 
/// @param vertexComponents array of vertex components
/// @param componentsCount quantity of components on the array
/// @return the populated VkPipelineVertexInputStateCreateInfo struct
static VkPipelineVertexInputStateCreateInfo internal_crenvk_pipeline_populate_visci(vkPipeline* pipeline, vkVertexComponent* vertexComponents, unsigned int componentsCount) {
	pipeline->pBindingsDescription = internal_crenvk_pipeline_get_binding_descriptions(pipeline->passingVertexData, &pipeline->bindingsDescriptionCount);
	pipeline->pAttributesDescription = internal_crenvk_get_attribute_descriptions(vertexComponents, componentsCount, &pipeline->attributesDescriptionCount);
	
	VkPipelineVertexInputStateCreateInfo visci = { 0 };
	visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	visci.pNext = NULL;
	visci.flags = 0;
	visci.vertexBindingDescriptionCount = pipeline->bindingsDescriptionCount;
	visci.pVertexBindingDescriptions = pipeline->pBindingsDescription;
	visci.vertexAttributeDescriptionCount = pipeline->attributesDescriptionCount;
	visci.pVertexAttributeDescriptions = pipeline->pAttributesDescription;

	return visci;
}

/// @brief setup the quad pipeline, used by all quads across the renderer
/// @param pipelines pipeline's hashtable
/// @param device vulkan device
/// @param usedRenderpass cren vulkan renderpass in context, may be default or viewport
/// @param pickingRenderpass cren vulkan picking renderpass
static void internal_crenvk_pipeline_quad_create(Hashtable* pipelines, vkRenderpass* usedRenderpass, vkRenderpass* pickingRenderpass, VkDevice device,  const char* rootPath) {
	
    // default pipeline
	vkPipeline* defaultPipeline = crenhashtable_lookup(pipelines, CREN_PIPELINE_QUAD_DEFAULT_NAME);
	if (defaultPipeline != NULL) crenvk_pipeline_destroy(device, defaultPipeline);

	char defaultVert[CREN_PATH_MAX_SIZE], defaultFrag[CREN_PATH_MAX_SIZE];
	cren_get_path("shader/compiled/quad_default.vert.spv", rootPath, 0, defaultVert, sizeof(defaultVert));
	cren_get_path("shader/compiled/quad_default.frag.spv", rootPath, 0, defaultFrag, sizeof(defaultFrag));

	vkPipelineCreateInfo ci = { 0 };
	ci.renderpass = usedRenderpass; // this will either be default or viewport renderpass
	ci.vertexShader = crenvk_shader_create(device, "quad_default.vert", defaultVert, SHADER_TYPE_VERTEX);
	ci.fragmentShader = crenvk_shader_create(device, "quad_default.frag", defaultFrag, SHADER_TYPE_FRAGMENT);
	ci.passingVertexData = 0;
	ci.alphaBlending = 1;

	// push constant
	ci.pushConstantsCount = 1;
	ci.pushConstants[0].offset = 0;
	ci.pushConstants[0].size = sizeof(vkPushConstant);
	ci.pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	// bindings
	ci.bindingsCount = 3;
	// camera data
	ci.bindings[0].binding = 0;
	ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[0].descriptorCount = 1;
	ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[0].pImmutableSamplers = NULL;
	// quad data
	ci.bindings[1].binding = 1;
	ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[1].descriptorCount = 1;
	ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[1].pImmutableSamplers = NULL;
	// colormap
	ci.bindings[2].binding = 2;
	ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ci.bindings[2].descriptorCount = 1;
	ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[2].pImmutableSamplers = NULL;

	defaultPipeline = crenvk_pipeline_create(device, &ci);
	defaultPipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;
	crenvk_pipeline_build(device, defaultPipeline);
	crenhashtable_insert(pipelines, CREN_PIPELINE_QUAD_DEFAULT_NAME, defaultPipeline);

	// picking pipeline
	vkPipeline* pickingPipeline = crenhashtable_lookup(pipelines, CREN_PIPELINE_QUAD_PICKING_NAME);
	if (pickingPipeline != NULL) crenvk_pipeline_destroy(device, pickingPipeline);

	char pickingVert[CREN_PATH_MAX_SIZE], pickingFrag[CREN_PATH_MAX_SIZE];
	cren_get_path("shader/compiled/quad_picking.vert.spv", rootPath, 0, pickingVert, sizeof(pickingVert));
	cren_get_path("shader/compiled/quad_picking.frag.spv", rootPath, 0, pickingFrag, sizeof(pickingFrag));

	ci = (vkPipelineCreateInfo){ 0 };
	ci.renderpass = pickingRenderpass;
	ci.vertexShader = crenvk_shader_create(device, "quad_picking.vert", pickingVert, SHADER_TYPE_VERTEX);
	ci.fragmentShader = crenvk_shader_create(device, "quad_picking.frag", pickingFrag, SHADER_TYPE_FRAGMENT);
	ci.passingVertexData = 0;
	ci.alphaBlending = 0;

	// push constant
	ci.pushConstantsCount = 1;
	ci.pushConstants[0].offset = 0;
	ci.pushConstants[0].size = sizeof(vkPushConstant);
	ci.pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	// bindings
	ci.bindingsCount = 3;
	// camera data
	ci.bindings[0].binding = 0;
	ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[0].descriptorCount = 1;
	ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[0].pImmutableSamplers = NULL;
	// quad data
	ci.bindings[1].binding = 1;
	ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ci.bindings[1].descriptorCount = 1;
	ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[1].pImmutableSamplers = NULL;
	// colormap
	ci.bindings[2].binding = 2;
	ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ci.bindings[2].descriptorCount = 1;
	ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ci.bindings[2].pImmutableSamplers = NULL;

	pickingPipeline = crenvk_pipeline_create(device, &ci);
	pickingPipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;
	crenvk_pipeline_build(device, pickingPipeline);
	crenhashtable_insert(pipelines, CREN_PIPELINE_QUAD_PICKING_NAME, pickingPipeline);
}

vkPipeline* crenvk_pipeline_create(VkDevice device, vkPipelineCreateInfo *ci) {
    vkPipeline* pipeline = (vkPipeline*)crenmemory_allocate(sizeof(vkPipeline), 1);
    if(!pipeline) return NULL;

	pipeline->passingVertexData = ci->passingVertexData;
	pipeline->cache = ci->pipelineCache;
	pipeline->shaderStages[0] = ci->vertexShader.shaderStageCI;
	pipeline->shaderStages[1] = ci->fragmentShader.shaderStageCI;
	pipeline->renderpass = ci->renderpass;

	// descriptor set
	VkDescriptorSetLayoutCreateInfo descSetLayoutCI = { 0 };
	descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutCI.pNext = NULL;
	descSetLayoutCI.flags = 0;
	descSetLayoutCI.bindingCount = ci->bindingsCount;
	descSetLayoutCI.pBindings = ci->bindings;
    if(vkCreateDescriptorSetLayout(device, &descSetLayoutCI, NULL, &pipeline->descriptorSetLayout) != VK_SUCCESS) {
        crenmemory_deallocate(pipeline);
        return 0;
    }

	// pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCI = { 0 };
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.pNext = NULL;
	pipelineLayoutCI.flags = 0;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &pipeline->descriptorSetLayout;
	pipelineLayoutCI.pushConstantRangeCount = ci->pushConstantsCount;
	pipelineLayoutCI.pPushConstantRanges = ci->pushConstants;
    if(vkCreatePipelineLayout(device, &pipelineLayoutCI, NULL, &pipeline->layout) != VK_SUCCESS) {
        vkDestroyDescriptorSetLayout(device, pipeline->descriptorSetLayout, NULL);
        crenmemory_deallocate(pipeline);
        return 0;
    }

	// vertex input state
	pipeline->vertexInputState = internal_crenvk_pipeline_populate_visci(pipeline, ci->vertexComponents, ci->vertexComponentsCount);
	// input vertex assembly state
	pipeline->inputVertexAssemblyState = (VkPipelineInputAssemblyStateCreateInfo){ 0 };
	pipeline->inputVertexAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipeline->inputVertexAssemblyState.pNext = NULL;
	pipeline->inputVertexAssemblyState.flags = 0;
	pipeline->inputVertexAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline->inputVertexAssemblyState.primitiveRestartEnable = VK_FALSE;
	// viewport state
	pipeline->viewportState = (VkPipelineViewportStateCreateInfo){ 0 };
	pipeline->viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipeline->viewportState.pNext = NULL;
	pipeline->viewportState.flags = 0;
	pipeline->viewportState.viewportCount = 1;
	pipeline->viewportState.pViewports = NULL; // using dynamic viewport
	pipeline->viewportState.scissorCount = 1;
	pipeline->viewportState.pScissors = NULL; // using dynamic scissor
	// rasterization state
	pipeline->rasterizationState = (VkPipelineRasterizationStateCreateInfo){ 0 };
	pipeline->rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipeline->rasterizationState.pNext = NULL;
	pipeline->rasterizationState.flags = 0;
	pipeline->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	pipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;
	pipeline->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline->rasterizationState.depthClampEnable = VK_FALSE;
	pipeline->rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	pipeline->rasterizationState.lineWidth = 1.0f;
	// multisampling state
	pipeline->multisampleState = (VkPipelineMultisampleStateCreateInfo){ 0 };
	pipeline->multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipeline->multisampleState.pNext = NULL;
	pipeline->multisampleState.flags = 0;
	pipeline->multisampleState.rasterizationSamples = ci->renderpass->msaa;
	pipeline->multisampleState.sampleShadingEnable = VK_FALSE;
	// depth stencil state
	pipeline->depthStencilState = (VkPipelineDepthStencilStateCreateInfo){ 0 };
	pipeline->depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipeline->depthStencilState.pNext = NULL;
	pipeline->depthStencilState.flags = 0;
	pipeline->depthStencilState.depthTestEnable = VK_TRUE;
	pipeline->depthStencilState.depthWriteEnable = VK_TRUE;
	pipeline->depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipeline->depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	// color blend attachment
	pipeline->colorBlendAttachmentState = (VkPipelineColorBlendAttachmentState){ 0 };
	pipeline->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipeline->colorBlendAttachmentState.blendEnable = ci->alphaBlending == 1 ? VK_TRUE : VK_FALSE;
	pipeline->colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipeline->colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipeline->colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipeline->colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipeline->colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipeline->colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	// color blend state
	pipeline->colorBlendState = (VkPipelineColorBlendStateCreateInfo){ 0 };
	pipeline->colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipeline->colorBlendState.pNext = NULL;
	pipeline->colorBlendState.flags = 0;
	pipeline->colorBlendState.attachmentCount = 1;
	pipeline->colorBlendState.pAttachments = &pipeline->colorBlendAttachmentState;
	pipeline->colorBlendState.logicOpEnable = VK_FALSE;
	pipeline->colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	pipeline->colorBlendState.blendConstants[0] = 0.0f;
	pipeline->colorBlendState.blendConstants[1] = 0.0f;
	pipeline->colorBlendState.blendConstants[2] = 0.0f;
	pipeline->colorBlendState.blendConstants[3] = 0.0f;
	
	return pipeline;
}

void crenvk_pipeline_destroy(VkDevice device, vkPipeline* pipeline) {
	vkDeviceWaitIdle(device);

	vkDestroyPipeline(device, pipeline->pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline->layout, NULL);
	vkDestroyDescriptorSetLayout(device, pipeline->descriptorSetLayout, NULL);

	if (pipeline->pBindingsDescription != NULL) crenmemory_deallocate(pipeline->pBindingsDescription);
	if (pipeline->pAttributesDescription != NULL) crenmemory_deallocate(pipeline->pAttributesDescription);

	// not ideal since shader module was first introduced on shader struct, but it's the same module after-all
	vkDestroyShaderModule(device, pipeline->shaderStages[0].module, NULL);
	vkDestroyShaderModule(device, pipeline->shaderStages[1].module, NULL);

	crenmemory_deallocate(pipeline);
}

void crenvk_pipeline_build(VkDevice device, vkPipeline *pipeline) {
	// dynamic state is here because dynamic states must be constant
	VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
	const VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo ci = { 0 };
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	ci.pNext = NULL;
	ci.flags = 0;
	ci.stageCount = CREN_PIPELINE_SHADER_STAGES_COUNT;
	ci.pStages = pipeline->shaderStages;
	ci.pVertexInputState = &pipeline->vertexInputState;
	ci.pInputAssemblyState = &pipeline->inputVertexAssemblyState;
	ci.pViewportState = &pipeline->viewportState;
	ci.pRasterizationState = &pipeline->rasterizationState;
	ci.pMultisampleState = &pipeline->multisampleState;
	ci.pDepthStencilState = &pipeline->depthStencilState;
	ci.pColorBlendState = &pipeline->colorBlendState;
	ci.pDynamicState = &dynamicState;
	ci.layout = pipeline->layout;
	ci.renderPass = pipeline->renderpass->renderPass;
	ci.subpass = 0;
	CREN_ASSERT(vkCreateGraphicsPipelines(device, pipeline->cache, 1, &ci, NULL, &pipeline->pipeline) == VK_SUCCESS, "Failed to create vulkan graphics pipeline");
}

void crenvk_renderpass_destroy(VkDevice device, vkRenderpass* renderpass) {
    if(!device || !renderpass) return;

    vkDeviceWaitIdle(device);

    if (renderpass->descriptorPool) vkDestroyDescriptorPool(device, renderpass->descriptorPool, NULL);
    if (renderpass->renderPass) vkDestroyRenderPass(device, renderpass->renderPass, NULL);
    if (renderpass->commandBuffers) vkFreeCommandBuffers(device, renderpass->commandPool, renderpass->commandBufferCount, renderpass->commandBuffers);
    if (renderpass->commandPool) vkDestroyCommandPool(device, renderpass->commandPool, NULL);

    for (unsigned int i = 0; i < renderpass->framebufferCount; i++) {
        vkDestroyFramebuffer(device, renderpass->framebuffers[i], NULL);
    }

    if(renderpass->framebuffers) crenmemory_deallocate(renderpass->framebuffers);
    if(renderpass->commandBuffers) crenmemory_deallocate(renderpass->commandBuffers);

    crenmemory_deallocate(renderpass);
}

vkShader crenvk_shader_create(VkDevice device, const char *name, const char *path, vkShaderType type) {
    vkShader shader = { 0 };
    shader.name = name;
    shader.path = path;
    shader.type = type;
    shader.shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader.shaderStageCI.pNext = NULL;
    shader.shaderStageCI.flags = 0;
    shader.shaderStageCI.pName = "main";
    shader.shaderStageCI.pSpecializationInfo = NULL;
    switch (type) {
        case SHADER_TYPE_VERTEX: { shader.shaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT; break; }
        case SHADER_TYPE_FRAGMENT: { shader.shaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break; }
        case SHADER_TYPE_COMPUTE: { shader.shaderStageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT; break; }
        case SHADER_TYPE_GEOMETRY: { shader.shaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break; }
        case SHADER_TYPE_TESS_CTRL: { shader.shaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break; }
        case SHADER_TYPE_TESS_EVAL: { shader.shaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break; }
        default: { break; }
    }

    size_t spirvSize = 0;
    unsigned int* spirvCode = internal_crenvk_shader_loadspirv(path, &spirvSize);
    VkShaderModuleCreateInfo moduleCI = { 0 };
    moduleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCI.pNext = NULL;
    moduleCI.flags = 0;
    moduleCI.codeSize = spirvSize;
    moduleCI.pCode = spirvCode;
    CREN_ASSERT(vkCreateShaderModule(device, &moduleCI, NULL, &shader.shaderStageCI.module) == VK_SUCCESS, "Failed to create shader module");
    
    crenmemory_deallocate(spirvCode);
    return shader;
}

void crenvk_shader_destroy(VkDevice device, vkShader shader)
{
    if (!device) return;
    if(shader.shaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, shader.shaderModule, NULL);
}

int crenvk_vertex_equals(vkVertex *v0, vkVertex *v1) {
    return float3_equal(&v0->position, &v1->position) &&
           float3_equal(&v0->normal, &v1->normal) &&
           float2_equal(&v0->uv_0, &v1->uv_0) &&
           float4_equal(&v0->color_0, &v1->color_0) &&
           float4_equal(&v0->joints_0, &v1->joints_0) &&
           float4_equal(&v0->weights_0, &v1->weights_0) == 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DefaultRenderphase-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief creates the cren vulkan renderphase, all resouces need to render all possible phases of cren
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param format the format of the window surface
/// @param msaa anti-alignsed sample count
/// @param finalPhase checks if the default phase is the final phase
/// @return the default object renderphase
static vkDefaultRenderphase internal_crenvk_renderphase_default_create(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat format, VkSampleCountFlagBits msaa, int finalPhase) {
    vkDefaultRenderphase renderPhase = { 0 };
    renderPhase.renderpass = (vkRenderpass*)crenmemory_allocate(sizeof(vkRenderpass), 1);
    renderPhase.renderpass->name = "Default";
    renderPhase.renderpass->surfaceFormat = format;
    renderPhase.renderpass->msaa = msaa;

    VkAttachmentDescription attachments[3] = { 0 };

    // color
    attachments[0].format = format;
    attachments[0].samples = renderPhase.renderpass->msaa;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // depth
    attachments[1].format = crenvk_find_depth_format(physicalDevice);
    attachments[1].samples = renderPhase.renderpass->msaa;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // resolve
    attachments[2].format = format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = finalPhase == 1 ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // finalLayout should be present src if it's the final renderpass used, normally an UI renderpass would have such layout but we can't be sure if that'll be the case

    // attachments references
    VkAttachmentReference references[3] = { 0 };

    references[0].attachment = 0;
    references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    references[1].attachment = 1;
    references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    references[2].attachment = 2;
    references[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // subpass
    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &references[0];
    subpass.pDepthStencilAttachment = &references[1];
    subpass.pResolveAttachments = &references[2];

    // subpass dependencies for layout transitions
    VkSubpassDependency dependencies[2] = { 0 };

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI = { 0 };
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 3u;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpass;
    renderPassCI.dependencyCount = 2u;
    renderPassCI.pDependencies = dependencies;
    CREN_ASSERT(vkCreateRenderPass(device, &renderPassCI, NULL, &renderPhase.renderpass->renderPass) == VK_SUCCESS, "Failed to create the Default renderphase renderpass");

    return renderPhase;
}

/// @brief destroys the default resources used by the default rendering phase
/// @param renderphase cren default renderphase
/// @param device vulkan device
/// @param destroyRenderpass hints if the renderpass should also be destroyed
/// @param destroyPipeline hints if pipeline should also be destroyed
static void internal_crenvk_renderphase_default_destroy(vkDefaultRenderphase* renderphase, VkDevice device, int destroyRenderpass, int destroyPipeline) {
    vkDeviceWaitIdle(device);

    if (destroyRenderpass ) crenvk_renderpass_destroy(device, renderphase->renderpass);
    if (destroyPipeline) crenvk_pipeline_destroy(device, renderphase->pipeline);

    vkDestroyImage(device, renderphase->colorImage, NULL);
    vkFreeMemory(device, renderphase->colorMemory, NULL);
    vkDestroyImageView(device, renderphase->colorView, NULL);

    vkDestroyImage(device, renderphase->depthImage, NULL);
    vkFreeMemory(device, renderphase->depthMemory, NULL);
    vkDestroyImageView(device, renderphase->depthView, NULL);
}

/// @brief creates the default renderphase command pool and related objects
/// @param phase default render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_default_commandpool_create(vkDefaultRenderphase* phase, vkDevice* device) {

    vkRenderpass* renderpass = phase->renderpass;
    vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(device->physicalDevice, device->surface);

    // command pool
    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(device->device, &cmdPoolInfo, NULL, &renderpass->commandPool) != VK_SUCCESS) return 0;

    // command buffers
    renderpass->commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
    renderpass->commandBuffers = crenmemory_allocate(sizeof(VkCommandBuffer) * renderpass->commandBufferCount, 1);
    if(!renderpass->commandBuffers) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        return 0;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.commandPool = renderpass->commandPool;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = renderpass->commandBufferCount;
    if(vkAllocateCommandBuffers(device->device, &cmdBufferAllocInfo, renderpass->commandBuffers) != VK_SUCCESS) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        crenmemory_deallocate(renderpass->commandBuffers);
        return 0;
    }

    return 1;
}

/// @brief creates the framebuffers used by the default render phase
/// @param phase cren default render phase
/// @param device cren vulkan device
/// @param swapchain cren swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_default_framebuffers_create(vkDefaultRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
    vkRenderpass* renderpass = phase->renderpass;
    VkFormat colorFormat = swapchain->swapchainFormat.format;
    VkFormat depthFormat = crenvk_find_depth_format(device->physicalDevice);

    // create color image
    crenvk_image_create
    (
        swapchain->swapchainExtent.width,
        swapchain->swapchainExtent.height,
        1,
        1,
        device->device,
        device->physicalDevice,
        &phase->colorImage,
        &phase->colorMemory,
        colorFormat,
        renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    phase->colorView = crenvk_image_view_create(device->device, phase->colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);

    // create depth image
    crenvk_image_create
    (
        swapchain->swapchainExtent.width,
        swapchain->swapchainExtent.height,
        1,
        1,
        device->device,
        device->physicalDevice,
        &phase->depthImage,
        &phase->depthMemory,
        depthFormat,
        renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    phase->depthView = crenvk_image_view_create(device->device, phase->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);

    // create framebuffers
    renderpass->framebufferCount = swapchain->swapchainImageCount;
    renderpass->framebuffers = (VkFramebuffer*)crenmemory_allocate(sizeof(VkFramebuffer) * renderpass->framebufferCount, 1);

    int failed = 0;
    for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) {
        const VkImageView attachments[3] = { phase->colorView, phase->depthView, swapchain->swapchainImageViews[i] };
        VkFramebufferCreateInfo fbci = { 0 };
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.renderPass = renderpass->renderPass;
        fbci.attachmentCount = 3U;
        fbci.pAttachments = &attachments[0];
        fbci.width = swapchain->swapchainExtent.width;
        fbci.height = swapchain->swapchainExtent.height;
        fbci.layers = 1;
        if(vkCreateFramebuffer(device->device, &fbci, NULL, &renderpass->framebuffers[i]) == VK_SUCCESS) {
            failed =  1;
        }
    }

    // framebuffer creation got an error, let's free any resource and return
    if(failed) {
        for (unsigned int i = 0; i < swapchain->swapchainImageCount; i++) {
            if(!renderpass->framebuffers[i]) {
                vkDestroyFramebuffer(device->device, renderpass->framebuffers[i], NULL);
            }
        }
    }

    return failed;
}

/// @brief creates the default render phase pipeline
/// @param phase cren default render phase 
/// @param device vulkan device
/// @param build tells to build the pipeline or not
/// @return the cren vkPipeline object used by the vkDefaultRenderphase
static vkPipeline* internal_crenvk_renderphase_default_pipeline_create(vkDefaultRenderphase* phase, VkDevice device,  int build, const char* rootPath) {
    char vert[CREN_PATH_MAX_SIZE], frag[CREN_PATH_MAX_SIZE];
    cren_get_path("shader/compiled/mesh_default.vert.spv", rootPath, 0, vert, sizeof(vert));
    cren_get_path("shader/compiled/mesh_default.frag.spv", rootPath, 0, frag, sizeof(frag));

    VkPushConstantRange pushConstant = { 0 };
    pushConstant.offset = 0;
    pushConstant.size = sizeof(vkPushConstant);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    vkPipelineCreateInfo ci = { 0 };
    ci.renderpass = phase->renderpass;
    ci.passingVertexData = 1;
    ci.pipelineCache = NULL;
    ci.vertexShader = crenvk_shader_create(device, "MeshDefault.vert", vert, SHADER_TYPE_VERTEX);
    ci.fragmentShader = crenvk_shader_create(device, "MeshDefault.frag", frag, SHADER_TYPE_FRAGMENT);
    ci.vertexComponentsCount = 3;
    ci.vertexComponents[0] = VK_VERTEX_COMPONENT_POSITION;
    ci.vertexComponents[1] = VK_VERTEX_COMPONENT_NORMAL;
    ci.vertexComponents[2] = VK_VERTEX_COMPONENT_UV_0;
    // push constant
    ci.pushConstantsCount = 1;
    ci.pushConstants[0] = pushConstant;
    // camera buffer
    ci.bindingsCount = 3;
    ci.bindings[0].binding = 0;
    ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ci.bindings[0].descriptorCount = 1;
    ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[0].pImmutableSamplers = NULL;
    // params
    ci.bindings[1].binding = 1;
    ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ci.bindings[1].descriptorCount = 1;
    ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[1].pImmutableSamplers = NULL;
    // albedo
    ci.bindings[2].binding = 2;
    ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ci.bindings[2].descriptorCount = 1;
    ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[2].pImmutableSamplers = NULL;

    // create and modify
    vkPipeline* pipeline = crenvk_pipeline_create(device, &ci);
    pipeline->rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

    // build
    if (build) crenvk_pipeline_build(device, pipeline);

    return pipeline;
}

/// @brief recreates the default renderphase, used
/// @param phase cren vulkan default render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @param width render phase width, usually same as window
/// @param height render phase height, usually same as window
/// @param vsync hints the vsync on/off
static void internal_crenvk_renderphase_default_recreate(vkDefaultRenderphase* phase, vkDevice* device, vkSwapchain* swapchain, unsigned int width, unsigned int height, int vsync) {
    vkDeviceWaitIdle(device->device);

    // must recreate some default phase objects
    vkDestroyImageView(device->device, phase->depthView, NULL);
    vkDestroyImage(device->device, phase->depthImage, NULL);
    vkFreeMemory(device->device, phase->depthMemory, NULL);

    vkDestroyImageView(device->device, phase->colorView, NULL);
    vkDestroyImage(device->device, phase->colorImage, NULL);
    vkFreeMemory(device->device, phase->colorMemory, NULL);

    for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) { vkDestroyFramebuffer(device->device, phase->renderpass->framebuffers[i], NULL); }
    crenmemory_deallocate(phase->renderpass->framebuffers);

    // recreate swapchain
    internal_crenvk_swapchain_destroy(swapchain, device->device);
    internal_crenvk_swapchain_create(swapchain, device->device, device->physicalDevice, device->surface, width, height, vsync);
    internal_crenvk_renderphase_default_framebuffers_create(phase, device, swapchain);
}

/// @brief performs the update of the current frame, effectly calling the rendering callback function who draws the objects
/// @param phase cren default render phase
/// @param context cren context
/// @param currentFrame current frame being processed, since multiple frames may be processing
/// @param swapchainImageIndex swapchain image index
/// @param usingViewport hints the usage of a custom viewport who will handle rendering in a further step
/// @param timestep interpolation state between frames, this will be passed on to the callback
/// @param callback render callback, a function defined by the used to handle the rendering
static void internal_crenvk_renderphase_default_update(vkDefaultRenderphase* phase, CRenContext* context, unsigned int currentFrame, unsigned int swapchainImageIndex, int usingViewport, double timestep, CRenCallback_Render callback) {
    
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    VkClearValue clearValues[2] = { 0 };
    const unsigned int clearValuesCount = 2;
    clearValues[0].color = (VkClearColorValue) {.float32[0] = 0.0f, .float32[1] = 0.0f, .float32[2] = 0.0f, .float32[3] =  1.0f };
    clearValues[1].depthStencil = (VkClearDepthStencilValue){ .depth = 1.0f, .stencil = 0 };

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin default renderphase command buffer");

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ .x = 0, .y = 0 };
    renderPassBeginInfo.renderArea.extent = renderer->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = clearValuesCount;
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set frame commandbuffer viewport
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->swapchain.swapchainExtent.width;
    viewport.height = (float)renderer->swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    // set frame commandbuffer scissor
    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){ .x = 0, .y =  0 };
    scissor.extent = renderer->swapchain.swapchainExtent;
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // not using viewport as the final target, therefore it's time to draw the objects
    if (!usingViewport) {
        if (callback != NULL) {
            callback(context, VK_RENDER_STAGE_DEFAULT, timestep);
        }
    }

    vkCmdEndRenderPass(cmdBuffer);

    // end command buffer
    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end default renderphase command buffer");
}

/// @brief creates the picking render phase
/// @param device vulkan device
/// @param physicalDevice vulkan physical device
/// @param format the desired format for each pixel on this phase
/// @param msaa anti-aliasing sample count
/// @return a cren picking render phase
static vkPickingRenderphase internal_crenvk_renderphase_picking_create(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat format, VkSampleCountFlagBits msaa) {
    vkPickingRenderphase phase = { 0 };
    phase.renderpass = (vkRenderpass*)crenmemory_allocate(sizeof(vkRenderpass), 1);

    phase.surfaceFormat = format;
    phase.renderpass->name = "Picking";
    phase.renderpass->msaa = msaa;
    phase.depthFormat = crenvk_find_depth_format(physicalDevice);
    
    // create render-pass
    VkAttachmentDescription attachments[2] = { 0 };

    attachments[0].format = phase.surfaceFormat;
    attachments[0].samples = phase.renderpass->msaa;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format = phase.depthFormat;
    attachments[1].samples = phase.renderpass->msaa;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0 };
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = { 0 };
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = { 0 };
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = NULL;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = NULL;
    subpassDescription.pResolveAttachments = NULL;

    VkSubpassDependency dependencies[2] = { 0 };
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCI = { 0 };
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 2U;
    renderPassCI.pAttachments = attachments;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDescription;
    renderPassCI.dependencyCount = 2U;
    renderPassCI.pDependencies = dependencies;
    CREN_ASSERT(vkCreateRenderPass(device, &renderPassCI, NULL, &phase.renderpass->renderPass) == VK_SUCCESS, "Failed to create picking renderphase renderpass");

    return phase;
}

/// @brief destroy all resources used by the ui picking render phase
/// @param phase cren picking render phase
/// @param device vulkan device
/// @param destroyRenderpass hints for the renderpass destruction
/// @param destroyPipeline hints for the pipeline destruction
static void internal_crenvk_renderphase_picking_destroy(vkPickingRenderphase* phase, VkDevice device, int destroyRenderpass, int destroyPipeline) {
    vkDeviceWaitIdle(device);

    if(destroyRenderpass) crenvk_renderpass_destroy(device, phase->renderpass);
    if (destroyPipeline) crenvk_pipeline_destroy(device, phase->pipeline);

    vkDestroyImageView(device, phase->depthView, NULL);
    vkDestroyImage(device, phase->depthImage, NULL);
    vkFreeMemory(device, phase->depthMemory, NULL);

    vkDestroyImageView(device, phase->colorView, NULL);
    vkDestroyImage(device, phase->colorImage, NULL);
    vkFreeMemory(device, phase->colorMemory, NULL);
}

/// @brief creates the picking render phase command pool/buffers
/// @param phase cren picking render phase
/// @param device cren vulkan device
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_picking_commandpool_create(vkPickingRenderphase* phase, vkDevice* device) {
    vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(device->physicalDevice, device->surface);
    VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(device->device, &cmdPoolInfo, NULL, &phase->renderpass->commandPool) != VK_SUCCESS) return 0;

    phase->renderpass->commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
    phase->renderpass->commandBuffers = (VkCommandBuffer*)crenmemory_allocate(sizeof(VkCommandBuffer) * phase->renderpass->commandBufferCount, 1);
    if(!phase->renderpass->commandBuffers) {
        vkDestroyCommandPool(device->device, phase->renderpass->commandPool, NULL);
        return 0;
    }

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandPool = phase->renderpass->commandPool;
    cmdBufferAllocInfo.commandBufferCount = phase->renderpass->commandBufferCount;
    if(vkAllocateCommandBuffers(device->device, &cmdBufferAllocInfo, phase->renderpass->commandBuffers) != VK_SUCCESS) {
        vkDestroyCommandPool(device->device, phase->renderpass->commandPool, NULL);
        crenmemory_deallocate(phase->renderpass->commandBuffers);
        return 0;
    }

    return 1;
}

/// @brief creates the framebuffer used by the picking render phase
/// @param phase cren picking render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_picking_framebuffers_create(vkPickingRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
    VkExtent2D imgSize = swapchain->swapchainExtent;

    // color image
    int res = crenvk_image_create
    (
        imgSize.width,
        imgSize.height,
        1,
        1,
        device->device,
        device->physicalDevice,
        &phase->colorImage,
        &phase->colorMemory,
        phase->surfaceFormat,
        phase->renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // for picking
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    phase->colorView = crenvk_image_view_create(device->device, phase->colorImage, phase->surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);
    
    if(!res) return 0; // could not created the color image resources, just return

    // depth buffer
    res &= crenvk_image_create
    (
        imgSize.width,
        imgSize.height,
        1,
        1,
        device->device,
        device->physicalDevice,
        &phase->depthImage,
        &phase->depthMemory,
        phase->depthFormat,
        phase->renderpass->msaa,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        0
    );
    phase->depthView = crenvk_image_view_create(device->device, phase->depthImage, phase->depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);

    if(!res) { // could not create depth image, but color image was created, must release it and return
        vkDestroyImage(device->device, phase->colorImage, NULL);
        vkFreeMemory(device->device, phase->colorMemory, NULL);
        return 0;
    }

    // command buffer
    VkCommandBuffer cmdBuffer = crenvk_commandbuffer_begin_singletime(device->device, phase->renderpass->commandPool);

    VkImageSubresourceRange subresourceRange = { 0 };
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    crenvk_image_memory_barrier_insert
    (
        cmdBuffer,
        phase->colorImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        subresourceRange
    );

    crenvk_commandbuffer_end_singletime(device->device, phase->renderpass->commandPool, cmdBuffer, device->graphicsQueue);

    // framebuffer
    phase->renderpass->framebufferCount = swapchain->swapchainImageCount;
    phase->renderpass->framebuffers = (VkFramebuffer*)crenmemory_allocate(sizeof(VkFramebuffer) * phase->renderpass->framebufferCount, 1);
    if(!phase->renderpass->framebuffers) { // could not create the framebuffers but the image resources were created, must free them and return
        vkDestroyImage(device->device, phase->colorImage, NULL);
        vkDestroyImage(device->device, phase->depthImage, NULL);
        vkFreeMemory(device->device, phase->colorMemory, NULL);
        vkFreeMemory(device->device, phase->depthMemory, NULL);
        return 0;
    }

    int success = 1;
    for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) {
        const VkImageView attachments[2] = { phase->colorView, phase->depthView };

        VkFramebufferCreateInfo framebufferCI = { 0 };
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = phase->renderpass->renderPass;
        framebufferCI.attachmentCount = 2U;
        framebufferCI.pAttachments = attachments;
        framebufferCI.width = swapchain->swapchainExtent.width;
        framebufferCI.height = swapchain->swapchainExtent.height;
        framebufferCI.layers = 1;
        if(vkCreateFramebuffer(device->device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]) != VK_SUCCESS) {
            success = 0;
        }
    }

    if(!success) { // and error ocurred when created a framebuffer
        for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) {
            if(phase->renderpass->framebuffers[i]) vkDestroyFramebuffer(device->device, phase->renderpass->framebuffers[i], NULL);
        }

        vkDestroyImage(device->device, phase->colorImage, NULL);
        vkDestroyImage(device->device, phase->depthImage, NULL);
        vkFreeMemory(device->device, phase->colorMemory, NULL);
        vkFreeMemory(device->device, phase->depthMemory, NULL);
        crenmemory_deallocate(phase->renderpass->framebuffers);
    }

    return success;
}

/// @brief creates the pipeline used by the picking render phase
/// @param phase cren vulkan picking render phase
/// @param device vulkan device
/// @param build hints the pipeline to be built
/// @param rootPath asset's path for shader look-up
/// @return the created pipeline or NULL if an error has ocurred
static vkPipeline* internal_crenvk_renderphase_picking_pipeline_create(vkPickingRenderphase* phase, VkDevice device, int build, const char* rootPath) {
    char vert[CREN_PATH_MAX_SIZE], frag[CREN_PATH_MAX_SIZE];
    cren_get_path("shader/compiled/mesh_picking.vert.spv", rootPath, 0, vert, sizeof(vert));
    cren_get_path("shader/compiled/mesh_picking.frag.spv", rootPath, 0, frag, sizeof(frag));

    VkPushConstantRange pushConstant = { 0 };
    pushConstant.offset = 0;
    pushConstant.size = sizeof(vkPushConstant);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    vkPipelineCreateInfo ci = { 0 };
    ci.renderpass = phase->renderpass;
    ci.passingVertexData = 1;
    ci.pipelineCache = NULL;
    ci.vertexShader = crenvk_shader_create(device, "MeshPicking.vert", vert, SHADER_TYPE_VERTEX);
    ci.fragmentShader = crenvk_shader_create(device, "MeshPicking.frag", frag, SHADER_TYPE_FRAGMENT);
    ci.vertexComponentsCount = 1;
    ci.vertexComponents[0] = VK_VERTEX_COMPONENT_POSITION;
    // push constant
    ci.pushConstantsCount = 1;
    ci.pushConstants[0] = pushConstant;
    // camera buffer
    ci.bindingsCount = 3;
    ci.bindings[0].binding = 0;
    ci.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ci.bindings[0].descriptorCount = 1;
    ci.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[0].pImmutableSamplers = NULL;
    // params
    ci.bindings[1].binding = 1;
    ci.bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ci.bindings[1].descriptorCount = 1;
    ci.bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[1].pImmutableSamplers = NULL;
    // albedo
    ci.bindings[2].binding = 2;
    ci.bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ci.bindings[2].descriptorCount = 1;
    ci.bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    ci.bindings[2].pImmutableSamplers = NULL;

    // create and modify
    vkPipeline* pipeline = crenvk_pipeline_create(device, &ci);
    pipeline->rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

    // build
    if (build) crenvk_pipeline_build(device, pipeline);

    return pipeline;
}

/// @brief recreates the picking render phase objects
/// @param phase cren picking render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
static void internal_crenvk_renderphase_picking_recreate(vkPickingRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
    vkDestroyImage(device->device, phase->depthImage, NULL);
    vkFreeMemory(device->device, phase->depthMemory, NULL);
    vkDestroyImageView(device->device, phase->depthView, NULL);

    vkDestroyImage(device->device, phase->colorImage, NULL);
    vkFreeMemory(device->device, phase->colorMemory, NULL);
    vkDestroyImageView(device->device, phase->colorView, NULL);

    for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) {
        vkDestroyFramebuffer(device->device, phase->renderpass->framebuffers[i], NULL);
    }
    crenmemory_deallocate(phase->renderpass->framebuffers);

    internal_crenvk_renderphase_picking_framebuffers_create(phase, device, swapchain);
}

/// @brief performs the update of the current frame, effectly calling the rendering callback function who draws the objects
/// @param phase cren picking render phase
/// @param context cren context
/// @param currentFrame the current frame being processed
/// @param swapchainImageIndex the swapchain image index, since it may use double-buffering/triple-buffering
/// @param usingViewport hints the usage of a custom viewport
/// @param timestep interpolation between frames, this is passed to the user's callback
/// @param callback user-defined callback
static void internal_crenvk_renderphase_picking_update(vkPickingRenderphase* phase, CRenContext* context, unsigned int currentFrame, unsigned int swapchainImageIndex, int usingViewport, double timestep, CRenCallback_Render callback) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color = (VkClearColorValue){ 0.0f,  0.0f,  0.0f, 1.0f };
    clearValues[1].depthStencil = (VkClearDepthStencilValue){ .depth = 1.0f, .stencil = 0 };

    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
    VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
    VkRenderPass renderPass = phase->renderpass->renderPass;

    vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

    VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = NULL;
    cmdBeginInfo.flags = 0;
    CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to beging picking renderphase command buffer");

    VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = frameBuffer;
    renderPassBeginInfo.renderArea.offset = (VkOffset2D){ .x = 0, .y = 0 };
    renderPassBeginInfo.renderArea.extent = renderer->swapchain.swapchainExtent;
    renderPassBeginInfo.clearValueCount = (uint32_t)CREN_ARRAYSIZE(clearValues);
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set frame commandbuffer viewport
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->swapchain.swapchainExtent.width;
    viewport.height = (float)renderer->swapchain.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    // if a viewport is in use, we just want to render the pixel undeneath the mousepos. This skips all other fragment computation
    if (usingViewport) {
        
        // boundaries is not a float2, but will be for now
        float2 boundaries = { 0.0f, 0.0f }; //IContext::GetRef()->GetViewportBoundariesRef();
        float2 mousePos = { 0.0f, 0.0f };   //glm::clamp(Platform::MainWindow::GetRef().GetViewportCursorPos(boundaries.position, boundaries.size), glm::vec2(0.0f, 0.0f), glm::vec2(extent.width, extent.height));

        VkRect2D scissor = { 0 };
        scissor.offset = (VkOffset2D){ .x = (unsigned int)mousePos.x, .y = (unsigned int)mousePos.y };
        scissor.extent = (VkExtent2D){ 1, 1 };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    else {
        VkRect2D scissor = { 0 };
        scissor.offset = (VkOffset2D){ .x = 0, .y = 0 };
        scissor.extent = renderer->swapchain.swapchainExtent;
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }

    if (callback != NULL) {
        callback(context, VK_RENDER_STAGE_PICKING, timestep);
    }

    // end render pass
    vkCmdEndRenderPass(cmdBuffer);

    // end command buffer
    CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to finish picking renderphase command buffer");
}

/// @brief creates the ui renderphase, used externally by the user on a UI setup
/// @param device vulkan device
/// @param format vulkan surface format
/// @param msaa anti-aliasing sample count
/// @param finalPhase hints if the ui is the last phase, wich it is if active
/// @return tje vkUIRenderphase object
static vkUIRenderphase internal_crenvk_renderphase_ui_create(VkDevice device, VkFormat format, VkSampleCountFlagBits msaa, int finalPhase) {
    vkUIRenderphase phase = { 0 };
    phase.renderpass = (vkRenderpass*)crenmemory_allocate(sizeof(vkRenderpass), 1);

    phase.renderpass->name = "UI";
    phase.renderpass->surfaceFormat = format;
    phase.renderpass->msaa = msaa;

	VkAttachmentDescription attachment = { 0 };
	attachment.format = format;
	attachment.samples = phase.renderpass->msaa;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = finalPhase == 1 ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachment = { 0 };
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachment;

	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = { 0 };
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;
	CREN_ASSERT(vkCreateRenderPass(device, &info, NULL, &phase.renderpass->renderPass) == VK_SUCCESS, "Failed to create ui renderphase renderpass");

	// ui descriptor set layout, follows ImGui specs
	VkDescriptorSetLayoutBinding binding[1] = { 0 };
	binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding[0].descriptorCount = 1;
	binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo descInfo = { 0 };
	descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descInfo.bindingCount = 1;
	descInfo.pBindings = binding;
	CREN_ASSERT(vkCreateDescriptorSetLayout(device, &descInfo, NULL, &phase.descSetLayout) == VK_SUCCESS, "Failed to create ui descriptor set layout");
	
	// ui descriptor pool, follows ImGui specs
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	
	VkDescriptorPoolCreateInfo poolCI = { 0 };
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.pNext = NULL;
	poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolCI.maxSets = 1000 * CREN_ARRAYSIZE(poolSizes);
	poolCI.poolSizeCount = (uint32_t)CREN_ARRAYSIZE(poolSizes);
	poolCI.pPoolSizes = poolSizes;
	CREN_ASSERT(vkCreateDescriptorPool(device, &poolCI, NULL, &phase.descPool) == VK_SUCCESS, "Failed to create descriptor pool for the User Interface");

    return phase;
}

/// @brief destroys and release all resources used by the ui render phase
/// @param phase cren ui render phase
/// @param device vulkan device
/// @param destroyRenderpass hints the function to destroy it's renderpass as well
static void internal_crenvk_renderphase_ui_destroy(vkUIRenderphase* phase, VkDevice device, int destroyRenderpass) {
	vkDeviceWaitIdle(device);
	
	if (destroyRenderpass) crenvk_renderpass_destroy(device, phase->renderpass);

	vkDestroyDescriptorSetLayout(device, phase->descSetLayout, NULL);
	vkDestroyDescriptorPool(device, phase->descPool, NULL);
}

/// @brief creates command pool/buffers used by the ui renderphase 
/// @param phase cren ui render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_ui_commandpool_create(vkUIRenderphase* phase,  vkDevice* device) {
	vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(device->physicalDevice, device->surface);
	vkRenderpass* renderpass = phase->renderpass;

	VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(device->device, &cmdPoolInfo, NULL, &renderpass->commandPool) != VK_SUCCESS) {
        return 0;
    }

	// command buffers
	phase->renderpass->commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
	phase->renderpass->commandBuffers = (VkCommandBuffer*)crenmemory_allocate(sizeof(VkCommandBuffer) * phase->renderpass->commandBufferCount, 1);

    if(!phase->renderpass->commandBuffers) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        return 0;
    }
	
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = phase->renderpass->commandPool;
	cmdBufferAllocInfo.commandBufferCount = phase->renderpass->commandBufferCount;
	if(vkAllocateCommandBuffers(device->device, &cmdBufferAllocInfo, phase->renderpass->commandBuffers) != VK_SUCCESS) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        crenmemory_deallocate(phase->renderpass->commandBuffers);
        return 0;
    }

    return 1;
}


/// @brief creates the framebuffers used by the ui render phase
/// @param phase cren ui render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_ui_framebuffers_create(vkUIRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
	phase->renderpass->framebufferCount = swapchain->swapchainImageCount;
	phase->renderpass->framebuffers = (VkFramebuffer*)crenmemory_allocate(sizeof(VkFramebuffer) * phase->renderpass->framebufferCount, 1);
    if(!phase->renderpass->framebuffers) return 0;

	for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) {
		const VkImageView attachments[] = { swapchain->swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferCI = { 0 };
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = phase->renderpass->renderPass;
		framebufferCI.attachmentCount = (uint32_t)CREN_ARRAYSIZE(attachments);
		framebufferCI.pAttachments = attachments;
		framebufferCI.width = swapchain->swapchainExtent.width;
		framebufferCI.height = swapchain->swapchainExtent.height;
		framebufferCI.layers = 1;
		if(vkCreateFramebuffer(device->device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]) != VK_SUCCESS) {
            crenmemory_deallocate(phase->renderpass->framebuffers);
            return 0;
        } 
	}

    return 1;
}

/// @brief recreates all resources for the ui render phase
/// @param phase cren vulkan ui render phase
/// @param device vulkan device
/// @param swapchain cren vulkan swapchain
static void internal_crenvk_renderphase_ui_recreate(vkUIRenderphase* phase,vkDevice* device, vkSwapchain* swapchain) {
	vkDeviceWaitIdle(device->device);

	for (uint32_t i = 0; i < phase->renderpass->framebufferCount; i++) {
		vkDestroyFramebuffer(device->device, phase->renderpass->framebuffers[i], NULL);
	}

	crenmemory_deallocate(phase->renderpass->framebuffers);
	internal_crenvk_renderphase_ui_framebuffers_create(phase, device, swapchain);
}

/// @brief performs the current frame for the ui-related drawing. This calls the user draw-ui data, who is responsible for the drawing part
/// @param phase cren ui render phase
/// @param context cren context
/// @param currentFrame current frame in process
/// @param swapchainImageIndex swapchain image index
/// @param callback user defined callback to call
static void internal_crenvk_renderphase_ui_update(vkUIRenderphase* phase, CRenContext* context, unsigned int currentFrame, unsigned int swapchainImageIndex, CRenCallback_DrawUIRawData callback) {
	CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
	VkFramebuffer frameBuffer = phase->renderpass->framebuffers[swapchainImageIndex];
	VkRenderPass renderPass = phase->renderpass->renderPass;

	vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

	VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = NULL;
	cmdBeginInfo.flags = 0;
	CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin ui renderphase command buffer");

	VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = frameBuffer;
	renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
	renderPassBeginInfo.renderArea.extent = renderer->swapchain.swapchainExtent;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;
	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// render raw data
	if (callback != NULL) {
		callback(context, cmdBuffer);
	}

	vkCmdEndRenderPass(cmdBuffer);

	CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end ui renderphase command buffer");
}


/// @brief creates the viewport renderphase resources
/// @param device vulkan device
/// @param physicalDevice vulkan physica device
/// @param surfaceFormat render phase image format
/// @param msaa anti-aliasing sample count
/// @return the vkViewportRenderphase or asserts, since it cannot fail
static vkViewportRenderphase internal_crenvk_renderphase_viewport_create(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat surfaceFormat, VkSampleCountFlagBits msaa) {
	vkViewportRenderphase phase = { 0 };

	phase.renderpass = (vkRenderpass*)crenmemory_allocate(sizeof(vkRenderpass), 1);
    CREN_ASSERT(phase.renderpass != NULL, "Could not allocate memory for renderpass");

	phase.renderpass->name = "UI";
	phase.renderpass->surfaceFormat = surfaceFormat;
	phase.renderpass->msaa = msaa;

	const unsigned int attachmentsSize = 2U;
	VkAttachmentDescription attachments[2] = { 0 };

	// color attachment
	attachments[0].format = phase.renderpass->surfaceFormat;
	attachments[0].samples = phase.renderpass->msaa;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// depth attachment
	attachments[1].format = crenvk_find_depth_format(physicalDevice);
	attachments[1].samples = phase.renderpass->msaa;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = { 0 };
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = { 0 };
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = { 0 };
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;
	subpassDescription.pResolveAttachments = NULL;

	// subpass dependencies for layout transitions
	const unsigned int dependenciesSize = 2U;
	VkSubpassDependency dependencies[2] = { 0 };

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCI = { 0 };
	renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCI.attachmentCount = attachmentsSize;
	renderPassCI.pAttachments = attachments;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpassDescription;
	renderPassCI.dependencyCount = dependenciesSize;
	renderPassCI.pDependencies = dependencies;
	CREN_ASSERT(vkCreateRenderPass(device, &renderPassCI, NULL, &phase.renderpass->renderPass) == VK_SUCCESS, "Failed to create vulkan renderpass for the viewport render phase");
	return phase;
}

/// @brief 
/// @param phase 
/// @param device 
/// @param destroyRenderpass 
static void internal_crenvk_renderphase_viewport_destroy(vkViewportRenderphase* phase, VkDevice device, int destroyRenderpass) {

	vkDeviceWaitIdle(device);
	if (destroyRenderpass) crenvk_renderpass_destroy(device, phase->renderpass);

	vkDestroySampler(device, phase->sampler, NULL);
	vkDestroyDescriptorPool(device, phase->descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(device, phase->descriptorSetLayout, NULL);

	vkDestroyImageView(device, phase->depthView, NULL);
	vkDestroyImage(device, phase->depthImage, NULL);
	vkFreeMemory(device, phase->depthMemory, NULL);

	vkDestroyImageView(device, phase->colorView, NULL);
	vkDestroyImage(device, phase->colorImage, NULL);
	vkFreeMemory(device, phase->colorMemory, NULL);
}

/// @brief creates the vulkan command pool/buffers used by the viewport render phase
/// @param phase viewport render phase
/// @param device cren vulkan device
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_viewport_commandpool_create(vkViewportRenderphase* phase, vkDevice* device) {
	vkRenderpass* renderpass = phase->renderpass;
	vkQueueFamilyIndices indices = internal_crenvk_find_queue_families(device->physicalDevice, device->surface);

	// command pool
	VkCommandPoolCreateInfo cmdPoolInfo = { 0 };
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = indices.graphicFamily;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if(vkCreateCommandPool(device->device, &cmdPoolInfo, NULL, &renderpass->commandPool) != VK_SUCCESS) return 0;

	// command buffers
	renderpass->commandBufferCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
	renderpass->commandBuffers = crenmemory_allocate(sizeof(VkCommandBuffer) * renderpass->commandBufferCount, 1);
    if(!renderpass->commandBuffers) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        return 0;
    }

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = renderpass->commandPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = renderpass->commandBufferCount;
	if(vkAllocateCommandBuffers(device->device, &cmdBufferAllocInfo, renderpass->commandBuffers) != VK_SUCCESS) {
        vkDestroyCommandPool(device->device, renderpass->commandPool, NULL);
        crenmemory_deallocate(renderpass->commandBuffers);
        return 0;
    }

    return 1;
}

/// @brief creates the framebuffers used by the viewport render phase
/// @param phase viewport render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
/// @return 1 on success, 0 on failure
static int internal_crenvk_renderphase_viewport_framebuffers_create(vkViewportRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
	phase->vpSize.x = (float)swapchain->swapchainExtent.width;
	phase->vpSize.y = (float)swapchain->swapchainExtent.height;

	unsigned int imageCount = swapchain->swapchainImageCount;
	VkFormat depthFormat = crenvk_find_depth_format(device->physicalDevice);

	// descriptor pool
	VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 } };
	VkDescriptorPoolCreateInfo poolCI = { 0 };
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.pNext = NULL;
	poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolCI.maxSets = (uint32_t)(2 * CREN_ARRAYSIZE(poolSizes));
	poolCI.poolSizeCount = (uint32_t)CREN_ARRAYSIZE(poolSizes);
	poolCI.pPoolSizes = poolSizes;
	CREN_ASSERT(vkCreateDescriptorPool(device->device, &poolCI, NULL, &phase->descriptorPool) == VK_SUCCESS, "Failed to create vulkan descriptor pool for the viewport render phase");

	// descriptor set layout
	VkDescriptorSetLayoutBinding binding[1] = { 0 };
	binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding[0].descriptorCount = 1;
	binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo info = { 0 };
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = binding;
	CREN_ASSERT(vkCreateDescriptorSetLayout(device->device, &info, NULL, &phase->descriptorSetLayout) == VK_SUCCESS, "Failed to create vulkan descriptor set layout for the viewport render phase");

	// sampler
	phase->sampler = crenvk_image_sampler_create
	(
		device->device,
		device->physicalDevice,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		1.0f
	);

	// color image
	crenvk_image_create
	(
		swapchain->swapchainExtent.width,
		swapchain->swapchainExtent.height,
		1,
		1,
		device->device,
		device->physicalDevice,
		&phase->colorImage,
		&phase->colorMemory,
		phase->renderpass->surfaceFormat,
		phase->renderpass->msaa,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // last one is for picking
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	);

	phase->colorView = crenvk_image_view_create(device->device, phase->colorImage, phase->renderpass->surfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);

	// depth buffer
	crenvk_image_create
	(
		swapchain->swapchainExtent.width,
		swapchain->swapchainExtent.height,
		1,
		1,
		device->device,
		device->physicalDevice,
		&phase->depthImage,
		&phase->depthMemory,
		depthFormat,
		phase->renderpass->msaa,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	);

	phase->depthView = crenvk_image_view_create(device->device, phase->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, VK_IMAGE_VIEW_TYPE_2D);

	// command buffer
	VkCommandBuffer command = crenvk_commandbuffer_begin_singletime(device->device, phase->renderpass->commandPool);

	VkImageSubresourceRange subresourceRange = { 0 };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	crenvk_image_memory_barrier_insert
	(
		command,
		phase->colorImage,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass, but that one could also use undefined
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		subresourceRange
	);

	crenvk_commandbuffer_end_singletime(device->device, phase->renderpass->commandPool, command, device->graphicsQueue);

	phase->descriptorSet = crenvk_image_descriptor_set_create(device->device, phase->descriptorPool, phase->descriptorSetLayout, phase->sampler, phase->colorView);

	// framebuffer
	phase->renderpass->framebufferCount = swapchain->swapchainImageCount;
	phase->renderpass->framebuffers = (VkFramebuffer*)crenmemory_allocate(sizeof(VkFramebuffer) * phase->renderpass->framebufferCount, 1);

	for (size_t i = 0; i < imageCount; i++) {
		const VkImageView attachments[2] = { phase->colorView, phase->depthView };

		VkFramebufferCreateInfo framebufferCI = { 0 };
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = phase->renderpass->renderPass;
		framebufferCI.attachmentCount = (unsigned int)CREN_ARRAYSIZE(attachments);
		framebufferCI.pAttachments = attachments;
		framebufferCI.width = swapchain->swapchainExtent.width;
		framebufferCI.height = swapchain->swapchainExtent.height;
		framebufferCI.layers = 1;
		CREN_ASSERT(vkCreateFramebuffer(device->device, &framebufferCI, NULL, &phase->renderpass->framebuffers[i]) == VK_SUCCESS, "Failed to create viewport renderphase framebuffer");
	}

    return 1;
}

/// @brief recreates the viewport render phase
/// @param phase cren viewport render phase
/// @param device cren vulkan device
/// @param swapchain cren vulkan swapchain
static void internal_crenvk_renderphase_viewport_recreate(vkViewportRenderphase* phase, vkDevice* device, vkSwapchain* swapchain) {
	internal_crenvk_renderphase_viewport_destroy(phase, device->device, 0);

	for (unsigned int i = 0; i < phase->renderpass->framebufferCount; i++) {
		vkDestroyFramebuffer(device->device, phase->renderpass->framebuffers[i], NULL);
	}
	crenmemory_deallocate(phase->renderpass->framebuffers);

	internal_crenvk_renderphase_viewport_framebuffers_create(phase, device, swapchain);
}

// vkDefaultRenderphase* phase, CRenContext* context, unsigned int currentFrame, unsigned int swapchainImageIndex, int usingViewport, double timestep, CRenCallback_Render callback
static void internal_crenvk_renderphase_viewport_update(vkViewportRenderphase* phase, CRenContext* context, unsigned int currentFrame, unsigned int swapchainImageIndex, int usingViewport, double timestep, CRenCallback_Render callback) {
	if (!usingViewport) return;

    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	VkClearValue clearValues[2] = { 0 };
	clearValues[0].color = (VkClearColorValue){ 0.0f,  0.0f,  0.0f, 1.0f };
	clearValues[1].depthStencil = (VkClearDepthStencilValue){ .depth = 1.0f, .stencil = 0 };

	VkCommandBuffer cmdBuffer = phase->renderpass->commandBuffers[currentFrame];
	VkFramebuffer framebuffer = phase->renderpass->framebuffers[swapchainImageIndex];
	VkRenderPass renderPass = phase->renderpass->renderPass;

	vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

	VkCommandBufferBeginInfo cmdBeginInfo = { 0 };
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = NULL;
	cmdBeginInfo.flags = 0;
	CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo) == VK_SUCCESS, "Failed to begin viewport renderphase command buffer");

	VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderArea.offset = (VkOffset2D){ 0, 0 };
	renderPassBeginInfo.renderArea.extent = renderer->swapchain.swapchainExtent;
	renderPassBeginInfo.clearValueCount = (uint32_t)CREN_ARRAYSIZE(clearValues);
	renderPassBeginInfo.pClearValues = clearValues;
	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// set frame commandbuffer viewport
	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)renderer->swapchain.swapchainExtent.width;
	viewport.height = (float)renderer->swapchain.swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	// set frame commandbuffer scissor
	VkRect2D scissor = { 0 };
	scissor.offset = (VkOffset2D){ 0, 0 };
	scissor.extent = renderer->swapchain.swapchainExtent;
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	// render objects
	if (callback != NULL) callback(context, VK_RENDER_STAGE_DEFAULT, timestep);

	vkCmdEndRenderPass(cmdBuffer);

	// end command buffer
	CREN_ASSERT(vkEndCommandBuffer(cmdBuffer) == VK_SUCCESS, "Failed to end viewport renderphase command buffer");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Core-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int cren_vulkan_init(CRenVulkanBackend *backend, CRenCreateInfo* ci) {

    backend->hint_viewport = ci->smallerViewport;

    int success = 1;
    success &= internal_crenvk_instance_create(&backend->instance, ci->appName, ci->appVersion, ci->apiVersion, ci->validations);
    success &= internal_crenvk_device_create(backend, ci->nativeWindow, ci->validations);
    success &= internal_crenvk_swapchain_create(&backend->swapchain, backend->device.device, backend->device.physicalDevice, backend->device.surface, ci->width, ci->height, ci->vsync);
    // swapchain does not have a pre-defined pipeline

    backend->defaultRenderphase = internal_crenvk_renderphase_default_create(backend->device.device, backend->device.physicalDevice, backend->swapchain.swapchainFormat.format, (VkSampleCountFlagBits)ci->msaa, 0);
    success &= internal_crenvk_renderphase_default_commandpool_create(&backend->defaultRenderphase, &backend->device);
    success &= internal_crenvk_renderphase_default_framebuffers_create(&backend->defaultRenderphase, &backend->device, &backend->swapchain);
    backend->defaultRenderphase.pipeline = internal_crenvk_renderphase_default_pipeline_create(&backend->defaultRenderphase, backend->device.device, 1, ci->assetsRoot);

    backend->pickingRenderphase = internal_crenvk_renderphase_picking_create(backend->device.device, backend->device.physicalDevice, VK_FORMAT_R32G32_UINT, (VkSampleCountFlagBits)ci->msaa);
    success &= internal_crenvk_renderphase_picking_commandpool_create(&backend->pickingRenderphase, &backend->device);
    success &= internal_crenvk_renderphase_picking_framebuffers_create(&backend->pickingRenderphase, &backend->device, &backend->swapchain);
    backend->pickingRenderphase.pipeline = internal_crenvk_renderphase_picking_pipeline_create(&backend->pickingRenderphase, backend->device.device, 1, ci->assetsRoot);

    backend->uiRenderphase = internal_crenvk_renderphase_ui_create(backend->device.device, backend->swapchain.swapchainFormat.format, VK_SAMPLE_COUNT_1_BIT, 1);
    success &= internal_crenvk_renderphase_ui_commandpool_create(&backend->uiRenderphase, &backend->device);
    success &= internal_crenvk_renderphase_ui_framebuffers_create(&backend->uiRenderphase, &backend->device, &backend->swapchain);
    // ui does not have a pre-defined pipeline

    if(backend->hint_viewport) {
        backend->viewportRenderphase = internal_crenvk_renderphase_viewport_create(backend->device.device, backend->device.physicalDevice, backend->swapchain.swapchainFormat.format, VK_SAMPLE_COUNT_1_BIT);
        success &= internal_crenvk_renderphase_viewport_commandpool_create(&backend->viewportRenderphase, &backend->device);
        success &= internal_crenvk_renderphase_viewport_framebuffers_create(&backend->viewportRenderphase, &backend->device, &backend->swapchain);
        // viewport does not have a pre-defined pipeline
    }

    // buffers
    backend->buffersLib = crenhashtable_create();
    crenhashtable_insert(backend->buffersLib, "Camera", crenvk_buffer_create(backend->device.device, backend->device.physicalDevice,  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(vkBufferCamera)));
    
    // pipelines
    vkRenderpass* mainRenderpass = backend->hint_viewport ? backend->viewportRenderphase.renderpass : backend->defaultRenderphase.renderpass;
    backend->pipelinesLib = crenhashtable_create();
    internal_crenvk_pipeline_quad_create(backend->pipelinesLib, mainRenderpass,  backend->pickingRenderphase.renderpass, backend->device.device, ci->assetsRoot);

    return success;
}

void cren_vulkan_shutdown(CRenVulkanBackend *backend) {

    crenvk_pipeline_destroy(backend->device.device, crenhashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME));
    crenvk_pipeline_destroy(backend->device.device, crenhashtable_lookup(backend->pipelinesLib, CREN_PIPELINE_QUAD_PICKING_NAME));

    crenvk_buffer_destroy(crenhashtable_lookup(backend->buffersLib, "Camera"), backend->device.device);

    if (backend->hint_viewport) internal_crenvk_renderphase_viewport_destroy(&backend->viewportRenderphase, backend->device.device, 1);

    internal_crenvk_renderphase_ui_destroy(&backend->uiRenderphase, backend->device.device, 1);
    internal_crenvk_renderphase_picking_destroy(&backend->pickingRenderphase, backend->device.device, 1, 1);
    internal_crenvk_renderphase_default_destroy(&backend->defaultRenderphase, backend->device.device, 1, 1);
    internal_crenvk_swapchain_destroy(&backend->swapchain, backend->device.device);
    internal_crenvk_device_destroy(&backend->instance, &backend->device);
    internal_crenvk_instance_destroy(&backend->instance);
}

void cren_vulkan_update(CRenContext* context, double timestep) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    if (renderer->hint_minimized) return;

    vkBufferCamera cameraData = { 0 };
    cameraData.view = context->camera.view;
    cameraData.proj = context->camera.perspective;

    // look up the camera buffer
    vkBuffer* cameraBuffer = crenhashtable_lookup(renderer->buffersLib, "Camera");
    if (cameraBuffer) {
        void* where = crenarray_at(cameraBuffer->mappedData, renderer->device.currentFrame);
        if (where != NULL) {
            crenmemory_copy(where, &cameraData, sizeof(vkBufferCamera));
        }
    }
}

void cren_vulkan_render(CRenContext* context, double timestep) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;

    // window is hinted as minimized
    if (renderer->hint_minimized) return;

    unsigned int currentFrame = renderer->device.currentFrame;
    vkWaitForFences(renderer->device.device, 1, &renderer->device.framesInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    VkResult res = vkAcquireNextImageKHR(renderer->device.device, renderer->swapchain.swapchain, UINT64_MAX, renderer->device.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &renderer->device.imageIndex);
    
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        internal_crenvk_renderphase_default_recreate(&renderer->defaultRenderphase, &renderer->device, &renderer->swapchain, context->createInfo->width, context->createInfo->height, context->createInfo->vsync);
        internal_crenvk_renderphase_picking_recreate(&renderer->pickingRenderphase, &renderer->device, &renderer->swapchain);
        internal_crenvk_renderphase_ui_recreate(&renderer->uiRenderphase, &renderer->device, &renderer->swapchain);
    
        if (renderer->hint_viewport) {
            internal_crenvk_renderphase_viewport_recreate(&renderer->viewportRenderphase, &renderer->device, &renderer->swapchain);
        }
        
        return;
    }

    CREN_ASSERT(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR, "Renderer update was not able to aquire an image from the swapchain");
    vkResetFences(renderer->device.device, 1, &renderer->device.framesInFlightFences[currentFrame]);

    // manage renderpasses/render phases
    int usingViewport = renderer->hint_viewport;
    internal_crenvk_renderphase_default_update(&renderer->defaultRenderphase, context, currentFrame, renderer->device.imageIndex, usingViewport, timestep, context->renderCallback);
    internal_crenvk_renderphase_viewport_update(&renderer->viewportRenderphase, context, currentFrame, renderer->device.imageIndex, usingViewport, timestep, context->renderCallback);
    internal_crenvk_renderphase_picking_update(&renderer->pickingRenderphase, context, currentFrame, renderer->device.imageIndex, usingViewport, timestep, context->renderCallback);
    internal_crenvk_renderphase_ui_update(&renderer->uiRenderphase, context, currentFrame, renderer->device.imageIndex, context->drawUIRawDataCallback);

    // submit command buffers
    VkSwapchainKHR swapChains[] = { renderer->swapchain.swapchain };
    VkSemaphore waitSemaphores[] = { renderer->device.imageAvailableSemaphores[currentFrame]};
    VkSemaphore signalSemaphores[] = { renderer->device.finishedRenderingSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (usingViewport) {
        const VkCommandBuffer commandBuffers[] = {
            renderer->defaultRenderphase.renderpass->commandBuffers[currentFrame],
            renderer->pickingRenderphase.renderpass->commandBuffers[currentFrame],
            renderer->viewportRenderphase.renderpass->commandBuffers[currentFrame],
            renderer->uiRenderphase.renderpass->commandBuffers[currentFrame]
        };

        submitInfo.commandBufferCount = (uint32_t)CREN_ARRAYSIZE(commandBuffers);
        submitInfo.pCommandBuffers = commandBuffers;
    }

    else {
        const VkCommandBuffer commandBuffers[] = {
            renderer->defaultRenderphase.renderpass->commandBuffers[currentFrame],
            renderer->pickingRenderphase.renderpass->commandBuffers[currentFrame],
            renderer->uiRenderphase.renderpass->commandBuffers[currentFrame]
        };

        submitInfo.commandBufferCount = (uint32_t)CREN_ARRAYSIZE(commandBuffers);
        submitInfo.pCommandBuffers = commandBuffers;
    }
    
    CREN_ASSERT(vkQueueSubmit(renderer->device.graphicsQueue, 1, &submitInfo, renderer->device.framesInFlightFences[currentFrame]) == VK_SUCCESS, "Renderer update was not able to submit frame to graphics queue");

    // present the image
    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &renderer->device.imageIndex;

    res = vkQueuePresentKHR(renderer->device.graphicsQueue, &presentInfo);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || renderer->hint_resize) {
        renderer->hint_resize = 0;

        internal_crenvk_renderphase_default_recreate(&renderer->defaultRenderphase, &renderer->device, &renderer->swapchain, context->createInfo->width, context->createInfo->height, context->createInfo->vsync);
        internal_crenvk_renderphase_picking_recreate(&renderer->pickingRenderphase, &renderer->device, &renderer->swapchain);
        internal_crenvk_renderphase_ui_recreate(&renderer->uiRenderphase, &renderer->device, &renderer->swapchain);
        
        if (renderer->hint_viewport) {
            internal_crenvk_renderphase_viewport_recreate(&renderer->viewportRenderphase, &renderer->device, &renderer->swapchain);
        }

        cren_camera_set_aspect_ratio(&context->camera, (float)(context->createInfo->width / context->createInfo->height));

        CRenCallback_ImageCount fnImageCount = (CRenCallback_ImageCount)context->imageCountCallback;
        CRenCallback_Resize fnResize = (CRenCallback_Resize)context->resizeCallback;
        if (context->resizeCallback != NULL) fnResize(context, context->createInfo->width, context->createInfo->height);
        if (context->imageCountCallback != NULL) fnImageCount(context, renderer->swapchain.swapchainImageCount);
    }

    else if (res != VK_SUCCESS) {
        CREN_ASSERT(1, "Renderer update was not able to properly presnet the graphics queue frame");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Image and related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int crenvk_image_create(unsigned int width, unsigned int height, unsigned int mipLevels, unsigned int arrayLayers, VkDevice device, VkPhysicalDevice physicalDevice, VkImage *image, VkDeviceMemory *memory, VkFormat format, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageCreateFlags flags) {
    // specify and create image
    VkImageCreateInfo imageCI = {0};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = NULL;
    imageCI.flags = flags;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.extent.width = width;
    imageCI.extent.height = height;
    imageCI.extent.depth = 1;
    imageCI.mipLevels = mipLevels;
    imageCI.arrayLayers = arrayLayers;
    imageCI.format = format;
    imageCI.tiling = tiling;
    imageCI.usage = usage;
    imageCI.samples = samples;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateImage(device, &imageCI, NULL, image) != VK_SUCCESS) {
        return 0;
    }

    // query memory requirements and allocate it
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = internal_crenvk_find_memory_type(physicalDevice, memRequirements.memoryTypeBits, memoryProperties);

	if (vkAllocateMemory(device, &allocInfo, NULL, memory) != VK_SUCCESS || vkBindImageMemory(device, *image, *memory, 0) != VK_SUCCESS) {
        vkDestroyImage(device, *image, NULL);
        vkFreeMemory(device, *memory, NULL);
        return 0;
    }
    
    return 1;
}

VkImageView crenvk_image_view_create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect, unsigned int mipLevel, unsigned int layerCount, VkImageViewType viewType) {
    VkImageView imageView = VK_NULL_HANDLE;
	VkImageViewCreateInfo imageViewCI = { 0 };
	imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCI.image = image;
	imageViewCI.viewType = viewType;
	imageViewCI.format = format;
	imageViewCI.subresourceRange.aspectMask = aspect;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.levelCount = mipLevel;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.layerCount = layerCount;
	CREN_ASSERT(vkCreateImageView(device, &imageViewCI, NULL, &imageView) == VK_SUCCESS, "Failed to create vulkan image view");
	return imageView;
}

VkSampler crenvk_image_sampler_create(VkDevice device, VkPhysicalDevice physicalDevice, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels)
{
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
    
	VkSamplerCreateInfo samplerCI = { 0 };
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = mag;
	samplerCI.minFilter = min;
	samplerCI.addressModeU = u;
	samplerCI.addressModeV = v;
	samplerCI.addressModeW = w;
	samplerCI.anisotropyEnable = VK_TRUE;
	samplerCI.maxAnisotropy = props.limits.maxSamplerAnisotropy;
	samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCI.unnormalizedCoordinates = VK_FALSE;
	samplerCI.compareEnable = VK_FALSE;
	samplerCI.maxLod = mipLevels;
	samplerCI.minLod = 0.0f;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VkSampler sampler;
	CREN_ASSERT(vkCreateSampler(device, &samplerCI, NULL, &sampler) == VK_SUCCESS, "Failed to create vulkan image sampler");

    return sampler;
}

VkDescriptorSet crenvk_image_descriptor_set_create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler sampler, VkImageView view)
{
	// create descriptor set
	VkDescriptorSet descriptorSet;
	VkDescriptorSetAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	CREN_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) == VK_SUCCESS, "Failed to allocate vulkan descriptor set");

	// update descriptor set
	VkDescriptorImageInfo descImage[1] = { 0 };
	descImage[0].sampler = sampler;
	descImage[0].imageView = view;
	descImage[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDesc[1] = { 0 };
	writeDesc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDesc[0].dstSet = descriptorSet;
	writeDesc[0].descriptorCount = 1;
	writeDesc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDesc[0].pImageInfo = descImage;
	vkUpdateDescriptorSets(device, 1, writeDesc, 0, NULL);

	return descriptorSet;
}

void crenvk_image_mipmaps_create(VkDevice device, VkQueue queue, VkCommandPool cmdPool, int width, int height, int mipLevels, VkImage image)
{
	VkCommandBuffer commandBuffer = crenvk_commandbuffer_begin_singletime(device, cmdPool);

	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;

	for (int32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		VkImageBlit blit = { 0 };
		blit.srcOffsets[0] = (VkOffset3D){ 0, 0, 0 };
		blit.srcOffsets[1] = (VkOffset3D){ mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = (VkOffset3D){ 0, 0, 0 };
		blit.dstOffsets[1] = (VkOffset3D){ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	crenvk_commandbuffer_end_singletime(device, cmdPool, commandBuffer, queue);
}

void crenvk_image_memory_barrier_insert(VkCommandBuffer cmdBuffer, VkImage image, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier = { 0 };
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = 0;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask = srcAccessFlags;
	imageMemoryBarrier.dstAccessMask = dstAccessFlags;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}

int crenvk_image_transition_layout(VkDevice device, VkQueue queue, VkCommandPool cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, unsigned int layerCount)
{
	VkCommandBuffer cmdBuffer = crenvk_commandbuffer_begin_singletime(device, cmdPool);

	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0; // no synchronization is needed for VK_IMAGE_LAYOUT_UNDEFINED
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // set destination access mask for writing to a color attachment

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // writes to the color attachment
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;          // reads for transfer operations

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // writing to color attachment
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;           // ready for transfer
	}

	else
	{
		return 0;
	}

	vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);
	crenvk_commandbuffer_end_singletime(device, cmdPool, cmdBuffer, queue);

    return 1;
}

VkFormat crenvk_find_suitable_format(VkPhysicalDevice physicalDevice, const VkFormat *candidates, unsigned int candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    VkFormat resFormat = VK_FORMAT_UNDEFINED;
    for (unsigned int i = 0; i < candidatesCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) resFormat = candidates[i];
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) resFormat = candidates[i];
    }

	CREN_ASSERT(resFormat != VK_FORMAT_UNDEFINED, "Failed to find suitable VkFormat");
    return resFormat;
}

VkFormat crenvk_find_depth_format(VkPhysicalDevice physicalDevice)
{
    const VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT };
    VkFormat format = crenvk_find_suitable_format(physicalDevice, candidates, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    return format;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vkBuffer* crenvk_buffer_create(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, VkDeviceSize size) {
    vkBuffer* buffer = (vkBuffer*)crenmemory_allocate(sizeof(vkBuffer), 1);
	if (buffer == NULL) return NULL;

    buffer->buffers = (VkBuffer*)crenmemory_allocate(sizeof(VkBuffer) * CREN_CONCURRENTLY_RENDERED_FRAMES, 1);
    if(!buffer->buffers) {
        crenmemory_deallocate(buffer);
        return NULL;
    }

    buffer->memories = (VkDeviceMemory*)crenmemory_allocate(sizeof(VkDeviceMemory) * CREN_CONCURRENTLY_RENDERED_FRAMES, 1);
    if(!buffer->memories) {
        crenmemory_deallocate(buffer->buffers);
        crenmemory_deallocate(buffer);
        return NULL;
    }

    buffer->mappedData = crenarray_create(CREN_CONCURRENTLY_RENDERED_FRAMES);
	if (buffer->mappedData == NULL)  {
        crenmemory_deallocate(buffer->memories);
        crenmemory_deallocate(buffer->buffers);
        crenmemory_deallocate(buffer);
        return NULL;
    }

    for(unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
		if (!crenvk_device_create_buffer(device, physicalDevice, usageFlags, memoryFlags, size, &buffer->buffers[i], &buffer->memories[i], NULL)) {
			crenvk_buffer_destroy(buffer, device);
			return NULL;
		}

		void* mapped = NULL;
		if (vkMapMemory(device, buffer->memories[i], 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS) {
            crenvk_buffer_destroy(buffer, device);
			return NULL;
		}

		if (crenarray_push_back(buffer->mappedData, mapped) != 0) {
            crenvk_buffer_destroy(buffer, device);
			return NULL;
		}
    }
	
	buffer->mapped = 1;
	return buffer;
}

void crenvk_buffer_destroy(vkBuffer* buffer, VkDevice device) {
    if(buffer == NULL) return;

    for(unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
		if (buffer->mapped) {
			vkUnmapMemory(device, buffer->memories[i]);
		}

		vkDestroyBuffer(device, buffer->buffers[i], NULL);
		vkFreeMemory(device, buffer->memories[i], NULL);
    }

	if(buffer->buffers) crenmemory_deallocate(buffer->buffers);
	if(buffer->memories) crenmemory_deallocate(buffer->memories);
	crenarray_destroy(buffer->mappedData);
	crenmemory_deallocate(buffer);
}

int crenvk_buffer_map(vkBuffer *buffer, VkDevice device) {
    if (buffer == NULL || device == VK_NULL_HANDLE) return 0;
	if (buffer->mapped) return 1;

	for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {

		void* data = (void*)crenarray_at(buffer->mappedData, i);
		if (!data) return 0; // data does not exists
		if (vkMapMemory(device, buffer->memories[i], 0, VK_WHOLE_SIZE, 0, &data) != VK_SUCCESS) return 0; // error when mapping
	}

	buffer->mapped = 1;
    return 1;
}

void crenvk_buffer_unmap(vkBuffer* buffer, VkDevice device) {
	if (buffer == NULL || device == VK_NULL_HANDLE) return;
	if (!buffer->mapped) return;
	
	for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {
		void** data = (void**)crenarray_at(buffer->mappedData, i);
		if (!data) return; // data does not exists, but it does not matter

		vkUnmapMemory(device, buffer->memories[i]);
		*data = NULL;
	}

	buffer->mapped = 0;
}

VkCommandBuffer crenvk_commandbuffer_create(VkDevice device, VkCommandPool cmdPool, VkCommandBufferLevel level, int begin) {
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.pNext = NULL;
	cmdBufferAllocInfo.commandBufferCount = 1;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.level = level;

	VkCommandBuffer cmdBuffer;
	CREN_ASSERT(vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &cmdBuffer) == VK_SUCCESS, "Failed to allocate command buffer");

	if (begin) {
		VkCommandBufferBeginInfo cmdBufferBI = { 0 };
		cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBI.pNext = NULL;
		cmdBufferBI.flags = 0;
		CREN_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI) == VK_SUCCESS, "Failed to begin command buffer");
	}

	return cmdBuffer;
}

VkCommandBuffer crenvk_commandbuffer_begin_singletime(VkDevice device, VkCommandPool cmdPool)
{
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = { 0 };
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &commandBuffer);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = { 0 };
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);
	return commandBuffer;
}

void crenvk_commandbuffer_end_singletime(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue)
{
    vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

int crenvk_commandbuffer_begin(VkCommandBuffer cmdBuffer)
{
    VkCommandBufferBeginInfo cmdBufferBI = { 0 };
	cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBI.pNext = NULL;
	cmdBufferBI.flags = 0;
    return vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI) == VK_SUCCESS;
}

int crenvk_commandbuffer_end(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, int free)
{
	if(vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) return 0;

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VkFenceCreateInfo fenceCI = { 0 };
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = NULL;
	fenceCI.flags = 0;

	VkFence fence = VK_NULL_HANDLE;
    if(vkCreateFence(device, &fenceCI, NULL, &fence) != VK_SUCCESS) return 0;

    if(vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) { 
        vkDestroyFence(device, fence, NULL);
        return 0;
    }

	if(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000) != VK_SUCCESS) {
        vkDestroyFence(device, fence, NULL);
        return 0;
    }

	vkDestroyFence(device, fence, NULL);

	if (free) vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRenTexture2D crenvk_texture2d_create_from_path(CRenContext* context, const char* path, int gui) {
    CRenTexture2D tex = { 0 };
    tex.backend = crenmemory_allocate(sizeof(CRenTexture2DBackend), 0);
	cren_strncpy(tex.path, path, sizeof(tex.path) - 1);

	int channels = 0;
    unsigned char* pixels = cren_stbimage_load_from_file(tex.path, 4, &tex.width, &tex.height, &channels);
	CREN_ASSERT(pixels != NULL, "Error when loading texture 2d");

	tex.mipLevels = gui == 1 ? 1 : (int)d_floor(d_log2(int_max(tex.width, tex.height))) + 1;

	CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	VkDeviceSize imgSize = (VkDeviceSize)(tex.width * tex.height * 4);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	crenvk_device_create_buffer
	(
		renderer->device.device,
        renderer->device.physicalDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		imgSize,
		&stagingBuffer,
		&stagingMemory,
		NULL
	);

	void* gpuData = NULL;
	vkMapMemory(renderer->device.device, stagingMemory, 0, imgSize, 0, &gpuData);
	crenmemory_copy(gpuData, pixels, (size_t)imgSize);
	vkUnmapMemory(renderer->device.device, stagingMemory);

	cren_stbimage_destroy(pixels);

	vkRenderpass* renderpass = renderer->hint_viewport == 1 ? renderer->viewportRenderphase.renderpass : renderer->defaultRenderphase.renderpass;

	crenvk_image_create
	(
		tex.width,
		tex.height,
		tex.mipLevels,
		1,
		renderer->device.device,
		renderer->device.physicalDevice,
		&tex.backend->image,
		&tex.backend->memory,
		VK_FORMAT_R8G8B8A8_SRGB,
		gui == 1 ? VK_SAMPLE_COUNT_1_BIT : renderpass->msaa,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	);

	// transition layout to data transfer
	crenvk_image_transition_layout
	(
		renderer->device.device,
		renderer->device.graphicsQueue,
		renderpass->commandPool,
		tex.backend->image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		tex.mipLevels,
		1
	);

	// copy staging buffer to actual image
	VkCommandBuffer cmdBuffer = crenvk_commandbuffer_begin_singletime(renderer->device.device, renderpass->commandPool);

	VkBufferImageCopy region = { 0 };
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = tex.width;
	region.imageExtent.height = tex.height;
	region.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, tex.backend->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	crenvk_commandbuffer_end_singletime(renderer->device.device, renderpass->commandPool, cmdBuffer, renderer->device.graphicsQueue);

	// mipmap generation
	crenvk_image_mipmaps_create
	(
		renderer->device.device,
		renderer->device.graphicsQueue,
		renderpass->commandPool,
		tex.width,
		tex.height,
		tex.mipLevels,
		tex.backend->image
	);

	vkDestroyBuffer(renderer->device.device, stagingBuffer, NULL);
	vkFreeMemory(renderer->device.device, stagingMemory, NULL);

	// image view and sampler
	tex.backend->view = crenvk_image_view_create(renderer->device.device, tex.backend->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, tex.mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D);
	tex.backend->sampler = crenvk_image_sampler_create(renderer->device.device, renderer->device.physicalDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)tex.mipLevels);
	tex.backend->uiDescriptor = crenvk_image_descriptor_set_create(renderer->device.device, renderer->uiRenderphase.descPool, renderer->uiRenderphase.descSetLayout, tex.backend->sampler, tex.backend->view);

    return tex;
}

CRenTexture2D crenvk_texture2d_create_from_buffer(CRenContext* context, CrenTexture2DBuffer* bufferInfo, int gui) {
    CRenTexture2D tex = { 0 };
	tex.backend = crenmemory_allocate(sizeof(CRenTexture2DBackend), 0);
	tex.width = bufferInfo->width;
	tex.height = bufferInfo->height;
	tex.mipLevels = gui == 1 ? 1 : (int)d_floor(d_log2(int_max(tex.width, tex.height))) + 1;

	VkDeviceSize imgSize = (VkDeviceSize)bufferInfo->lenght;

	// create staging buffer for image
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	vkRenderpass* renderpass = renderer->hint_viewport == 1 ? renderer->viewportRenderphase.renderpass : renderer->defaultRenderphase.renderpass;

	crenvk_device_create_buffer
	(
		renderer->device.device,
        renderer->device.physicalDevice, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		imgSize,
		&stagingBuffer,
		&stagingMemory,
		NULL
	);

	// copy texture data into staging buffer
	void* gpuData = NULL;
	vkMapMemory(renderer->device.device, stagingMemory, 0, imgSize, 0, &gpuData);
    crenmemory_copy(gpuData, bufferInfo->data, (size_t)imgSize);
	vkUnmapMemory(renderer->device.device, stagingMemory);

	// create image resource
	crenvk_image_create
	(
		tex.width,
		tex.height,
		tex.mipLevels,
		1,
		renderer->device.device,
		renderer->device.physicalDevice,
		&tex.backend->image,
		&tex.backend->memory,
		VK_FORMAT_R8G8B8A8_SRGB,
		renderpass->msaa,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0
	);

	// transition layout to transfer data
	crenvk_image_transition_layout
	(
		renderer->device.device,
		renderer->device.graphicsQueue,
		renderpass->commandPool,
		tex.backend->image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		tex.mipLevels,
		1
	);

	// copy buffer to image
	VkCommandBuffer cmdBuffer = crenvk_commandbuffer_begin_singletime(renderer->device.device, renderpass->commandPool);

	VkBufferImageCopy region = { 0 };
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = tex.width;
	region.imageExtent.height = tex.height;
	region.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, tex.backend->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	crenvk_commandbuffer_end_singletime(renderer->device.device, renderpass->commandPool, cmdBuffer, renderer->device.graphicsQueue);

	// free staging buffer
	vkDestroyBuffer(renderer->device.device, stagingBuffer, NULL);
	vkFreeMemory(renderer->device.device, stagingMemory, NULL);

	crenvk_image_mipmaps_create(renderer->device.device, renderer->device.graphicsQueue, renderpass->commandPool, tex.width, tex.height, tex.mipLevels, tex.backend->image);

	// image view and sampler
	tex.backend->view = crenvk_image_view_create(renderer->device.device, tex.backend->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, tex.mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D);
	tex.backend->sampler = crenvk_image_sampler_create(renderer->device.device, renderer->device.physicalDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)tex.mipLevels);
	tex.backend->uiDescriptor = crenvk_image_descriptor_set_create(renderer->device.device, renderer->uiRenderphase.descPool, renderer->uiRenderphase.descSetLayout, tex.backend->sampler, tex.backend->view);

	return tex;
}

void crenvk_texture2d_destroy(CRenContext* context, CRenTexture2D* texture)
{
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;

	vkDeviceWaitIdle(renderer->device.device);
	vkDestroyImageView(renderer->device.device, texture->backend->view, NULL);
	vkDestroyImage(renderer->device.device, texture->backend->image, NULL);
	vkFreeMemory(renderer->device.device, texture->backend->memory, NULL);
	vkDestroySampler(renderer->device.device, texture->backend->sampler, NULL);

	crenmemory_deallocate(context->backend);
}

VkSampler crenvk_texture2d_get_sampler(CRenTexture2D* texture) {
    if (texture == NULL) return NULL;
	if (texture->backend == NULL) return NULL;

	return texture->backend->sampler;
}

VkImageView crenvk_texture2d_get_image_view(CRenTexture2D* texture) {
    if (texture == NULL) return NULL;
	if (texture->backend == NULL) return NULL;

	return texture->backend->view;
}

VkDescriptorSet crenvk_texture2d_get_descriptor(CRenTexture2D *texture) {
    if (texture == NULL) return NULL;
	if (texture->backend == NULL) return NULL;

	return texture->backend->uiDescriptor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quad-related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief updates the quad descriptor sets
/// @param context cren context
/// @param quad the quad to update
static void internal_crenvk_quad_update_descriptors(CRenContext* context, CRenQuad* quad) {
	CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	vkQuadBackend* backend = (vkQuadBackend*)quad->backend;

	for (unsigned int i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {

		// 0: camera data
		vkBuffer* cameraBuffer = crenhashtable_lookup(renderer->buffersLib, "Camera");
		VkDescriptorBufferInfo camInfo = { 0 };
		camInfo.buffer = cameraBuffer->buffers[i];
		camInfo.offset = 0;
		camInfo.range = sizeof(vkBufferCamera);
		
		VkWriteDescriptorSet camDesc = { 0 };
		camDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		camDesc.dstSet = backend->descriptorSets[i];
		camDesc.dstBinding = 0;
		camDesc.dstArrayElement = 0;
		camDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camDesc.descriptorCount = 1;
		camDesc.pBufferInfo = &camInfo;
		vkUpdateDescriptorSets(renderer->device.device, 1, &camDesc, 0, NULL);

		// 1: quad data
		VkDescriptorBufferInfo quadInfo = { 0 };
		quadInfo.buffer = backend->buffer->buffers[i];
		quadInfo.offset = 0;
		quadInfo.range = sizeof(QuadParams);

		VkWriteDescriptorSet quadDesc = { 0 };
		quadDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		quadDesc.dstSet = backend->descriptorSets[i];
		quadDesc.dstBinding = 1;
		quadDesc.dstArrayElement = 0;
		quadDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		quadDesc.descriptorCount = 1;
		quadDesc.pBufferInfo = &quadInfo;
		vkUpdateDescriptorSets(renderer->device.device, 1, &quadDesc, 0, NULL);

		// 2: color map
		VkDescriptorImageInfo colorMapInfo = { 0 };
		colorMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorMapInfo.imageView = (VkImageView)crenvk_texture2d_get_image_view(&backend->colormap);
		colorMapInfo.sampler = (VkSampler)crenvk_texture2d_get_sampler(&backend->colormap);

		VkWriteDescriptorSet desc = { 0 };
		desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc.dstSet = backend->descriptorSets[i];
		desc.dstBinding = 2;
		desc.dstArrayElement = 0;
		desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		desc.descriptorCount = 1;
		desc.pImageInfo = &colorMapInfo;
		vkUpdateDescriptorSets(renderer->device.device, 1, &desc, 0, NULL);
	}

	// update the mapped data
    crenvk_quad_apply_buffer_changes(context, quad);
}

CRenQuad* crenvk_quad_create(CRenContext* context, const char* albedoPath) {
    
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;

    CRenQuad* quad = crenmemory_allocate(sizeof(CRenQuad), 1);
    if(!quad) return NULL;

    quad->backend = crenmemory_allocate(sizeof(vkQuadBackend), 1);
    if(!quad->backend) {
        crenmemory_deallocate(quad);
        return NULL;
    }

	
	quad->backend->buffer = crenvk_buffer_create(renderer->device.device, renderer->device.physicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(QuadParams));
    if(!quad->backend->buffer) {
        crenmemory_deallocate(quad->backend);
        crenmemory_deallocate(quad);
        return NULL;
    }

    quad->id = crenid_generate();

	// descriptors
	VkDescriptorPoolSize poolSizes[3] = { 0 };
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;

	VkDescriptorPoolCreateInfo descriptorPoolCI = { 0 };
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = (uint32_t)CREN_ARRAYSIZE(poolSizes);
	descriptorPoolCI.pPoolSizes = poolSizes;
	descriptorPoolCI.maxSets = CREN_CONCURRENTLY_RENDERED_FRAMES;
	if(vkCreateDescriptorPool(renderer->device.device, &descriptorPoolCI, NULL, &quad->backend->descriptorPool) != VK_SUCCESS) {
        crenvk_buffer_destroy(quad->backend->buffer, renderer->device.device);
        crenmemory_deallocate(quad->backend);
        crenmemory_deallocate(quad);
        return NULL;
    }

	vkPipeline* pipeline = crenhashtable_lookup(renderer->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME);
	VkDescriptorSetLayout layouts[CREN_CONCURRENTLY_RENDERED_FRAMES] = { { pipeline->descriptorSetLayout },  {pipeline->descriptorSetLayout } };

	VkDescriptorSetAllocateInfo descSetAllocInfo = { 0 };
	descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo.descriptorPool = quad->backend->descriptorPool;
	descSetAllocInfo.descriptorSetCount = (unsigned int)CREN_ARRAYSIZE(layouts);
	descSetAllocInfo.pSetLayouts = layouts;
	if(vkAllocateDescriptorSets(renderer->device.device, &descSetAllocInfo, quad->backend->descriptorSets) != VK_SUCCESS) {
        vkDestroyDescriptorPool(renderer->device.device, quad->backend->descriptorPool, NULL);
        crenvk_buffer_destroy(quad->backend->buffer, renderer->device.device);
        crenmemory_deallocate(quad->backend);
        crenmemory_deallocate(quad);
        return NULL;
    }

	// load colormap and update descriptors
	quad->backend->colormap = crenvk_texture2d_create_from_path(context, albedoPath, 0);
	internal_crenvk_quad_update_descriptors(context, quad);

	return quad;
}

void crenvk_quad_destroy(CRenContext* context, CRenQuad* quad) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	vkQuadBackend* backend = (vkQuadBackend*)quad->backend;

	vkDeviceWaitIdle(renderer->device.device);
	vkDestroyDescriptorPool(renderer->device.device, backend->descriptorPool, NULL);

	crenvk_texture2d_destroy(context, &backend->colormap);
    crenvk_buffer_destroy(quad->backend->buffer, renderer->device.device);

	crenmemory_deallocate(quad->backend);
	crenmemory_deallocate(quad);
}

void crenvk_quad_apply_buffer_changes(CRenContext* context, CRenQuad* quad) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    vkQuadBackend* backend = (vkQuadBackend*)quad->backend;
	vkBuffer* quadParams = backend->buffer;

	if (quadParams) {
		void* where = crenarray_at(quadParams->mappedData, renderer->device.currentFrame);

		if (where != NULL) {
			crenmemory_copy(where, &quad->params, sizeof(QuadParams));
		}
	}
}

void crenvk_quad_render(CRenContext* context, vkRenderStage stage, CRenQuad* quad, const mat4 transform) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
	vkQuadBackend* backend = (vkQuadBackend*)quad->backend;
	VkDeviceSize offsets[] = { 0 };
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipelinePtr = VK_NULL_HANDLE;
	unsigned int currentFrame = renderer->device.currentFrame;

	switch (stage) {
		case VK_RENDER_STAGE_DEFAULT:
		{
			vkPipeline* pipeline = crenhashtable_lookup(renderer->pipelinesLib, CREN_PIPELINE_QUAD_DEFAULT_NAME);

			cmdBuffer = renderer->hint_viewport == 1 ? renderer->viewportRenderphase.renderpass->commandBuffers[currentFrame] : renderer->defaultRenderphase.renderpass->commandBuffers[currentFrame];
			pipelineLayout = pipeline->layout;
			pipelinePtr = pipeline->pipeline;
			break;
		}

		case VK_RENDER_STAGE_PICKING:
		{
			vkPipeline* pipeline = crenhashtable_lookup(renderer->pipelinesLib, CREN_PIPELINE_QUAD_PICKING_NAME);

			cmdBuffer = renderer->pickingRenderphase.renderpass->commandBuffers[currentFrame];
			pipelineLayout = pipeline->layout;
			pipelinePtr = pipeline->pipeline;
			break;
		}

		default: { break; }
	}

	vkPushConstant constants = { 0 };
	constants.id = quad->id;
	constants.model = transform;
	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vkPushConstant), &constants);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &backend->descriptorSets[currentFrame], 0, NULL);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinePtr);
	vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
}
