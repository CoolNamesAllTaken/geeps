#include "geeps_gui.hh"
#include <stdio.h>  // for printf
#include <string.h> // for c-string operations
#include "gui_bitmaps.hh"

/* GUIBitMap */
GUIBitMap::GUIBitMap(GeepsGUIElementConfig config_in) : GeepsGUIElement(config_in)
{
}
void GUIBitMap::Draw(EPaperDisplay &display)
{
    display.DrawBitMap(pos_x, pos_y, bitmap, size_x, size_y, EPaperDisplay::EPaper_Color_t::EPAPER_BLACK);
}

/* GUITextBox */
GUITextBox::GUITextBox(GeepsGUIElementConfig config)
    : GeepsGUIElement(config)
{
}

void GUITextBox::Draw(EPaperDisplay &display)
{
    uint16_t chars_to_print = MIN(strlen(text), kTextMaxLen);
    uint16_t chars_printed = 0;
    for (uint row = 0; row < kMaxNumRows && chars_printed < chars_to_print; row++)
    {
        // Copy characters to the line of text on each row one by one.
        char row_chars[kMaxNumCols];
        bool encountered_newline = false;
        for (uint16_t col = 0; col < width_chars && !encountered_newline && chars_printed < chars_to_print; col++)
        {
            if (text[chars_printed] == '\n')
            {
                // Jump to the next line upon receiving newline characters.
                encountered_newline = true;
            }
            else
            {
                row_chars[col] = text[chars_printed];
            }
            chars_printed++;
        }
        row_chars[chars_printed] = '\0'; // Add null terminator.

        // Print the row of text.
        display.DrawText(pos_x, pos_y + row * kCharHeight, row_chars);
        memset(row_chars, '\0', kMaxNumCols);
    }
}

/* GUIStatusBar */
GUIStatusBar::GUIStatusBar(GeepsGUIElementConfig config)
    : GeepsGUIElement(config), battery_percent(0.0f), position_fix(GGAPacket::FIX_NOT_AVAILABLE), num_satellites(0)
{
    memset(time_string, '\0', NMEAPacket::kMaxPacketFieldLen);
    memset(latitude_string, '\0', NMEAPacket::kMaxPacketFieldLen);
    memset(longitude_string, '\0', NMEAPacket::kMaxPacketFieldLen);
}

/**
 * @brief Draws a status bar in the specified y location (always takes full width of screen).
 */
void GUIStatusBar::Draw(EPaperDisplay &display)
{
    // TODO: Draw battery icon with fill bar in top left.
    display.DrawBitMap(pos_x + 0, pos_y + 0, battery_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    char battery_text[kNumberStringLength]; // this could be shorter
    sprintf(battery_text, "%.0f%%", battery_percent);
    display.DrawText(pos_x + 20, pos_y + kStatusBarHeight / 3, battery_text);

    display.DrawText(pos_x + 0, pos_y + 20, time_string);

    // TODO: Draw Satellite icon in middle with number of sats (red if none).
    char satellites_str[kNumberStringLength];
    sprintf(satellites_str, "%d", num_satellites);
    display.DrawBitMap(pos_x + 50, pos_y + 0, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    display.DrawText(pos_x + 70, pos_y + kStatusBarHeight / 3, satellites_str, EPaperDisplay::EPAPER_BLACK);

    // TODO: Draw GPS coordinates on second line (red "NO FIX" if no fix).
    display.DrawText(pos_x + 0, pos_y + 50, latitude_string);
    display.DrawText(pos_x + 0, pos_y + 70, longitude_string);
    // config_.display.DrawBitMap(70, 0, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
}