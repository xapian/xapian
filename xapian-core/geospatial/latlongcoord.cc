/** @file
 * @brief Latitude and longitude representations.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2011 Richard Boulton
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

#include "xapian/geospatial.h"
#include "xapian/error.h"

#include "geoencode.h"
#include "str.h"

#include <cmath>

using namespace Xapian;
using namespace std;

LatLongCoord::LatLongCoord(double latitude_, double longitude_)
	: latitude(latitude_),
	  longitude(longitude_)
{
    if (latitude < -90.0 || latitude > 90.0)
	throw InvalidArgumentError("Latitude out-of-range");
    longitude = fmod(longitude_, 360);
    if (longitude < 0) longitude += 360;
}

void
LatLongCoord::unserialise(const string & serialised)
{
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    unserialise(&ptr, end);
    if (ptr != end)
	throw SerialisationError(
		"Junk found at end of serialised LatLongCoord");
}

void
LatLongCoord::unserialise(const char ** ptr, const char * end)
{
    size_t len = end - *ptr;
    if (len < 2) {
	latitude = 0;
	longitude = 0;
	return;
    }
    GeoEncode::decode(*ptr, end - *ptr, latitude, longitude);
    if (len < 6) {
	*ptr = end;
    } else {
	*ptr += 6;
    }
}

string
LatLongCoord::serialise() const
{
    string result;
    GeoEncode::encode(latitude, longitude, result);
    return result;
}

string
LatLongCoord::get_description() const
{
    string res("Xapian::LatLongCoord(");
    res += str(latitude);
    res += ", ";
    res += str(longitude);
    res += ")";
    return res;
}

void
LatLongCoords::unserialise(const string & serialised)
{
    const char * ptr = serialised.data();
    const char * end_ptr = ptr + serialised.size();
    coords.clear();
    while (ptr != end_ptr) {
	coords.emplace_back();
	coords.back().unserialise(&ptr, end_ptr);
    }
}

string
LatLongCoords::serialise() const
{
    string result;
    vector<LatLongCoord>::const_iterator coord;
    for (coord = coords.begin(); coord != coords.end(); ++coord)
    {
	GeoEncode::encode(coord->latitude, coord->longitude, result);
    }
    return result;
}

string
LatLongCoords::get_description() const
{
    string res("Xapian::LatLongCoords(");
    vector<LatLongCoord>::const_iterator coord;
    for (coord = coords.begin(); coord != coords.end(); ++coord) {
	if (coord != coords.begin()) {
	    res += ", ";
	}
	res += "(";
	res += str(coord->latitude);
	res += ", ";
	res += str(coord->longitude);
	res += ")";
    }
    res += ")";
    return res;
}
