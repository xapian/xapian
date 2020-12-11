/** @file
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008,2009,2011 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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

#ifndef XAPIAN_INCLUDED_VALUERANGEPOSTLIST_H
#define XAPIAN_INCLUDED_VALUERANGEPOSTLIST_H

#include "backends/database.h"
#include "api/postlist.h"
#include "backends/valuelist.h"

class ValueRangePostList : public PostList {
  protected:
    const Xapian::Database::Internal *db;

    Xapian::valueno slot;

    const std::string begin, end;

    Xapian::doccount db_size;

    ValueList * valuelist;

    /// Disallow copying.
    ValueRangePostList(const ValueRangePostList &);

    /// Disallow assignment.
    void operator=(const ValueRangePostList &);

  public:
    ValueRangePostList(const Xapian::Database::Internal *db_,
		       Xapian::valueno slot_,
		       const std::string &begin_, const std::string &end_)
	: db(db_), slot(slot_), begin(begin_), end(end_),
	  db_size(db->get_doccount()), valuelist(0) { }

    ~ValueRangePostList();

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::doccount get_termfreq_max() const;

    TermFreqs get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const;

    double get_maxweight() const;

    Xapian::docid get_docid() const;

    double get_weight() const;

    Xapian::termcount get_doclength() const;

    Xapian::termcount get_unique_terms() const;

    double recalc_maxweight();

    PositionList * read_position_list();

    PostList * next(double w_min);

    PostList * skip_to(Xapian::docid, double w_min);

    PostList * check(Xapian::docid did, double w_min, bool &valid);

    bool at_end() const;

    Xapian::termcount count_matching_subqs() const;

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_VALUERANGEPOSTLIST_H */
