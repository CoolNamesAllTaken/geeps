#pragma once

// #include "api/Common.h"
// #include "api/String.h"
#include "bsp.hh"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// Use std math functions instead of custom macros to avoid conflicts.
#include "math.h"
using namespace std;
// #ifndef max
// #define max(a, b) ((a) > (b) ? (a) : (b))
// #endif
// #ifndef min
// #define min(a, b) ((a) < (b) ? (a) : (b))
// #endif
#define map(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

// GPIO Defines
#define HIGH                                     1
#define LOW                                      0
#define INPUT                                    0
#define OUTPUT                                   1
#define INPUT_PULLUP                             2

#define SCK                                      BSP::epaper_spi_clk_pin
#define MOSI                                     BSP::epaper_spi_mosi_pin

// SPI Defines
#define SPI_MODE0                                0x00
#define SPI_MODE1                                0x01
#define SPI_MODE2                                0x02
#define SPI_MODE3                                0x03

typedef uint8_t pin_size_t;
typedef uint8_t byte;

// GPIO Functions
inline void pinMode(uint8_t pin, uint8_t mode) {
    if (mode == OUTPUT) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    } else if (mode == INPUT) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    } else if (mode == INPUT_PULLUP) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
}
inline void digitalWrite(uint8_t pin, bool value) { gpio_put(pin, value); }
inline bool digitalRead(uint8_t pin) { return gpio_get(pin); }

// Time Functions
inline void delay(uint32_t ms) { sleep_ms(ms); }
inline uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }
inline void delayMicroseconds(uint32_t us) { busy_wait_us_32(us); }

// Convenience Utilities for bit shifting
inline void bitSet(uint8_t& value, uint8_t bit) { value |= (0b1 << bit); }
inline uint8_t bitRead(uint8_t value, uint8_t bit) { return value & (0b1 << bit) ? 0b1 : 0b0; }
inline void bitClear(uint8_t& value, uint8_t bit) { value &= ~(0b1 << bit); }

// Interrupt Stuff
inline void noInterrupts();
inline void interrupts();

class String {
    char* buffer;
    uint32_t len;

   public:
    String() {
        buffer = (char*)malloc(256);
        len = 0;
    }

    String(const char* str) {
        buffer = (char*)malloc(256);
        len = strlen(str);
        strcpy(buffer, str);
    }

    String(const String& str) {
        buffer = (char*)malloc(256);
        len = str.len;
        strcpy(buffer, str.buffer);
    }

    ~String() { free(buffer); }

    inline void operator=(const char* str) {
        len = strlen(str);
        strcpy(buffer, str);
    }

    inline void operator=(const String& str) {
        len = str.len;
        strcpy(buffer, str.buffer);
    }

    inline void operator+=(const char* str) {
        strcat(buffer, str);
        len += strlen(str);
    }

    inline void operator+=(const String& str) {
        strcat(buffer, str.buffer);
        len += str.len;
    }

    inline char operator[](uint32_t index) { return buffer[index]; }

    inline uint32_t length() { return len; }

    inline const char* c_str() { return buffer; }

    inline char charAt(uint32_t index) { return buffer[index]; }

    inline String substring(uint32_t start, uint32_t end) {
        char* sub = (char*)malloc(256);
        strncpy(sub, buffer + start, end - start);
        return String(sub);
    }

    inline void toCharArray(char* buf, uint32_t size) { strncpy(buf, buffer, size); }
};

class SerialClass {
   public:
    inline void begin(uint32_t baud) {}
    inline void print(const char* str) { printf("%s", str); }
    inline void println(const char* str) { printf("%s\r\n", str); };
    inline void println() { printf("\r\n"); }
    inline void print(String str) { printf("%s", str.c_str()); }
    inline void println(String str) { printf("%s\r\n", str.c_str()); };
    inline void flush() {};
};
extern SerialClass Serial;

struct SPISettings {
    SPISettings() {};
    SPISettings(uint32_t clock_in, uint8_t bit_order_in, uint8_t data_mode_in)
        : clock(clock_in), bit_order(bit_order_in), mode(data_mode_in) {}
    uint32_t clock;
    uint8_t bit_order;
    uint8_t mode;

    inline uint8_t getDataMode() const { return mode; }
    inline uint32_t getClockFreq() const { return clock; }
    inline uint8_t getBitOrder() const { return bit_order; }
    inline bool operator==(const SPISettings& other) const {
        return clock == other.clock && bit_order == other.bit_order && mode == other.mode;
    }
};