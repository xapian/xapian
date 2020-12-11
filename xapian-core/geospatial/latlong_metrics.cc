/** @file
 * @brief Geospatial distance metrics.
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
#include "serialise-double.h"

#include <cmath>

using namespace Xapian;
using namespace std;

/** Quadratic mean radius of the Earth in metres.
 */
#define QUAD_EARTH_RADIUS_METRES 6372797.6

LatLongMetric::~LatLongMetric()
{
}

double
LatLongMetric::operator()(const LatLongCoords& a,
			  const LatLongCoords& b) const
{
    if (a.empty() || b.empty()) {
	throw InvalidArgumentError("Empty coordinate list supplied to LatLongMetric::operator()()");
    }
    double min_dist = 0.0;
    bool have_min = false;
    for (LatLongCoordsIterator a_iter = a.begin();
	 a_iter != a.end();
	 ++a_iter)
    {
	for (LatLongCoordsIterator b_iter = b.begin();
	     b_iter != b.end();
	     ++b_iter)
	{
	    double dist = pointwise_distance(*a_iter, *b_iter);
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

double
LatLongMetric::operator()(const LatLongCoords& a,
			  const char* b_ptr, size_t b_len) const
{
    if (a.empty() || b_len == 0) {
	throw InvalidArgumentError("Empty coordinate list supplied to LatLongMetric::operator()()");
    }
    double min_dist = 0.0;
    bool have_min = false;
    LatLongCoord b;
    const char * b_end = b_ptr + b_len;
    while (b_ptr != b_end) {
	b.unserialise(&b_ptr, b_end);
	for (LatLongCoordsIterator a_iter = a.begin();
	     a_iter != a.end();
	     ++a_iter)
	{
	    double dist = pointwise_distance(*a_iter, b);
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
GreatCircleMetric::pointwise_distance(const LatLongCoord& a,
				      const LatLongCoord& b) const
{
    double lata = a.latitude * (M_PI / 180.0);
    double latb = b.latitude * (M_PI / 180.0);

    double latdiff = lata - latb;
    double longdiff = (a.longitude - b.longitude) * (M_PI / 180.0);

    double sin_half_lat = sin(latdiff / 2);
    double sin_half_long = sin(longdiff / 2);
    double h = sin_half_lat * sin_half_lat +
	    sin_half_long * sin_half_long * cos(lata) * cos(latb);
    if (rare(h > 1.0)) {
	// Clamp to 1.0, asin(1.0) = M_PI / 2.0.
	return radius * M_PI;
    }
    return 2 * radius * asin(sqrt(h));
}

LatLongMetric *
GreatCircleMetric::clone() const
{
    return new GreatCircleMetric(radius);
}

string
GreatCircleMetric::name() const
{
    return "Xapian::GreatCircleMetric";
}

string
GreatCircleMetric::serialise() const
{
    return serialise_double(radius);
}

LatLongMetric *
GreatCircleMetric::unserialise(const string& s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    double new_radius = unserialise_double(&p, end);
    if (p != end) {
	throw Xapian::NetworkError("Bad serialised GreatCircleMetric - junk at end");
    }

    return new GreatCircleMetric(new_radius);
}
