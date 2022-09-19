#include "gps_utils.hh"
#include <math.h>

#define SQ(a) ((a) * (a))

const float kEquatorialRadius = 6378137; // [m] Earth's equatorial radius.
const float kPolarRadius = 6356752; // [m] Earth's polar radius.

/**
 * @brief Returns the radius from the center of the earth to a point on the surface of the earth
 * at a given latitude, in radians, assuming that the earth is an ellipsoid shape.
 * @param[in] lat Latitude, in radians.
 * @retval Radius in meters.
 */
float EarthEllipsoidRadius(float lat) {
    // sqr((sq(kEquatorialRadius*kEquatorialRadius*cos(lat))+sq(b*b*sin(lat)))/(sq(kEquatorialRadius*cos(lat))+sq(b*sin(lat))));
    return sqrt(
        (
            SQ(SQ(kEquatorialRadius) * cos(lat)) + 
            SQ(SQ(kPolarRadius) * sin(lat))
        ) / 
        (
            SQ(kEquatorialRadius*cos(lat)) + 
            SQ(kPolarRadius*sin(lat))
        )
    );
}
/**
 * @brief Converts a value from degrees to radians.
 * @param[in] degrees Parameter to convert, in degrees.
 * @retval Result in radians.
 */
float DegreesToRadians(float degrees) {
    return degrees * M_PI / 180.0f;
}

/**
 * @brief Calculates the straight line distance through the earth for two lat/long coordinate sets.
 * @param lat_a Latitude of coordinate A, in degrees. + for N, - for S.
 * @param lon_a Longitude of coordinate A, in degrees. + for E, - for W.
 * @param lat_b Latitude of coordinate B, in degrees. + for N, - for S.
 * @param lon_b Longitude of coordinate B, in degrees. + for E, - for W.
 * @retval Geographical distance between coordinates A and B.
 */
float CalculateStraightLineDistance(float lat_a, float lon_a, float lat_b, float lon_b) {
    // Adapted from this stackoverflow answer, which uses an oblate spheroid assumption but only calculates
    // straight-line distances: https://stackoverflow.com/a/49916544/4625627.
    // const a = 6378.137; // equitorial radius in km
    // const b = 6356.752; // polar radius in km

    // var sq = x => (x*x);
    // var sqr = x => Math.sqrt(x);
    // var cos = x => Math.cos(x);
    // var sin = x => Math.sin(x);

    // var radius = lat => sqr((sq(kEquatorialRadius*kEquatorialRadius*cos(lat))+sq(b*b*sin(lat)))/(sq(kEquatorialRadius*cos(lat))+sq(b*sin(lat))));

    // lat1 = lat1 * Math.PI / 180;
    // lng1 = lng1 * Math.PI / 180;
    // lat2 = lat2 * Math.PI / 180;
    // lng2 = lng2 * Math.PI / 180;
    float lat_a_rad = DegreesToRadians(lat_a);
    float lon_a_rad = DegreesToRadians(lon_a);
    float lat_b_rad = DegreesToRadians(lat_b);
    float lon_b_rad = DegreesToRadians(lon_b);

    float radius_a = EarthEllipsoidRadius(lat_a_rad);
    float x_a = radius_a * cos(lat_a_rad) * cos(lon_a_rad);
    float y_a = radius_a * cos(lat_a_rad) * sin(lon_a_rad);
    float z_a = radius_a * sin(lat_a_rad);

    // var R1 = radius(lat1);
    // var x1 = R1*cos(lat1)*cos(lng1);
    // var y1 = R1*cos(lat1)*sin(lng1);
    // var z1 = R1*sin(lat1);

    float radius_b = EarthEllipsoidRadius(lat_b_rad);
    float x_b = radius_b * cos(lat_b_rad) * cos(lon_b_rad);
    float y_b = radius_b * cos(lat_b_rad) * sin(lon_b_rad);
    float z_b = radius_b * sin(lat_b_rad);

    // var R2 = radius(lat2);
    // var x2 = R2*cos(lat2)*cos(lng2);
    // var y2 = R2*cos(lat2)*sin(lng2);
    // var z2 = R2*sin(lat2);

    // return sqr(sq(x1-x2)+sq(y1-y2)+sq(z1-z2));

    return sqrt(SQ(x_a-x_b) + SQ(y_a-y_b) + SQ(z_a-z_b)); // TODO: make this return surface distance
}