#include "nmea_utils.hh"

#include <string.h>
#include <stdlib.h> // for strtol
#include <math.h> // for macros

#define GPS_PACKET_DELIM ","
#define GPS_NUMBERS_BASE 10
#define GPS_CHECKSUM_BASE 16

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

NMEAPacket::NMEAPacket(
    char packet_str[kMaxPacketLen],
    uint16_t packet_str_len)
    : packet_str_len_(MIN(kMaxPacketLen, packet_str_len))
    , is_valid_(false)
{
    strncpy(packet_str_, packet_str, packet_str_len_);

    char * end_token_ptr = strchr(packet_str_, '*');
    if (!end_token_ptr) {
        is_valid_ = false;
        return; // guard against case where end token is not sent
    }
    uint16_t transmitted_checksum = static_cast<uint16_t>(strtol(end_token_ptr+1, NULL, GPS_CHECKSUM_BASE));
    if (CalculateChecksum() == transmitted_checksum) {
        is_valid_ = true;
    }

    char strtok_buf[kMaxPacketLen];
    strncpy(strtok_buf, packet_str, kMaxPacketFieldLen); // strtok modifies the input string, be safe!
    char * header_str = strtok(strtok_buf, GPS_PACKET_DELIM);
    if (strcmp(header_str, "$GPGGA") == 0) {
        packet_type_ = GGA;
    } else if (strcmp(header_str, "$GPGSA") == 0) {
        packet_type_ = GSA;
    } else if (strcmp(header_str, "$GPGSV") == 0) {
        packet_type_ = GSV;
    } else if (strcmp(header_str, "$GPRMC") == 0) {
        packet_type_ = RMC;
    } else if (strcmp(header_str, "$GPVTG") == 0) {
        packet_type_ = VTG;
    } else {
        packet_type_ = UNKNOWN;
    }
}

uint8_t NMEAPacket::CalculateChecksum() {
    uint8_t checksum = 0;

    // GPS message is bounded by '$' prefix token and '*' suffix token. Tokens not used in checksum.
    char * start_token_ptr = strchr(packet_str_, '$');
    char * end_token_ptr = strchr(packet_str_, '*');
    if (!start_token_ptr || !end_token_ptr) {
        return 0; // can't find start or end token; invalid!
    } 
    uint16_t packet_start_ind = static_cast<uint16_t>(start_token_ptr - packet_str_) + 1;
    uint16_t checksum_start_ind = static_cast<uint16_t>(end_token_ptr - packet_str_);
    for (uint16_t i = packet_start_ind; i < checksum_start_ind && i < packet_str_len_; i++) {
        checksum ^= packet_str_[i];
    }

    return checksum;
}

bool NMEAPacket::IsValid() {
    return is_valid_;
}

NMEAPacket::PacketType_t NMEAPacket::GetPacketType() {
    return packet_type_;
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
    char strtok_buf[kMaxPacketLen];
    strncpy(strtok_buf, packet_str, kMaxPacketFieldLen); // strtok modifies the input string, be safe!
    char * header_str = strtok(strtok_buf, GPS_PACKET_DELIM);
    if (strcmp(header_str, "$GPGGA")) {
        // Header is wrong (different packet type).
        is_valid_ = false;
        return;
    }

    // strncpy(utc_time_str_, strtok(NULL, GPS_PACKET_DELIM), kMaxPacketFieldLen); // utc time
    // strncpy(latitude_str_, strtok(NULL, GPS_PACKET_DELIM), kMaxPacketFieldLen); // latitude
    // strcat(latitude_str_, strtok(NULL, GPS_PACKET_DELIM)); // N/S indicator
    // strncpy(longitude_str_, strtok(NULL, GPS_PACKET_DELIM), kMaxPacketFieldLen); // longitude
    // strcat(longitude_str_, strtok(NULL, GPS_PACKET_DELIM)); // E/W indicator
    // char pos_fix_str_[kMaxPacketFieldLen];
    // strncpy(pos_fix_str_, strtok(NULL, GPS_PACKET_DELIM), kMaxPacketFieldLen); // position fix indicator
    // pos_fix_ = static_cast<PositionFixIndicator_t>(strtol(pos_fix_str_, NULL, GPS_NUMBERS_BASE));
    // // satellites_used_ = strtol(strtok(NULL, GPS_PACKET_DELIM), NULL, GPS_NUMBERS_BASE);
    // hdop_ = strtof(strtok(NULL, GPS_PACKET_DELIM), NULL); // horizontal dilution of position
    // msl_altitude_ = strtof(strtok(NULL, GPS_PACKET_DELIM), NULL); // mean sea level altitude
    // geoidal_separation_ = strtof(strtok(NULL, GPS_PACKET_DELIM), NULL); // geoidal separation
}

void GGAPacket::GetUTCTimeStr(char * str_buf) {
    strcpy(str_buf, utc_time_str_);
}

void GGAPacket::GetLatitudeStr(char * str_buf) {
    strcpy(str_buf, latitude_str_);
}

void GGAPacket::GetLongitudeStr(char * str_buf) {
    strcpy(str_buf, longitude_str_);
}

GGAPacket::PositionFixIndicator_t GGAPacket::GetPositionFixIndicator() {
    return pos_fix_;
}

uint16_t GGAPacket::GetSatellitesUsed() {
    return satellites_used_;
}

float GGAPacket::GetHDOP() {
    return hdop_;
}

float GGAPacket::GetMSLAltitude() {
    return msl_altitude_;
}

float GGAPacket::GetGeoidalSeparation() {
    return geoidal_separation_;
}