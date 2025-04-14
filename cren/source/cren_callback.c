#include "cren_callback.h"

#include "cren_context.h"

void cren_set_user_pointer(CRenContext* context, void* pointer) {
    context->userPointer = pointer;
}

void* cren_get_user_pointer(CRenContext* context) {
    return context->userPointer;
}

void cren_set_render_callback(CRenContext *context, CRenCallback_Render callback) {
    context->renderCallback = callback;
}

void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback) {
    context->resizeCallback = callback;
}

void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback) {
    context->imageCountCallback = callback;
}

void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback) {
    context->drawUIRawDataCallback = callback;
}
