#pragma once

#include "core/input.h"

// forward declarations
struct GLFWwindow;
namespace Cosmos { class Application; }

namespace Cosmos
{
	class Window
	{
	public:

		/// @brief constructs the window and set's up it's resources 
		Window(Application* app, const char* title, int width, int height);

		/// @brief shutsdown the window and it's resources
		~Window();

		/// @brief returns the window's underneath raw pointer, casted as void* so unnecessary includes can be avoided
		inline GLFWwindow* GetGLFWWindow() { return mNativeWindow; }

		/// @brief returns the window's width
		inline const int GetWidth() const { return mWidth; }

		/// @brief returns the window's height
		inline const int GetHeight() const { return mHeight; }

		/// @brief returns if a close request has been made and the window should be closed
		inline bool const ShouldClose() const { return mShouldClose; }

		/// @brief returns if the window is currently minimized
		inline bool const IsMinimized() const { return mMinimized; }

	public:

		/// @brief updates the input/output window devices as well as the windows events, call this at the begining of the frame
		void OnUpdate();

		/// @brief hinds/shows the cursor on the window (locks it within the window if hidden)
		void ToogleCursor(bool hide);

		/// @brief returns if a key is currently pressed
		bool IsKeyDown(Input::Keycode key);

		/// @brief returns the clock's time
		unsigned long long GetTimer();

		/// @brief returns the timer's frequency
		unsigned long long GetTimerFrequency();

		/// @brief returns the window's current aspect ratio ( or 1.0 if height is 0)
		float GetFramebufferAspectRatio();

		/// @brief returns the framebuffer size
		void GetFramebufferSize(int* outWidth, int* outHeight);

		/// @brief returns the window size
		void GetWindowSize(int* outWidth, int* outHeight);

		/// @brief returns the cursor position, relative to the window's size
		void GetCursorPosition(double* outX, double* outY);

		/// @brief returns the cursor position, relative to a custom viewport/framebuffer specified
		void GetViewportCursorPosition(const double vpPositionX, const double vpPositionY, const double vpSizeX, const double vpSizeY, double* outX, double* outY);

		/// @brief returns the native os window object
		void* GetNativeWindow();

	protected:

		Application* mApp;
		GLFWwindow* mNativeWindow = nullptr; // glfw's window is underneath this void*
		const char* mTitle;
		int mWidth;
		int mHeight;
		bool mShouldClose = false;
		bool mMinimized = false;
		double mLastMousePosX = 0.0;
		double mLastMousePosY = 0.0;
	};
}