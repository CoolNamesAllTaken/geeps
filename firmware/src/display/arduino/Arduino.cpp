#include "Arduino.h"

static uint32_t interrupt_status;
inline void noInterrupts() { interrupt_status = save_and_disable_interrupts(); }
inline void interrupts() { restore_interrupts(interrupt_status); }