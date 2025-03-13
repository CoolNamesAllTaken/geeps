#pragma once

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

class Servo {
   public:
    static const uint16_t kServoPWMFreqHz = 50;
    static const uint16_t kMinPulseWidthUs = 1000;
    static const uint16_t kMaxPulseWidthUs = 2000;
    static const uint16_t kClockDiv = 256;

    struct ServoConfig {
        uint16_t pwm_pin;
        uint16_t enable_pin;  // Active LO.
        pwm_chan pwm_channel = PWM_CHAN_A;
    };

    Servo(ServoConfig config_in) : config_(config_in) {}

    inline void Init() {
        // Initialize enable pin.
        gpio_init(config_.enable_pin);
        gpio_set_dir(config_.enable_pin, GPIO_OUT);
        gpio_put(config_.enable_pin, 1);  // Disable

        // Initialize PWM output.
        // gpio_init(config_.pwm_pin);
        gpio_set_function(config_.pwm_pin, GPIO_FUNC_PWM);
        slice_num_ = pwm_gpio_to_slice_num(config_.pwm_pin);
        pwm_config_ = pwm_get_default_config();
        pwm_config_set_clkdiv(&pwm_config_, kClockDiv);  // 125kHz
        pwm_init(slice_num_, &pwm_config_, true);

        // Calculate the wrap value for 50Hz frequency
        wrap_value_ = clock_get_hz(clk_sys) / kClockDiv / kServoPWMFreqHz;
        pwm_set_wrap(slice_num_, wrap_value_);

        // Set initial duty cycle to 0 degrees
        SetAngle(0.0f);

        // Enable PWM
        pwm_set_enabled(slice_num_, true);
    }

    inline void Enable() { gpio_put(config_.enable_pin, 0); }
    inline void Disable() { gpio_put(config_.enable_pin, 1); }
    inline void SetAngle(float angle) {
        // Constrain angle to 0-180 degrees
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        // Map angle to pulse width (1000-2000 microseconds)
        float pulse_width = kMinPulseWidthUs + (angle / 180.0) * (kMaxPulseWidthUs - kMinPulseWidthUs);

        // Calculate duty cycle value (pulse width / total period) * wrap_value
        float period_us = 1e6 / kServoPWMFreqHz;  // Period in microseconds
        uint16_t duty_cycle = (pulse_width / period_us) * wrap_value_;

        pwm_set_chan_level(slice_num_, config_.pwm_channel, duty_cycle);
        printf("Servo::SetAngle: angle=%f, pulse_width=%f, duty_cycle=%d\r\n", angle, pulse_width, duty_cycle);
    }

   private:
    ServoConfig config_;
    uint slice_num_;
    pwm_config pwm_config_;
    uint16_t wrap_value_;
};