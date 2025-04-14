#ifndef CREN_CALLBACK_INCLUDED
#define CREN_CALLBACK_INCLUDED

#include "cren_context.h"

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief set-up the callback that tells the application it's time to render the world objects 
/// CRen delegate the rendering process to external application since its the application's job to optmize it's world, this may change in the future depending on community request, internally handling the rendering process
/// @param context cren context
/// @param stage rendering stage, default = 0; picking = 1
/// @param context deltatime/timestep, the renderstate interpolation
typedef void (*CRenCallback_Render)(CRenContext* context, int stage, double timestep);

/// @brief set-up the callback that tells the application the appropriate time to resize renderer objects
/// @param context cren context
/// @param width renderer (and window) width
/// @param width renderer (and window) height
typedef void (*CRenCallback_Resize)(CRenContext* context, unsigned int width, unsigned int height);

/// @brief set-up the callback that tells the application the ammount of swapchain images has changed
/// @param context cren context
/// @param count the new ammount of swapchain images
typedef void (*CRenCallback_ImageCount)(CRenContext* context, unsigned int count);

/// @brief set-up the callback that tells the application it's time to draw the ui raw data
/// @param context cren context
/// @param commandbuffer vulkan command object raw-ptr
typedef void (*CRenCallback_DrawUIRawData)(CRenContext* context, void* commandbuffer);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief sets the user defined pointer address
/// @param context cren context memory address
/// @param pointer user-defined pointer
void cren_set_user_pointer(CRenContext* context, void* pointer);

/// @brief returns the user defined pointer address
/// @param context cren context memory address
void* cren_get_user_pointer(CRenContext* context);

/// @brief allows the application to know when it's time to render the world
/// @param context cren context memory address
/// @param callback callback to redirect the code to
void cren_set_render_callback(CRenContext* context, CRenCallback_Render callback);

/// @brief allows the application to know when the renderer was resized
/// @param context cren context memory address
/// @param callback callback to redirect the code to
void cren_set_resize_callback(CRenContext* context, CRenCallback_Resize callback);

/// @brief allows the application to know when the renderer has changed the ammount of swapchain image it has
/// @param context cren context memory address
/// @param callback callback to redirect the code to
void cren_set_ui_image_count_callback(CRenContext* context, CRenCallback_ImageCount callback);

/// @brief allows the application to know when it's time to draw the ui raw data
/// @param context cren context memory address
/// @param callback callback to redirect the code to
void cren_set_draw_ui_raw_data_callback(CRenContext* context, CRenCallback_DrawUIRawData callback);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CALLBACK_INCLUDED