#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pa1616s.hh"
#include "epaper.hh"
#include "geeps_gui.hh"
#include "string.h"
// #include "gui_bitmaps.hh"

// #define GPS_UART_ID uart1
// #define GPS_UART_BAUD 9600
// #define GPS_UART_DATA_BITS 8
// #define GPS_UART_STOP_BITS 1
// #define GPS_UART_PARITY UART_PARITY_NONE

// #define GPS_UART_TX_PIN 4 // UART1 TX
// #define GPS_UART_RX_PIN 5 // UART1 RX

const uint16_t kGPSUpdateIntervalMs = 5;        // [ms]
const uint16_t kDisplayUpdateIntervalMs = 1000; // [ms]
const uint16_t kStatusLEDBlinkIntervalMs = 500;
const uint16_t kMsPerSec = 1e3;

const uint16_t kStatusLEDPin = 15;

PA1616S gps = PA1616S({});
EPaperDisplay display = EPaperDisplay({});
GeepsGUI gui = GeepsGUI({.display = display});

GUIStatusBar status_bar = GUIStatusBar({});
GUITextBox hint_box = GUITextBox({.pos_x = 10, .pos_y = 30});
GUIBitMap splash_screen = GUIBitMap({});

void RefreshGPS()
{
    // gpio_put(kStatusLEDPin, 1);
    gps.Update();
    // gpio_put(kStatusLEDPin, 0);
    gps.latest_gga_packet.GetUTCTimeStr(status_bar.time_string);
    gps.latest_gga_packet.GetLatitudeStr(status_bar.latitude_string);
    gps.latest_gga_packet.GetLongitudeStr(status_bar.longitude_string);
    status_bar.num_satellites = gps.latest_gga_packet.GetSatellitesUsed();
}

void BlinkStatusLED(uint16_t blink_rate_hz)
{
    static uint32_t last_on_timestamp_ms;

    uint32_t blink_interval_ms = kMsPerSec / blink_rate_hz;
    uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
    if (timestamp_ms - last_on_timestamp_ms > blink_interval_ms)
    {
        // TIme to turn on the LED and start a blink interval.
        gpio_put(kStatusLEDPin, 1);
        last_on_timestamp_ms = timestamp_ms;
    }
    else if (timestamp_ms - last_on_timestamp_ms > blink_interval_ms / 2)
    {
        // Time to turn off the LED to enforce 50% duty cycle.
        gpio_put(kStatusLEDPin, 0);
    }
}

int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(kStatusLEDPin, "On-board LED"));

    stdio_init_all();

    gpio_init(kStatusLEDPin);
    gpio_set_dir(kStatusLEDPin, GPIO_OUT);

    gps.Init();

    gui.AddElement(&status_bar);
    hint_box.width_chars = 20;
    gui.AddElement(&hint_box);
    gui.AddElement(&splash_screen);

    display.Init();

    snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Hello!\nBooting up...\n");
    gui.Draw();

    uint32_t last_dot_ms = to_ms_since_boot(get_absolute_time());
    while (gps.latest_gga_packet.GetPositionFixIndicator() != GGAPacket::PositionFixIndicator_t::GPS_FIX)
    {
        uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
        if (timestamp_ms - last_dot_ms > 1000)
        {
            strncat(hint_box.text, ".", 2);
            last_dot_ms = timestamp_ms;
            gui.Draw(true);
        }

        gps.Update();
        status_bar.num_satellites = gps.latest_gga_packet.GetSatellitesUsed();
        gps.latest_gga_packet.GetLatitudeStr(status_bar.latitude_string);
        gps.latest_gga_packet.GetLongitudeStr(status_bar.longitude_string);
        BlinkStatusLED(10);
    }

    // while(true) {
    //     gpio_put(kStatusLEDPin, 1);
    //     sleep_ms(500);
    //     gpio_put(kStatusLEDPin, 0);
    //     sleep_ms(500);
    //     printf("timestamp: %d\r\n", to_ms_since_boot(get_absolute_time()));
    // }

    uint32_t gps_refresh_time_ms = 0;
    uint32_t display_refresh_time_ms = 0;
    uint32_t status_led_last_toggled_timestamp_ms = 0;
    while (true)
    {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
        printf("timestamp: %d\r\n", to_ms_since_boot(get_absolute_time()));
        if (curr_time_ms >= gps_refresh_time_ms)
        {
            // Refresh the GPS.
            RefreshGPS();
            gps_refresh_time_ms = curr_time_ms + kGPSUpdateIntervalMs;
        }
        if (curr_time_ms >= display_refresh_time_ms)
        {
            // Refresh the display.

            display.Clear();
            gui.Draw();
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }
        if (curr_time_ms - status_led_last_toggled_timestamp_ms > kStatusLEDBlinkIntervalMs)
        {
            bool curr_out_level = gpio_get_out_level(kStatusLEDPin);
            gpio_put(kStatusLEDPin, !curr_out_level);
            status_led_last_toggled_timestamp_ms = curr_time_ms;
        }
    }
}