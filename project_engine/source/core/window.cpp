#include "core/window.h"

#include "core/application.h"
#include "core/input.h"
#include "core/logger.h"

#include <cren.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <backends/imgui_impl_sdl3.h>

namespace Cosmos
{
	static unsigned char* sIcon = nullptr; // holds the icon image ptr, this is here be

	Window::Window(Application* app, const char* title, int width, int height, bool fullscreen)
		: mApp(app), mTitle(title), mWidth(width), mHeight(height)
	{
		if (SDL_Init(SDL_INIT_VIDEO) == false) {
			COSMOS_LOG(LogSeverity::Error, "SDL could not initialize. Error message: %s", SDL_GetError());
			return;
		}

		SDL_WindowFlags flags = fullscreen == true ? SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_FULLSCREEN
			: SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

		mNativeWindow = SDL_CreateWindow(title, width, height, flags);

		if (!mNativeWindow) {
			COSMOS_LOG(LogSeverity::Error, "Window could not be created. Error message: %s", SDL_GetError());
			SDL_Quit();
			return;
		}

		COSMOS_LOG(LogSeverity::Todo, "Implement window icon");
		COSMOS_LOG(LogSeverity::Todo, "Implement touch events");

		char iconPath[128];
		cren_get_path("texture/logo.png", "data", 0, iconPath, sizeof(iconPath));
		
		//int iconWidth, iconHeight, iconChannels;
		//if (sIcon) cren_stbimage_destroy(sIcon);
		//sIcon = cren_stbimage_load_from_file(iconPath, 4, &iconWidth, &iconHeight, &iconChannels);
		//if (!sIcon) COSMOS_LOG(LogSeverity::Error, "%s", cren_stbimage_get_error());
		
		//GLFWimage icons[1];
		//icons[0].pixels = sIcon;
		//icons[0].width = iconWidth;
		//icons[0].height = iconHeight;
		//glfwSetWindowIcon(mNativeWindow, 1, icons);
	}

	Window::~Window()
	{
		SDL_DestroyWindow((SDL_Window*)mNativeWindow);
		SDL_Quit();
	}

	void Window::OnUpdate()
	{
		SDL_Event event = { 0 };

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			switch (event.type)
			{
				case SDL_EVENT_QUIT:
				{
					mShouldClose = true;
					break;
				}

				case SDL_EVENT_KEY_DOWN:
				{
					mApp->OnKeyPress((Input::Keycode)event.key.scancode, (Input::Keymod)event.key.mod, false);
					break;
				}

				case SDL_EVENT_KEY_UP:
				{
					mApp->OnKeyRelease((Input::Keycode)event.key.scancode);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					mApp->OnButtonPress((Input::Buttoncode)event.button.button, Input::Keymod::KEYMOD_NONE);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					mApp->OnButtonRelease((Input::Buttoncode)event.button.button);
					break;
				}

				case SDL_EVENT_MOUSE_WHEEL:
				{
					mApp->OnMouseScroll((double)event.wheel.x, -event.wheel.y);
					break;
				}

				case SDL_EVENT_MOUSE_MOTION:
				{
					mApp->OnMouseMove((double)event.motion.xrel, (double)event.motion.yrel);
					break;
				}

				case SDL_EVENT_WINDOW_RESIZED:
				{
					mWidth = event.window.data1;
					mHeight = event.window.data2;
					mApp->OnResize(event.window.data1, event.window.data2);
					break;
				}

				case SDL_EVENT_WINDOW_MINIMIZED:
				{
					mMinimized = true;
					mApp->OnMinimize();
					break;
				}

				case SDL_EVENT_WINDOW_RESTORED:
				{
					mMinimized = false;
					mApp->OnRestore(mWidth, mHeight);
					break;
				}

				case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				{
					if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
						mApp->OnDPIChange(SDL_GetWindowDisplayScale(mNativeWindow));
					}

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

	void Window::ToogleCursor(bool hide)
	{
		SDL_ShowCursor();
	}

	bool Window::IsKeyDown(Input::Keycode key)
	{
		const bool* keyboardState = SDL_GetKeyboardState(NULL);
		return keyboardState[key] == 1;
	}

	unsigned long long Window::GetTimer()
	{
		return SDL_GetPerformanceCounter();
	}

	unsigned long long Window::GetTimerFrequency()
	{
		return SDL_GetPerformanceFrequency();
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
		SDL_GetWindowSizeInPixels(mNativeWindow, outWidth, outHeight);
	}

	void Window::GetWindowSize(int* outWidth, int* outHeight)
	{
		SDL_GetWindowSize(mNativeWindow, outWidth, outHeight);
	}

	void Window::GetCursorPosition(float* outX, float* outY)
	{
		SDL_GetMouseState(outX, outY);
	}

	void Window::GetViewportCursorPosition(const double vpPositionX, const double vpPositionY, const double vpSizeX, const double vpSizeY, double* outX, double* outY)
	{
		int width, height;
		GetWindowSize(&width, &height);

		float cursorX, cursorY;
		GetCursorPosition(&cursorX, &cursorY);

		double relativeX = (double)cursorX - vpPositionX;
		double relativeY = (double)cursorY - vpPositionY;
		double normalizedX = relativeX / vpSizeX;
		double normalizedY = relativeY / vpSizeY;

		*outX = (double)width * normalizedX;
		*outY = (double)height * normalizedY;
	}

	void* Window::GetNativeWindow()
	{
		#ifdef PLATFORM_WINDOWS
		return SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)mNativeWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		#elif defined PLATFORM_ANDROID
		return SDL_GetPointerProperty(SDL_GetWindowProperties((SDL_Window*)mNativeWindow), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
		#else 
		COSMOS_LOG(LogSeverity::Todo, "Use platform equivalent");
		return nullptr;
		#endif
	}
}