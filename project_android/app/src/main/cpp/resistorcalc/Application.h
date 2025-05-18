#ifndef PROJECT_ANDROID_APPLICATION_H
#define PROJECT_ANDROID_APPLICATION_H

#include <cosmos.h>
#include "ResistorCalc.h"

namespace Cosmos::Android
{
    class Application : public Cosmos::Application {
    public:

        /// @brief constructor
        Application(const CreateInfo& ci);

        /// @brief destructor
        virtual ~Application() = default;

    public:

    private:
        ResistorCalc* mScreenWidget = nullptr;
    };

} // Cosmos

#endif //PROJECT_ANDROID_APPLICATION_H
