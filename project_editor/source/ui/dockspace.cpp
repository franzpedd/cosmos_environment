#include "ui/dockspace.h"

namespace Cosmos::Editor
{
    Dockspace::Dockspace(Application* app)
        : Widget("Dockspace", true), mApp(app)
    {
    }

    void Dockspace::OnUpdate()
    {
		ImVec2 position = ImGui::GetMainViewport()->Pos;

		ImVec2 size = ImGui::GetMainViewport()->Size;

		ImGui::SetNextWindowPos(position);
		ImGui::SetNextWindowSize(size);
			
		ImGui::Begin("Dockspace", 0, ImGuiWindowFlags_NoResize 
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus
		);

		ImGui::DockSpace(ImGui::GetID("MyDockspace"));

		ImGui::End();
    }
}