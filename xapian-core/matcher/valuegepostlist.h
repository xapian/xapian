/** @file
 * @brief Return document ids matching a >= test on a specified doc value.
 */
/* Copyright 2007,2011 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_VALUEGEPOSTLIST_H
#define XAPIAN_INCLUDED_VALUEGEPOSTLIST_H

#include "valuerangepostlist.h"

class ValueGePostList: public ValueRangePostList {
    /// Disallow copying.
    ValueGePostList(const ValueGePostList &);

    /// Disallow assignment.
    void operator=(const ValueGePostList &);

  public:
    ValueGePostList(const Xapian::Database::Internal *db_,
		    Xapian::valueno slot_,
		    const std::string &begin_)
	: ValueRangePostList(db_, slot_, begin_, std::string()) {}

    PostList * next(double w_min);

    PostList * skip_to(Xapian::docid, double w_min);

    PostList * check(Xapian::docid did, double w_min, bool &valid);

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_VALUEGEPOSTLIST_H */
