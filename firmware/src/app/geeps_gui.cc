#include "geeps_gui.hh"

#include <stdio.h>   // for printf
#include <string.h>  // for c-string operations

#include "gui_bitmaps.hh"

/* GUIBitMap */
GUIBitMap::GUIBitMap(GeepsGUIElementConfig config_in) : GeepsGUIElement(config_in) {}
void GUIBitMap::Draw(EPaperDisplay &display) {
    display.DrawBitMap(pos_x, pos_y, bitmap, size_x, size_y, EPaperDisplay::EPaper_Color_t::EPAPER_BLACK);
}

/* GUITextBox */
GUITextBox::GUITextBox(GeepsGUIElementConfig config) : GeepsGUIElement(config) {}

void GUITextBox::Draw(EPaperDisplay &display) {
    uint16_t chars_to_print = MIN(strlen(text), kTextMaxLen);
    uint16_t chars_printed = 0;
    for (uint row = 0; row < kMaxNumRows && chars_printed < chars_to_print; row++) {
        // Copy characters to the line of text on each row one by one.
        char row_chars[kMaxNumCols];
        bool encountered_newline = false;
        uint16_t word_start = chars_printed;
        for (uint16_t col = 0; col < width_chars && !encountered_newline && chars_printed < chars_to_print; col++) {
            if (text[chars_printed] == '\r') {
                chars_printed++;
                col--;  // Don't advance the column for \r
                continue;
            }
            if (text[chars_printed] == '\n') {
                encountered_newline = true;
            } else if (text[chars_printed] == ' ') {
                word_start = chars_printed + 1;
                row_chars[col] = text[chars_printed];
            } else {
                // Look ahead to see if current word fits
                if (col == width_chars - 1 && chars_printed + 1 < chars_to_print && text[chars_printed + 1] != ' ' &&
                    text[chars_printed + 1] != '\n') {
                    // Word continues beyond line width, go back to start of word
                    chars_printed = word_start;
                    encountered_newline = true;
                    break;
                }
                row_chars[col] = text[chars_printed];
            }
            chars_printed++;
        }
        row_chars[chars_printed] = '\0';  // Add null terminator.

        // Print the row of text.
        display.DrawText(pos_x, pos_y + row * kRowHeight, row_chars);
        memset(row_chars, '\0', kMaxNumCols);
    }
}

/* GUIStatusBar */
GUIStatusBar::GUIStatusBar(GeepsGUIElementConfig config)
    : GeepsGUIElement(config),
      battery_charge_frac(0.0f),
      position_fix(GGAPacket::FIX_NOT_AVAILABLE),
      num_satellites(0) {
    memset(time_string, '\0', NMEAPacket::kMaxPacketFieldLen);
    memset(latitude_string, '\0', NMEAPacket::kMaxPacketFieldLen);
    memset(longitude_string, '\0', NMEAPacket::kMaxPacketFieldLen);
}

/**
 * @brief Draws a status bar in the specified y location (always takes full width of screen).
 */
void GUIStatusBar::Draw(EPaperDisplay &display) {
    // Draw battery icon and percentage.
    display.DrawBitMap(pos_x + 0, pos_y + 0, battery_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    char battery_text[kNumberStringLength];  // this could be shorter
    sprintf(battery_text, "%.0f%%", 100 * battery_charge_frac);
    display.DrawText(pos_x + 18, pos_y + 4, battery_text);
    display.DrawRectangle(pos_x + 2, pos_y + 5, battery_charge_frac * 9, 4, EPaperDisplay::EPAPER_BLACK, true);

    // Draw satellite icon and number of satellites visible.
    char satellites_str[kNumberStringLength];
    sprintf(satellites_str, "%d", num_satellites);
    display.DrawBitMap(pos_x + 41, pos_y + 0, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    display.DrawText(pos_x + 58, pos_y + 5, satellites_str, EPaperDisplay::EPAPER_BLACK);

    // Draw progress bar.
    display.DrawRectangle(pos_x + 75, pos_y + 0, 135, 15, EPaperDisplay::EPAPER_BLACK, false);
    display.DrawRectangle(pos_x + 75, pos_y + 0, progress_frac * 135, 15, EPaperDisplay::EPAPER_BLACK, true);

    // Draw latitude and longitude string below the satellite and battery icons.
    display.DrawText(pos_x + 0, pos_y + 20, latitude_string);
    display.DrawText(pos_x + 70, pos_y + 20, longitude_string);

    // Draw time string to the right of lat and lon.
    display.DrawText(pos_x + 150, pos_y + 20, time_string);

    // Draw line below the status bar.
    display.DrawLine(pos_x, pos_y + kStatusBarHeight, GeepsGUI::kScreenWidth, pos_y + kStatusBarHeight,
                     EPaperDisplay::EPAPER_BLACK);

    // Draw up and down arrows on the sidebar.
    display.DrawBitMap(pos_x + 180, pos_y + kStatusBarHeight, button_up_pressed ? arrow_up_solid_30x30 : arrow_up_30x30,
                       30, 30, EPaperDisplay::EPAPER_BLACK);
    display.DrawRectangle(pos_x + 180, pos_y + kStatusBarHeight + 30, 30, 20, EPaperDisplay::EPAPER_BLACK,
                          button_center_pressed);
    display.DrawBitMap(pos_x + 180, pos_y + kStatusBarHeight + 50,
                       button_down_pressed ? arrow_down_solid_30x30 : arrow_down_30x30, 30, 30,
                       EPaperDisplay::EPAPER_BLACK);
}