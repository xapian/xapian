/** \file geospatial.cc
 * \brief Geospatial search support routines.
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

#include <xapian/geospatial.h>
#include <xapian/error.h>
#include <math.h>

#include "serialise.h"
#include "serialise-double.h"
#include "stringutils.h"

using namespace Xapian;

/** Quadratic mean radius of the earth in metres.
 */
#define QUAD_EARTH_RADIUS_METRES 6372797.6

/** Set M_PI if it's not already set.
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LatLongCoord::LatLongCoord(double latitude_, double longitude_)
	: latitude(latitude_),
	  longitude(longitude_)
{
    if (latitude < -90.0 || latitude > 90.0)
	throw InvalidArgumentError("Latitude out-of-range");
    longitude = remainder(longitude, 360.0);
    if (longitude == -180.0)
	longitude = 180.0;
    if (longitude == -0.0)
	longitude = 0.0;
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
    LatLongCoords coords = unserialise(&ptr, end);
    if (ptr != end)
	throw InvalidArgumentError(
		"Junk found at end of serialised LatLongCoords");
    return coords;
}

LatLongCoords
LatLongCoords::unserialise(const char ** ptr, const char * end)
{
    try {
	LatLongCoords result;
	// This will raise NetworkError for invalid serialisations.
	size_t count = decode_length(ptr, end, false);
	while (count != 0) {
	    result.coords.insert(LatLongCoord::unserialise(ptr, end));
	    --count;
	}
	return result;
    } catch (const NetworkError & e) {
	// FIXME - modify unserialise_double somehow so we don't have to catch
	// and rethrow the exceptions it raises.
	throw InvalidArgumentError(e.get_msg());
    }
}

std::string
LatLongCoords::serialise() const
{
    std::string result(encode_length(coords.size()));
    std::set<LatLongCoord>::const_iterator coord;
    for (coord = coords.begin(); coord != coords.end(); ++coord)
    {
	result += serialise_double(coord->latitude);
	result += serialise_double(coord->longitude);
    }
    return result;
}


LatLongMetric::~LatLongMetric()
{
}

double
LatLongMetric::distance(const LatLongCoords & a, const LatLongCoords &b) const
{
    if (a.empty() || b.empty()) {
	throw InvalidArgumentError("Empty coordinate list supplied to LatLongMetric::distance().");
    }
    double min_dist = 0.0;
    bool have_min = false;
    for (std::set<LatLongCoord>::const_iterator a_iter = a.begin();
	 a_iter != a.end();
	 ++a_iter)
    {
	for (std::set<LatLongCoord>::const_iterator b_iter = b.begin();
	     b_iter != b.end();
	     ++b_iter)
	{
	    double dist = distance(*a_iter, *b_iter);
	    if (!have_min) {
		min_dist = dist;
		have_min = true;
	    } else if (dist < min_dist) {
		min_dist = dist;
	    }
	}
    }
    return min_dist;
}


GreatCircleMetric::GreatCircleMetric()
	: radius(QUAD_EARTH_RADIUS_METRES)
{}

GreatCircleMetric::GreatCircleMetric(double radius_)
	: radius(radius_)
{}

double
GreatCircleMetric::distance(const LatLongCoord & a,
			    const LatLongCoord & b) const
{
    double lata = a.latitude * (M_PI / 180.0);
    double latb = b.latitude * (M_PI / 180.0);

    double latdiff = lata - latb;
    double longdiff = (a.longitude - b.longitude) * (M_PI / 180.0);

    double sin_half_lat = sin(latdiff / 2);
    double sin_half_long = sin(longdiff / 2);
    double h = sin_half_lat * sin_half_lat +
	    sin_half_long * sin_half_long * cos(lata) * cos(latb);
    double sqrt_h = sqrt(h);
    if (sqrt_h > 1.0) sqrt_h = 1.0;
    return 2 * radius * asin(sqrt_h);
}


HTMCalculator::HTMCalculator(int min_depth_,
			     int max_depth_)
	: min_depth(min_depth_),
	  max_depth(max_depth_)
{
}

void
HTMCalculator::get_nodes_for_location(HTMNodeSet & nodes,
				      const LatLongCoord & location) const
{
    // FIXME - implement
    (void) nodes;
    (void) location;
}

void
HTMCalculator::simplify(HTMNodeSet & nodes) const
{
    (void) nodes;
}

void
HTMCalculator::reduce(HTMNodeSet & nodes, int max_nodes) const
{
    (void) nodes;
    (void) max_nodes;
}

void
HTMCalculator::get_nodes_covering_circle(HTMNodeSet & nodes,
					 const LatLongCoord & centre,
					 double radius,
					 const LatLongMetric * metric,
					 int desired_nodes) const
{
    (void) nodes;
    (void) centre;
    (void) radius;
    (void) metric;
    (void) desired_nodes;
}

// Check if a character is uppercase.
// FIXME - copied from queryparser.lemony - needs to be moved to a shared location.
inline bool
U_isupper(unsigned ch) {
    return (ch < 128 && C_isupper((unsigned char)ch));
}

// Check if a prefix needs a colon.
// FIXME - copied from queryparser.lemony - needs to be moved to a shared location.
inline bool
prefix_needs_colon(const std::string & prefix, unsigned ch)
{
    if (!U_isupper(ch)) return false;
    std::string::size_type len = prefix.length();
    return (len > 1 && prefix[len - 1] != ':');
}

void
HTMCalculator::add_nodeterms_to_document(Document & doc,
					 const HTMNodeSet & nodes,
					 const std::string & prefix) const
{
    HTMNodeSet::const_iterator i;
    for (i = nodes.begin(); i != nodes.end(); ++i) {
	if (prefix_needs_colon(prefix, (*i)[0])) {
	    doc.add_term(prefix + ':' + *i);
	} else {
	    doc.add_term(prefix + *i);
	}
    }
}

Query
HTMCalculator::query_for_nodes(const HTMNodeSet & nodes,
			       const std::string & prefix) const
{
    HTMNodeSet::const_iterator i;
    Query query;
    for (i = nodes.begin(); i != nodes.end(); ++i) {
	std::string term(prefix);
	if (prefix_needs_colon(prefix, (*i)[0])) {
	    term += prefix;
	}
	term += *i;
	query = Query(Query::OP_OR, query, Query(term));
    }
    return query;
}

bool
LatLongRangeMatchDecider::operator()(const Document &doc) const
{
    std::string val(doc.get_value(valno));
    if (val.empty() == 0)
	return false;
    LatLongCoords coords = LatLongCoords::unserialise(val);
    return (metric->distance(centre, coords) <= range);
}
