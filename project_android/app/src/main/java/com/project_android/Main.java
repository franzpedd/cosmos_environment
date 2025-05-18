package com.project_android;
import android.content.res.AssetManager;
import android.os.Bundle;

public class Main extends org.libsdl.app.SDLActivity
{
    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL3",
                "vulkan",
                "Engine",
                "project_android"
        };
    }

    public static native void initAssetsManager(AssetManager assetManager);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initAssetsManager(getAssets());
    }
}