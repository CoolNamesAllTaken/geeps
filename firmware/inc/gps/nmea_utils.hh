#ifndef _NMEA_UTILS_HH_
#define _NMEA_UTILS_HH_

#include <stdint.h>

class NMEAPacket {
public:
    static const uint16_t kMaxPacketLen = 200;
    static const uint16_t kMessageIDStrLen = 7;
    static const uint16_t kUTCTimeStrLen = 11;
    static const uint16_t kLatitudeStrLen = 11;
    static const uint16_t kLongitudeStrLen = 12;

    NMEAPacket(char packet_str[kMaxPacketLen], uint16_t packet_str_len);

    // Static utility functions.
    static uint32_t UTCTimeStrToUint(char utc_time_str[kUTCTimeStrLen]);

    // Public interface functions.
    uint8_t CalculateChecksum();
protected:
    char packet_str_[kMaxPacketLen];
    uint16_t packet_str_len_;
};

/**
 * Packet containing time, position, and fix type data.
 */
class GGAPacket : public NMEAPacket {
public:
    enum PositionFixIndicator {
        FIX_NOT_AVAILABLE = 0,
        GPS_FIX = 1,
        DIFFERENTIAL_GPS_FIX = 2
    };

    GGAPacket(char packet_str[kMaxPacketLen], uint16_t packet_str_len); // constructor

    void GetUTCTimeStr(char str_buf[kUTCTimeStrLen]);
    void GetLatitudeStr(char str_buf[kLatitudeStrLen]); // includes "N" or "S" suffix
    void GetLongitudeStr(char str_buf[kLongitudeStrLen]); // includes "E" ow "W" suffix
    GGAPacket::PositionFixIndicator GetPositionFixIndicator();
    uint16_t GetSatellitesUsed();
    float GetHDOP();
    float GetMSLAltitude(); // [meters]
    float GetGeoidalSeparation();

    bool IsValid();

private:
    bool is_valid_;
};

#endif /* _NMEA_UTILS_HH_ */