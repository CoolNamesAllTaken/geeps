#pragma once
#include "hardware/spi.h"
#include "hardware/uart.h"
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

    static const uint16_t button_top_pin = 0;
    static const uint16_t button_middle_pin = 1;
    static const uint16_t button_bottom_pin = 2;

    static uart_inst_t *gps_uart_inst;
    static const uint32_t gps_uart_baud = 9600;
    static const uint16_t gps_uart_data_bits = 8;
    static const uint16_t gps_uart_stop_bits = 1;
    static const uart_parity_t gps_uart_parity = UART_PARITY_NONE;
    static const uint16_t gps_fix_pin = 3;
    static const uint16_t gps_tx_pin = 4;
    static const uint16_t gps_rx_pin = 5;
    static const uint16_t gps_reset_pin = 6;
};

inline spi_inst_t *BSP::epaper_spi_inst = spi1;
inline spi_inst_t *BSP::sd_card_spi_inst = spi0;
inline uart_inst_t *BSP::gps_uart_inst = uart1;
