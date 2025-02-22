#include <stdio.h>

#include "epaper.hh"
#include "geeps_gui.hh"
#include "hardware/gpio.h"
#include "pa1616s.hh"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "scavenger_hunt.hh"
#include "string.h"
// #include "gui_bitmaps.hh"

// #define GPS_UART_ID uart1
// #define GPS_UART_BAUD 9600
// #define GPS_UART_DATA_BITS 8
// #define GPS_UART_STOP_BITS 1
// #define GPS_UART_PARITY UART_PARITY_NONE

// #define GPS_UART_TX_PIN 4 // UART1 TX
// #define GPS_UART_RX_PIN 5 // UART1 RX

const uint16_t kGPSUpdateIntervalMs = 5;          // [ms]
const uint16_t kDisplayUpdateIntervalMs = 2'000;  // [ms]
const uint16_t kStatusLEDBlinkIntervalMs = 500;
const uint16_t kMsPerSec = 1e3;

const uint16_t kStatusLEDPin = 15;

PA1616S gps = PA1616S({
    .uart_id = BSP::gps_uart_inst,
    .uart_baud = BSP::gps_uart_baud,
    .uart_tx_pin = BSP::gps_tx_pin,
    .uart_rx_pin = BSP::gps_rx_pin,
    .data_bits = BSP::gps_uart_data_bits,
    .stop_bits = BSP::gps_uart_stop_bits,
    .parity = BSP::gps_uart_parity,
    .reset_pin = BSP::gps_reset_pin,
    .fix_pin = BSP::gps_fix_pin,
});
EPaperDisplay display = EPaperDisplay({});
GeepsGUI gui = GeepsGUI({.display = display});

GUIStatusBar status_bar = GUIStatusBar({});
GUITextBox hint_box = GUITextBox({.pos_x = 10, .pos_y = 30});
GUIBitMap splash_screen = GUIBitMap({});

ScavengerHunt scavenger_hunt = ScavengerHunt();

void RefreshGPS() {
    gps.Update();
    gps.latest_gga_packet.GetUTCTimeStr(status_bar.time_string);
    float latitude_deg = gps.latest_gga_packet.GetLatitude();
    float longitude_deg = gps.latest_gga_packet.GetLongitude();
    sprintf(status_bar.latitude_string, "%f%c", latitude_deg, latitude_deg > 0 ? 'N' : 'S');
    sprintf(status_bar.longitude_string, "%f%c", longitude_deg, longitude_deg > 0 ? 'E' : 'W');
    status_bar.num_satellites = gps.latest_gga_packet.GetSatellitesUsed();
}

void BlinkStatusLED(uint16_t blink_rate_hz) {
    static uint32_t last_on_timestamp_ms;

    uint32_t blink_interval_ms = kMsPerSec / blink_rate_hz;
    uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
    if (timestamp_ms - last_on_timestamp_ms > blink_interval_ms) {
        // TIme to turn on the LED and start a blink interval.
        gpio_put(kStatusLEDPin, 1);
        last_on_timestamp_ms = timestamp_ms;
    } else if (timestamp_ms - last_on_timestamp_ms > blink_interval_ms / 2) {
        // Time to turn off the LED to enforce 50% duty cycle.
        gpio_put(kStatusLEDPin, 0);
    }
}

void main_core1() {
    gps.Init();
    scavenger_hunt.Init();

    while (true) {
        RefreshGPS();
        strncpy(hint_box.text, scavenger_hunt.status_text, GUITextBox::kTextMaxLen);
    }
}

int main() {
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(kStatusLEDPin, "On-board LED"));

    stdio_init_all();

    gpio_init(kStatusLEDPin);
    gpio_set_dir(kStatusLEDPin, GPIO_OUT);

    multicore_reset_core1();
    multicore_launch_core1(main_core1);

    gui.AddElement(&status_bar);
    hint_box.width_chars = 25;
    hint_box.pos_x = 10;
    hint_box.pos_y = GUIStatusBar::kStatusBarHeight + 10;
    gui.AddElement(&hint_box);
    gui.AddElement(&splash_screen);

    display.Init();

    snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Waiting for GPS fix\n");
    gui.Draw();

    uint32_t last_dot_ms = to_ms_since_boot(get_absolute_time());
    while (gps.latest_gga_packet.GetPositionFixIndicator() != GGAPacket::PositionFixIndicator_t::GPS_FIX) {
        uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
        if (timestamp_ms - last_dot_ms > 1000) {
            strncat(hint_box.text, ".", 2);
            last_dot_ms = timestamp_ms;
            gui.Draw(true);
        }

        BlinkStatusLED(10);
    }
    snprintf(hint_box.text, GUITextBox::kTextMaxLen, "GPS fix acquired!\n");
    gui.Draw(false);

    // Placeholder stuff
    status_bar.progress_frac = 0.75;
    status_bar.battery_charge_frac = 0.5;

    uint32_t display_refresh_time_ms = 0;
    while (true) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            gui.Draw(true);
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }

        BlinkStatusLED(2);
    }
}