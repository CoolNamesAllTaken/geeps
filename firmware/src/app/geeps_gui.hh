#pragma once

#include "epaper.hh"
#include "savenger_hunt.hh"
#include "nmea_utils.hh"

class GeepsGUIElement
{
public:
    struct GeepsGUIElementConfig
    {
        EPaperDisplay *display;
        uint16_t x, y;
    };

    GeepsGUIElement(GeepsGUIElementConfig config_in) : config_(config_in) {};
    abstract void Draw() = 0;

private:
    GeepsGUIElementConfig config_;
}

class GUIStatusBar : public GeepsGUIElement
{
public:
    float battery_frac = 1.0f;
    uint8_t num_satellites = 0;
    GGAPacket::PositionFixIndicator_t position_fix = GGAPacket::FIX_NOT_AVAILABLE;
    float completion_frac = 0.0f;

    GUIStatusBar(GeepsGUIElementConfig config_in) : GeepsGUIElement(config_in) {};

    void Draw();
};

class GUIHintBox : public GeepsGUIElement
{
public:
    ScavengerHuntHint hint;

    void Draw();
};

class GeepsGUI
{
public:
    static const uint16_t kMaxNumGUIElements = 10;
    GeepsGUIElement gui_elements[kMaxNumGUIElements];
    EpaperDisplay *display;

    uint16_t num_elements = 0;

    struct GeepsGUIConfig
    {
        EPaperDisplay *display;
    }

    GeepsGUI(EPaperDisplay *display_in)
    {
        config_.display = display_in;
    }

    inline void Draw()
    {
        for (uint16_t i = 0; i < kMaxNumGUIElements; i++)
        {
            gui_elements[i].Draw();
        }
    }

    inline void AddElement(GeepsGUIElement element)
    {
        gui_elements[num_elements] = element;
        num_elements++;
    }

private:
    GeepsGUIConfig config_;
};

extern GeepsGUI gui;