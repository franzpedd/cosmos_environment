#include "ui/gui.h"
#include "core/application.h"

#include "core/logger.h"
#include "ui/icon.h"
#include "ui/theme.h"
#include "ui/wrapper_imgui.h"

#include <cren_error.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

#include <backends/imgui_impl_sdl3.cpp>
#include <backends/imgui_impl_vulkan.cpp>
#include <imguizmo/imguizmo.h>

// fonts
#include <lucide.c>
#include <awesome.c>
#include <robotomono_medium.c>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Cosmos
{
	static int Internal_SDL3_CreateVulkanSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
	{
		ImGui_ImplSDL3_ViewportData* vd = (ImGui_ImplSDL3_ViewportData*)viewport->PlatformUserData;
		(void)vk_allocator;

		// lets use SDL's surface creation to easy-up things. Could potentially use cren_surface_create
		bool ret = SDL_Vulkan_CreateSurface(vd->Window, (VkInstance)vk_instance, NULL, (VkSurfaceKHR*)out_vk_surface);
		return ret ? 0 : 1; // ret ? VK_SUCCESS : VK_NOT_READY 
	}

	static ImFont* sIconFA = nullptr;
	static ImFont* sIconLC = nullptr;
	static ImFont* sRobotoMono = nullptr;

	GUI::GUI(Application* app)
		: mApp(app)
	{
		// initial config
		IMGUI_CHECKVERSION();
		mContext = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		#ifdef PLATFORM_ANDROID
		io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
		#endif

		if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports) {
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

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
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_CreateVkSurface = Internal_SDL3_CreateVulkanSurface;

		ImGui::SetCurrentContext((ImGuiContext*)mContext);
		ImGui_ImplSDL3_InitForVulkan(mApp->GetWindowRef().GetAPIWindow());

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
		// fonts
		constexpr const ImWchar iconRanges1[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		constexpr const ImWchar iconRanges2[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		#ifdef PLATFORM_ANDROID
		CREN_LOG("[Android:Todo]: Figure out an automate way to resize things, maybe wait ImGui-texture branch?");
		constexpr float iconSize = 13.0f;
		constexpr float fontSize = 18.0f;
		#else
		constexpr float iconSize = 13.0f;
		constexpr float fontSize = 18.0f;
		#endif

		ImFontConfig iconCFG;
		iconCFG.MergeMode = true;
		iconCFG.GlyphMinAdvanceX = iconSize;
		iconCFG.PixelSnapH = true;

		sRobotoMono = io.Fonts->AddFontFromMemoryCompressedTTF(txt_robotomono_medium_compressed_data, txt_robotomono_medium_compressed_size, fontSize);
		sIconFA = io.Fonts->AddFontFromMemoryCompressedTTF(icon_awesome_compressed_data, icon_awesome_compressed_size, iconSize, &iconCFG, iconRanges1);
		sIconLC = io.Fonts->AddFontFromMemoryCompressedTTF(icon_lucide_compressed_data, icon_lucide_compressed_size, iconSize, &iconCFG, iconRanges2);
		io.Fonts->Build();
		ImGui_ImplVulkan_CreateFontsTexture();

	}

	GUI::~GUI()
	{
		CRenContext* renderer = mApp->GetRendererRef().GetContext();
		CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)renderer->backend;
		vkDeviceWaitIdle(rendererBackend->device.device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();

		for (auto& widget : mWidgets.GetElementsRef()) {
			delete widget;
		}

		mWidgets.GetElementsRef().clear();
		ImGui::DestroyContext((ImGuiContext*)mContext);
	}

	void GUI::OnUpdate()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

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

	void GUI::OnRender(int stage)
	{
		for (auto& widget : mWidgets.GetElementsRef()) {
			widget->OnRender(stage);
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
		//int pixel_width, pixel_height;
		//SDL_GetWindowSizeInPixels(mApp->GetWindowRef().GetAPIWindow(), &pixel_width, &pixel_height);
		//int logical_width, logical_height;
		//SDL_GetWindowSize(mApp->GetWindowRef().GetAPIWindow(), &logical_width, &logical_height);
		//
		//ImGuiIO& io = ImGui::GetIO();
		//io.DisplaySize = ImVec2((float)logical_width, (float)logical_height);
		//io.DisplayFramebufferScale = ImVec2((float)pixel_width / (float)logical_width, (float)pixel_height / (float)logical_height);

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

	void GUI::OnDPIChange(float scale)
	{
		

		// Apply to ImGui
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = scale;
		//ImGui::GetStyle().ScaleAllSizes(scale);

		//// Reload fonts if needed (with scaled size)
		//if (io.Fonts->IsBuilt()) {
		//	io.Fonts->Clear();
		//}

		
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
		style->FramePadding = ImVec2(6.0f, 4.0f);
		style->ItemSpacing = ImVec2(9.0f, 4.0f);
		style->ItemInnerSpacing = ImVec2(3.0f, 3.0f);
		style->TouchExtraPadding = ImVec2(10.0f, 10.0f);
		style->IndentSpacing = 5.0f;
		style->ScrollbarSize = 20.0f;
		style->GrabMinSize = 30.0f;
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


