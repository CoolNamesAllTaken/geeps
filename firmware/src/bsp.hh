#pragma once
#include "hardware/spi.h"
#include "stdint.h"

class BSP {
   public:
    static const uint16_t epaper_spi_panel_busy_pin = 9;
    static const uint16_t epaper_spi_data_command_pin = 7;
    static const uint16_t epaper_spi_reset_pin = 8;
    static const uint16_t epaper_spi_cs_pin = 12;
    static const uint16_t epaper_spi_clk_pin = 10;
    static const uint16_t epaper_spi_mosi_pin = 11;
    static const uint16_t epaper_power_enable_pin = 23;
    static const uint32_t epaper_spi_clk_rate_hz = 4'000'000;  // Default to 4MHz, don't exceed 16MHz.
    static const uint16_t epaper_spi_bits_per_transfer = 8;
    static spi_inst_t *epaper_spi_inst;

    static const uint16_t sd_card_detect_pin = 17;
    static const uint16_t sd_card_clk_pin = 18;
    static const uint16_t sd_card_mosi_pin = 19;
    static const uint16_t sd_card_miso_pin = 20;
    static const uint16_t sd_card_cs_pin = 21;
    static const uint32_t sd_card_spi_clk_rate_hz = 12'000'000;  // 12 MHz
    static spi_inst_t *sd_card_spi_inst;
};

inline spi_inst_t *BSP::epaper_spi_inst = spi1;
inline spi_inst_t *BSP::sd_card_spi_inst = spi0;
