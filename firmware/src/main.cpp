#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pa1616s.hh"

#define GPS_UART_ID uart1
#define GPS_UART_BAUD 9600
#define GPS_UART_DATA_BITS 8
#define GPS_UART_STOP_BITS 1
#define GPS_UART_PARITY     UART_PARITY_NONE

#define GPS_UART_TX_PIN 4 // UART1 TX
#define GPS_UART_RX_PIN 5 // UART1 RX

const uint LED_PIN = 25;

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    puts("Hi hello starting program.\r\n");
    // while (1) {
    //     gpio_put(LED_PIN, 0);
    //     sleep_ms(250);
    //     gpio_put(LED_PIN, 1);
    //     puts("Hello World\n");
    //     sleep_ms(1000);
    // }

    PA1616S::PA1616SConfig_t gps_config = {
        .uart_id = GPS_UART_ID,
        .uart_baud = GPS_UART_BAUD,
        .uart_tx_pin = GPS_UART_TX_PIN,
        .uart_rx_pin = GPS_UART_RX_PIN
    };

    PA1616S gps = PA1616S(gps_config);
    gps.Init();

    while(true) {
        gpio_put(LED_PIN, 1);
        gps.Update();
        uart_puts(GPS_UART_ID, "hi");
        gpio_put(LED_PIN, 0);
        sleep_ms(10);
    }
}