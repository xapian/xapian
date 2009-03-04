/** @file range_accelerator.cc
 * @brief Range search acceleration.
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

#include <config.h>

#include "xapian/range_accelerator.h"
#include "serialise-double.h"
#include "serialise.h"

template<class T>
inline std::string to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

std::string
Xapian::RangeAccelerator::make_range_term(unsigned int count) {
    return prefix + encode_length(count);
}

Xapian::RangeAccelerator::RangeAccelerator(const std::string& prefix_,
					   double lo,
					   double hi,
					   double step)
	: prefix(prefix_)
{
    double x = lo;
    unsigned int count = 0;
    while (x <= hi) {
	double next = x + step;
	std::pair<double, double> r = std::make_pair(x, next);
	ranges.push_back(r);
	range_terms.push_back(make_range_term(count));
	midpoints.push_back(x + (step / 2.0));
	x = next;
	++count;
    }
    total_range = ranges.back().second - ranges.front().first;
}


void
Xapian::RangeAccelerator::add_val_terms(Xapian::Document& doc, double val) const
{
    for (int i = 0; i < ranges.size(); ++i) {
	if ((val >= ranges[i].first) & (val <= ranges[i].second)) {
	    doc.add_term(range_terms[i], 0);
	}
    }
}

Xapian::Query
Xapian::RangeAccelerator::query_for_distance(double val,
					     double cutoff) const
{
    std::vector<Xapian::Query> subqs;
    for (unsigned int i = 0; i < ranges.size(); ++i) {
	double distance = fabs(val - midpoints[i]);
	double weight = total_range - distance;
	if (weight > cutoff) {
	    FixedWeightPostingSource ps(weight);
	    subqs.push_back(Xapian::Query(Xapian::Query::OP_FILTER,
					  Xapian::Query(&ps),
					  Xapian::Query(range_terms[i])));
	}
    }
    Xapian::Query query(Xapian::Query::OP_XOR, subqs.begin(), subqs.end());
    return query;
}

Xapian::Query 
Xapian::RangeAccelerator::query_for_distance(const Xapian::Document& doc, double cutoff) const
{
    Xapian::TermIterator it = doc.termlist_begin();
    it.skip_to(prefix);
    if ((*it).find(prefix) == 0) {
	std::vector<std::string>::const_iterator pos = 
		std::find(range_terms.begin(), range_terms.end(), *it);
	if (pos != range_terms.end())
	    return query_for_distance(midpoints[std::distance(range_terms.begin(), pos)], cutoff);
    }
    return Xapian::Query();
}
