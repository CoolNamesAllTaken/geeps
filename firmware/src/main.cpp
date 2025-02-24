#include <stdio.h>

#include "buttons.hh"
#include "epaper.hh"
#include "geeps_gui.hh"
#include "gps_utils.hh"  // For calculating distance to hints.
#include "hardware/gpio.h"
#include "pa1616s.hh"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "scavenger_hunt.hh"
#include "sd_utils.hh"  // For accessing SD card GPIO IRQ callback.
#include "string.h"

const uint16_t kGPSUpdateIntervalMs = 5;        // [ms]
const uint16_t kDisplayUpdateIntervalMs = 500;  // [ms]
const uint16_t kStatusLEDBlinkIntervalMs = 500;
const uint16_t kMsPerSec = 1e3;
static const uint32_t kButtonDebounceIntervalMs = 10;

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

void up_button_callback() { scavenger_hunt.IncrementRenderedHint(); }
void down_button_callback() { scavenger_hunt.DecrementRenderedHint(); }

Buttons::ButtonsConfig buttons_config = {
    .button_pins = {BSP::button_top_pin, BSP::button_middle_pin, BSP::button_bottom_pin},
    .button_pressed_callbacks = {up_button_callback, nullptr, down_button_callback},
};
Buttons buttons = Buttons(buttons_config);

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

void gpio_irq_callback(uint gpio, uint32_t events) {
    if (gpio == BSP::sd_card_detect_pin) {
        // IRQ is from SD card.
        card_detect_callback(gpio, events);
        sprintf(scavenger_hunt.status_text, "SD card %s.", pSD->mounted ? "mounted" : "unmounted");
    } else {
        // IRQ is from buttons.
        buttons.GPIOIRQCallback(gpio, events);
    }
}

/**
 * Core 1 Main Function. Slow / blocking stuff happens here.
 */
void main_core1() {
    gui.AddElement(&status_bar);
    hint_box.width_chars = 25;
    hint_box.pos_x = 10;
    hint_box.pos_y = GUIStatusBar::kStatusBarHeight + 10;
    gui.AddElement(&hint_box);
    gui.AddElement(&splash_screen);

    display.Init();
    gui.Draw();

    char spinner_chars[] = {'|', '/', '-', '\\'};
    uint16_t spinner_char_index = 0;

    uint32_t last_spin_ms = to_ms_since_boot(get_absolute_time());
    bool gps_fix_acquired = false;
    bool sd_card_mounted = false;
    uint32_t display_refresh_time_ms = 0;

    // Initialization loop.
    while (!scavenger_hunt.skip_initialization && (!gps_fix_acquired || !sd_card_mounted)) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
        if (!gpio_get(BSP::button_top_pin && gpio_get(BSP::button_bottom_pin))) {
            // Skip initialization if both top and bottom buttons are held.
            scavenger_hunt.skip_initialization = true;
            scavenger_hunt.LogMessage("Skipped initialization.\n");
        }

        uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
        gps_fix_acquired =
            gps.latest_gga_packet.GetPositionFixIndicator() == GGAPacket::PositionFixIndicator_t::GPS_FIX;
        sd_card_mounted = pSD->mounted;

        if (timestamp_ms - last_spin_ms > 1000) {
            spinner_char_index++;
            spinner_char_index %= sizeof(spinner_chars);
            last_spin_ms = timestamp_ms;
        }

        snprintf(hint_box.text, Hint::kHintTextMaxLen, "Initializing... %c \n  SD CARD [%s]\n  GPS Fix [%s]\n",
                 spinner_chars[spinner_char_index], sd_card_mounted ? "OK  " : "  NO",
                 gps_fix_acquired ? "OK  " : "  NO");
        strcat(hint_box.text, scavenger_hunt.status_text);

        BlinkStatusLED(10);
        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            gui.Draw(true);
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }
    }

    // Placeholder stuff
    status_bar.progress_frac = 0.75;
    status_bar.battery_charge_frac = 0.5;

    // Main display and LED loop.
    while (true) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());

        Hint& rendered_hint = scavenger_hunt.hints[scavenger_hunt.rendered_hint_index];
        switch (rendered_hint.hint_type) {
            case Hint::kHintTypeText: {
                strncpy(hint_box.text, rendered_hint.hint_text, GUITextBox::kTextMaxLen);
                break;
            }
            case Hint::kHintTypeImage: {
                strncpy(hint_box.text, rendered_hint.hint_image_filename, GUITextBox::kTextMaxLen);
                break;
            }
            case Hint::kHintTypeDistance: {
                float distance_to_hint_m =
                    CalculateGeoidalDistance(gps.latest_gga_packet.GetLatitude(), gps.latest_gga_packet.GetLongitude(),
                                             rendered_hint.lat_deg, rendered_hint.lon_deg);
                snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Distance to hint: %.2f m", distance_to_hint_m);
                break;
            }
            case Hint::kHintTypeHeading: {
                float heading_to_hint_deg = CalculateHeadingToWaypoint(gps.latest_gga_packet.GetLatitude(),
                                                                       gps.latest_gga_packet.GetLongitude(),
                                                                       rendered_hint.lat_deg, rendered_hint.lon_deg);
                snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Heading to hint: %.2f deg", heading_to_hint_deg);
                break;
            }
            default: {
                strncpy(hint_box.text, "Invalid hint type.", GUITextBox::kTextMaxLen);
                break;
            }
        }

        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            gui.Draw(true);
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }

        BlinkStatusLED(2);
    }
}

/**
 * Main function. Fast stuff happens here.
 */
int main() {
    stdio_init_all();

    gpio_init(kStatusLEDPin);
    gpio_set_dir(kStatusLEDPin, GPIO_OUT);

    multicore_reset_core1();
    multicore_launch_core1(main_core1);

    gps.Init();

    // gpio_set_irq_callback(gpio_irq_callback);
    gpio_set_irq_enabled_with_callback(BSP::button_top_pin, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled(BSP::button_middle_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BSP::button_bottom_pin, GPIO_IRQ_EDGE_FALL, true);
    buttons.Init();
    scavenger_hunt.Init();
    gpio_set_irq_enabled(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    // TODO: Plugging in the SD card when the MCU is already started causes the SPI peripheral to fail. There's an
    // assert buried in the SD Card filesystem module that causes everything to lock up when this happens. Maybe just
    // wrap the MountSDCard() function in a watchdog timer?

    // Mimic SD card being plugged in.
    gpio_irq_callback(BSP::sd_card_detect_pin, GPIO_IRQ_EDGE_FALL);
    // Block on initializing scavenger hunt from SD card.
    while (!scavenger_hunt.skip_initialization && !pSD->mounted) {
        // Wait for SD card to be mounted.
    }
    scavenger_hunt.LoadHints();
    while (!scavenger_hunt.skip_initialization &&
           gps.latest_gga_packet.GetPositionFixIndicator() != GGAPacket::PositionFixIndicator_t::GPS_FIX) {
        // Wait for GPS fix.
        RefreshGPS();
    }

    while (true) {
        RefreshGPS();
        scavenger_hunt.Update(gps.latest_gga_packet.GetLatitude(), gps.latest_gga_packet.GetLongitude(),
                              gps.latest_gga_packet.GetUTCTimeUint());
        // strncpy(hint_box.text, scavenger_hunt.status_text, GUITextBox::kTextMaxLen);
        buttons.Update();
        status_bar.button_up_pressed = buttons.pressed[0];
        status_bar.button_center_pressed = buttons.pressed[1];
        status_bar.button_down_pressed = buttons.pressed[2];
    }
}