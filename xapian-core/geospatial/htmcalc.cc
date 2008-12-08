/** \file htmcalc.cc
 * \brief Heirarchical triangular mesh calculations.
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

#include "bitstream.h"
#include "stringutils.h"

#include <cfloat>
#include <cmath>
#include <vector>

using namespace Xapian;

/** A vector, representing a lat long on the unit sphere.
 *
 *  X axis is from centre of earth to 0 latitude & longitude.
 *  Y axis is from centre of earth to 0 latitude, 90 longitude.
 */
struct HTMVector {
    double x, y, z;

    HTMVector(double x_, double y_, double z_)
	    : x(x_), y(y_), z(z_)
    {
    }

    HTMVector(const LatLongCoord & coord)
    {
	double equator_d = cos(coord.latitude * M_PI / 180.0);
	x = cos(coord.longitude * M_PI / 180.0) * equator_d;
	y = sin(coord.longitude * M_PI / 180.0) * equator_d;
	z = sin(coord.latitude * M_PI / 180.0);
    }

    void normalise()
    {
	double l = sqrt(x * x + y * y + z * z);
	x /= l;
	y /= l;
	z /= l;
    }

    HTMVector operator +(const HTMVector & other) const
    {
	return HTMVector(x + other.x, y + other.y, z + other.z);
    }

    HTMVector operator ^(const HTMVector & other) const
    {
	return HTMVector(y * other.z - z * other.y,
			 z * other.x - x * other.z,
			 x * other.y - y * other.x);
    }

    double operator *(const HTMVector & other) const
    {
	return x * other.x + y * other.y + z * other.z;
    }
};

HTMCalculator::HTMCalculator(int min_depth_,
			     int max_depth_)
	: min_depth(min_depth_),
	  max_depth(max_depth_)
{
}

// Return true iff v is in triangle a, b, c.
// a, b, c must be in anticlockwise order, seen from above.
static bool
in_triangle(const HTMVector & a,
	    const HTMVector & b,
	    const HTMVector & c,
	    const HTMVector & v)
{
    if ((a ^ b) * v < -DBL_EPSILON) return false;
    if ((b ^ c) * v < -DBL_EPSILON) return false;
    if ((c ^ a) * v < -DBL_EPSILON) return false;
    return true;
}

static const double initial_vectors[6][3] = {
    {0.0,  0.0,  1.0},
    {1.0,  0.0,  0.0},
    {0.0,  1.0,  0.0},
    {-1.0,  0.0,  0.0},
    {0.0, -1.0,  0.0},
    {0.0,  0.0, -1.0}
};

// Triangles at at first level
static const int initial_triangles[8][3] = {
    {5, 2, 1}, // S0
    {5, 3, 2}, // S1
    {5, 4, 3}, // S2
    {5, 1, 4}, // S3
    {0, 4, 1}, // N0
    {0, 3, 4}, // N1
    {0, 2, 3}, // N2
    {0, 1, 2}  // N3
};

// Triangles at a new level
// 0,1,2 are the points of the current triangle
// 3,4,5 are the new points, at midpoints opposite the corresponding vertex
static const int new_triangles[4][3] = {
    {0, 5, 4},
    {1, 3, 5},
    {2, 4, 3},
    {3, 4, 5}
};

void
HTMCalculator::get_nodes_for_location(HTMNodeSet & nodes,
				      const LatLongCoord & location) const
{
    std::vector<HTMVector> v;
    v.reserve(6);
    HTMVector loc(location);
    int i;
    for (i = 0; i != 6; ++i) {
	v[i] = HTMVector(initial_vectors[i][0],
			 initial_vectors[i][1],
			 initial_vectors[i][2]);
    }

    for (i = 0; i != 8; ++i) {
	if (in_triangle(v[initial_triangles[i][0]],
			v[initial_triangles[i][1]],
			v[initial_triangles[i][2]],
			loc)) {
	    break;
	}
    }

    BitWriter node;
    node.encode(i, 8);
    int level = 0;
    if (level >= min_depth) {
	nodes.insert(std::string(1, char(level)) + node.slush());
    }

    // For each level, construct the new 3 points, find which of the 4 triangles we're in, and add to node.

    for (int depth = 0; depth <= max_depth; ++depth) {
	if (depth >= min_depth) {
	    nodes.insert(std::string(1, char(level)) + node.slush());
	}
    }
}

void
HTMCalculator::simplify(HTMNodeSet & nodes) const
{
    // FIXME - implement
    (void) nodes;
}

void
HTMCalculator::reduce(HTMNodeSet & nodes, int max_nodes) const
{
    // FIXME - implement
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
    // FIXME - implement
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
