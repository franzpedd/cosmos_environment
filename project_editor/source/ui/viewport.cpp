#include "ui/viewport.h"

namespace Cosmos::Editor
{
    Viewport::Viewport(Application* app)
        : Widget("Viewport", true), mGizmo(app->GetRendererRef()), mApp(app)
    {
        COSMOS_LOG(LogSeverity::Todo, "Update camera aspect ration uppon resize event");
		COSMOS_LOG(LogSeverity::Todo, "Update gizmo on selected entity");
		CreateGridResources();
    }

	Viewport::~Viewport()
	{
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)mApp->GetRendererRef().GetContext()->backend;

		vkDeviceWaitIdle(renderer->device.device);
		crenvk_pipeline_destroy(renderer->device.device, mGrid.crenPipeline);
		vkDestroyDescriptorPool(renderer->device.device, mGrid.descPool, NULL);
	}

    void Viewport::OnUpdate()
    {
        CRenVulkanBackend* rendererBackend = (CRenVulkanBackend*)mApp->GetRendererRef().GetContext()->backend;
		ImGui::Begin("Viewport");
        
		ImVec2 position = ImGui::GetWindowPos();
        
		ImVec2 vpRegion = ImGui::GetContentRegionAvail();

		ImGui::Image((ImTextureID)rendererBackend->viewportRenderphase.descriptorSet, vpRegion);
        
		ImVec2 vpSize = ImGui::GetWindowSize();
        
		DrawMenu(position.x, position.y);
		mGizmo.OnUpdate(NULL); // modify this when entity-selection is enabled
        
        ImGui::End();
    }

    void Viewport::OnRender(int stage)
    {
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)mApp->GetRendererRef().GetContext()->backend;

        // draw grid
		if (mGrid.visible && stage != Picking) {
			unsigned int currentFrame = renderer->device.currentFrame;
			VkDeviceSize offsets[] = { 0 };
			VkCommandBuffer cmdbuffer = renderer->viewportRenderphase.renderpass->commandBuffers[currentFrame];
			
			vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGrid.crenPipeline->pipeline);
			vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGrid.crenPipeline->layout, 0, 1, &mGrid.descSets[currentFrame], 0, nullptr);
			vkCmdDraw(cmdbuffer, 6, 1, 0, 0);
		}
    }

    void Viewport::OnResize(int width, int height)
    {
        // viewport is internally-resized
    }

	void Viewport::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
	{
		CRenCamera& camera = mApp->GetRendererRef().GetContext()->camera;

		if (keycode == Input::KEYCODE_Z) {
			if (camera.shouldMove) {
				mApp->GetWindowRef().ToogleCursor(false);
				mApp->GetGUIRef().ToggleCursor(false);
				camera.shouldMove = 0;
			}

			else if (!camera.shouldMove) {
				mApp->GetWindowRef().ToogleCursor(true);
				mApp->GetGUIRef().ToggleCursor(true);
				camera.shouldMove = 1;
			}
		}
	}

    void Viewport::DrawMenu(float xpos, float ypos)
    {
		ImGui::SetNextWindowPos(ImVec2(xpos + 15.0f, ypos + 35.0f));

		ImGui::BeginChild("##ViewportMenubar");

		ImVec4 activeCol = { 1.0f, 1.0f, 1.0f, 0.5f };

		// gizmo
		static unsigned int selectedGizmos = 0;
		Gizmo::Mode modes[4] = { Gizmo::Mode::Undefined, Gizmo::Mode::Translate, Gizmo::Mode::Rotate, Gizmo::Mode::Scale };
		std::string texts[4] = { ICON_LC_MOUSE_POINTER, ICON_LC_MOVE_3D, ICON_LC_ROTATE_3D, ICON_LC_SCALE_3D };
		std::string tooltips[4] = { "Selection", "Translation", "Rotation", "Scale" };

		for (unsigned short i = 0; i < 4; i++) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);

			bool coloredButton = selectedGizmos == i;

			if (coloredButton) {
				ImGui::PushStyleColor(ImGuiCol_Button, activeCol);
			}

			if (ImGui::Button(texts[i].c_str())) {
				mGizmo.SetMode(modes[i]);
				selectedGizmos = i;
			}

			if (coloredButton) {
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("%s", tooltips[i].c_str());
			}

			ImGui::SameLine();
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
		
		// grid
		ImGui::PushItemWidth(50.0f);
		ImGui::PushStyleVar(20, 2.0f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		
		bool selectedButton = false;
		float snapping = mGizmo.GetSnappingValue();
		if(ImGui::SliderFloat("##Snapping", &snapping, 0.005f, 10.0f, "%.2f")) { mGizmo.SetSnappingValue(snapping); }
		
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Grid snapping value"); }
		
		ImGui::SameLine();
		
		bool selectedSnapping = mGizmo.GetSnapping();
		selectedButton = selectedSnapping;
		if (selectedSnapping) {
			ImGui::PushStyleColor(ImGuiCol_Button, activeCol);
		}
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		
		if (ImGui::Button(ICON_LC_MAGNET)) { mGizmo.SetSnapping(!mGizmo.GetSnapping()); }
		
		if (selectedButton) { ImGui::PopStyleColor(); }
		
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Enables/Disables snapping with the grid"); }
		
		ImGui::SameLine();
		
		static bool selectedGrid = true;
		selectedButton = selectedGrid;
		if (selectedButton) {
			ImGui::PushStyleColor(ImGuiCol_Button, activeCol);
		}
		
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5.0f);
		
		if (ImGui::Button(ICON_LC_GRID_3X3)) {
			mGrid.visible = !mGrid.visible;
			selectedGrid = !selectedGrid;
		}
		
		if (selectedButton) { ImGui::PopStyleColor(); }
		
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Enables/Disables grid on viewport"); }
		
		ImGui::EndChild();
    }

	void Viewport::CreateGridResources()
	{
		CRenVulkanBackend* renderer = (CRenVulkanBackend*)mApp->GetRendererRef().GetContext()->backend;

		// create grid pipeline
		char vert[CREN_PATH_MAX_SIZE], frag[CREN_PATH_MAX_SIZE];
		cren_get_path("shader/compiled/grid.vert.spv", "data", 0, vert, sizeof(vert));
		cren_get_path("shader/compiled/grid.frag.spv", "data", 0, frag, sizeof(frag));
		
		vkPipelineCreateInfo pipeCI = {};
		pipeCI.renderpass = renderer->viewportRenderphase.renderpass;
		pipeCI.vertexShader = crenvk_shader_create(renderer->device.device, "Grid.vert", vert, vkShaderType::SHADER_TYPE_VERTEX);
		pipeCI.fragmentShader = crenvk_shader_create(renderer->device.device, "Grid.frag", frag, vkShaderType::SHADER_TYPE_FRAGMENT);
		pipeCI.vertexComponentsCount = 0;
		pipeCI.passingVertexData = false;
		pipeCI.bindingsCount = 1;
		pipeCI.bindings[0].binding = 0;
		pipeCI.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pipeCI.bindings[0].descriptorCount = 1;
		pipeCI.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pipeCI.bindings[0].pImmutableSamplers = NULL;

		mGrid.crenPipeline = crenvk_pipeline_create(renderer->device.device, &pipeCI);
		crenvk_pipeline_build(renderer->device.device, mGrid.crenPipeline);

		// create descriptor pool and descriptor sets
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = CREN_CONCURRENTLY_RENDERED_FRAMES;

		VkDescriptorPoolCreateInfo descPoolCI = {};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = 1;
		descPoolCI.pPoolSizes = &poolSize;
		descPoolCI.maxSets = CREN_CONCURRENTLY_RENDERED_FRAMES;
		COSMOS_ASSERT(vkCreateDescriptorPool(renderer->device.device, &descPoolCI, nullptr, &mGrid.descPool) == VK_SUCCESS, "Failed to create descriptor pool");
		
		std::vector<VkDescriptorSetLayout> layouts(CREN_CONCURRENTLY_RENDERED_FRAMES, mGrid.crenPipeline->descriptorSetLayout);

		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = mGrid.descPool;
		descSetAllocInfo.descriptorSetCount = CREN_CONCURRENTLY_RENDERED_FRAMES;
		descSetAllocInfo.pSetLayouts = layouts.data();

		mGrid.descSets.resize(CREN_CONCURRENTLY_RENDERED_FRAMES);
		COSMOS_ASSERT(vkAllocateDescriptorSets(renderer->device.device, &descSetAllocInfo, mGrid.descSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets");

		for (size_t i = 0; i < CREN_CONCURRENTLY_RENDERED_FRAMES; i++) {

			vkBuffer* crenBuffer = (vkBuffer*)crenhashtable_lookup(renderer->buffersLib, "Camera");
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = crenBuffer->buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(vkBufferCamera);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mGrid.descSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(renderer->device.device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}