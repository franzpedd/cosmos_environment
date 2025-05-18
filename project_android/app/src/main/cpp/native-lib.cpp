// this in in here so SDL may do it's macro-magic and redefine int main to whatever android want's it to be
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// this is in here to initialize some JNI functions and the assets manager
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// finally our code
#include "resistorcalc/Application.h"

int main(int argc, char** argv) {

    Cosmos::Application::CreateInfo ci;
    ci.appName = "Resistor Calculator";
    ci.requestViewport = false;
    ci.requestFullscreen = true;
    ci.requestValidations = false; // change this to true when emulation on new android versions

    Cosmos::Android::Application app(ci);
    app.Run();

    return 0;
}

/// @brief called from java to setup the assets manager
extern "C" JNIEXPORT void JNICALL Java_com_project_1android_Main_initAssetsManager(JNIEnv* env, jclass clazz, jobject assetManager) {
    AAssetManager* nativeManager = AAssetManager_fromJava(env, assetManager);
    cren_android_assets_manager_init(nativeManager);
}
