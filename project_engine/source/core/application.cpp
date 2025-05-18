#include "core/application.h"

#include <cren_math.h>

namespace Cosmos
{
    Application::Application(const CreateInfo& ci) : 
        mWindow(this, ci.appName, ci.width, ci.height, ci.requestFullscreen), 
        mRenderer(this, ci.appName, ci.requestViewport, ci.requestValidations),
        mGUI(this)
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

            mTimeStep = f_min(mTimeStep, 0.1); // max 100ms
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

    void Application::Quit()
    {
        mWindow.Quit();
    }

    void Application::OnMinimize()
    {
        mRenderer.Minimize();
        mGUI.OnMinimize();
    }

    void Application::OnRestore(int width, int height)
    {
        mRenderer.Restore();
        mGUI.OnRestore(width, height);
    }

    void Application::OnResize(int width, int height)
    {
        mRenderer.Resize(width, height);
        mGUI.OnResize(width, height);
    }

    void Application::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
    {
        mGUI.OnKeyPress(keycode, mod, held);

        // camera
        CRenCamera& cam = mRenderer.GetContext()->camera;
        if (cam.shouldMove) {
            if (keycode == Input::KEYCODE_A) cam.movingForward = 1;
            if (keycode == Input::KEYCODE_S) cam.movingBackward = 1;
            if (keycode == Input::KEYCODE_A) cam.movingLeft = 1;
            if (keycode == Input::KEYCODE_D) cam.movingRight = 1;
            if (keycode == Input::KEYCODE_LSHIFT) cam.modifierPressed = 1;
        }
    }

    void Application::OnKeyRelease(Input::Keycode keycode)
    {
        mGUI.OnKeyRelease(keycode);

        // camera
        CRenCamera& cam = mRenderer.GetContext()->camera;
        if (cam.shouldMove) {
            if (keycode == Input::KEYCODE_W) cam.movingForward = 0;
            if (keycode == Input::KEYCODE_S) cam.movingBackward = 0;
            if (keycode == Input::KEYCODE_A) cam.movingLeft = 0;
            if (keycode == Input::KEYCODE_D) cam.movingRight = 0;
            if (keycode == Input::KEYCODE_LSHIFT) cam.modifierPressed = 0;
        }
    }

    void Application::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
    {
        mGUI.OnButtonPress(buttoncode, mod);
    }

    void Application::OnButtonRelease(Input::Buttoncode buttoncode)
    {
        mGUI.OnButtonRelease(buttoncode);
    }

    void Application::OnMouseScroll(double xoffset, double yoffset)
    {
        mGUI.OnMouseScroll(xoffset, yoffset);
    }

    void Application::OnMouseMove(double xpos, double ypos)
    {
        mGUI.OnMouseMove(xpos, ypos);

        // camera
        CRenCamera& cam = mRenderer.GetContext()->camera;
        if (cam.shouldMove) {

            // avoid scene flip
            if (cam.rotation.x >= 89.0f) cam.rotation.x = 89.0f;
            if (cam.rotation.x <= -89.0f) cam.rotation.x = -89.0f;

            // reset rotation on 360 degrees
            if (cam.rotation.x >= 360.0f) cam.rotation.x = 0.0f;
            if (cam.rotation.x <= -360.0f) cam.rotation.x = 0.0f;
            if (cam.rotation.y >= 360.0f) cam.rotation.y = 0.0f;
            if (cam.rotation.y <= -360.0f) cam.rotation.y = 0.0f;

            float rotationspeed = 1.0f;
            float3 rot = { float(-ypos) * rotationspeed * 0.5f , float(xpos) * rotationspeed * 0.5f, 0.0f };
            cren_camera_rotate(&cam, rot);
        }
    }

    void Application::OnDPIChange(float scale)
    {
        mGUI.OnDPIChange(scale);
    }
}