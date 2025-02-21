#include "epaper.hh"

#include <string.h>  // for string tools

// #include "hV_Configuration.h"
#include "hV_List_Screens.h"

#define BITS_PER_BYTE 8

/**
 * @brief EPaperDisplay constructor.
 * @param[in] config Struct with configuration info for initializing the screen.
 */
EPaperDisplay::EPaperDisplay(EPaper_Config_t config) : config_(config), refresh_counter_(0) {
    pins_t epd_pins;
    epd_pins.panelBusy = config_.panel_busy_pin;
    epd_pins.panelDC = config_.panel_data_command_pin;
    epd_pins.panelReset = config_.panel_reset_pin;
    epd_pins.flashCS = config_.flash_cs_pin;
    epd_pins.panelCS = config_.panel_cs_pin;
    epd_pins.flashCSS = config_.flash_css_pin;
    epd_pins.panelCSS = config_.panel_css_pin;
    epd_pins.touchReset = config_.touch_reset_pin;
    epd_pins.touchInt = config_.touch_interrupt_pin;
    epd_pins.cardCS = config_.card_cs_pin;
    epd_pins.cardDetect = config_.card_detect_pin;
    epd_pins.panelPower = NOT_CONNECTED;

    screen_ = new Screen_EPD_EXT3_Fast(eScreen_EPD_213_PS_0E, epd_pins);
    size_x_ = screen_->screenSizeX();  // [pixels] Hardcoded for selected screen.
    size_y_ = screen_->screenSizeY();  // [pixe;s] Hardcoded for selected screen.
}

/**
 * @brief Starts up the EPaper screen to do what we want.
 */
void EPaperDisplay::Init() {
    gpio_init(config_.panel_enable);
    gpio_set_dir(config_.panel_enable, GPIO_OUT);
    gpio_put(config_.panel_enable, 0);  // Power on

    delay_ms(500);

    // SPI initialization taken care of in SPI.h.

    screen_->begin();
    screen_->setOrientation(3);  // landscape
    screen_->regenerate();       // clear ghosts
}

/**
 * @brief Wipes the EPaper screen of all drawings.
 */
void EPaperDisplay::Clear() { screen_->clear(myColours.white); }

/**
 * @brief Draws the contents onto the EPaper screen.
 */
void EPaperDisplay::Update(bool fast) {
    // if (refresh_counter_ >= kDisplayRegenerationInterval) {
    //     screen_->regenerate();
    //     refresh_counter_ = 0;
    // }
    // screen_->regenerate();
    if (fast) {
        // screen_->flush_fast();
        screen_->flush();
    } else {
        screen_->flush();
    }
    // refresh_counter_++;
}

/**
 * @brief Returns the x dimension of the screen.
 * @retval X dimension, in pixels.
 */
uint16_t EPaperDisplay::GetSizeX() { return size_x_; }

/**
 * @brief Returns the y dimension of the screen.
 * @retval Y dimension, in pixels.
 */
uint16_t EPaperDisplay::GetSizeY() { return size_y_; }

// /**
//  * @brief Getter for a pointer to the private EPaper screen instance.
//  * @retval Pointer to the screen's Screen_EPD_EXT3 object.
//  */
// Screen_EPD_EXT3 * EPaperDisplay::GetScreen() {
//     return screen_;
// }

/**
 * @brief Helper function that converts an EPaperDisplay color enum into a color used by the underlying
 * display class. Used to hide underlying display grossness from users of the EPaperDisplay class.
 * @param[in] color EPaper_Color_t color to convert.
 * @retval Color converted to type used by underlying display class.
 */
uint16_t EPaperDisplay::EPaperColorsToScreenColors(EPaper_Color_t color) {
    switch (color) {
        case EPAPER_RED:
            return myColours.red;
            break;
        default: /* EPAPER_BLACK, EPAPER_NONE */
            return myColours.black;
            break;
    }
}

/**
 * @brief Draws a line on the display.
 * @param[in] pos_x X position of start of line.
 * @param[in] pos_y Y position of start of line.
 * @param[in] end_x X position of end of line.
 * @param[in] end_y Y position of end of line.
 * @param[in] color EPaper_Color_t of line.
 */
void EPaperDisplay::DrawLine(uint16_t pos_x, uint16_t pos_y, uint16_t end_x, uint16_t end_y, EPaper_Color_t color) {
    screen_->line(pos_x, pos_y, end_x, end_y, EPaperColorsToScreenColors(color));
}

/**
 * @brief Draws a point on the display.
 * @param[in] pos_x X position of point.
 * @param[in] pos_y Y position of point.
 * @param[in] color EPaper_Color_t of point.
 */
void EPaperDisplay::DrawPoint(uint16_t pos_x, uint16_t pos_y, EPaper_Color_t color) {
    // Note: underlying class is expected to check bounds / handle errors.
    screen_->point(pos_x, pos_y, EPaperColorsToScreenColors(color));
}

/**
 * @brief Draws a rectangle on the display.
 * @param[in] pos_x Top left corner of text, x-coordinate.
 * @param[in] pos_y Top left corner of text, y-coordinate.
 * @param[in] size_x X dimension of rectangle.
 * @param[in] size_y Y dimensin of rectangle.
 * @param[in] color EPaper_Color_t of rectangle.
 * @param[in] filled Flag indicating whether to make the rectangle solid or wireframe. True = solid, false = wireframe.
 */
void EPaperDisplay::DrawRectangle(uint16_t pos_x, uint16_t pos_y, uint16_t size_x, uint16_t size_y,
                                  EPaper_Color_t color, bool filled) {
    screen_->setPenSolid(filled);
    screen_->rectangle(pos_x, pos_y, pos_x + size_x, pos_y + size_y, EPaperColorsToScreenColors(color));
}

/**
 * @brief Draws text on the display.
 * @param[in] pos_x Top left corner of text, x-coordinate.
 * @param[in] pos_y Top left corner of text, y-coordinate.
 * @param[in] text Char buffer to display. Must end in `\0`.
 * @param[in] text_color EPaper_Color_t of lettering.
 * @param[in] background_color EPaper_Color_t of text background.
 * @param[in] font EPaper_Font_t to use for text.
 */
void EPaperDisplay::DrawText(uint16_t pos_x, uint16_t pos_y, char *text, EPaper_Color_t text_color,
                             EPaper_Color_t background_color, EPaper_Font_t font) {
    // Select font with screen-specific font types.
    fontNumber_e screen_font;
    switch (font) {
        case EPAPER_TERMINAL_8X12:
            screen_font = Font_Terminal8x12;
            break;
        case EPAPER_TERMINAL_12X16:
            screen_font = Font_Terminal12x16;
            break;
        case EPAPER_TERMINAL_16X24:
            screen_font = Font_Terminal16x24;
            break;
        default: /* EPAPER_TERMINAL_6X8 */
            screen_font = Font_Terminal6x8;
            break;
    }
    screen_->selectFont(screen_font);

    // Draw text
    uint16_t screen_text_color = EPaperColorsToScreenColors(text_color);
    uint16_t screen_background_color = EPaperColorsToScreenColors(background_color);
    screen_->gText(pos_x, pos_y, text, screen_text_color, screen_background_color);
}

/**
 * @brief Draw a bitmap image on the display.
 * @param[in] pos_x X-coordinate of top left corner of bitmap.
 * @param[in] pos_y Y-coordinate of top left corner of bitmap.
 * @param[in] bitmap Byte array of pixels. Each byte represents 8 horizontal pixels, with the MSb being the leftmost
 * pixel. Pixels are drawn with the selected color if their corresponding bit is set. Otherwise the pixel is ignored.
 *                      Bitmap must be at least of size size_x * size_y / BITS_PER_BYTE, or errors will occur.
 * @param[in] size_x Width of bitmap, in pixels.
 * @param[in] size_y Height of bitmap, in pixels.
 * @param[in] color EPaper_Color_t of bitmap.
 */
void EPaperDisplay::DrawBitMap(uint16_t pos_x, uint16_t pos_y, const uint8_t bitmap[], uint16_t size_x, uint16_t size_y,
                               EPaper_Color_t color) {
    uint16_t row_width =
        (size_x + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE;  // [Bytes] Each new row starts with a new byte.
    for (uint16_t row = 0; row < size_y; row++) {
        for (uint16_t col = 0; col < size_x; col++) {
            uint16_t pixel_index = row * row_width * BITS_PER_BYTE + col;
            uint8_t pixel_chunk = bitmap[pixel_index / BITS_PER_BYTE];
            if (!(pixel_chunk & (0b10000000 >> (pixel_index % BITS_PER_BYTE)))) {
                // 0 bit indicates opaque pixel. 1 bit indicates pixel that will not be filled.
                DrawPoint(pos_x + col, pos_y + row, color);
            }
        }
    }
}