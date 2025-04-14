#include "core/window.h"

#include "core/application.h"
#include "core/input.h"
#include "core/logger.h"

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

namespace Cosmos
{
	static unsigned char* sIcon = nullptr; // holds the icon image ptr, this is here be

	Window::Window(Application* app, const char* title, int width, int height)
		: mApp(app), mTitle(title), mWidth(width), mHeight(height)
	{
        COSMOS_ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize the GLFW library");

		// set glfw hints
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// create wndow and load engine's logo
        mNativeWindow = glfwCreateWindow(width, height, title, NULL, NULL);
        COSMOS_ASSERT(mNativeWindow != NULL, "Failed to create the GLFW window");

		char iconPath[128];
		cren_get_path("texture/logo.png", "../data", 0, iconPath, sizeof(iconPath));

		int iconWidth, iconHeight, iconChannels;
		if (sIcon) cren_stbimage_destroy(sIcon);
		sIcon = cren_stbimage_load_from_file(iconPath, 4, &iconWidth, &iconHeight, &iconChannels);

		GLFWimage icons[1];
		icons[0].pixels = sIcon;
		icons[0].width = iconWidth;
		icons[0].height = iconHeight;
		glfwSetWindowIcon(mNativeWindow, 1, icons);

		// set glfw user-ptr, an address to return to when a callback happens
		glfwSetWindowUserPointer(mNativeWindow, this);

		// any glfw error will be callbacked to this lambda-function
		glfwSetErrorCallback([](int code, const char* msg) {
			COSMOS_LOG(LogSeverity::Error, "[GLFW Internal Error]:[Code:%d]:[Message:%s]", code, msg);
			});

		// window close event
		glfwSetWindowCloseCallback(mNativeWindow, [](GLFWwindow* window) {
			Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
			windowClass.mShouldClose = true;
			});

		// window was either minimized/restored
		glfwSetWindowIconifyCallback(mNativeWindow, [](GLFWwindow* window, int iconified) {
			Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
			iconified == 1 ? windowClass.mApp->OnMinimize() : windowClass.mApp->OnRestore(windowClass.mWidth, windowClass.mHeight);
			});

		// window's framebuffer was resized, we must signal this to application so it can properly adjust the new size
		glfwSetFramebufferSizeCallback(mNativeWindow, [](GLFWwindow* window, int width, int height) {
			Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
			windowClass.mWidth = width;
			windowClass.mHeight = height;
			windowClass.mApp->OnResize(width, height);
			});

		// window itself was resized, we must signal this to application so it can properly adjust the new size
		glfwSetWindowSizeCallback(mNativeWindow, [](GLFWwindow* window, int width, int height) {
			Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
			windowClass.mWidth = width;
			windowClass.mHeight = height;
			windowClass.mApp->OnResize(width, height);
			});

		// physical key event
		glfwSetKeyCallback(mNativeWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						windowClass.mApp->OnKeyPress((Input::Keycode)key, (Input::Keymod)mods, false);
						break;
					}
					case GLFW_RELEASE:
					{
						windowClass.mApp->OnKeyRelease((Input::Keycode)key);
						break;
					}

					case GLFW_REPEAT:
					{
						windowClass.mApp->OnKeyPress((Input::Keycode)key, (Input::Keymod)mods, true);
						break;
					}
				}
			});

		// mouse button event 
		glfwSetMouseButtonCallback(mNativeWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						windowClass.mApp->OnButtonPress((Input::Buttoncode)button, Input::Keymod(mods));
						break;
					}

					case GLFW_RELEASE:
					{
						windowClass.mApp->OnButtonRelease((Input::Buttoncode)button);
						break;
					}

					default: { break; }
				}
			});

		// mouse scroll event
		glfwSetScrollCallback(mNativeWindow, [](GLFWwindow* window, double xoffset, double yoffset)
			{
				Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
				windowClass.mApp->OnMouseScroll(xoffset, yoffset);
			});

		// mouse move event
		glfwSetCursorPosCallback(mNativeWindow, [](GLFWwindow* window, double xpos, double ypos)
			{
				Window& windowClass = *(Window*)glfwGetWindowUserPointer(window);
				windowClass.mLastMousePosX = xpos - windowClass.mLastMousePosX;
				windowClass.mLastMousePosY = ypos - windowClass.mLastMousePosY;
				windowClass.mApp->OnMouseMove(windowClass.mLastMousePosX, windowClass.mLastMousePosY);
			});

		// freeing resources
	}

	Window::~Window()
	{
		if (sIcon) cren_stbimage_destroy(sIcon);
		glfwDestroyWindow(mNativeWindow);
		glfwTerminate();
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	void Window::ToogleCursor(bool hide)
	{
		if (hide) {
			glfwSetInputMode(mNativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			return;
		}

		glfwSetInputMode(mNativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool Window::IsKeyDown(Input::Keycode key)
	{
		return glfwGetKey(mNativeWindow, key) == GLFW_PRESS;
	}

	unsigned long long Window::GetTimer()
	{
		return glfwGetTimerValue();
	}

	unsigned long long Window::GetTimerFrequency()
	{
		return glfwGetTimerFrequency();
	}

	float Window::GetFramebufferAspectRatio()
	{
		int width = 0;
		int height = 0;
		GetFramebufferSize(&width, &height);

		if (height == 0) return 1.0f; // avoid division by 0

		return (float)(width / height);
	}

	void Window::GetFramebufferSize(int* outWidth, int* outHeight)
	{
		glfwGetFramebufferSize(mNativeWindow, outWidth, outHeight);
	}

	void Window::GetWindowSize(int* outWidth, int* outHeight)
	{
		glfwGetWindowSize(mNativeWindow, outWidth, outHeight);
	}
	void Window::GetCursorPosition(double* outX, double* outY)
	{
		glfwGetCursorPos(mNativeWindow, outX, outY);
	}

	void Window::GetViewportCursorPosition(const double vpPositionX, const double vpPositionY, const double vpSizeX, const double vpSizeY, double* outX, double* outY)
	{
		int width, height;
		GetWindowSize(&width, &height);

		double cursorX, cursorY;
		GetCursorPosition(&cursorX, &cursorY);

		double relativeX = cursorX - vpPositionX;
		double relativeY = cursorY - vpPositionY;
		double normalizedX = relativeX / vpSizeX;
		double normalizedY = relativeY / vpSizeY;

		*outX = (double)width * normalizedX;
		*outY = (double)height * normalizedY;
	}

	void* Window::GetNativeWindow()
	{
		#ifdef PLATFORM_WINDOWS
		return (void*)glfwGetWin32Window(mNativeWindow);
		#else
		COSMOS_LOG(LogSeverity::Todo, "Use platform equivalent");
		return nullptr;
		#endif
	}
}