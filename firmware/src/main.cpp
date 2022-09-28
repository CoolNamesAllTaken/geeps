#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pa1616s.hh"
#include "epaper.hh"
#include "geeps_gui.hh"

#define GPS_UART_ID         uart1
#define GPS_UART_BAUD       9600
#define GPS_UART_DATA_BITS  8
#define GPS_UART_STOP_BITS  1
#define GPS_UART_PARITY     UART_PARITY_NONE

#define GPS_UART_TX_PIN 4 // UART1 TX
#define GPS_UART_RX_PIN 5 // UART1 RX

const uint16_t kGPSUpdateInterval = 50; // [ms]
const uint16_t kDisplayUpdateInterval = 5000; // [ms]

const uint16_t LED_PIN = 25;

PA1616S * gps = NULL;
EPaperDisplay * display = NULL;
GUIStatusBar * status_bar = NULL;

void RefreshGPS() {
    gpio_put(LED_PIN, 1);
    gps->Update();
    gpio_put(LED_PIN, 0);
}

void RefreshScreen() {
    display->Clear();

    status_bar->Draw();

    display->Update();
}

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    puts("Hi hello starting program.\r\n");

    EPaperDisplay::EPaper_Config_t display_config;
    display = new EPaperDisplay(display_config);
    display->Init();
    display->Clear();
    // display->Update();

    GeepsGUIElement::GeepsGUIElementConfig_t status_bar_config;
    status_bar_config.screen = display->GetScreen();
    status_bar = new GUIStatusBar(status_bar_config);

    PA1616S::PA1616SConfig_t gps_config = {
        .uart_id = GPS_UART_ID,
        .uart_baud = GPS_UART_BAUD,
        .uart_tx_pin = GPS_UART_TX_PIN,
        .uart_rx_pin = GPS_UART_RX_PIN
    };

    gps = new PA1616S(gps_config);
    gps->Init();

    uint32_t gps_refresh_time_ms = 0;
    uint32_t display_refresh_time_ms = 0;
    while(true) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
        if (curr_time_ms >= gps_refresh_time_ms) {
            // Refresh the GPS.
            RefreshGPS();
            gps_refresh_time_ms = curr_time_ms + kGPSUpdateInterval;
        }
        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            RefreshScreen();
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateInterval;
        }
    }
}