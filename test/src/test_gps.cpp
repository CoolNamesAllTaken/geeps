#include "gtest/gtest.h"
#include "nmea_utils.hh"
#include <string.h>

TEST(GPSTest, TestGGAPacketChecksum) {
	// Example sentence from https://www.rfwireless-world.com/Terminology/GPS-sentences-or-NMEA-sentences.html
	char str_buf[100] = "$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18";
	GGAPacket test_packet = GGAPacket(str_buf, 70);
	ASSERT_EQ(strtol("18", NULL, 16), test_packet.CalculateChecksum());

	// strcpy(str_buf, "$GPGGA,091626.000,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*66");
	// test_packet = GGAPacket(str_buf, 71);
	// ASSERT_EQ(strtol("66", NULL, 16), test_packet.CalculateChecksum());
}