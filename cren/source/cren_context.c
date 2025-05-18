#include "cren_context.h"

#include "cren_error.h"
#include "cren_vulkan.h"

CRenContext* cren_initialize(CRenCreateInfo createInfo) {
    CREN_LOG("CRen: Todo: Make the assertion macro to call a function to display to the console, better than including stdio in a header\n");

    CRenContext* context = (CRenContext*)crenmemory_allocate(sizeof(CRenContext), 1);
    if (!context) {
        cren_set_error(ContextIntializationFailed);
        return NULL;
    }

    context->backend = (CRenVulkanBackend*)crenmemory_allocate(sizeof(CRenVulkanBackend), 1);
    if (!context->backend) {
        cren_set_error(ContextIntializationFailed);
        return NULL;
    }

    context->createInfo = createInfo;
    context->camera = cren_camera_create(CAMERA_TYPE_FREE_LOOK, (float)createInfo.width / (float)createInfo.height);

    cren_vulkan_init((CRenVulkanBackend*)context->backend, &context->createInfo);

    return context;
}

void cren_terminate(CRenContext* context) {
    if(!context) return;
    if(!context->backend) return;

    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;

    cren_vulkan_shutdown(renderer);

    if(context->backend) crenmemory_deallocate(context->backend);
    if(context) crenmemory_deallocate(context);
}

void cren_update(CRenContext* context, double timestep) {
    cren_camera_update(&context->camera, timestep);
    cren_vulkan_update(context, timestep);
}

void cren_render(CRenContext* context, double timestep)
{
    cren_vulkan_render(context, timestep);
}

void cren_resize(CRenContext *context, int width, int height)
{
    context->createInfo.width = width;
    context->createInfo.height = height;

    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    renderer->hint_resize = 1;
}

void cren_minimize(CRenContext* context) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    renderer->hint_minimized = 1;
}

void cren_restore(CRenContext* context) {
    CRenVulkanBackend* renderer = (CRenVulkanBackend*)context->backend;
    renderer->hint_minimized = 0;
}
