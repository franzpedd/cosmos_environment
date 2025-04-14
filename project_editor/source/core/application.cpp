#include "core/application.h"

namespace Cosmos::Editor
{
	Application::Application()
	{
		mDockspace = new Dockspace(this);
		mGUI.AddWidget(mDockspace);

		mDemo = new Demo(this);
		mGUI.AddWidget(mDemo);

	}

	Application::~Application()
	{

	}
}