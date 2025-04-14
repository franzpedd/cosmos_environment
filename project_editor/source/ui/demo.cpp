#include "ui/demo.h"

namespace Cosmos::Editor
{
	Demo::Demo(Application* app)
		: Widget("Demo Window", true), mApp(app)
	{
	}

	void Demo::OnUpdate()
	{
		double cursorX, cursorY = 0;
		mApp->GetWindowRef().GetCursorPosition(&cursorX, &cursorY);

		CRenContext* renderer = mApp->GetRendererRef().GetContext();
		CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)renderer->backend;

		UI::Begin(ICON_FA_INFO_CIRCLE " Debug Info", nullptr);

		UI::SeparatorText("Engine");

		UI::Text("Timestep: %f", mApp->GetTimestep());
		UI::Text("FPS: %d", mApp->GetAverageFPS());
		UI::Text("Mouse Pos: %lf x %lf", cursorX, cursorY);
		
		UI::SeparatorText("Renderer");

		UI::Text("Cam Pos (%.3f %.3f %.3f)", renderer->camera.viewPosition.x, renderer->camera.viewPosition.y, renderer->camera.viewPosition.z);
		UI::Text("Cam Rot: (%.3f %.3f %.3f)", renderer->camera.rotation.x, renderer->camera.rotation.y, renderer->camera.rotation.z);
		UI::Text("Cam Front: (%.3f %.3f %.3f)", renderer->camera.frontPosition.x, renderer->camera.frontPosition.y, renderer->camera.frontPosition.z);
		UI::Text("Size (Swapchain): %dx%d", rendererBackend->swapchain.swapchainExtent.width, rendererBackend->swapchain.swapchainExtent.height);
		
		UI::End();

		UI::ShowDemo();
	}
}
