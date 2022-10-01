#include "geeps_gui.hh"
#include <stdio.h> // for printf
#include <string.h> // for c-string operations
#include "gui_bitmaps.hh"

#define MIN(A, B) ((A) < (B) ? (A) : (B))

GeepsGUIElement::GeepsGUIElement(GeepsGUIElementConfig_t config)
: config_(config) {
    if (!config.display) {
        printf("geeps_gui: GeepsGUIElement: Trying to initialize a GeepsGUIElement with a NULL display, will SEGFAULT!\r\n");
    }
}

/* GUIStatusBar */
GUIStatusBar::GUIStatusBar(GeepsGUIElementConfig_t config)
: GeepsGUIElement(config) {

}

/**
 * @brief Draws a status bar in the specified y location (always takes full width of screen).
 */
void GUIStatusBar::Draw() {
    // TODO: Draw battery icon with fill bar in top left.
    // TODO: Draw Satellite icon in middle with number of sats (red if none).
    // TODO: Draw GPS coordinates on second line (red "NO FIX" if no fix).
    config_.display->DrawBitmap(50, 0, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
}

/* GUIHintBox */
GUIHintBox::GUIHintBox(GeepsGUIElementConfig_t config) 
: GeepsGUIElement(config) {


}

void GUIHintBox::Draw() {
    // uint16_t y = 10;
    // config_.screen->selectFont(Font_Terminal6x8);
    // config_.screen->gText(10, y, (char *)"Hello World!\r\n", myColours.red);
    // config_.screen->gText(10, y+config_.screen->characterSizeY(), (char *)"Doot Doot.\r\n", myColours.black);
    
    // config_.screen->selectFont(Font_Terminal6x8);
    // char hint_text_row[kRowNumChars];
    // for (uint row = 0; row < kHintTextMaxLen / kRowNumChars; row++) {
    //     strncpy(hint_text_row, hint_text+(row*kRowNumChars), kRowNumChars);
    //     config_.screen->gText(config_.pos_x + kTextMargin, config_.pos_y + row*kCharHeight, hint_text_row, myColours.black);
    // }
}