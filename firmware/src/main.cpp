#include <stdio.h>

#include "buttons.hh"
#include "epaper.hh"
#include "geeps_gui.hh"
#include "gps_utils.hh"  // For calculating distance to hints.
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pa1616s.hh"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "scavenger_hunt.hh"
#include "sd_utils.hh"  // For accessing SD card GPIO IRQ callback.
#include "servo.hh"
#include "string.h"

const uint16_t kGPSUpdateIntervalMs = 5;        // [ms]
const uint16_t kDisplayUpdateIntervalMs = 500;  // [ms]
const uint16_t kMsPerSec = 1e3;
static const uint32_t kButtonDebounceIntervalMs = 10;

const uint16_t kStatusLEDPin = 15;

SDUtil sd_util = SDUtil();
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
Servo servo = Servo({.pwm_pin = BSP::servo_pwm_pin, .enable_pin = BSP::servo_enable_pin});

GUIStatusBar status_bar = GUIStatusBar({});
GUITextBox hint_box = GUITextBox({.pos_x = 10, .pos_y = GUIStatusBar::kStatusBarHeight + 10});
GUIBitMap hint_image = GUIBitMap({.pos_x = 0, .pos_y = GUIStatusBar::kStatusBarHeight});
GUIBitMap splash_screen = GUIBitMap({});
GUICompass compass = GUICompass({.pos_x = 150, .pos_y = 75});
GUIMenu menu = GUIMenu({.pos_x = 10, .pos_y = 40});
GUINotification notification = GUINotification({.pos_x = 10, .pos_y = 40});

ScavengerHunt* scavenger_hunt_p = nullptr;

void up_button_callback() {
    status_bar.button_up_clicked_timestamp = to_ms_since_boot(get_absolute_time());
    if (menu.visible) {
        menu.ScrollPrev();
    } else {
        scavenger_hunt_p->DecrementRenderedHint();
    }
}
void center_button_callback() {
    status_bar.button_center_clicked_timestamp = to_ms_since_boot(get_absolute_time());
    if (menu.visible) {
        menu.Select();
    } else {
        scavenger_hunt_p->TryHint(scavenger_hunt_p->rendered_hint_index);
    }
}
void down_button_callback() {
    status_bar.button_down_clicked_timestamp = to_ms_since_boot(get_absolute_time());
    if (menu.visible) {
        menu.ScrollNext();
    } else {
        scavenger_hunt_p->IncrementRenderedHint();
    }
}

Buttons::ButtonsConfig buttons_config = {
    .button_pins = {BSP::button_top_pin, BSP::button_middle_pin, BSP::button_bottom_pin},
    .button_pressed_callbacks = {up_button_callback, center_button_callback, down_button_callback},
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

void BlinkStatusLED(uint16_t blink_period_ms, float duty_cycle = 0.5f) {
    static uint32_t last_on_timestamp_ms;

    uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
    if (timestamp_ms - last_on_timestamp_ms > blink_period_ms) {
        // Time to turn on the LED and start a blink interval.
        gpio_put(kStatusLEDPin, 1);
        last_on_timestamp_ms = timestamp_ms;
    } else if (timestamp_ms - last_on_timestamp_ms > blink_period_ms * duty_cycle) {
        // Time to turn off the LED to enforce 50% duty cycle.
        gpio_put(kStatusLEDPin, 0);
    }
}

void gpio_irq_callback(uint gpio, uint32_t events) { buttons.GPIOIRQCallback(gpio, events); }

void RefreshBatteryVoltage() {
    static constexpr float kADCCountsToVolts = 1.51f * 3.3f / 4095.0f;
    static constexpr float kADCLpFilterWeight = 0.5f;
    static float voltage = 0.0f;
    // Enable battery voltage sense.
    gpio_put(BSP::batt_vsense_enable_pin, 1);
    // Read analog voltage from batt_vsense pin
    adc_select_input(0);
    uint16_t adc_counts = adc_read();
    // Disable battery voltage sense.
    gpio_put(BSP::batt_vsense_enable_pin, 0);
    // Convert ADC counts to voltage and low pass filter it.
    voltage = (1.0f - kADCLpFilterWeight) * (adc_counts * kADCCountsToVolts) + kADCLpFilterWeight * voltage;
    // Battery % is linear representation of voltage between fully charged and nominal voltage (not actual good
    // representtion of capacity).
    status_bar.battery_charge_frac = max(min(1.0f, (voltage - 3.7f) / (4.2f - 3.7f)), 0.0f);
    // printf("UpdateBatteryVoltage: ADC Counts: %d, Voltage: %.2f V, Percent: %.2f%%\n", adc_counts, voltage,
    //    status_bar.battery_charge_frac * 100);

    // if (status_bar.battery_charge_frac < 0.05f) {
    //     scavenger_hunt->LogMessage("Battery critically low. Powering off.");
    //     delay_ms(2000);  // Allow display to update.
    //     PowerOff();
    // }
}

/**
 * Core 1 Main Function. Slow / blocking stuff happens here.
 */
void main_core1() {
    gui.AddElement(&status_bar);
    hint_image.visible = false;
    gui.AddElement(&hint_image);
    gui.AddElement(&hint_box);
    splash_screen.visible = false;
    gui.AddElement(&splash_screen);
    gui.AddElement(&compass);

    servo.Init();
    menu.AddRow((char*)"Open access hatch.", []() {
        servo.Enable();
        servo.SetAngle(0.0f);
        delay_ms(1000);
        servo.Disable();
    });
    menu.AddRow((char*)"Close access hatch.", []() {
        servo.Enable();
        servo.SetAngle(180.0f);
        delay_ms(1000);
        servo.Disable();
    });
    menu.AddRow((char*)"Reset scavenger hunt.", []() { scavenger_hunt_p->Reset(); });
    menu.AddRow((char*)"Power off (batt only).", []() {
        // Power off the system.
        scavenger_hunt_p->PowerOff();
    });
    menu.AddRow((char*)"Exit menu.", []() { menu.visible = false; });
    menu.visible = false;
    gui.AddElement(&menu);

    gui.AddElement(&notification);

    compass.visible = false;

    display.Init();
    gui.Draw();

    char spinner_chars[] = {'|', '/', '-', '\\'};
    uint16_t spinner_char_index = 0;

    uint32_t last_spin_ms = to_ms_since_boot(get_absolute_time());
    bool gps_fix_acquired = false;
    uint32_t display_refresh_time_ms = 0;

    // Initialization loop.
    while (!scavenger_hunt_p->skip_initialization && (!gps_fix_acquired || !sd_util.sd_card_mounted)) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
        if (!gpio_get(BSP::button_top_pin) && !gpio_get(BSP::button_bottom_pin)) {
            if (!gpio_get(BSP::button_middle_pin)) {
                // Open admin menu if all three buttons are held on bootup.
                menu.visible = true;
            } else {
                // Skip initialization if both top and bottom buttons are held on bootup.
                scavenger_hunt_p->skip_initialization = true;
                scavenger_hunt_p->LogMessage("Skipped initialization.");
            }
        }

        uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
        gps_fix_acquired =
            gps.latest_gga_packet.GetPositionFixIndicator() == GGAPacket::PositionFixIndicator_t::GPS_FIX;
        if (!sd_util.sd_card_mounted) {
            // Try mounting the SD card if it hasn't yet been mounted.
            sd_util.Mount();
        }

        if (timestamp_ms - last_spin_ms > 1000) {
            spinner_char_index++;
            spinner_char_index %= sizeof(spinner_chars);
            last_spin_ms = timestamp_ms;
        }

        snprintf(hint_box.text, Hint::kHintTextMaxLen, "Initializing... %c \n  SD CARD [%s]\n  GPS Fix [%s]\n",
                 spinner_chars[spinner_char_index], sd_util.sd_card_mounted ? "OK  " : "  NO",
                 gps_fix_acquired ? "OK  " : "  NO");
        strcat(hint_box.text, scavenger_hunt_p->status_text);

        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            gui.Draw(true);
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }
    }

    // Display splash screen for 2 seconds.
    splash_screen.ReadBitMapFromFile(scavenger_hunt_p->splash_image_filename);
    splash_screen.white_background = true;
    splash_screen.visible = true;
    gui.Draw(true);
    delay_ms(2000);
    splash_screen.visible = false;

    // Main display and LED loop.
    while (true) {
        uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());

        scavenger_hunt_p->Render();

        if (curr_time_ms >= display_refresh_time_ms) {
            // Refresh the display.
            gui.Draw(true);
            display_refresh_time_ms = curr_time_ms + kDisplayUpdateIntervalMs;
        }

        BlinkStatusLED(1000, 0.5);
    }
}

/**
 * Main function. Fast stuff happens here.
 */
int main() {
    stdio_init_all();

    gpio_init(kStatusLEDPin);
    gpio_set_dir(kStatusLEDPin, GPIO_OUT);

    // Enable battery voltage reading GPIO.
    gpio_init(BSP::batt_vsense_enable_pin);
    gpio_set_dir(BSP::batt_vsense_enable_pin, GPIO_OUT);
    gpio_put(BSP::batt_vsense_enable_pin, 0);  // Disable battery voltage sense by default.
    adc_init();
    adc_gpio_init(BSP::batt_vsense_pin);
    RefreshBatteryVoltage();

    scavenger_hunt_p = new ScavengerHunt(status_bar, hint_box, hint_image, compass, notification);

    multicore_reset_core1();
    multicore_launch_core1(main_core1);

    gps.Init();

    gpio_set_irq_enabled_with_callback(BSP::button_top_pin, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled(BSP::button_middle_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BSP::button_bottom_pin, GPIO_IRQ_EDGE_FALL, true);
    buttons.Init();
    scavenger_hunt_p->Init();

    // Mimic SD card being plugged in.
    gpio_irq_callback(BSP::sd_card_detect_pin, GPIO_IRQ_EDGE_FALL);
    // Block on initializing scavenger hunt from SD card.
    while (!scavenger_hunt_p->skip_initialization && !sd_util.sd_card_mounted) {
        // Wait for SD card to be mounted.
    }
    scavenger_hunt_p->LoadHints();
    while (!scavenger_hunt_p->skip_initialization &&
           gps.latest_gga_packet.GetPositionFixIndicator() != GGAPacket::PositionFixIndicator_t::GPS_FIX) {
        // Wait for GPS fix.
        BlinkStatusLED(100);
        RefreshGPS();
        RefreshBatteryVoltage();
    }

    while (true) {
        RefreshGPS();
        RefreshBatteryVoltage();
        scavenger_hunt_p->Update(gps.latest_gga_packet.GetLatitude(), gps.latest_gga_packet.GetLongitude(),
                                 gps.latest_gga_packet.GetUTCTimeUint());
        // strncpy(hint_box.text, scavenger_hunt->status_text, GUITextBox::kTextMaxLen);
        buttons.Update();
    }
}