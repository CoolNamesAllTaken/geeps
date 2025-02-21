#pragma once

#include "Arduino.h"
#include "bsp.hh"
#include "hardware/gpio.h"
#include "hardware/spi.h"

class SPIInterface {
   public:
    inline void begin() {
        // Assume SPI peripheral is pre-configured.
    }
    inline void beginTransaction(SPISettings settings) {
        // Ignore settings, assume SPI peripheral is pre-configured.
        gpio_set_function(BSP::epaper_spi_clk_pin, GPIO_FUNC_SPI);
        gpio_set_function(BSP::epaper_spi_mosi_pin, GPIO_FUNC_SPI);

        spi_init(BSP::epaper_spi_inst, BSP::epaper_spi_clk_rate_hz);
        spi_set_format(BSP::epaper_spi_inst, BSP::epaper_spi_bits_per_transfer, SPI_CPOL_0, SPI_CPHA_0,
                       SPI_MSB_FIRST);  // SPI Mode 0, MSB first, 1 Byte
        // gpio_put(BSP::epaper_spi_cs_pin, 0);
    }

    inline void end() { gpio_put(BSP::epaper_spi_cs_pin, 1); }

    inline uint8_t transfer(uint8_t data) { return spi_write_blocking(BSP::epaper_spi_inst, &data, 1); }

   private:
};

extern SPIInterface SPI;