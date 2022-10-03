#ifndef _GEEPS_GUI_HH_
#define _GEEPS_GUI_HH_

#include "pico/stdlib.h" // idk if we need this
#include "nmea_utils.hh"
#include "epaper.hh"
#include "gui_bitmaps.hh"

class GeepsGUIElement {
public:
    static const uint16_t kScreenWidth = 104; // [pixels] Screen X dimension.
    static const uint16_t kScreenHeight = 212; // [pixels] Screen Y dimension.

    typedef struct {
        uint16_t pos_x = 0;
        uint16_t pos_y = 0;
        EPaperDisplay * display = NULL;
    } GeepsGUIElementConfig_t;
    
    GeepsGUIElement(GeepsGUIElementConfig_t config); // constructor

    void DrawBitmap(uint8_t bitmap[][kScreenWidth], uint16_t size_x, uint16_t size_y, uint16_t pos_x, uint16_t pos_y);

    virtual void Draw() = 0; // pure virtual function; no base class implementation

protected:
    GeepsGUIElementConfig_t config_;
};

class GUIStatusBar: public GeepsGUIElement {
public:
    static const uint16_t kStatusBarHeight = 15;
    static const uint16_t kNumberStringLength = 10;

    float battery_percent;
    GGAPacket::PositionFixIndicator_t position_fix;
   

    uint16_t num_satellites;
    char time_string[NMEAPacket::kMaxPacketFieldLen];
    char latitude_string[NMEAPacket::kMaxPacketFieldLen];
    char longitude_string[NMEAPacket::kMaxPacketFieldLen];
    
    GUIStatusBar(GeepsGUIElementConfig_t config); // constructor
    void Draw();
};

class GUIHintBox: public GeepsGUIElement {
public:
    static const uint kHintTextMaxLen = 300;
    static const uint16_t kCharWidth = 6; // [pixels]
    static const uint16_t kCharHeight = 8; // [pixels]
    static const uint16_t kTextMargin = 10; // [pixels]
    static const uint16_t kRowNumChars = ((kScreenWidth - 2*kTextMargin) / kCharWidth); // [chars]

    char hint_text[kHintTextMaxLen];

    GUIHintBox(GeepsGUIElementConfig_t config); // constructor
    void Draw();

    
};

#endif /* _GEEPS_GUI_HH_ */