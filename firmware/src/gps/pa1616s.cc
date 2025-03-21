#include "pa1616s.hh"

#include <stdio.h>  // for debug printfs
#include <string.h>

/**
 * @brief Construct a new PA1616S::PA1616S object.
 *
 * @param config Configuration parameters for PA1616S.
 */
PA1616S::PA1616S(PA1616SConfig_t config)
    : latest_gga_packet(GGAPacket((char *)"", 0)), config_(config), uart_buf_len_(0) {}

/**
 * @brief Initializes the PA1616S.
 */
void PA1616S::Init() {
    printf("PA1616S::Init: Init started.\r\n");
    uart_init(config_.uart_id, config_.uart_baud);
    gpio_set_function(config_.uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(config_.uart_rx_pin, GPIO_FUNC_UART);

    uart_set_hw_flow(config_.uart_id, false, false);  // no CTS/RTS
    uart_set_format(config_.uart_id, config_.data_bits, config_.stop_bits, config_.parity);
    uart_set_fifo_enabled(config_.uart_id, true);

    memset(uart_buf_, '\0', kMaxUARTBufLen);
    uart_buf_len_ = 0;

    // Enable reset pin and set it high.
    gpio_init(config_.reset_pin);
    gpio_set_dir(config_.reset_pin, GPIO_OUT);
    gpio_put(config_.reset_pin, 1);

    printf("PA1616S::Init: Init completed.\r\n");
}

/**
 * @brief Ingests pending UART packets and updates parameters.
 *
 */
void PA1616S::Update() {
    while (uart_is_readable(config_.uart_id)) {
        char new_char = uart_getc(config_.uart_id);
        if (new_char == '$') {
            // Start of new string.
            // printf("PA1616S::Update: Received sentence %s", uart_buf_);
            NMEAPacket packet = NMEAPacket(uart_buf_, strlen(uart_buf_));
            if (packet.IsValid()) {
                // printf("PA1616S::Update:     Packet is valid!\r\n");
                if (packet.GetPacketType() == NMEAPacket::GGA) {
                    latest_gga_packet = GGAPacket(uart_buf_, uart_buf_len_);
                    if (latest_gga_packet.IsValid()) {
                        // printf("PA1616S::Update:         Formed a valid GGA Packet!\r\n");
                    }
                }
            } else {
                printf("PA1616S::Update:     Packet is invalid.\r\n");
            }
            FlushUARTBuf();
        } else if (uart_buf_len_ >= kMaxUARTBufLen) {
            // String too long! Abort.
            printf("PA1616S::Update: String too long! Aborting.\r\n");
            FlushUARTBuf();
        }
        uart_buf_[uart_buf_len_] = new_char;  // add new char to end of buffer
        uart_buf_[uart_buf_len_ + 1] = '\0';  // null terminate
        uart_buf_len_++;
    }
}

void PA1616S::FlushUARTBuf() {
    memset(uart_buf_, '\0', kMaxUARTBufLen);
    uart_buf_len_ = 0;
}