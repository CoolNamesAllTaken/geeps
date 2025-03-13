#pragma once

#include "Arduino.h"

class TwoWireInterface {
   public:
    inline void begin() {};
    inline uint16_t write(uint8_t data) { return 0; };
    inline uint8_t read() { return 0; };
    inline void end() {};
    inline void requestFrom(uint8_t address, uint8_t size) {};
    inline uint16_t available() { return 0; };
    inline void beginTransmission(uint8_t address) {};
    inline void endTransmission() {};
    inline void setClock(uint32_t clock_rate_hz) {};
};

extern TwoWireInterface Wire;