#pragma once

#include <cren.h>

// forward declarations
namespace Cosmos { class Application; }

namespace Cosmos
{
    class Renderer
    {
    public:

        /// @brief initializes the renderer with standart configurations
        Renderer(Application* app, const char* appName, bool requestViewport);

        /// @brief shutsdown the renderer and release it's resources
        ~Renderer();

        /// @brief returns the renderer context/backend
        inline CRenContext* GetContext() { return mContext; }

    public:

        /// @brief updates the renderer, sending frame data to the gpu at a fixed period
        void OnUpdate(double timestep);

        /// @brief draw stuff
        void OnRender(double timestep);

    public:

        /// @brief tells the renderer to minimize
        void Minimize();

        /// @brief tells the renderer to restore it's last known window size
        void Restore();

        /// @brief tells the renderer to resize
        void Resize(int width, int height);

    protected:

        /// @brief the renderer has signaled it's time to render objects
        void OnRenderCallback(int stage, double timestep);

        /// @brief the renderer was resized, must propagate this to any other renderer sub-class
        void OnResizeCallback(int width, int height);

    private:

        Application* mApp;
        CRenContext* mContext = nullptr;
    };
}