#ifndef _EPAPER_HH_
#define _EPAPER_HH_

#include "pico/stdlib.h"
#include "hardware/gpio.h" // for UART inst
#include "hardware/spi.h"
#include "hV_Configuration.h" // for pins_t object
#include "Screen_EPD_EXT3.h" // for screen object
#include "nmea_utils.hh" // for GPS stuff

class EPaperDisplay {
public:
    // Configuration struct used to define GPIO and SPI config.
    typedef struct {
        uint8_t panel_busy_pin = 8;
        uint8_t panel_data_command_pin = 6;
        uint8_t panel_reset_pin = 7;
        uint8_t flash_cs_pin = NOT_CONNECTED;
        uint8_t panel_cs_pin = 12;
        uint8_t flash_css_pin = NOT_CONNECTED;
        uint8_t panel_css_pin = 13;
        uint8_t touch_reset_pin = NOT_CONNECTED;
        uint8_t touch_interrupt_pin = NOT_CONNECTED;
        uint8_t card_cs_pin = NOT_CONNECTED;
        uint8_t card_detect_pin = NOT_CONNECTED;
        
        spi_inst_t * spi_inst = spi1;
        uint spi_clk_pin = 10;
        uint spi_mosi_pin = 11;
    } EPaper_Config_t;

    EPaperDisplay(EPaper_Config_t config);
    void Init();
    void Clear();
    void Update();

    static const uint kHintTextMaxLen = 300;

    GGAPacket gps_packet;
    char hint_text[kHintTextMaxLen];

private:
    EPaper_Config_t config_;
    Screen_EPD_EXT3 * screen_;

};

#endif /* _EPAPER_HH_ */