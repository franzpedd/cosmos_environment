#ifndef CREN_CONTEXT_INCLUDED
#define CREN_CONTEXT_INCLUDED

#include "cren_camera.h"

/// @brief used for creating the cren context, specifies various details about the cren graphics context. They may however, latter be modified by functions
typedef struct {
    const char* appName;
    unsigned int appVersion;
    const char* assetsRoot;
    unsigned int apiVersion;
    int validations;
    int vsync;
    int msaa;
    int width;
    int height;
    int smallerViewport;
    void* nativeWindow;
} CRenCreateInfo;

/// @brief holds all current state about the cren context, like camera, renderer's backend, userpointer
typedef struct {
    CRenCreateInfo* createInfo;
    CRenCamera camera;
    void* backend;
    
    void* userPointer;
    void* renderCallback;
    void* resizeCallback;
    void* imageCountCallback;
    void* drawUIRawDataCallback;

} CRenContext;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates the cren context, initializing the library
/// @param createInfo the address of a create info specification
/// @return the cren context or NULL if failed
CRenContext* cren_initialize(CRenCreateInfo* createInfo);

/// @brief shutdown all cren components and objects
/// @param context cren context memory address
void cren_terminate(CRenContext* context);

/// @brief updates the renderer frame, sending information to be processed by the gpu
/// @param context cren context memory address
/// @param timestep interpolation step, time betweent last and current frame
void cren_update(CRenContext* context, double timestep);

/// @brief performs the frame rendering, setting-up all resources required for drawing the current frame and presenting the previously rendered
/// @param context cren context memory address
/// @param timestep interpolation step, time betweent last and current frame
void cren_render(CRenContext* context, double timestep);

/// @brief resizes the renderer, call this on every window resize event
/// @param context cren context memory address
/// @param width new width
/// @param height new height
void cren_resize(CRenContext* context, int width, int height);

/// @brief minimizes the renderer, stopping rendering without staling it
/// @param context cren context memory address
void cren_minimize(CRenContext* context);

/// @brief restores the renderer to it's last known size, resuming the rendering process
/// @param context cren context memory address
void cren_restore(CRenContext* context);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CONTEXT_INCLUDED