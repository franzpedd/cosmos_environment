#pragma once

#include "ui/widget.h"
#include "util/stack.h"

// forward declarations
namespace Cosmos { class Application; }
namespace Cosmos { class Widget; }

namespace Cosmos
{
    class GUI
    {
    public:

        /// @brief setup and prepare all needed resources
        GUI(Application* app);

        /// @brief release all used resources
        ~GUI();

    public:

        /// @brief called when updating the ui widgets
        void OnUpdate();

        /// @brief called when rendering the ui widgets
        void OnRender();

		/// @brief adds a new widget to the widgets list
		void AddWidget(Widget* widget);

		/// @brief returns the first widget by it's name or NULL if not found
		Widget* FindWidgetByName(const char* name);

	public:

		// hides/unhides the mouse cursor from the ui
		void ToggleCursor(bool hide);

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

	public:

		/// @brief informs the ui-backend that the quantity of images in the swapchain has changed
		void SetMinImageCount(unsigned int count);

		/// @brief informs the ui-backend that it's time to draw it's data
		void DrawRawData(void* commandbuffer);

	private:

		/// @brief set's the spectrum style
		void SetSpectrumStyle();

    private:

		Application* mApp;
		Stack<Widget*> mWidgets;
    };
}