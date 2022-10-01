#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pa1616s.hh"
#include "epaper.hh"
#include "geeps_gui.hh"
// #include "gui_bitmaps.hh"

#define GPS_UART_ID         uart1
#define GPS_UART_BAUD       9600
#define GPS_UART_DATA_BITS  8
#define GPS_UART_STOP_BITS  1
#define GPS_UART_PARITY     UART_PARITY_NONE

#define GPS_UART_TX_PIN 4 // UART1 TX
#define GPS_UART_RX_PIN 5 // UART1 RX

const uint16_t kGPSUpdateInterval = 50; // [ms]
const uint16_t kDisplayUpdateInterval = 10000; // [ms]

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

    // status_bar->Draw();
    // Latitude
    char latitude_str[NMEAPacket::kMaxPacketFieldLen];
    char longitude_str[NMEAPacket::kMaxPacketFieldLen];
    gps->latest_gga_packet.GetLatitudeStr(latitude_str);
    gps->latest_gga_packet.GetLongitudeStr(longitude_str);
    display->DrawText(0, 200, latitude_str);
    display->DrawText(0, 220, longitude_str);

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

    // EPaper display testing
    display->DrawText(
        0, 0, 
        (char *)"Black6x8", 
        EPaperDisplay::EPAPER_BLACK,
        EPaperDisplay::EPAPER_NONE,
        EPaperDisplay::EPAPER_TERMINAL_6X8);
    display->DrawText(
        0, 15, 
        (char *)"12x16", 
        EPaperDisplay::EPAPER_BLACK,
        EPaperDisplay::EPAPER_RED,
        EPaperDisplay::EPAPER_TERMINAL_12X16);
    display->DrawText(
        0, 30,
        (char *)"16x24",
        EPaperDisplay::EPAPER_RED,
        EPaperDisplay::EPAPER_NONE,
        EPaperDisplay::EPAPER_TERMINAL_16X24);
    display->DrawRectangle(0, 50, 20, 20, EPaperDisplay::EPAPER_RED, true);
    display->DrawRectangle(20, 50, 20, 20, EPaperDisplay::EPAPER_BLACK, true);
    display->DrawRectangle(40, 50, 20, 20, EPaperDisplay::EPAPER_RED, false);
    display->DrawRectangle(60, 50, 20, 20, EPaperDisplay::EPAPER_BLACK, false);
    display->DrawPoint(85, 50, EPaperDisplay::EPAPER_RED);
    display->DrawBitmap(0, 100, satellite_icon_20x20, 20, 20, EPaperDisplay::EPAPER_RED);
    display->DrawBitmap(30, 100, satellite_icon_15x15, 15, 15, EPaperDisplay::EPAPER_BLACK);
    display->Update();

    // GeepsGUIElement::GeepsGUIElementConfig_t status_bar_config;
    // status_bar_config.screen = display->GetScreen();
    // status_bar = new GUIStatusBar(status_bar_config);

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