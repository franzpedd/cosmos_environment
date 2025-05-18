#include <cren.h>

#include "core/application.h"

int main(int argc, char** argv) {

    Cosmos::Application::CreateInfo ci;
    ci.appName = "Engine";
    ci.requestViewport = true;
    ci.requestFullscreen = false;
    ci.width = 1366;
    ci.height = 768;

    Cosmos::Editor::Application application(ci);
    application.Run();

    return 0;
}