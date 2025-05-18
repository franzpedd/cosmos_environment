
#include "Application.h"

namespace Cosmos::Android
{
    Application::Application(const CreateInfo& ci) : Cosmos::Application(ci)
    {
        mScreenWidget = new ResistorCalc(this);
        mGUI.AddWidget(mScreenWidget);
    }
}