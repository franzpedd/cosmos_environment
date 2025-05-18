#include "core/application.h"

namespace Cosmos::Editor
{
	Application::Application(const CreateInfo& ci) : Cosmos::Application(ci)
	{
		mDockspace = new Dockspace(this);
		mGUI.AddWidget(mDockspace);

		mConsole = new Console();
		mGUI.AddWidget(mConsole);
		mDemo = new Demo(this);
		mGUI.AddWidget(mDemo);

		mViewport = new Viewport(this);
		mGUI.AddWidget(mViewport);
	}

	Application::~Application()
	{

	}
}