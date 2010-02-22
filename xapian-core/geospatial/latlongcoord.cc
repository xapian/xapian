/** \file latlong.cc
 * \brief Latitude and longitude representations.
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

#include <math.h>

using namespace Xapian;

LatLongCoord::LatLongCoord(double latitude_, double longitude_)
	: latitude(latitude_),
	  longitude(longitude_)
{
    if (latitude < -90.0 || latitude > 90.0)
	throw InvalidArgumentError("Latitude out-of-range");
    longitude = fmod(longitude_, 360);
    if (longitude <= -180) longitude += 360;
    if (longitude > 180) longitude -= 360;
    if (longitude == -0.0) longitude = 0.0;
}

LatLongCoord
LatLongCoord::unserialise(const std::string & serialised)
{
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    LatLongCoord result = unserialise(&ptr, end);
    if (ptr != end)
	throw InvalidArgumentError(
		"Junk found at end of serialised LatLongCoord");
    return result;
}

LatLongCoord
LatLongCoord::unserialise(const char ** ptr, const char * end)
{
    try {
	// This will raise NetworkError for invalid serialisations.
	double latitude = unserialise_double(ptr, end);
	double longitude = unserialise_double(ptr, end);
	return LatLongCoord(latitude, longitude);
    } catch (const NetworkError & e) {
	// FIXME - modify unserialise_double somehow so we don't have to catch
	// and rethrow the exceptions it raises.
	throw InvalidArgumentError(e.get_msg());
    }
}

std::string
LatLongCoord::serialise() const
{
    std::string result(serialise_double(latitude));
    result += serialise_double(longitude);
    return result;
}

LatLongCoords
LatLongCoords::unserialise(const std::string & serialised)
{
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    LatLongCoords coords = unserialise(ptr, end);
    return coords;
}

LatLongCoords
LatLongCoords::unserialise(const char * ptr, const char * end)
{
    LatLongCoords result;
    try {
	while (ptr != end) {
	    // This will raise NetworkError for invalid serialisations (so we
	    // catch and re-throw it).
	    result.coords.insert(LatLongCoord::unserialise(&ptr, end));
	}
    } catch (const NetworkError & e) {
	// FIXME - modify unserialise_double somehow so we don't have to catch
	// and rethrow the exceptions it raises.
	throw InvalidArgumentError(e.get_msg());
    }
    if (ptr != end)
	throw InvalidArgumentError(
		"Junk found at end of serialised LatLongCoords");
    return result;
}

std::string
LatLongCoords::serialise() const
{
    std::string result;
    std::set<LatLongCoord>::const_iterator coord;
    for (coord = coords.begin(); coord != coords.end(); ++coord)
    {
	result += serialise_double(coord->latitude);
	result += serialise_double(coord->longitude);
    }
    return result;
}
