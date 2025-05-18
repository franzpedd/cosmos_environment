#pragma once

#include <cosmos.h>
#include "ui/console.h"
#include "ui/demo.h"
#include "ui/dockspace.h"
#include "ui/viewport.h"

namespace Cosmos::Editor
{
    class Application : public Cosmos::Application
    {
    public:

        /// @brief constructor
        Application(const CreateInfo& ci);

        /// @brief destructor
        ~Application();

    private:

        Console* mConsole = nullptr;
        Demo* mDemo = nullptr;
        Dockspace* mDockspace = nullptr;
        Viewport* mViewport = nullptr;
    };
}