#include "ui/demo.h"

namespace Cosmos::Editor
{
	Demo::Demo(Application* app)
		: Widget("Demo Window", true), mApp(app)
	{
	}

	void Demo::OnUpdate()
	{
		float cursorX, cursorY = 0;
		mApp->GetWindowRef().GetCursorPosition(&cursorX, &cursorY);

		CRenContext* renderer = mApp->GetRendererRef().GetContext();
		CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)renderer->backend;

		ImGui::Begin(ICON_FA_INFO_CIRCLE " Debug Info", 0);

		ImGui::SeparatorText("Engine");

		ImGui::Text("Timestep: %f", mApp->GetTimestep());
		ImGui::Text("FPS: %d", mApp->GetAverageFPS());
		ImGui::Text("Mouse Pos: %lf x %lf", cursorX, cursorY);
		
		ImGui::SeparatorText("Renderer");

		ImGui::Text("Cam Pos (%.3f %.3f %.3f)", renderer->camera.viewPosition.x, renderer->camera.viewPosition.y, renderer->camera.viewPosition.z);
		ImGui::Text("Cam Rot: (%.3f %.3f %.3f)", renderer->camera.rotation.x, renderer->camera.rotation.y, renderer->camera.rotation.z);
		ImGui::Text("Cam Front: (%.3f %.3f %.3f)", renderer->camera.frontPosition.x, renderer->camera.frontPosition.y, renderer->camera.frontPosition.z);
		ImGui::Text("Size (Swapchain): %dx%d", rendererBackend->swapchain.swapchainExtent.width, rendererBackend->swapchain.swapchainExtent.height);
		
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
}
