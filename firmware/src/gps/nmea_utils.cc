#include "nmea_utils.hh"

#include <string.h>
#include <stdlib.h> // for strotl

NMEAPacket::NMEAPacket(
    char packet_str[kMaxPacketLen],
    uint16_t packet_str_len)
    : packet_str_len_(packet_str_len)
    , is_valid_(false)
{
    strncpy(packet_str_, packet_str, packet_str_len_);

    uint16_t transmitted_checksum = static_cast<uint16_t>(strtol(strchr(packet_str_, '*')+1, NULL, 16));
    if (CalculateChecksum() == transmitted_checksum) {
        is_valid_ = true;
    }
}

uint8_t NMEAPacket::CalculateChecksum() {
    uint8_t checksum = 0;

    // GPS message is bounded by '$' prefix token and '*' suffix token. Tokens not used in checksum.
    uint16_t packet_start_ind = static_cast<uint16_t>(strchr(packet_str_, '$') - packet_str_) + 1;
    uint16_t checksum_start_ind = static_cast<uint16_t>(strchr(packet_str_, '*') - packet_str_);
    for (uint16_t i = packet_start_ind; i < checksum_start_ind && i < packet_str_len_; i++) {
        checksum ^= packet_str_[i];
    }

    return checksum;
}

bool NMEAPacket::IsValid() {
    return is_valid_;
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