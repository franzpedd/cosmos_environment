#pragma once

#include "core/input.h"

namespace Cosmos
{
	class Widget
	{
	public:

		// constructor
		Widget(const char* name, bool visible = false) : mName(name), mVisible(visible) {};

		// destructor
		virtual ~Widget() = default;

		// returns it's name
		inline const char* GetName() { return mName; }

		// returns if the widget is visible/displaying
		inline bool GetVisibility() { return mVisible; }

		// sets the widget visibility
		inline void SetVisibility(bool value) { mVisible = value; }

	public:

		// user interface drawing
		inline virtual void OnUpdate() {};

		// renderer drawing
		inline virtual void OnRender() {};

	public:

		/// @brief this is called by the window, signaling it's iconification
		inline virtual void OnMinimize() {};

		/// @brief this is called by the window, signaling it's size is not iconified/minimized anymore
		inline virtual void OnRestore(int width, int height) {};

		/// @brief this is called by the window, signaling the application and it's components to be resized to a new window size
		inline virtual void OnResize(int width, int height) {};

		/// @brief this is called by the window, signaling a key was pressed with/without a modfier and being held for a while or not
		inline virtual void OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held) {};

		/// @brief this is called by the window, signaling a previously pressed key was released
		inline virtual void OnKeyRelease(Input::Keycode keycode) {};

		/// @brief this is called by the window, signaling a button was pressed with/without mods
		inline virtual void OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod) {};

		/// @brief this is called by the window, signaling a previously pressed button was released
		inline virtual void OnButtonRelease(Input::Buttoncode buttoncode) {};

		/// @brief this is called by the window, signaling the mouse scroll was 'scroll'
		inline virtual void OnMouseScroll(double xoffset, double yoffset) {};

		/// @brief this is called by the window, signaling the mouse was moved to a new location
		inline virtual void OnMouseMove(double xpos, double ypos) {};

	protected:

		const char* mName = nullptr;
		bool mVisible = false;
	};
}

namespace Cosmos::UI
{
	/// @brief starts a window context
	/// @param name unique window name
	/// @param open variable to address the window's visibility to
	/// @param flags ImGuiWindowFlags equivalent
	void Begin(const char* name, bool* open = nullptr, int flags = 0);

	/// @brief ends the window context
	void End();

	/// @brief display unicode text
	/// @param fmt va_args of the text
	void Text(const char* fmt, ...);

	/// @brief displays a text center-aligned
	/// @param text the text to display
	void TextCentered(const char* fmt, ...);

	/// @brief draws a separator
	/// @param flags ImGuiSeparatorFlags equivalent
	/// @param thickness the sperator's thickness
	void Separator(int flags, float thickness = 1.0f);

	/// @brief draws a separator with a text in it
	/// @param txt the text that'll apear in the separator
	void SeparatorText(const char* txt);

	/// @brief draws a 3 component float controller
	/// @param label float's label/unique id
	/// @param x the first value
	/// @param y the second value
	/// @param z the third value
	void Float3Controller(const char* label, float* x, float* y, float* z);

	/// @brief draws a 2 component float controller
	/// @param label float's label/unique id
	/// @param x the first value
	/// @param y the second value
	void Float2Control(const char* label, float* x, float* y);

	/// @brief draws a float controller
	/// @param label float label/unique id
	/// @param value the controller's value
	void FloatControl(const char* label, float* value);

	/// @brief displays a checkbox field
	/// @param label checkbox text to be displayed
	/// @param v boolean value that refers the checkbox
	bool Checkbox(const char* label, bool* v);

	/// @brief displays a checkbox slider field
	/// @param label checkbox text to be displayed
	/// @param v boolean value that refers the checkbox
	bool CheckboxSliderEx(const char* label, bool* v);

	/// @brief displays the imgui window demo
	void ShowDemo();

	/// @brief enables dockspace on the current window, on the entire window area
	/// @param id the dockspace's id
	void Dockspace(unsigned int id);
}

namespace Cosmos::UI::Params
{
	/// @brief sets the next window's position
	/// @param x x-axis coordinate
	/// @param y y-axis coordinate
	void SetNextWindowPosition(float x, float y);

	/// @brief sets the next window's size
	/// @param x width 
	/// @param y height
	void SetNextWindowSize(float x, float y);

	/// @brief returns the main viewport size
	/// @param x viewport's width
	/// @param y viewport's height
	void GetMainViewportSize(float* x, float* y);

	/// @brief returns the main viewport position
	/// @param x viewport's x position
	/// @param y viewport's y position
	void GetMainViewportPos(float* x, float* y);

	/// @returns an id given the unique string of a widget
	/// @param str the unique window's string/name
	unsigned int GetWidgetID(const char* str);
}