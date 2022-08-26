#include "epaper.hh"
#include "hV_Configuration.h"
#include <string.h> // for string tools

#define SCREEN_WIDTH 104 // [pixels] Screen X dimension.
#define SCREEN_HEIGHT 212 // [pixels] Screen Y dimension.

#define HINT_TEXT_POS_Y 100 // [pixels]
#define HINT_TEXT_MARGIN 10 // [pixels]
// Use Terminal 6x8 font for hint.
#define HINT_TEXT_CHAR_WIDTH 6 // [pixels]
#define HINT_TEXT_CHAR_HEIGHT 8 // [pixels]
#define HINT_TEXT_ROW_NUM_CHARS ((SCREEN_WIDTH - 2*HINT_TEXT_MARGIN) / HINT_TEXT_CHAR_WIDTH) // [chars]

EPaperDisplay::EPaperDisplay(EPaper_Config_t config) :
    gps_packet(GGAPacket(NULL, 0)),
    config_(config)
{
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

    screen_ = new Screen_EPD_EXT3(eScreen_EPD_EXT3_213_0C, epd_pins, config_.spi_inst);
    
    // char gps_packet_init_string[] = "";
    // gps_packet = GGAPacket(gps_packet_init_string, 1), // initilaize with empty, invalid packet
}

void EPaperDisplay::Init() {
    // Init SPI pins
    gpio_set_function(config_.spi_clk_pin, GPIO_FUNC_SPI);
    gpio_set_function(config_.spi_mosi_pin, GPIO_FUNC_SPI);
    screen_->begin();
    screen_->setOrientation(6); // portrait
}

void EPaperDisplay::Clear() {
    screen_->clear();
}

void EPaperDisplay::Update() {
    uint16_t y = 10;
    screen_->selectFont(Font_Terminal6x8);
    screen_->gText(10, y, "Hello World!\r\n", myColours.red);
    screen_->gText(10, y+screen_->characterSizeY(), "Doot Doot.\r\n", myColours.black);
    
    screen_->selectFont(Font_Terminal6x8);
    char hint_text_row[HINT_TEXT_ROW_NUM_CHARS];
    for (uint row = 0; row < kHintTextMaxLen / HINT_TEXT_ROW_NUM_CHARS; row++) {
        strncpy(hint_text_row, hint_text+(row*HINT_TEXT_ROW_NUM_CHARS), HINT_TEXT_ROW_NUM_CHARS);
        screen_->gText(HINT_TEXT_MARGIN, HINT_TEXT_POS_Y + row*HINT_TEXT_CHAR_HEIGHT, hint_text_row, myColours.black);
    }
    screen_->flush();
}

// const pins_t kBoardGeepsRP2040 =
// {
//     .panelBusy = 13, ///< EXT3 pin 3 Red -> GP13
//     .panelDC = 12, ///< EXT3 pin 4 Orange -> GP12
//     .panelReset = 11, ///< EXT3 pin 5 Yellow -> GP11
//     .flashCS = NOT_CONNECTED, ///< EXT3 pin 8 Violet -> GP10
//     .panelCS = NOT_CONNECTED, ///< EXT3 pin 9 Grey -> GP17
//     .panelCSS = 14, ///< EXT3 pin 12 Grey2 -> GP14
//     .flashCSS = 15, ///< EXT3 pin 20 Black2 -> GP15
//     .touchReset = NOT_CONNECTED, ///< Separate touch board -> GP8
//     .touchInt = NOT_CONNECTED, ///< Separate touch board -> GP9
//     .cardCS = NOT_CONNECTED, ///< Separate SD-card board
//     .cardDetect = NOT_CONNECTED, ///< Separate SD-card board
// };