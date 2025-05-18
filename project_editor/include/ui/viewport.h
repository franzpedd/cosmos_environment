#pragma once

#include <cosmos.h>

namespace Cosmos::Editor
{
	class Viewport : public Cosmos::Widget
	{
	public:

		// constructor
		Viewport(Application* app);

		// destructor
		virtual ~Viewport();

	public:

		/// @brief updates the ui logic
		virtual void OnUpdate() override;

		/// @brief right-place to draw/render objects related to the viewport
		virtual void OnRender(int stage) override;

		/// @brief the application was resized, must also resize the viewport
		virtual void OnResize(int width, int height) override;

		/// @brief a key was pressed, must address it
		virtual void OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held) override;

	private:

		/// @brief displays a menu within the viewport
		void DrawMenu(float xpos, float ypos);

		/// @brief create and setup the grid resources
		void CreateGridResources();

	private:

		Application* mApp = nullptr;
		Gizmo mGizmo;
		
		struct Grid
		{
			bool visible = true;
			vkShader vertexShader = {};
			vkShader fragmentShader = {};
			VkDescriptorPool descPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> descSets = {};
			vkPipeline* crenPipeline = NULL;
		} mGrid;

	};
}