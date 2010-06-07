/** @file latlong.cc
 * @brief Latitude and longitude representations.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#include "serialise.h"
#include "serialise-double.h"
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
    if (longitude <= -180) longitude += 360;
    if (longitude > 180) longitude -= 360;
}

LatLongCoord
LatLongCoord::unserialise(const string & serialised)
{
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    LatLongCoord result = unserialise(&ptr, end);
    if (ptr != end)
	throw SerialisationError(
		"Junk found at end of serialised LatLongCoord");
    return result;
}

LatLongCoord
LatLongCoord::unserialise(const char ** ptr, const char * end)
{
    double latitude = unserialise_double(ptr, end);
    double longitude = unserialise_double(ptr, end);
    return LatLongCoord(latitude, longitude);
}

string
LatLongCoord::serialise() const
{
    string result(serialise_double(latitude));
    result += serialise_double(longitude);
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

LatLongCoords
LatLongCoords::unserialise(const string & serialised)
{
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    LatLongCoords result;
    while (ptr != end) {
	result.coords.push_back(LatLongCoord::unserialise(&ptr, end));
    }
    if (ptr != end) {
	throw SerialisationError("Junk found at end of serialised "
				 "LatLongCoords");
    }
    return result;
}

string
LatLongCoords::serialise() const
{
    string result;
    vector<LatLongCoord>::const_iterator coord;
    for (coord = coords.begin(); coord != coords.end(); ++coord)
    {
	result += serialise_double(coord->latitude);
	result += serialise_double(coord->longitude);
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
