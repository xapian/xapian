/** @file range_acceleration.h
 * @brief Generation of terms for range acceleration.
 */
/* Copyright 2009 Lemur Consulting Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_RANGEACCEL_H
#define XAPIAN_INCLUDED_RANGEACCEL_H

#include <iostream>
#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <sstream>

#include <xapian/database.h>
#include <xapian/error.h>
#include <xapian/postingsource.h>
#include <xapian/query.h>
#include <xapian/visibility.h>

namespace Xapian {

typedef std::vector< std::pair<double, double> > Ranges;
class XAPIAN_VISIBILITY_DEFAULT RangeAccelerator {
    Xapian::valueno valnum;
    Ranges ranges;
    std::vector<double> midpoints;
    std::vector<std::string> range_terms;
    std::string prefix;
    double total_range;
    std::string make_range_term(const std::pair<double, double> r);
  public:

    RangeAccelerator(const std::string& prefix_, Xapian::valueno valnum_, double lo, double hi, double step)
	    : valnum(valnum_),
	      prefix(prefix_)
    {
	double x = lo;
	while (x <= hi) {
	    double next = x + step;
	    std::pair<double, double> r = std::make_pair(x, next);
	    ranges.push_back(r);
	    range_terms.push_back(make_range_term(r));
	    midpoints.push_back(x + (step / 2.0));
	    x = next;
	}
	total_range = ranges.back().second - ranges.front().first;
    }

    void add_val(Xapian::Document& doc, double val) const;
  
    Xapian::Query query_for_val_distance(double val, double cutoff = 0.0) const {
	Xapian::Query query;
	for (unsigned int i = 0; i < ranges.size(); ++i) {
	    double distance = fabs(val - midpoints[i]);
	    double weight = distance / total_range;
	    if (weight > cutoff) {
		query = Xapian::Query(Xapian::Query::OP_OR,
				      query,
				      Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
						    Xapian::Query(range_terms[i]),
						    weight));
	    }
	}
	return query;
    }
};

}
#endif /* XAPIAN_INCLUDED_RANGEACCEL_H */
