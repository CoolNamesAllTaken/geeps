#include "geeps_gui.hh"

#include <stdio.h>   // for printf
#include <string.h>  // for c-string operations

#include "gui_bitmaps.hh"
#include "math.h"

/* GUIBitMap */
GUIBitMap::GUIBitMap(GeepsGUIElementConfig config_in) : GeepsGUIElement(config_in) {}
void GUIBitMap::Draw(EPaperDisplay &display) {
    display.DrawBitMap(pos_x, pos_y, bitmap, size_x, size_y, EPaperDisplay::EPaper_Color_t::EPAPER_BLACK);
}

/* GUITextBox */
GUITextBox::GUITextBox(GeepsGUIElementConfig config) : GeepsGUIElement(config) {}

void GUITextBox::Draw(EPaperDisplay &display) {
    if (!visible) {
        return;
    }
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
                    row_chars[col - (chars_printed - word_start)] = '\0';  // Null terminate the string
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
        // printf("Printing row %d: %s\n", row, row_chars);
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
    if (!visible) {
        return;
    }
    // Draw battery icon and percentage.
    display.DrawBitMap(pos_x + 0, pos_y + 0, battery_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    char battery_text[kNumberStringLength];  // this could be shorter
    sprintf(battery_text, "%.0f%%", 100 * battery_charge_frac);
    display.DrawText(pos_x + 18, pos_y + 4, battery_text);
    display.DrawRectangle(pos_x + 2, pos_y + 5, battery_charge_frac * 9, 4, EPaperDisplay::EPAPER_BLACK, true);

    // Draw satellite icon and number of satellites visible.
    char satellites_str[kNumberStringLength];
    sprintf(satellites_str, "%d", num_satellites);
    display.DrawBitMap(pos_x + 43, pos_y + 0, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    display.DrawText(pos_x + 60, pos_y + 5, satellites_str, EPaperDisplay::EPAPER_BLACK);

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
    uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
    display.DrawBitMap(pos_x + 180, pos_y + kStatusBarHeight,
                       timestamp_ms - button_up_clicked_timestamp < kButtonClickHighlightIntervalMs
                           ? arrow_up_solid_30x30
                           : arrow_up_30x30,
                       30, 30, EPaperDisplay::EPAPER_BLACK);
    display.DrawRectangle(pos_x + 180, pos_y + kStatusBarHeight + 30, 30, 20, EPaperDisplay::EPAPER_BLACK,
                          timestamp_ms - button_center_clicked_timestamp < kButtonClickHighlightIntervalMs);
    display.DrawBitMap(pos_x + 180, pos_y + kStatusBarHeight + 50,
                       timestamp_ms - button_down_clicked_timestamp < kButtonClickHighlightIntervalMs
                           ? arrow_down_solid_30x30
                           : arrow_down_30x30,
                       30, 30, EPaperDisplay::EPAPER_BLACK);
}

/* GUICompass */
GUICompass::GUICompass(GeepsGUIElementConfig config) : GeepsGUIElement(config) {}

void GUICompass::Draw(EPaperDisplay &display) {
    if (!visible) {
        return;
    }
    // Draw a circle of tick marks separated by 30 degrees.
    for (uint16_t i = 0; i < 12; i++) {
        float angle = i * 360.0f / kNumTickMarks;
        float angle_rad = angle * M_PI / 180.0f;
        int32_t x_inner = pos_x + kTicksRadius * cos(angle_rad);
        int32_t y_inner = pos_y - kTicksRadius * sin(angle_rad);
        int32_t x_outer = pos_x + (kTicksRadius + kTickMarkLength) * cos(angle_rad);
        int32_t y_outer = pos_y - (kTicksRadius + kTickMarkLength) * sin(angle_rad);
        display.DrawLine(x_inner, y_inner, x_outer, y_outer, EPaperDisplay::EPAPER_BLACK);
    }

    // Draw a circle at heading degrees.
    float heading_rad = heading_deg * M_PI / 180.0f;
    int32_t x = pos_x + (kTicksRadius + kTickMarkLength / 2) * sin(heading_rad);
    int32_t y = pos_y - (kTicksRadius + kTickMarkLength / 2) * cos(heading_rad);
    display.DrawCircle(x, y, kHeadingBugRadius, EPaperDisplay::EPAPER_BLACK, true);

    // Draw a triangle pointing upwards at 12 o'clock.
    display.DrawTriangle(pos_x, pos_y - kTicksRadius - kTickMarkLength - kNorthTriangleSideLength - 2,
                         pos_x - kNorthTriangleSideLength / 2, pos_y - kTicksRadius - kTickMarkLength - 2,
                         pos_x + kNorthTriangleSideLength / 2, pos_y - kTicksRadius - kTickMarkLength - 2,
                         EPaperDisplay::EPAPER_BLACK, true);
    // Draw an N for north.
    display.DrawText(pos_x - 3, pos_y - kTicksRadius - 12, (char *)"N", EPaperDisplay::EPAPER_WHITE,
                     EPaperDisplay::EPAPER_BLACK);
}

GUIMenu::GUIMenu(GeepsGUIElementConfig config) : GeepsGUIElement(config) {}

void GUIMenu::Draw(EPaperDisplay &display) {
    if (!visible) {
        return;
    }
    display.DrawRectangle(pos_x, pos_y, width, num_rows * kRowHeight, EPaperDisplay::EPAPER_WHITE, true);
    // Draw a rectangle for each menu item.
    for (uint16_t i = 0; i < num_rows; i++) {
        if (i == selected_row) {
            // If row is selected, draw filled rectangle as background and use white text.
            display.DrawRectangle(pos_x, pos_y + i * kRowHeight - 2, width, kRowHeight, EPaperDisplay::EPAPER_BLACK,
                                  true);
            display.DrawText(pos_x + 5, pos_y + i * kRowHeight, rows[i], EPaperDisplay::EPAPER_WHITE);
        } else {
            // If row is not selected, draw regular text.
            display.DrawText(pos_x + 5, pos_y + i * kRowHeight, rows[i], EPaperDisplay::EPAPER_BLACK);
        }
    }
}