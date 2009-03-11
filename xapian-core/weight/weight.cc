/** @file weight.cc
 * @brief Xapian::Weight base class
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"

#include "weightinternal.h"

#include "autoptr.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

void
Weight::init_(const Internal & stats, Xapian::termcount query_length)
{
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    average_length_ = stats.get_average_length();
    doclength_upper_bound_ = stats.db.get_doclength_upper_bound();
    doclength_lower_bound_ = stats.db.get_doclength_lower_bound();
    wdf_upper_bound_ = 0;
    termfreq_ = 0;
    reltermfreq_ = 0;
    query_length_ = query_length;
    wqf_ = 1;
}

void
Weight::init_(const Internal & stats, Xapian::termcount query_length,
	      const string & term, Xapian::termcount wqf, double factor)
{
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    average_length_ = stats.get_average_length();
    doclength_upper_bound_ = stats.db.get_doclength_upper_bound();
    doclength_lower_bound_ = stats.db.get_doclength_lower_bound();
    wdf_upper_bound_ = stats.db.get_wdf_upper_bound(term);
    termfreq_ = stats.get_termfreq(term);
    reltermfreq_ = stats.get_reltermfreq(term);
    query_length_ = query_length;
    wqf_ = wqf;
    init(factor);
}

Weight::~Weight() { }

bool Weight::get_sumpart_needs_doclength() const { return true; }

}
