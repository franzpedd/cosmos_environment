#pragma once

#include "core/renderer.h"
#include "core/window.h"
#include "ui/gui.h"

namespace Cosmos
{
	class Application 
	{
	public:

		/// @brief requested information to properly initialize an application
		struct CreateInfo
		{
			/// @brief this is the application name, used in the window as well as internally by the renderer
			const char* appName = nullptr;

			/// @brief tells the renderer we're going to create a customized viewport that is pottentially different than the one provided by cren (wich covers the entire window)
			bool requestViewport = false;

			/// @brief tells the renderer we're going to need validations (not a good idea on an physical android device, only emulated)
			bool requestValidations = true;

			/// @brief tells the window manager that the application will cover the entire window area
			bool requestFullscreen = false;
			
			/// @brief tells the window manager it's width size, this can latter be changed
			int width = 1366;

			/// @brief tells the window manager it's height size, this can latter be changed
			int height = 768;
		};

	public:
	
		/// @brief constructs and initializes the engine and it's core components
		Application(const CreateInfo& ci);

		/// @brief terminates and shutsdown the engine and it's core components
		~Application();

		/// @brief returns a reference to the window underneath the application
		inline Window& GetWindowRef() { return mWindow; }

		/// @brief returns a reference to the renderer context/class
		inline Renderer& GetRendererRef() { return mRenderer; }

		/// @brief returns a reference to the general user interface context/class
		inline GUI& GetGUIRef() { return mGUI; }

		/// @brief returns the elapsed-time between two frames
		inline double GetTimestep() { return mTimeStep; }

		/// @brief returns the average fps 
		inline int GetAverageFPS() { return mAverageFPS; }
		
	public:

		/// @brief begins the updating cycle
		void Run();
		
		/// @brief hints the closing of the application
		void Quit();

	public:

		/// @brief this is called by the window, signaling it's iconification
		void OnMinimize();

		/// @brief this is called by the window, signaling it's size is not iconified/minimized anymore
		void OnRestore(int width, int height);

		/// @brief this is called by the window, signaling the application and it's components to be resized to a new window size
		void OnResize(int width, int height);

		/// @brief this is called by the window, signaling a key was pressed with/without a modfier and being held for a while or not
		void OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held);

		/// @brief this is called by the window, signaling a previously pressed key was released
		void OnKeyRelease(Input::Keycode keycode);

		/// @brief this is called by the window, signaling a button was pressed with/without mods
		void OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod);

		/// @brief this is called by the window, signaling a previously pressed button was released
		void OnButtonRelease(Input::Buttoncode buttoncode);

		/// @brief this is called by the window, signaling the mouse scroll was 'scroll'
		void OnMouseScroll(double xoffset, double yoffset);

		/// @brief this is called by the window, signaling the mouse was moved to a new location
		void OnMouseMove(double xpos, double ypos);

		/// @brief this is called by the window, the dots per inch has changed
		void OnDPIChange(float scale);

	protected:

		Window mWindow;
		Renderer mRenderer;
		GUI mGUI;

		double mTimeStep = 0.0;
		int mAverageFPS = 0;
	};
}