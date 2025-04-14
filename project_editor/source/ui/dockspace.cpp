#include "ui/dockspace.h"

namespace Cosmos::Editor
{
    Dockspace::Dockspace(Application* app)
        : Widget("Dockspace", true), mApp(app)
    {
    }

    void Dockspace::OnUpdate()
    {
		float2 position = {};
		UI::Params::GetMainViewportPos(&position.x, &position.y);

		float2 size = {};
		UI::Params::GetMainViewportSize(&size.x, &size.y);

		UI::Params::SetNextWindowPosition(position.x, position.y);
		UI::Params::SetNextWindowSize(size.x, size.y);

		int flags = 1;		// ImGuiWindowFlags_NoTitleBar
		flags |= 2;			// ImGuiWindowFlags_NoResize
		flags |= 4;			// ImGuiWindowFlags_NoMove
		flags |= 32;		// ImGuiWindowFlags_NoCollapse
		flags |= 8192;		// ImGuiWindowFlags_NoBringToFrontOnFocus
		flags |= 131072;	// ImGuiWindowFlags_NoNavFocus

		UI::Begin("Dockspace", 0, flags);
		UI::Dockspace(UI::Params::GetWidgetID("MyDockSpace"));
		UI::End();
    }
}