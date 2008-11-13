/* api_geospatial.cc: tests of geospatial functionality
 *
 * Copyright 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "api_geospatial.h"
#include <xapian/geospatial.h>
#include <xapian/error.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"
#include <iomanip>

using namespace std;
using namespace Xapian;

// #######################################################################
// # Tests start here

struct latlongtest1 {
    const char * coord;
    double latitude;
    double longitude;
};

static latlongtest1 test_combined_coords[] = {
    { "0.0 0.0", 0.0, 0.0 },
    { "1.0 0.0", 1.0, 0.0 },
    { "0.0 1.0", 0.0, 1.0 },
    { "1.0 1.0", 1.0, 1.0 },
    { "2.0 3.0", 2.0, 3.0 },
    { "2.0 -3.0", 2.0, -3.0 },
    { "-2.0 3.0", -2.0, 3.0 },
    { "-2.0 -3.0", -2.0, -3.0 },
    { "-270.0 -3.0", -3.0, 90.0 },
    { "0 -3.0", 0.0, -3.0 },
    { "-3.0 -360", -3.0, 0.0 },
    { "-3.0 -180", -3.0, 180.0 },
    { "-3.0 180", -3.0, 180.0 },
    { "-3.0 360", -3.0, 0.0 },
    { "45.5 135.7", 45.5, 135.7},
    { "45\xc2\xb0 135.7", 45.0, 135.7},
    { "45\xc2\xb0, 135.7", 45.0, 135.7},
    { "45,135.7d", 45.0, 135.7},
    { "45\xc2\xb0 135o", 45.0, 135.0},
    { "45\xc2\xb0""30\x27 135.7", 45.5, 135.7},
    { "45\xc2\xb0""29\x27""60\x22 135.7", 45.5, 135.7},
    { "45d 29m 60s 135.7", 45.5, 135.7},
    { "45N,135.7d", 45.0, 135.7},
    { "45N,135.7E", 45.0, 135.7},
    { "45N,135.7dE", 45.0, 135.7},
    { "45d29'60''E 40", 40.0, 45.5},
    { "45d29'60sS 45dW", -45.5, -45},
    { NULL, 0.0, 0.0 },
};

// test parsing of latitude/longitude coordinates.
DEFINE_TESTCASE(latlongparse1, !backend) {
    for (latlongtest1 *ptr = test_combined_coords; ptr->coord; ++ptr) {
	tout << "Coord: " << ptr->coord << '\n';
	LatLongCoord parsed = LatLongCoord::parse_latlong(ptr->coord);
	TEST_EQUAL_DOUBLE(parsed.latitude, ptr->latitude);
	TEST_EQUAL_DOUBLE(parsed.longitude, ptr->longitude);
    }
    return true;
}


struct latlongtest2 {
    const char * latstring;
    const char * longstring;
    double latitude;
    double longitude;
};

static latlongtest2 test_separate_coords[] = {
    { "0.0", "0.0", 0.0, 0.0 },
    { "1.0", "0.0", 1.0, 0.0 },
    { "0.0", "1.0", 0.0, 1.0 },
    { "45.5", "135.7", 45.5, 135.7},
    { "45\xc2\xb0", "135.7", 45.0, 135.7},
    { "45\xc2\xb0""30.0\x27", "135.7", 45.5, 135.7},
    { "45\xc2\xb0""30\x27", "135.7", 45.5, 135.7},
    { "45\xc2\xb0""29\x27""60\x22", "135.7", 45.5, 135.7},
    { "45d 29m 60s", "135.7", 45.5, 135.7},
    { "45N", "135.7", 45.0, 135.7},
    { "45d 29m 60s N", "135.7", 45.5, 135.7},
    { "45d 29m 60s S", "135.7", -45.5, 135.7},
    { NULL, NULL, 0.0, 0.0 },
};

// test parsing of separated latitude and longitude pairs.
DEFINE_TESTCASE(latlongparse2, !backend) {
    for (latlongtest2 *ptr = test_separate_coords; ptr->latstring; ++ptr) {
	tout << "Coord: " << ptr->latstring << ":" << ptr->longstring << '\n';
	LatLongCoord parsed = LatLongCoord::parse_latlong(ptr->latstring, ptr->longstring);
	TEST_EQUAL_DOUBLE(parsed.latitude, ptr->latitude);
	TEST_EQUAL_DOUBLE(parsed.longitude, ptr->longitude);
    }
    return true;
}

// Check that appropriate errors are thrown.
DEFINE_TESTCASE(latlongparse3, !backend) {
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong(""));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("", ""));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("a"));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("a", "b"));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("1"));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("1", "2 1"));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("1N 2N"));
    TEST_EXCEPTION(Xapian::LatLongParserError,
		   LatLongCoord::parse_latlong("fN 2N"));
    return true;
}
