#include <stdio.h>
#include <cren.h>

#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32    // glfwGetWin32Window
#elif defined(PLATFORM_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND  // glfwGetWaylandWindow
#elif defined(PLATFORM_X11)
#define GLFW_EXPOSE_NATIVE_X11      // glfwGetX11Window
#elif defined(PLATFORM_APP) 
#define GLFW_EXPOSE_NATIVE_COCOA    // glfwGetCocoaWindow
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

static CRenContext* sContext = NULL;

// required cren callback, it signals the user when it's time to draw objects so the user can manager how and what to draw
static void render_callback(CRenContext* context, int stage, double timestep) {}

// optional cren callback, it signals the user when 
static void resize_callback(CRenContext* context, unsigned int width, unsigned int height) {}

// optional cren callback, it signals the user when the swapchain image count has changed, usually requested by UI APIs
static void uiimagecount_callback(CRenContext* context, unsigned int count) {}

// optional cren callback, it signals the user when to draw the ui raw data, usually requested by UI APIs
static void drawuirawdata_callback(CRenContext* context, void* commandbuffer) {}

// glfw callback for minimizing/restoring the window's renderer
static void glfwiconfy_callback(GLFWwindow* window, int iconified) {
    iconified == 1 ? cren_minimize(sContext) : cren_restore(sContext);
}

// glfw callback for resizing the window's renderer
static void glfwresize_callback(GLFWwindow* window, int width, int height) {
    cren_resize(sContext, width, height);
}

static GLFWwindow* custom_init_glfw(int width, int height, const char* title) {

    // this example uses glfw, so we must initialize it
    if (glfwInit() != GLFW_TRUE) {
        printf("Failed to initialize GLFW library\n");
        return NULL;
    }

    // use glfw default hint values and request for no OpenGL API
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* winPtr = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!winPtr) {
        printf("The main window could not be created\n");
        glfwTerminate();
    }

    // set the required callbacks for cren
    glfwSetWindowIconifyCallback(winPtr, glfwiconfy_callback);
    glfwSetWindowSizeCallback(winPtr, glfwresize_callback);

    return winPtr;
}

int main(int arg, char** argv) {
    
    // we use glfw on this example
    int width = 1366;
    int height = 728;
    const char* title = "Example";
    GLFWwindow* window = custom_init_glfw(width, height, title);
    if(!window) return 0; // glfw couldn't be initialized, just return early

    // configure the cren initial specification
    CRenCreateInfo ci = { 0 };
    ci.appName = title;                             // the application name
    ci.appVersion = CREN_MAKE_VERSION(0, 1, 0, 0);  // the application version
    ci.assetsRoot = "../data";                      // must be the path the folder shaders resides (CMake puts it into the compiled binary folder (folder previous to Debug/Release))
    ci.apiVersion = CREN_MAKE_VERSION(0, 1, 0, 2);  // desired vulkan api version
    ci.validations = 1;                             // enables the vulkan message errors
    ci.vsync = 0;                                   // vertical syncronization disabled, rendering either triple-buffering(if available) or as fast as possible
    ci.msaa = 4;                                    // how many samples the anti-aliasing uses
    ci.width = width;                               // initial window width
    ci.height = height;                             // initial window height
    ci.smallerViewport = 1;                         // request the renderer to be displayed on a smaller viewport and not the window entirelly
    ci.nativeWindow = glfwGetWin32Window(window);   // ptr to the window object so a window-surface may be created for that window in particular, in this case a HWND

    // initialize the CRen
    sContext = cren_initialize(&ci);
    if (!sContext) {
        printf("Could not initialize CRen\n");
        return 0;
    }

    // set callbacks to have cren feedback
    cren_set_render_callback(sContext, render_callback);
    cren_set_resize_callback(sContext, resize_callback);
    cren_set_ui_image_count_callback(sContext, uiimagecount_callback);
    cren_set_draw_ui_raw_data_callback(sContext, drawuirawdata_callback);

    // main-loop
    uint64_t previousTicks = glfwGetTimerValue();
    double accumulator = 0.0;
    const double FIXED_TIMESTEP = 1.0 / 60.0; // 60 FPS
    const int MAX_UPDATES = 5;

    while (!glfwWindowShouldClose(window)) {

        // pre-frame timing
        uint64_t currentTicks = glfwGetTimerValue();
        double timeStep = (currentTicks - previousTicks) / (double)glfwGetTimerFrequency();
        previousTicks = currentTicks;

        glfwPollEvents(); // process window events

        // clamp timestep to prevent spiral of death
        timeStep = d_min(timeStep, 0.1); // max 100ms
        accumulator += timeStep;

        int updateCount = 0;
        while (accumulator >= FIXED_TIMESTEP && updateCount < MAX_UPDATES) {
            cren_update(sContext, FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
            updateCount++;
        }

        const double alpha = accumulator / FIXED_TIMESTEP;
        cren_render(sContext, alpha);
    }

    // we must destroy the window and terminate glfw at the end of our program
    cren_terminate(sContext);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}