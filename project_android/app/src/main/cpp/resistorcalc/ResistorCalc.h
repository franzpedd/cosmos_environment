#ifndef PROJECT_ANDROID_RESISTORCALC_H
#define PROJECT_ANDROID_RESISTORCALC_H

#include <cosmos.h>
#include <string>

namespace Cosmos::Android
{
    class ResistorCalc : public Cosmos::Widget {
    public:

        typedef enum BandColor {
            Black = 0, Brown, Red, Orange, Yellow, Green, Blue, Violet, Grey, White, Gold, Silver
        } BandColor;

        typedef enum Scale {
            MilliOhm = 0, Ohm = 1, KiloOhm = 1000, MegaOhm = 100000, GigaOhm = 1000000,
        } Scale;

        typedef int BandFlags;
        enum BandFlags_ {
            BandFlags_None = 0,
            BandFlags_Band1 = 1 << 0,
            BandFlags_Band2 = 1 << 1,
            BandFlags_Band3 = 1 << 2,
            BandFlags_Multiplier = 1 << 3,
            BandFlags_Tolerance = 1 << 4,
            BandFlags_Temperature = 1 << 5
        };

    public:

        /// @brief constructor
        ResistorCalc(Application* app);

        /// @brief destructor (fuck off clang, override destructor, really?)
        ~ResistorCalc() override = default;

    public:

        /// @brief user interface updating
        void OnUpdate() override;

    private:

        /// @brief information about this app
        void Info();

        /// @brief draws the bands according with flags
        /// @param flags the band flags
        void ModeBand(BandFlags flags);

        /// @brief draws a band color given the options
        void DrawBand(BandColor* option, bool drawBlack = true, bool drawWhite = true, bool extraOptions = false);

        /// @brief calculates the resistor resistance based on witch ModeBand is currently active
        /// @param bandMode related to ModeBandX
        void CalculateResistance(int bandMode);

        /// @brief returns the scaled-down resistance
        /// @param ohms the ohms value
        /// @param output the output string
        void Resistance_Format(double ohms, char *output);

        /// @brief returns the const char* equivalent for the tolerance
        /// @param color the tolerance band color
        /// @return the string-fied info
        static const char* Tolerance_cstr(BandColor color);

        /// @brief returns the const char* equivalent for the temperature
        /// @param color the temperature band color
        /// @return the string-fied info
        static const char* Temperature_cstr(BandColor color);

        /// @brief utility to convert prue RGBA into it's floating version
        static float4 ConvertRGB_Vec4(int r, int g, int b);

    private:

        Application* mApp = nullptr;
        float4 mBlack, mBrown, mRed, mOrange, mYellow, mGreen, mBlue, mViolet, mGrey, mWhite, mGold, mSilver;
        BandColor mBand1, mBand2, mBand3, mMultiplier, mTolerance, mTemperature;

        struct ResistorInfo {
            float resistance;
            Scale scale;
            std::ostringstream msg;
        } mResistorInfo;
    };
}

#endif //PROJECT_ANDROID_RESISTORCALC_H
