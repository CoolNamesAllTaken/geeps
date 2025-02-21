#ifndef _GEEPS_GUI_HH_
#define _GEEPS_GUI_HH_

#include "pico/stdlib.h" // idk if we need this
#include "nmea_utils.hh"
#include "epaper.hh"
#include "gui_bitmaps.hh"

class GeepsGUIElement
{
public:
    typedef struct
    {
        uint16_t pos_x = 0;
        uint16_t pos_y = 0;
        bool visible = true;
    } GeepsGUIElementConfig;

    GeepsGUIElement(GeepsGUIElementConfig config_in) : config_(config_in)
    {
        pos_x = config_.pos_x;
        pos_y = config_.pos_y;
        visible = config_.visible;
    }; // constructor

    virtual void Draw(EPaperDisplay &display) = 0; // pure virtual function; no base class implementation

    uint16_t pos_x, pos_y;
    bool visible;

protected:
    GeepsGUIElementConfig config_;
};

class GeepsGUI
{
public:
    static const uint16_t kScreenWidth = 212;  // [pixels] Screen X dimension.
    static const uint16_t kScreenHeight = 104; // [pixels] Screen Y dimension.
    static const uint16_t kMaxNumElements = 10;

    struct GeepsGUIConfig
    {
        EPaperDisplay &display;
    };

    GeepsGUI(GeepsGUIConfig config_in) : config_(config_in) {};

    inline void Draw(bool fast = false)
    {
        config_.display.Clear();
        for (uint i = 0; i < kMaxNumElements; i++)
        {
            if (elements_[i] != nullptr)
            {
                elements_[i]->Draw(config_.display);
            }
        }
        config_.display.Update(fast);
    }

    void AddElement(GeepsGUIElement *element)
    {
        for (uint i = 0; i < kMaxNumElements; i++)
        {
            if (elements_[i] == nullptr)
            {
                elements_[i] = element;
                break;
            }
        }
    }

    void RemoveElement(GeepsGUIElement *element)
    {
        for (uint i = 0; i < kMaxNumElements; i++)
        {
            if (elements_[i] == element)
            {
                elements_[i] = nullptr;
                break;
            }
        }
    }

private:
    GeepsGUIConfig config_;
    GeepsGUIElement *elements_[kMaxNumElements] = {nullptr}; // array of pointers to GeepsGUIElement objects
};

class GUIBitMap : public GeepsGUIElement
{
public:
    GUIBitMap(GeepsGUIElementConfig config_in);
    void Draw(EPaperDisplay &display);

    uint16_t size_x, size_y;
    uint8_t *bitmap;

private:
};

class GUITextBox : public GeepsGUIElement
{
public:
    static const uint kTextMaxLen = 300;
    static const uint16_t kCharWidth = 6;    // [pixels]
    static const uint16_t kCharHeight = 8;   // [pixels]
    static const uint16_t kTextMargin = 10;  // [pixels]
    static const uint16_t kMaxNumCols = 300; // [chars] Used for buffer sizing.
    static const uint16_t kMaxNumRows = (GeepsGUI::kScreenHeight - 2 * kTextMargin) / kCharHeight;

    GUITextBox(GeepsGUIElementConfig config); // constructor
    void Draw(EPaperDisplay &display);

    char text[kTextMaxLen];
    uint16_t width_chars = kMaxNumCols;
};

class GUIStatusBar : public GeepsGUIElement
{
public:
    static const uint16_t kStatusBarHeight = 30;
    static const uint16_t kNumberStringLength = 10;

    float battery_charge_frac;
    GGAPacket::PositionFixIndicator_t position_fix;

    uint16_t num_satellites;
    char time_string[NMEAPacket::kMaxPacketFieldLen];
    char latitude_string[NMEAPacket::kMaxPacketFieldLen];
    char longitude_string[NMEAPacket::kMaxPacketFieldLen];

    float progress_frac = 0.0f;

    GUIStatusBar(GeepsGUIElementConfig config); // constructor
    void Draw(EPaperDisplay &display);
};

#endif /* _GEEPS_GUI_HH_ */