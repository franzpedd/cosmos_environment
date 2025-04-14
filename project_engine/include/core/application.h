#pragma once

#include "core/renderer.h"
#include "core/window.h"
#include "ui/gui.h"

namespace Cosmos
{
	class Application 
	{
	public:
	
		/// @brief constructs and initializes the engine and it's core components
		Application();

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

	protected:

		Window mWindow;
		Renderer mRenderer;
		GUI mGUI;

		double mTimeStep = 0.0;
		int mAverageFPS = 0;
	};
}