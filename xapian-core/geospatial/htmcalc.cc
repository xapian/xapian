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

#include "stringutils.h"

using namespace Xapian;

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
