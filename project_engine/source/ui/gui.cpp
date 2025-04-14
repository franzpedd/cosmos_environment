#include "ui/gui.h"
#include "core/application.h"

#include "core/logger.h"
#include "ui/icon.h"
#include "ui/theme.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_glfw.cpp>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

namespace Cosmos
{
	static ImFont* sIconFA = nullptr;
	static ImFont* sIconLC = nullptr;
	static ImFont* sRobotoMono = nullptr;

	GUI::GUI(Application* app)
		: mApp(app)
	{
		// initial config
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports) {
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

		static const ImWchar iconRanges1[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar iconRanges2[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		constexpr float iconSize = 13.0f;
		constexpr float fontSize = 18.0f;

		ImFontConfig iconCFG;
		iconCFG.MergeMode = true;
		iconCFG.GlyphMinAdvanceX = iconSize;
		iconCFG.PixelSnapH = true;

		char roboto[128], awesome[128], lucide[128];
		cren_get_path("font/txt-robotomono-medium.ttf", "../data", 0, roboto, sizeof(roboto));
		cren_get_path("font/icon-awesome.ttf", "../data", 0, awesome, sizeof(awesome));
		cren_get_path("font/icon-lucide.ttf", "../data", 0, lucide, sizeof(lucide));

		sRobotoMono = io.Fonts->AddFontFromFileTTF(roboto, fontSize);
		sIconFA = io.Fonts->AddFontFromFileTTF(awesome, iconSize, &iconCFG, iconRanges1);
		sIconLC = io.Fonts->AddFontFromFileTTF(lucide, iconSize, &iconCFG, iconRanges2);
		io.Fonts->Build();

		io.IniFilename = "UI.ini";
		io.WantCaptureMouse = true;

		ImGui::StyleColorsDark();
		SetSpectrumStyle();

		// create descriptor pool
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		CRenContext* renderer = mApp->GetRendererRef().GetContext();
		CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)renderer->backend;

		// sdl and vulkan initialization
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(mApp->GetWindowRef().GetGLFWWindow(), true);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = rendererBackend->instance.instance;
		initInfo.PhysicalDevice = rendererBackend->device.physicalDevice;
		initInfo.Device = rendererBackend->device.device;
		initInfo.Queue = rendererBackend->device.graphicsQueue;
		initInfo.DescriptorPool = rendererBackend->uiRenderphase.descPool;
		initInfo.MinImageCount = rendererBackend->swapchain.swapchainImageCount;
		initInfo.ImageCount = rendererBackend->swapchain.swapchainImageCount;
		initInfo.MSAASamples = rendererBackend->uiRenderphase.renderpass->msaa;
		initInfo.Allocator = nullptr;
		initInfo.RenderPass = rendererBackend->uiRenderphase.renderpass->renderPass;
		ImGui_ImplVulkan_Init(&initInfo);

		// upload fonts
		ImGui_ImplVulkan_CreateFontsTexture();
	}

	GUI::~GUI()
	{
		CRenContext* renderer = mApp->GetRendererRef().GetContext();
		CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)renderer->backend;
		vkDeviceWaitIdle(rendererBackend->device.device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		for (auto& widget : mWidgets.GetElementsRef()) {
			delete widget;
		}

		mWidgets.GetElementsRef().clear();
		ImGui::DestroyContext();
	}

	void GUI::OnUpdate()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnUpdate();
		}

		// end frame
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUI::OnRender()
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRender();
		}
	}

	void GUI::AddWidget(Widget* widget)
	{
		if (!widget) return;

		if (!FindWidgetByName(widget->GetName())) {
			mWidgets.Push(widget);
		}
	}

	Widget* GUI::FindWidgetByName(const char* name)
	{
		Widget* found = nullptr;

		for (auto& widget : mWidgets.GetElementsRef()) {
			if (widget->GetName() == name) {
				found = widget;
				break;
			}
		}

		return found;
	}

	void GUI::ToggleCursor(bool hide)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (hide) {
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			return;
		}

		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
	}

	void GUI::OnMinimize()
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMinimize();
		}
	}

	void GUI::OnRestore(int width, int height)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRestore(width, height);
		}
	}

	void GUI::OnResize(int width, int height)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnResize(width, height);
		}
	}

	void GUI::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnKeyPress(keycode, mod, held);
		}
	}

	void GUI::OnKeyRelease(Input::Keycode keycode)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnKeyRelease(keycode);
		}
	}

	void GUI::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnButtonPress(buttoncode, mod);
		}
	}

	void GUI::OnButtonRelease(Input::Buttoncode buttoncode)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnButtonRelease(buttoncode);
		}
	}

	void GUI::OnMouseScroll(double xoffset, double yoffset)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMouseScroll(xoffset, yoffset);
		}
	}

	void GUI::OnMouseMove(double xpos, double ypos)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnMouseMove(xpos, ypos);
		}
	}

	void GUI::SetMinImageCount(unsigned int count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void GUI::DrawRawData(void* commandbuffer)
	{
		ImDrawData* data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(data, (VkCommandBuffer)commandbuffer);
	}

	void GUI::SetSpectrumStyle()
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(10.0f, 10.0f);
		style->FramePadding = ImVec2(3.0f, 3.0f);
		style->ItemSpacing = ImVec2(6.0f, 6.0f);
		style->ItemInnerSpacing = ImVec2(3.0f, 3.0f);
		style->TouchExtraPadding = ImVec2(0.0f, 0.0f);
		style->IndentSpacing = 5.0f;
		style->ScrollbarSize = 14.0f;
		style->GrabMinSize = 10.0f;
		style->WindowBorderSize = 0.0f;
		style->ChildBorderSize = 0.0f;
		style->PopupBorderSize = 0.0f;
		style->FrameBorderSize = 0.0f;
		style->WindowRounding = 5.0f;
		style->ChildRounding = 0.0f;
		style->FrameRounding = 3.0f;
		style->PopupRounding = 3.0f;
		style->ScrollbarRounding = 3.0f;
		style->GrabRounding = 3.0f;
		style->TabBorderSize = 1.0f;
		style->TabBarBorderSize = 1.0f;
		style->TabCloseButtonMinWidthSelected = -1.0f;
		style->TabCloseButtonMinWidthUnselected = 8.0f;
		style->TabRounding = 3.0f;
		style->CellPadding = ImVec2(3.0f, 3.0f);
		style->TableAngledHeadersAngle = 10.0f;
		style->TableAngledHeadersTextAlign = ImVec2(0.5f, 0.0f);
		style->WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style->WindowBorderHoverPadding = 10.0f;
		style->WindowMenuButtonPosition = ImGuiDir_Left;
		style->ColorButtonPosition = ImGuiDir_Right;
		style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
		style->SeparatorTextBorderSize = 4.0f;
		style->SeparatorTextAlign = ImVec2(0.0f, 0.5f);
		style->LogSliderDeadzone = 4.0f;
		style->ImageBorderSize = 0.0f;
		style->Alpha = 1.0f;

		ImVec4* colors = style->Colors;

		bool darkMode = true;
		const ImVec4 blank = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		const ImVec4 background  = darkMode ? ImVec4(0.08f, 0.08f, 0.08f, 0.86f) : ImVec4(0.69f, 0.69f, 0.69f, 1.0f);
		const ImVec4 framing = darkMode ? ImVec4(0.19f, 0.19f, 0.19f, 0.33f) : ImVec4(0.80f, 0.80f, 0.80f, 0.33f);
		const ImVec4 text = darkMode ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

		// text
		{
			colors[ImGuiCol_Text] = text;
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.50f, 0.0f, 0.50f, 1.0f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.92f, 0.92f, 0.89f, 1.0f);
			colors[ImGuiCol_TextLink] = ImVec4(0.35f, 0.35f, 0.92f, 0.86f);
		}
		// windows
		{
			colors[ImGuiCol_WindowBg] = background;
			colors[ImGuiCol_ChildBg] = blank;
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
			colors[ImGuiCol_MenuBarBg] = blank;
		}
		// titles
		{
			colors[ImGuiCol_TitleBg] = background;
			colors[ImGuiCol_TitleBgActive] = background;
			colors[ImGuiCol_TitleBgCollapsed] = background;
		}
		// frames
		{
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			const ImVec4 active = ImVec4(0.43f, 0.43f, 0.43f, 1.0f);

			colors[ImGuiCol_FrameBg] = framing;
			colors[ImGuiCol_FrameBgHovered] = hovered;
			colors[ImGuiCol_FrameBgActive] = active;
			colors[ImGuiCol_InputTextCursor] = hovered;
		}
		// decorations
		{
			const ImVec4 idle = ImVec4(1.0f, 1.0f, 1.0f, 0.27f);
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 0.95f);
			const ImVec4 active = ImVec4(1.0f, 1.0f, 1.0f, 0.50f);

			colors[ImGuiCol_Border] = blank;
			colors[ImGuiCol_BorderShadow] = blank;
			colors[ImGuiCol_ScrollbarBg] = blank;
			colors[ImGuiCol_ScrollbarGrab] = framing;
			colors[ImGuiCol_ScrollbarGrabHovered] = idle;
			colors[ImGuiCol_ScrollbarGrabActive] = idle;
			colors[ImGuiCol_Separator] = idle;
			colors[ImGuiCol_ResizeGrip] = idle;
			colors[ImGuiCol_ResizeGripHovered] = hovered;
			colors[ImGuiCol_ResizeGripActive] = active;
			colors[ImGuiCol_SliderGrab] = idle;
			colors[ImGuiCol_SliderGrabActive] = active;
			colors[ImGuiCol_CheckMark] = active;
		}
		// widgets
		{
			const ImVec4 idle = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
			const ImVec4 header = ImVec4(0.0f, 0.0f, 0.0f, 0.31f);
			const ImVec4 hovered = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
			const ImVec4 active = ImVec4(0.43f, 0.43f, 0.43f, 1.0f);

			colors[ImGuiCol_Button] = header;
			colors[ImGuiCol_ButtonHovered] = hovered;
			colors[ImGuiCol_ButtonActive] = active;
			colors[ImGuiCol_Header] = header;
			colors[ImGuiCol_HeaderActive] = framing;
			colors[ImGuiCol_HeaderHovered] = framing;
			colors[ImGuiCol_Tab] = header;
			colors[ImGuiCol_TabHovered] = hovered;
			colors[ImGuiCol_TabSelected] = active;
			colors[ImGuiCol_TabSelectedOverline] = active;
			colors[ImGuiCol_TabDimmed] = hovered;
			colors[ImGuiCol_TabDimmedSelected] = active;
			colors[ImGuiCol_TabDimmedSelectedOverline] = active;
			colors[ImGuiCol_DragDropTarget] = header;
			colors[ImGuiCol_NavCursor] = header;
			colors[ImGuiCol_NavWindowingHighlight] = header;
			colors[ImGuiCol_NavWindowingDimBg] = hovered;
			colors[ImGuiCol_ModalWindowDimBg] = active;
			colors[ImGuiCol_PlotLines] = text;
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 1.0f, 0.35f, 1.0f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 1.0f, 0.7f, 1.0f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.23f, 1.0f);
			colors[ImGuiCol_TableRowBg] = blank;
			colors[ImGuiCol_TableRowBgAlt] = blank;
			colors[ImGuiCol_DockingPreview] = idle;
			colors[ImGuiCol_DockingEmptyBg] = hovered;
		}
	}
}


