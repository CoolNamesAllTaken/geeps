#pragma once
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "stdint.h"

class Buttons {
   public:
    static const uint16_t kNumButtons = 3;
    static const uint32_t kDebounceIntervalMs = 10;

    struct ButtonsConfig {
        uint16_t button_pins[kNumButtons];
        void (*button_pressed_callbacks[kNumButtons])() = {nullptr};
    };

    Buttons(ButtonsConfig config_in) : config_(config_in) {};

    /**
     * Sets up each button, but requires that interrupts get set up externally.
     */
    inline void Init() {
        for (uint16_t i = 0; i < kNumButtons; i++) {
            gpio_init(config_.button_pins[i]);
            gpio_set_dir(config_.button_pins[i], GPIO_IN);
        }
    }

    inline void GPIOIRQCallback(uint16_t gpio, uint32_t events) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        for (uint16_t i = 0; i < kNumButtons; i++) {
            if (gpio == config_.button_pins[i] && events == GPIO_IRQ_EDGE_FALL &&
                now - last_interrupt_timestamp_ms_[i] > kDebounceIntervalMs) {
                // Button was pressed.
                last_interrupt_timestamp_ms_[i] = now;

                if (!pressed[i]) {
                    // Pressed indicator reflects current state, but pressed callback only triggers once per hold.
                    pressed[i] = true;
                    if (config_.button_pressed_callbacks[i] != nullptr) {
                        // Callback exists, send it!
                        ((void (*)())config_.button_pressed_callbacks[i])();
                    }
                }
            }
        }
    }

    inline void Update() {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        for (uint16_t i = 0; i < kNumButtons; i++) {
            // Release butons if debounce interval is over and they are no longer pressed.
            if (now - last_interrupt_timestamp_ms_[i] > kDebounceIntervalMs && gpio_get(config_.button_pins[i]) == 1) {
                pressed[i] = false;
            }
        }
    }

    bool pressed[kNumButtons] = {false};

   private:
    ButtonsConfig config_;
    uint32_t last_interrupt_timestamp_ms_[kNumButtons] = {0};
};

extern Buttons buttons;