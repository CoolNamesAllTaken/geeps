#include "epaper.hh"
#include "hV_Configuration.h"
#include <string.h> // for string tools

/**
 * @brief EPaperDisplay constructor.
 * @param[in] config Struct with configuration info for initializing the screen.
 */
EPaperDisplay::EPaperDisplay(EPaper_Config_t config)
:config_(config) {
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

/**
 * @brief Starts up the EPaper screen to do what we want.
 */
void EPaperDisplay::Init() {
    // Init SPI pins
    gpio_set_function(config_.spi_clk_pin, GPIO_FUNC_SPI);
    gpio_set_function(config_.spi_mosi_pin, GPIO_FUNC_SPI);
    screen_->begin();
    screen_->setOrientation(6); // portrait
}

/**
 * @brief Wipes the EPaper screen of all drawings.
 */
void EPaperDisplay::Clear() {
    screen_->clear();
}

/**
 * @brief Draws the contents onto the EPaper screen.
 */
void EPaperDisplay::Update() {
    screen_->flush();
}

/**
 * @brief Getter for a pointer to the private EPaper screen instance.
 * @retval Pointer to the screen's Screen_EPD_EXT3 object.
 */
Screen_EPD_EXT3 * EPaperDisplay::GetScreen() {
    return screen_;
}