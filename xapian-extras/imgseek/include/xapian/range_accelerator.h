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
    Ranges ranges;
    std::vector<double> midpoints;
    std::vector<std::string> range_terms;
    std::string prefix;
    double total_range;
    std::string make_range_term(unsigned int count);

  public:
    RangeAccelerator(const std::string& prefix_,
		     double lo,
		     double hi,
		     double step);

    /** Add terms for the given value to the document.
     */
    void add_val_terms(Xapian::Document& doc, double val) const;
  
    /** Make a query which returns 
     */
    Xapian::Query query_for_distance(double val, double cutoff = 0.0) const;
    Xapian::Query query_for_distance(const Xapian::Document& doc, double cutoff = 0.0) const;
};

}
#endif /* XAPIAN_INCLUDED_RANGEACCEL_H */
