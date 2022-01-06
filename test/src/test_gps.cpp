#include "gtest/gtest.h"
#include "nmea_utils.hh"
#include <string.h>

const uint16_t kStrBufLen = 100;

TEST(NMEAPacketCalculateChecksum, GGAPacket) {
	// Example sentence from https://www.rfwireless-world.com/Terminology/GPS-sentences-or-NMEA-sentences.html.
	char str_buf[kStrBufLen] = "$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18";
	GGAPacket test_packet = GGAPacket(str_buf, 70);
	uint16_t expected_checksum = strtol("18", NULL, 16);
	ASSERT_EQ(expected_checksum, test_packet.CalculateChecksum());

	// Example sentence from PA1616S datasheet.
	memset(str_buf, '\0', kStrBufLen);
	strcpy(str_buf, "$GPGGA,091626.000,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*66");
	test_packet = GGAPacket(str_buf, 71);
	expected_checksum = strtol("66", NULL, 16);
	ASSERT_EQ(expected_checksum, test_packet.CalculateChecksum());
}

TEST(NMEAPacketIsValid, ValidPacket) {
	// Example GPGGA sentence from https://www.rfwireless-world.com/Terminology/GPS-sentences-or-NMEA-sentences.html.
	char str_buf[kStrBufLen] = "$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18";
	GGAPacket test_packet = GGAPacket(str_buf, 70);
	ASSERT_TRUE(test_packet.IsValid());
}

TEST(NMEAPacketIsValid, BadTransmittedChecksum) {
	// Invalid transmitted checksum.
	char str_buf[kStrBufLen] = "$GPGGA,091626.000,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*67";
	GGAPacket test_packet = GGAPacket(str_buf, 71);
	ASSERT_FALSE(test_packet.IsValid());
}

TEST(NMEAPacketIsValid, InjectedGibberish) {
	// Corrupted packet contents
	char str_buf[kStrBufLen] = "$GPGGA,09162ashgsaggh88586N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*66";
	GGAPacket test_packet = GGAPacket(str_buf, 71);
	ASSERT_FALSE(test_packet.IsValid());
}

TEST(NMEAPacketIsValid, NoEndToken) {
	// '*' suffix token not transmitted
	char str_buf[kStrBufLen] = "$GPGGA,09162ashgsaggh88586N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,66";
	GGAPacket test_packet = GGAPacket(str_buf, 71);
	ASSERT_FALSE(test_packet.IsValid());
}

TEST(NMEAPacketIsValid, NoStartToken) {
	// '$' prefix token not transmitted
	char str_buf[kStrBufLen] = "GPGGA,091626.000,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*66";
	GGAPacket test_packet = GGAPacket(str_buf, 71);
	ASSERT_FALSE(test_packet.IsValid());
}