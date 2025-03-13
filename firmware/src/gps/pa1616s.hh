#ifndef _PA1616S_HH_
#define _PA1616S_HH_

#include "pico/stdlib.h"
#include "hardware/gpio.h" // for UART inst
#include "hardware/uart.h"

#include "nmea_utils.hh"

class PA1616S
{
public:
    typedef struct
    {
        uart_inst_t *uart_id = uart1;
        uint uart_baud = 9600;
        uint uart_tx_pin = 4;
        uint uart_rx_pin = 5;
        uint data_bits = 8;
        uint stop_bits = 1;
        uart_parity_t parity = UART_PARITY_NONE;
        uint reset_pin = 6;
        uint fix_pin = 3;
    } PA1616SConfig_t;

    static const uint16_t kMaxUARTBufLen = 200;
    GGAPacket latest_gga_packet;

    PA1616S(PA1616SConfig_t config);
    void Init();
    void Update();

private:
    PA1616SConfig_t config_;
    char uart_buf_[kMaxUARTBufLen];
    uint16_t uart_buf_len_;

    void FlushUARTBuf();
};

#endif /* PA1616S_HH_ */