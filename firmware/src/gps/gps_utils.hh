#ifndef _GPS_UTILS_H_
#define _GPS_UTILS_H_

float CalculateStraightLineDistance(float lat_a, float lon_a, float lat_b, float lon_b);
float CalculateGeoidalDistance(float lat_a, float lon_a, float lat_b, float lon_b);
float CalculateHeadingToWaypoint(float lat_a, float lon_a, float lat_b, float lon_b);

#endif /*_GPS_UTILS_H_*/