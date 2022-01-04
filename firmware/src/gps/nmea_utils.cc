#include "nmea_utils.hh"

#include <string.h>

NMEAPacket::NMEAPacket(
    char packet_str[kMaxPacketLen],
    uint16_t packet_str_len)
    : packet_str_len_(packet_str_len)
{
    strncpy(packet_str_, packet_str, packet_str_len_);
}

uint8_t NMEAPacket::CalculateChecksum() {
    uint8_t checksum = 0;
    uint16_t checksum_start_ind = static_cast<uint16_t>(strchr(packet_str_, '*') - packet_str_);
    for (uint16_t i = 0; i < checksum_start_ind && i < packet_str_len_; i++) {
        checksum ^= packet_str_[i];
    }

    return checksum;
}

/**
 * @brief Constructor for a $GPGGA packet.
 * @param[in] packet_str C-string buffer with packet contents.
 */
GGAPacket::GGAPacket(
    char packet_str[kMaxPacketLen],
    uint16_t packet_str_len) 
    : NMEAPacket(packet_str, packet_str_len)
{

    

    if (strcmp(strtok(packet_str, ","), "$GPGGA")) {
        // Header is wrong (different packet type).
        is_valid_ = false;
        return;
    }
}