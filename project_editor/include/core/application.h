#pragma once

#include <cosmos.h>
#include "ui/demo.h"
#include "ui/dockspace.h"

namespace Cosmos::Editor
{
    class Application : public Cosmos::Application
    {
    public:

        /// @brief constructor
        Application();

        /// @brief destructor
        ~Application();

    private:

        Demo* mDemo = nullptr;
        Dockspace* mDockspace = nullptr;
    };
}