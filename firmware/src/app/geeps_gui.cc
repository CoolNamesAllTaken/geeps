#include "geeps_gui.hh"
#include <stdio.h> // for printf
#include <string.h> // for c-string operations

#define MIN(A, B) ((A) < (B) ? (A) : (B))

GeepsGUIElement::GeepsGUIElement(GeepsGUIElementConfig_t config) 
: config_(config) {
    if (!config.screen) {
        printf("geeps_gui: GeepsGUIElement: Trying to initialize a GeepsGUIElement with a NULL config, will SEGFAULT!\r\n");
    }
}

void GeepsGUIElement::DrawBitmap(uint8_t bitmap[][kScreenWidth], uint16_t size_x, uint16_t size_y, uint16_t pos_x, uint16_t pos_y) {
    for (uint16_t row = 0; row <= MIN(size_y, kScreenHeight-pos_y); row++) {
        for (uint16_t col = 0; col <= MIN(size_x, kScreenWidth-pos_x); col++) {
            switch(bitmap[row][col]) {
            case 1:
                config_.screen->point(pos_x + col, pos_y + row, myColours.black);
                break;
            case 2:
                config_.screen->point(pos_x + col, pos_y + row, myColours.red);
                break;
            }
            
        }
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
    config_.screen->rectangle(0, config_.pos_y, kScreenWidth-5, kStatusBarHeight, myColours.red);
}

/* GUIHintBox */
GUIHintBox::GUIHintBox(GeepsGUIElementConfig_t config) 
: GeepsGUIElement(config) {


}

void GUIHintBox::Draw() {
    uint16_t y = 10;
    config_.screen->selectFont(Font_Terminal6x8);
    config_.screen->gText(10, y, (char *)"Hello World!\r\n", myColours.red);
    config_.screen->gText(10, y+config_.screen->characterSizeY(), (char *)"Doot Doot.\r\n", myColours.black);
    
    config_.screen->selectFont(Font_Terminal6x8);
    char hint_text_row[kRowNumChars];
    for (uint row = 0; row < kHintTextMaxLen / kRowNumChars; row++) {
        strncpy(hint_text_row, hint_text+(row*kRowNumChars), kRowNumChars);
        config_.screen->gText(config_.pos_x + kTextMargin, config_.pos_y + row*kCharHeight, hint_text_row, myColours.black);
    }
}