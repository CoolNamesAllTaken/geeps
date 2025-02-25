#ifndef _EPAPER_HH_
#define _EPAPER_HH_

#include "Screen_EPD_EXT3.h"  // for screen object
#include "bsp.hh"
#include "hV_Configuration.h"  // for pins_t object
#include "hardware/gpio.h"     // for UART inst
#include "hardware/spi.h"
#include "nmea_utils.hh"  // for GPS stuff
#include "pico/stdlib.h"

class EPaperDisplay {
   public:
    const uint16_t kDisplayRegenerationInterval =
        5;  // Number of fast updates before a regeneration is forced to clear ghosting.

    // Configuration struct used to define GPIO and SPI config.
    typedef struct {
        uint8_t panel_busy_pin = BSP::epaper_spi_panel_busy_pin;
        uint8_t panel_data_command_pin = BSP::epaper_spi_data_command_pin;
        uint8_t panel_reset_pin = BSP::epaper_spi_reset_pin;
        uint8_t flash_cs_pin = NOT_CONNECTED;
        uint8_t panel_cs_pin = BSP::epaper_spi_cs_pin;
        uint8_t flash_css_pin = NOT_CONNECTED;
        uint8_t panel_css_pin = NOT_CONNECTED;
        uint8_t touch_reset_pin = NOT_CONNECTED;
        uint8_t touch_interrupt_pin = NOT_CONNECTED;
        uint8_t card_cs_pin = NOT_CONNECTED;
        uint8_t card_detect_pin = NOT_CONNECTED;

        spi_inst_t *spi_inst = BSP::epaper_spi_inst;
        uint32_t spi_clk_rate_hz = BSP::epaper_spi_clk_rate_hz;
        uint spi_clk_pin = BSP::epaper_spi_clk_pin;
        uint spi_mosi_pin = BSP::epaper_spi_mosi_pin;

        uint16_t panel_enable = BSP::epaper_power_enable_pin;  // Power enable pin.
    } EPaper_Config_t;

    typedef enum { EPAPER_NONE = 0, EPAPER_WHITE, EPAPER_BLACK, EPAPER_RED } EPaper_Color_t;

    typedef enum {
        EPAPER_TERMINAL_6X8 = 0,
        EPAPER_TERMINAL_8X12,
        EPAPER_TERMINAL_12X16,
        EPAPER_TERMINAL_16X24
    } EPaper_Font_t;

    EPaperDisplay(EPaper_Config_t config);
    void Init();
    void Clear();
    void Update(bool fast = false);

    uint16_t GetSizeX();
    uint16_t GetSizeY();

    void DrawLine(uint16_t pos_x, uint16_t pos_y, uint16_t end_x, uint16_t end_y, EPaper_Color_t color);
    void DrawPoint(uint16_t pos_x, uint16_t pos_y, EPaper_Color_t color);
    void DrawRectangle(uint16_t pos_x, uint16_t pos_y, uint16_t size_x, uint16_t size_y, EPaper_Color_t color,
                       bool filled = false);
    void DrawCircle(uint16_t pos_x, uint16_t pos_y, uint16_t radius, EPaper_Color_t color, bool filled = false);
    void DrawTriangle(uint16_t pos_x1, uint16_t pos_y1, uint16_t pos_x2, uint16_t pos_y2, uint16_t pos_x3,
                      uint16_t pos_y3, EPaper_Color_t color, bool filled = false);
    void DrawText(uint16_t pos_x, uint16_t pos_y,
                  char *text,  // Should probably be const except that we pass it unprotected to gText().
                  EPaper_Color_t text_color = EPAPER_BLACK, EPaper_Color_t background_color = EPAPER_NONE,
                  EPaper_Font_t font = EPAPER_TERMINAL_6X8);
    void DrawBitMap(uint16_t pos_x, uint16_t pos_y, const uint8_t bitmap[], uint16_t size_x, uint16_t size_y,
                    EPaper_Color_t color);

    // Screen_EPD_EXT3 * GetScreen();

   private:
    EPaper_Config_t config_;
    Screen_EPD_EXT3_Fast *screen_;
    uint16_t size_x_;
    uint16_t size_y_;
    uint16_t refresh_counter_;

    uint16_t EPaperColorsToScreenColors(EPaper_Color_t color);
};

extern EPaperDisplay display;
#endif /* _EPAPER_HH_ */