/** @file geoencode.cc
 * @brief Encodings for geospatial coordinates.
 */
/* Copyright (C) 2011 Richard Boulton
 * Based closely on a python version, copyright (C) 2010 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>
#include "geoencode.h"

#include <cmath>

using namespace std;

/** Angles, split into degrees, minutes and seconds.
 *
 *  Only designed to work with positive angles.
 */
struct DegreesMinutesSeconds {
    /** Number of degrees.
     *
     *  Range 0 <= degrees <= 180 for latitude, 0 <= degrees < 360 for
     *  longitude.
     */
    int degrees;

    /** Number of minutes: 0 to 59 */
    int minutes;

    /** Number of seconds: 0 to 59 */
    int seconds;

    /** Number of 16ths of a second: 0 to 15 */
    int sec16ths;

    /** Initialise with a (positive) angle, as an integer representing the
     *  number of 16ths of a second, rounding to nearest.
     *
     *  The range of valid angles is assumed to be 0 <= angle in degrees < 360,
     *  so range of angle_16th_secs is 0..20735999, which fits easily into a 32
     *  bit int.  (Latitudes are represented in the range 0 <= angle <= 180,
     *  where 0 is the south pole.)
     */
    explicit DegreesMinutesSeconds(int angle_16th_secs) {
	degrees = angle_16th_secs / (3600 * 16);
	angle_16th_secs = angle_16th_secs % (3600 * 16);
	minutes = angle_16th_secs / (60 * 16);
	angle_16th_secs = angle_16th_secs % (60 * 16);
	seconds = angle_16th_secs / 16;
	sec16ths = angle_16th_secs % 16;
    }
};

bool
GeoEncode::encode(double lat, double lon, string & result)
{
    // Check range of latitude.
    if (rare(lat < -90.0 || lat > 90.0)) {
	return false;
    }

    // Wrap longitude to range [0,360).
    lon = fmod(lon, 360.0);
    if (lon < 0) {
	lon += 360;
    }

    int lat_16ths, lon_16ths;
    lat_16ths = lround((lat + 90.0) * 57600.0);
    if (lat_16ths == 0 || lat_16ths == 57600 * 180) {
	lon_16ths = 0;
    } else {
	lon_16ths = lround(lon * 57600.0);
	if (lon_16ths == 57600 * 360) {
	    lon_16ths = 0;
	}
    }

    DegreesMinutesSeconds lat_dms(lat_16ths);
    DegreesMinutesSeconds lon_dms(lon_16ths);

    size_t old_len = result.size();
    result.resize(old_len + 6);

    // Add degrees parts as first two bytes.
    unsigned dd = lat_dms.degrees + lon_dms.degrees * 181;
    // dd is in range 0..180*360+359 = 0..65159
    result[old_len] = char(dd >> 8);
    result[old_len + 1] = char(dd & 0xff);

    // Add minutes next; 4 bits from each in the first byte.
    result[old_len + 2] = char(((lat_dms.minutes / 4) << 4) |
			       (lon_dms.minutes / 4)
			      );

    result[old_len + 3] = char(
			       ((lat_dms.minutes % 4) << 6) |
			       ((lon_dms.minutes % 4) << 4) |
			       ((lat_dms.seconds / 15) << 2) |
			       (lon_dms.seconds / 15)
			      );

    result[old_len + 4] = char(
			       ((lat_dms.seconds % 15) << 4) |
			       (lon_dms.seconds % 15)
			      );

    result[old_len + 5] = char(
			       (lat_dms.sec16ths << 4) |
			       lon_dms.sec16ths
			      );

    return true;
}

void
GeoEncode::decode(const char * value, size_t len,
		  double & lat_ref, double & lon_ref)
{
    const unsigned char * ptr
	    = reinterpret_cast<const unsigned char *>(value);
    unsigned tmp = (ptr[0] & 0xff) << 8 | (ptr[1] & 0xff);
    lat_ref = tmp % 181;
    lon_ref = tmp / 181;
    if (len > 2) {
	tmp = ptr[2];
	double lat_m = (tmp >> 4) * 4;
	double lon_m = (tmp & 0xf) * 4;

	if (len > 3) {
	    tmp = ptr[3];
	    lat_m += (tmp >> 6) & 3;
	    lon_m += (tmp >> 4) & 3;
	    double lat_s = ((tmp >> 2) & 3) * 15;
	    double lon_s = (tmp & 3) * 15;

	    if (len > 4) {
		tmp = ptr[4];
		lat_s += (tmp >> 4) & 0xf;
		lon_s += tmp & 0xf;

		if (len > 5) {
		    tmp = ptr[5];
		    lat_s += ((tmp >> 4) / 16.0);
		    lon_s += ((tmp & 0xf) / 16.0);
		}
	    }

	    lat_m += lat_s / 60.0;
	    lon_m += lon_s / 60.0;
	}

	lat_ref += lat_m / 60.0;
	lon_ref += lon_m / 60.0;
    }

    lat_ref -= 90.0;
}

/// Calc latitude and longitude in integral number of 16ths of a second
static void
calc_latlon_16ths(double lat, double lon, int & lat_16ths, int & lon_16ths)
{
    lat_16ths = lround((lat + 90.0) * 57600.0);
    lon_16ths = lround(lon * 57600.0);
    if (lon_16ths == 57600 * 360) {
	lon_16ths = 0;
    }
}

GeoEncode::DecoderWithBoundingBox::DecoderWithBoundingBox
(double lat1, double lon1_, double lat2, double lon2_)
	: lon1(lon1_), lon2(lon2_),
	  min_lat(lat1), max_lat(lat2),
	  include_poles(false)
{
    // Wrap longitudes to range [0,360).
    lon1 = fmod(lon1, 360.0);
    if (lon1 < 0) {
	lon1 += 360;
    }

    lon2 = fmod(lon2, 360.0);
    if (lon2 < 0) {
	lon2 += 360;
    }

    // Calculate start1
    int lat_16ths, lon_16ths;
    calc_latlon_16ths(lat1, lon1, lat_16ths, lon_16ths);
    if (lat_16ths == 0 || lat_16ths == 57600 * 180) {
	include_poles = true;
    }
    unsigned dd = lat_16ths / (3600 * 16) + (lon_16ths / (3600 * 16)) * 181;
    start1 = char(dd >> 8);

    calc_latlon_16ths(lat2, lon2, lat_16ths, lon_16ths);
    if (lat_16ths == 0 || lat_16ths == 57600 * 180) {
	include_poles = true;
    }
    dd = lat_16ths / (3600 * 16) + (lon_16ths / (3600 * 16)) * 181;
    start2 = char(dd >> 8);

    discontinuous_longitude_range = (lon1 > lon2);
}

bool
GeoEncode::DecoderWithBoundingBox::decode(const std::string & value,
					  double & lat_ref,
					  double & lon_ref) const
{
    unsigned char start = value[0];
    if (discontinuous_longitude_range) {
	// start must be outside range of (start2..start1)
	// (start2 will be > start1)
	if (start2 < start && start < start1) {
	    if (!(include_poles && start == 0))
		return false;
	}
    } else {
	// start must be inside range of [start1..start2] (inclusive of ends).
	if (start < start1 || start2 < start) {
	    if (!(include_poles && start == 0))
		return false;
	}
    }
    double lat, lon;
    GeoEncode::decode(value, lat, lon);
    if (lat < min_lat || lat > max_lat) {
	return false;
    }
    if (lat == -90 || lat == 90) {
	// It's a pole, so the longitude isn't meaningful (will be zero)
	// and we've already checked that the latitude is in range.
	lat_ref = lat;
	lon_ref = 0;
	return true;
    }
    if (discontinuous_longitude_range) {
	if (lon2 < lon && lon < lon1)
	    return false;
    } else {
	if (lon < lon1 || lon2 < lon)
	    return false;
    }

    lat_ref = lat;
    lon_ref = lon;
    return true;
}
