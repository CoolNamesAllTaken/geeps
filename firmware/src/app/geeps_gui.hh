#ifndef _GEEPS_GUI_HH_
#define _GEEPS_GUI_HH_

#include "epaper.hh"
#include "gui_bitmaps.hh"
#include "nmea_utils.hh"
#include "pico/stdlib.h"  // idk if we need this

class GeepsGUIElement {
   public:
    typedef struct {
        uint16_t pos_x = 0;
        uint16_t pos_y = 0;
        bool visible = true;
    } GeepsGUIElementConfig;

    GeepsGUIElement(GeepsGUIElementConfig config_in) : config_(config_in) {
        pos_x = config_.pos_x;
        pos_y = config_.pos_y;
        visible = config_.visible;
    };  // constructor

    virtual void Draw(EPaperDisplay &display) = 0;  // pure virtual function; no base class implementation
    void DisplayWrappedText(EPaperDisplay &display, const char *text, uint16_t x, uint16_t y, uint16_t width,
                            uint16_t height, uint16_t font_size, uint16_t line_spacing);

    uint16_t pos_x, pos_y;
    bool visible;

   protected:
    GeepsGUIElementConfig config_;
};

class GeepsGUI {
   public:
    static const uint16_t kScreenWidth = 212;   // [pixels] Screen X dimension.
    static const uint16_t kScreenHeight = 104;  // [pixels] Screen Y dimension.
    static const uint16_t kMaxNumElements = 10;

    struct GeepsGUIConfig {
        EPaperDisplay &display;
    };

    GeepsGUI(GeepsGUIConfig config_in) : config_(config_in) {};

    inline void Draw(bool fast = false) {
        config_.display.Clear();
        for (uint i = 0; i < kMaxNumElements; i++) {
            if (elements_[i] != nullptr) {
                elements_[i]->Draw(config_.display);
            }
        }
        config_.display.Update(fast);
    }

    void AddElement(GeepsGUIElement *element) {
        for (uint i = 0; i < kMaxNumElements; i++) {
            if (elements_[i] == nullptr) {
                elements_[i] = element;
                break;
            }
        }
    }

    void RemoveElement(GeepsGUIElement *element) {
        for (uint i = 0; i < kMaxNumElements; i++) {
            if (elements_[i] == element) {
                elements_[i] = nullptr;
                break;
            }
        }
    }

   private:
    GeepsGUIConfig config_;
    GeepsGUIElement *elements_[kMaxNumElements] = {nullptr};  // array of pointers to GeepsGUIElement objects
};

class GUIBitMap : public GeepsGUIElement {
   public:
    GUIBitMap(GeepsGUIElementConfig config_in);
    void Draw(EPaperDisplay &display);

    uint16_t size_x, size_y;
    uint8_t *bitmap;

   private:
};

class GUITextBox : public GeepsGUIElement {
   public:
    static const uint kTextMaxLen = 300;
    static const uint16_t kRowHeight = 10;    // [pixels]
    static const uint16_t kTextMargin = 10;   // [pixels]
    static const uint16_t kMaxNumCols = 300;  // [chars] Used for buffer sizing.
    static const uint16_t kMaxNumRows = (GeepsGUI::kScreenHeight - 2 * kTextMargin) / kRowHeight;

    GUITextBox(GeepsGUIElementConfig config);  // constructor
    void Draw(EPaperDisplay &display);

    char text[kTextMaxLen + 1];
    uint16_t width_chars = kMaxNumCols;
};

class GUIStatusBar : public GeepsGUIElement {
   public:
    static const uint16_t kStatusBarHeight = 30;
    static const uint16_t kNumberStringLength = 10;
    static const uint32_t kButtonClickHighlightIntervalMs = 500;
    static const uint16_t kCenterButtonLabelLen = 5;

    float battery_charge_frac;
    GGAPacket::PositionFixIndicator_t position_fix;

    uint16_t num_satellites;
    char time_string[NMEAPacket::kMaxPacketFieldLen];
    char latitude_string[NMEAPacket::kMaxPacketFieldLen];
    char longitude_string[NMEAPacket::kMaxPacketFieldLen];

    float progress_frac = 0.0f;
    float rendered_hint_frac = 0.0f;

    uint32_t button_up_clicked_timestamp = 0;
    uint32_t button_center_clicked_timestamp = 0;
    uint32_t button_down_clicked_timestamp = 0;
    char center_button_label[kCenterButtonLabelLen + 1] = "";

    GUIStatusBar(GeepsGUIElementConfig config);  // constructor
    void Draw(EPaperDisplay &display);
};

class GUICompass : public GeepsGUIElement {
   public:
    static const uint16_t kTicksRadius = 20;  // [pixels]
    static const uint16_t kTickMarkLength = 5;
    static const uint16_t kNumTickMarks = 12;
    static const uint16_t kHeadingBugRadius = 2;
    static const uint16_t kNorthTriangleSideLength = 10;

    GUICompass(GeepsGUIElementConfig config);  // constructor
    void Draw(EPaperDisplay &display);

    float heading_deg;
};

class GUIMenu : public GeepsGUIElement {
   public:
    static const uint16_t kRowHeight = 10;  // [pixels]
    static const uint16_t kMaxNumRows = 10;
    static const uint16_t kMaxNumCols = 30;

    GUIMenu(GeepsGUIElementConfig config);  // constructor
    void Draw(EPaperDisplay &display);

    inline void ScrollNext() {
        if (selected_row < num_rows - 1) {
            selected_row++;
        }
    }
    inline void ScrollPrev() {
        if (selected_row > 0) {
            selected_row--;
        }
    }
    inline void Select() {
        printf("GUIMenu::Select: selected_row = %d\r\n", selected_row);
        if (callbacks[selected_row] != nullptr) {
            callbacks[selected_row]();
        }
    }

    inline void AddRow(char row[kMaxNumCols], void (*callback)()) {
        if (num_rows < kMaxNumRows) {
            strcpy(rows[num_rows], row);
            num_rows++;
        }
        if (callback != nullptr) {
            callbacks[num_rows - 1] = callback;
        }
    }

    char rows[kMaxNumRows][kMaxNumCols];
    void (*callbacks[kMaxNumRows])();
    uint16_t num_rows;
    uint16_t selected_row;
    uint16_t width = kMaxNumCols * 5;
};

class GUINotification : public GeepsGUIElement {
   public:
    static const uint16_t kNotificationHeight = 50;
    static const uint16_t kNotificationWidth = 150;
    static const uint16_t kTextBoxMargin = 10;
    static const uint16_t kNotificationMaxNumChars = GUITextBox::kTextMaxLen;

    GUINotification(GeepsGUIElementConfig config);  // constructor
    void Draw(EPaperDisplay &display) override;
    void DisplayNotification(char *text_in, uint16_t duration_ms = 2000);

    uint32_t display_until_ms = 0;
    GUITextBox text_box;
};

#endif /* _GEEPS_GUI_HH_ */