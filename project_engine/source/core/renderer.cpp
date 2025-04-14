#include "core/renderer.h"
#include "core/application.h"
#include "core/logger.h"

namespace Cosmos
{
	Renderer::Renderer(Application* app) 
        : mApp(app)
	{
        // initialization
        CRenCreateInfo ci = { 0 };
        ci.appName = "Engine";
        ci.appVersion = CREN_MAKE_VERSION(0, 1, 0, 0);
        ci.assetsRoot = "../data";
        ci.apiVersion = CREN_MAKE_VERSION(0, 1, 0, 2);
        ci.validations = 1;
        ci.vsync = 0;
        ci.msaa = 4;
        ci.width = mApp->GetWindowRef().GetWidth();
        ci.height = mApp->GetWindowRef().GetHeight();
        ci.smallerViewport = 1;
        ci.nativeWindow = mApp->GetWindowRef().GetNativeWindow();

        mContext = cren_initialize(&ci);
        COSMOS_ASSERT(mContext != nullptr, "Failed to initialize CRen");

        // callbacks
        cren_set_user_pointer(mContext, this);

        cren_set_ui_image_count_callback(mContext, [](CRenContext* context, unsigned int count) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.mApp->GetGUIRef().SetMinImageCount(count);
            });

        cren_set_draw_ui_raw_data_callback(mContext, [](CRenContext* context, void* commandbuffer) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.mApp->GetGUIRef().DrawRawData(commandbuffer);
            });

        cren_set_resize_callback(mContext, [](CRenContext* context, unsigned int width, unsigned int height) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnResizeCallback(width, height);
            });

        cren_set_render_callback(mContext, [](CRenContext* context, int stage, double timestep) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnRenderCallback(stage, timestep);
            });
	}

	Renderer::~Renderer()
	{
        cren_terminate(mContext);
	}

    void Renderer::OnUpdate(double timestep)
    {
        cren_update(mContext, timestep);
    }

    void Renderer::OnRender(double timestep)
    {
        cren_render(mContext, timestep);
    }

    void Renderer::OnRenderCallback(int stage, double timestep)
    {
    }

    void Renderer::OnResizeCallback(int width, int height)
    {
    }
}