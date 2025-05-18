#include "ResistorCalc.h"

namespace Cosmos::Android
{
    ResistorCalc::ResistorCalc(Application* app) :
        Widget("Main Screen", true),
        mApp(app),
        mBlack(ConvertRGB_Vec4(0, 0, 0)),
        mBrown(ConvertRGB_Vec4(88, 57, 39)),
        mRed(ConvertRGB_Vec4(255, 0, 0)),
        mOrange(ConvertRGB_Vec4(255, 91, 31)),
        mYellow(ConvertRGB_Vec4(255, 255, 0)),
        mGreen(ConvertRGB_Vec4(0, 255, 0)),
        mBlue(ConvertRGB_Vec4(0, 0, 255)),
        mViolet(ConvertRGB_Vec4(148, 0, 211)),
        mGrey(ConvertRGB_Vec4(128, 128, 128)),
        mWhite(ConvertRGB_Vec4(255, 255, 255)),
        mGold(ConvertRGB_Vec4(212, 175, 55)),
        mSilver(ConvertRGB_Vec4(165, 169, 180))
        {

        // initial band values
        mTolerance = BandColor::Brown;
        mBand1 = mBand2 = mBand3 = mMultiplier  = mTemperature = mBand1 = BandColor::Black;
    }

    void ResistorCalc::OnUpdate() {

        static int bandType = 1; // defaulting to 4 bands mode

        if(!mApp) return;

        //// set's set a ui-widget-window that covers the framebuffer entirely
        int width, height;
        mApp->GetWindowRef().GetWindowSize(&width, &height);
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
        ImGui::NewLine();

        // widget start
        if(ImGui::Begin("Resistor Calculator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {

            Info();

            ImGui::NewLine();
            CENTERED_CONTROL(ImGui::Text("%s", mResistorInfo.msg.str().c_str()));

            ImGui::SeparatorText("Resistor stripes " ICON_FA_QUESTION_CIRCLE);
            ImGui::SetItemTooltip("How many stripes/bands the resistor has");

            // resistor's band mode
            if(ImGui::RadioButton("3 Bands", bandType == 0)) { bandType = 0; }
            ImGui::SameLine();
            if(ImGui::RadioButton("4 Bands", bandType == 1)) { bandType = 1; }
            ImGui::SameLine();
            if(ImGui::RadioButton("5 Bands", bandType == 2)) { bandType = 2; }
            ImGui::SameLine();
            if(ImGui::RadioButton("6 Bands", bandType == 3)) { bandType = 3; }
            ImGui::NewLine();

            switch (bandType)
            {
                case 0: { ModeBand(BandFlags_Band1 | BandFlags_Band2 | BandFlags_Multiplier); break; }
                case 1: { ModeBand(BandFlags_Band1 | BandFlags_Band2 | BandFlags_Multiplier | BandFlags_Tolerance); break; }
                case 2: { ModeBand(BandFlags_Band1 | BandFlags_Band2 | BandFlags_Band3 | BandFlags_Multiplier | BandFlags_Tolerance); break; }
                case 3: { ModeBand(BandFlags_Band1 | BandFlags_Band2 | BandFlags_Band3 | BandFlags_Multiplier | BandFlags_Tolerance | BandFlags_Temperature); break; }
            }

            CalculateResistance(bandType);

            ImGui::SetCursorPos(ImVec2(width - 70.0f, height / 2.0f));
            if (ImGui::ArrowButton("##Up", ImGuiDir_Up)) {
                ImGui::SetScrollY(ImGui::GetScrollY() - 300.0f);
            }

            ImGui::SetCursorPos(ImVec2(width - 70.0f, (height / 2.0f) + 75.0f));
            if (ImGui::ArrowButton("##Down", ImGuiDir_Down)) {
                ImGui::SetScrollY(ImGui::GetScrollY() + 300.0f);
            }

            ImGui::End();
        }
    }

    void ResistorCalc::Info() {
        const char* title = ICON_LC_INFO " Resistor Calculator";
        if (CENTERED_CONTROL(ImGui::Button(title, ImVec2(ImGui::CalcTextSize(title).x + 20.0f, 100.0f)))) { ImGui::OpenPopup("InfoPopup"); }
        //ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 150.0f);
        if(ImGui::Button("Quit", ImVec2(150.0f, 100.0f))) {
            mApp->Quit();
        }

        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.1f, 0.15f, 0.2f, 0.5f)); // Example: dark blue-gray
        if (ImGui::BeginPopupModal("InfoPopup", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) {
            ImGui::Text("Developed in C++ (and Java)");
            ImGui::Text("Vulkan API (Cosmos Engine)");
            ImGui::Text("It uses SDL3 for Window and ImGui for UI");
            ImGui::Text("github.com/franzpedd/cosmos_environment");
            if(CENTERED_CONTROL(ImGui::Button("Back"))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
    }

    void ResistorCalc::ModeBand(BandFlags flags) {
        if(flags & BandFlags_Band1) {
            ImGui::SeparatorText("1st Color");

            ImGui::PushID("##1stID");
            DrawBand(&mBand1);
            ImGui::PopID();

            ImGui::NewLine();
        }

        if(flags & BandFlags_Band2) {
            ImGui::SeparatorText("2nd Color");

            ImGui::PushID("##2ndID");
            DrawBand(&mBand2);
            ImGui::PopID();

            ImGui::NewLine();
        }

        if(flags & BandFlags_Band3) {
            ImGui::SeparatorText("3nd Color");

            ImGui::PushID("##3ndID");
            DrawBand(&mBand3);
            ImGui::PopID();

            ImGui::NewLine();
        }

        if(flags & BandFlags_Multiplier) {
            ImGui::SeparatorText("Multiplier");

            ImGui::PushID("##MultiplierID");
            DrawBand(&mMultiplier, true, true, true);
            ImGui::PopID();

            ImGui::NewLine();
        }

        if(flags & BandFlags_Tolerance) {
            ImGui::SeparatorText("Tolerance");

            ImGui::PushID("##ToleranceID");
            DrawBand(&mTolerance, false, false, true);
            ImGui::PopID();

            ImGui::NewLine();
        }

        if(flags & BandFlags_Temperature) {
            ImGui::SeparatorText("Temperature");

            ImGui::PushID("##TemperatureID");
            DrawBand(&mTemperature, true, false, false);
            ImGui::PopID();

            ImGui::NewLine();
        }
    }

    void ResistorCalc::DrawBand(BandColor* option, bool drawBlack, bool drawWhite,  bool extraOptions) {
        // black
        ImGui::BeginDisabled(!drawBlack);
        if(ImGui::RadioButton("##BandBlack", *option == BandColor::Black)) { *option = BandColor::Black; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mBlack, mWhite, "#BandBlackTxt", "Black ");
        ImGui::SameLine();
        ImGui::EndDisabled();

        // brown
        if(ImGui::RadioButton("##BandBrown", *option == BandColor::Brown)) { *option = BandColor::Brown; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mBrown, mWhite,"#BandBrownTxt", "Brown ");
        ImGui::SameLine();

        // red
        if(ImGui::RadioButton("##BandRed", *option == BandColor::Red)) { *option = BandColor::Red; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mRed, mWhite, "#BandRedTxt", "Red   ");
        ImGui::SameLine();

        // orange
        if(ImGui::RadioButton("##BandOrange", *option == BandColor::Orange)) { *option = BandColor::Orange; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mOrange, mWhite, "#BandOrangeTxt", "Orange");
        //ImGui::SameLine();

        // yellow
        if(ImGui::RadioButton("##BandYellow", *option == BandColor::Yellow)) { *option = BandColor::Yellow; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mYellow, mBlack, "#BandYellowTxt", "Yellow");
        ImGui::SameLine();

        // green
        if(ImGui::RadioButton("##BandGreen", *option == BandColor::Green)) { *option = BandColor::Green; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mGreen, mBlack, "#BandGreenTxt", "Green ");
        ImGui::SameLine();

        // blue
        if(ImGui::RadioButton("##BandBlue", *option == BandColor::Blue)) { *option = BandColor::Blue; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mBlue, mWhite, "#BandBlueTxt", "Blue  ");
        ImGui::SameLine();

        // violet
        if(ImGui::RadioButton("##BandViolet", *option == BandColor::Violet)) { *option = BandColor::Violet; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mViolet, mWhite, "#BandVioletTxt", "Violet");
        //ImGui::SameLine();

        // grey
        if(ImGui::RadioButton("##BandGrey", *option == BandColor::Grey)) { *option = BandColor::Grey; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mGrey, mWhite, "#BandGreyTxt", "Grey  ");
        ImGui::SameLine();

        // white
        ImGui::BeginDisabled(!drawWhite);
        if(ImGui::RadioButton("##BandWhite", *option == BandColor::White)) { *option = BandColor::White; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mWhite, mBlack, "#BandWhiteTxt", "White ");
        ImGui::SameLine();
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!extraOptions);
        // gold
        if(ImGui::RadioButton("##BandGold", *option == BandColor::Gold)) { *option = BandColor::Gold; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mGold, mBlack, "#BandGoldTxt", "Gold  ");
        ImGui::SameLine();

        // silver
        if(ImGui::RadioButton("##BandSilver", *option == BandColor::Silver)) { *option = BandColor::Silver; }
        ImGui::SameLine();
        WidgetExtended::TextBackground(mSilver, mBlack, "#BandSilverTxt", "Silver");
        //ImGui::SameLine();
        ImGui::EndDisabled();
    }

    void ResistorCalc::CalculateResistance(int bandMode) {

        mResistorInfo = ResistorInfo();

        switch (bandMode) {
            case 0: // 3 bands
            {
                double res = (10 * mBand1 + mBand2) * pow(10, (int)mMultiplier);
                if(mMultiplier == BandColor::Gold) res = (10 * mBand1 + mBand2) * 0.1f;
                if(mMultiplier == BandColor::Silver) res = (10 * mBand1 + mBand2) * 0.01f;

                char formatted[20];
                Resistance_Format(res, formatted);
                mResistorInfo.msg << formatted << " 20% (M)";
                break;
            }

            case 1: // 4 bands
            {
                double res = (10 * mBand1 + mBand2) * pow(10, (int)mMultiplier);
                if(mMultiplier == BandColor::Gold) res = (10 * mBand1 + mBand2) * 0.1f;
                if(mMultiplier == BandColor::Silver) res = (10 * mBand1 + mBand2) * 0.01f;

                char formatted[20];
                Resistance_Format(res, formatted);
                mResistorInfo.msg << formatted << Tolerance_cstr(mTolerance);
            }

            case 2: // 5 bands
            {
                double res = (100 * mBand1 + 10 * mBand2 + mBand3) * pow(10, (int)mMultiplier);
                if(mMultiplier == BandColor::Gold) res =  (100 * mBand1 + 10 * mBand2 + mBand3) * 0.1f;
                if(mMultiplier == BandColor::Silver) res =  (100 * mBand1 + 10 * mBand2 + mBand3) * 0.01f;

                char formatted[20];
                Resistance_Format(res, formatted);
                mResistorInfo.msg << formatted << Tolerance_cstr(mTolerance);
                break;
            }

            case 3: // 6 bands
            {
                double res = (100 * mBand1 + 10 * mBand2 + mBand3) * pow(10, (int)mMultiplier);
                if(mMultiplier == BandColor::Gold) res =  (100 * mBand1 + 10 * mBand2 + mBand3) * 0.1f;
                if(mMultiplier == BandColor::Silver) res =  (100 * mBand1 + 10 * mBand2 + mBand3) * 0.01f;

                char formatted[20];
                Resistance_Format(res, formatted);
                mResistorInfo.msg << formatted << Tolerance_cstr(mTolerance) << " " << Temperature_cstr(mTemperature);

                break;
            }
        }
    }

    void ResistorCalc::Resistance_Format(double ohms, char *output) {
        if (ohms >= 1e9) sprintf(output, "%.2f G" ICON_LC_OMEGA, ohms / 1e9);
        else if (ohms >= 1e6) sprintf(output, "%.2f M" ICON_LC_OMEGA, ohms / 1e6);
        else if (ohms >= 1e3) sprintf(output, "%.2f k" ICON_LC_OMEGA, ohms / 1e3);
        else sprintf(output, "%.2f " ICON_LC_OMEGA, ohms);
    }

    const char* ResistorCalc::Tolerance_cstr(BandColor color) {
        switch (color) {
            case Black: return " ";
            case Brown: return ICON_LC_DIFF "1% (F)";
            case Red: return ICON_LC_DIFF "2% (G)";
            case Orange: return ICON_LC_DIFF "0.05% (W)";
            case Yellow: return ICON_LC_DIFF "0.02% (P)";
            case Green: return ICON_LC_DIFF "0.5% (D)";
            case Blue: return ICON_LC_DIFF "0.25% (C)";
            case Violet: return ICON_LC_DIFF "0.1% (B)";
            case Grey: return ICON_LC_DIFF "0.01% (L)";
            case White: return " ";
            case Gold: return ICON_LC_DIFF "5% (J)";
            case Silver: return ICON_LC_DIFF "10% (K)";
        }
        return " ";
    }

    const char* ResistorCalc::Temperature_cstr(BandColor color) {
        switch (color) {
            case Black: return "250 ppm/K(U)";
            case Brown: return "100 ppm/K(S)";
            case Red: return "50 ppm/K(R)";
            case Orange: return "15 ppm/K(P)";
            case Yellow: return "25 ppm/K(Q)";
            case Green: return "20 ppm/K(Z)";
            case Blue: return "10 ppm/K(Z)";
            case Violet: return "5 ppm/K(M)";
            case Grey: return "1 ppm/K(M)";
            case White: return " ";
            case Gold: return " ";
            case Silver: return " ";
        }
        return " ";
    }

    float4 ResistorCalc::ConvertRGB_Vec4(int r, int g, int b) {
        return {(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f};
    }
}