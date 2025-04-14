#include "core/application.h"

#include <cren_math.h>

namespace Cosmos
{
    Application::Application()
        : mWindow(this, "Engine", 1366, 728), mRenderer(this), mGUI(this)
    {
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {
        unsigned long long previousTicks = mWindow.GetTimer();
        double accumulator = 0.0;
        const double FIXED_TIMESTEP = 1.0 / 60.0; // 60 FPS
        const int MAX_UPDATES = 5;
        double fpsElapsedTime = 0.0;
        int frameCount = 0;

        while (!mWindow.ShouldClose()) {

            unsigned long long currentTicks = mWindow.GetTimer();
            mTimeStep = (currentTicks - previousTicks) / (double)mWindow.GetTimerFrequency();
            previousTicks = currentTicks;

            mWindow.OnUpdate();
            mGUI.OnUpdate();

            mTimeStep = d_min(mTimeStep, 0.1); // max 100ms
            accumulator += mTimeStep;

            // average frames per second
            fpsElapsedTime += mTimeStep;
            frameCount++;
            if (fpsElapsedTime >= 1.0) {
                mAverageFPS = frameCount / (int)fpsElapsedTime;
                frameCount = 0;
                fpsElapsedTime = 0.0;
            }

            int updateCount = 0;
            while (accumulator >= FIXED_TIMESTEP && updateCount < MAX_UPDATES) {
                mRenderer.OnUpdate(FIXED_TIMESTEP);
                accumulator -= FIXED_TIMESTEP;
                updateCount++;
            }

            const double alpha = accumulator / FIXED_TIMESTEP;
            mRenderer.OnRender(alpha);
        }
    }

    void Application::OnMinimize()
    {
    }

    void Application::OnRestore(int width, int height)
    {
    }

    void Application::OnResize(int width, int height)
    {
    }

    void Application::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
    {
    }

    void Application::OnKeyRelease(Input::Keycode keycode)
    {
    }

    void Application::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
    {
    }

    void Application::OnButtonRelease(Input::Buttoncode buttoncode)
    {
    }

    void Application::OnMouseScroll(double xoffset, double yoffset)
    {
    }

    void Application::OnMouseMove(double xpos, double ypos)
    {
    }


}