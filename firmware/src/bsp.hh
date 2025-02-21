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
};

inline spi_inst_t *BSP::epaper_spi_inst = spi1;
