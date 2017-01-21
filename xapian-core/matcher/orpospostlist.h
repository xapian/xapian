/** @file orpospostlist.h
 * @brief Wrapper postlist providing positions for an OR
 */
/* Copyright 2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_ORPOSPOSTLIST_H
#define XAPIAN_INCLUDED_ORPOSPOSTLIST_H

#include "api/postlist.h"
#include "orpositionlist.h"

/** Wrapper postlist providing positions for an OR. */
class OrPosPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const OrPosPostList &);

    /// Don't allow copying.
    OrPosPostList(const OrPosPostList &);

    PostList* pl;

    OrPositionList position_list;

  public:
    OrPosPostList(PostList* pl_)
	: pl(pl_) { }

    ~OrPosPostList();

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_max() const;

    Xapian::doccount get_termfreq_est() const;

    TermFreqs get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const;

    double get_maxweight() const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_doclength() const;

    Xapian::termcount get_unique_terms() const;

    double get_weight() const;

    bool at_end() const;

    double recalc_maxweight();

    PositionList * read_position_list();

    Internal *next(double w_min);

    Internal *skip_to(Xapian::docid, double w_min);

    std::string get_description() const;

    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;
};

#endif // XAPIAN_INCLUDED_ORPOSPOSTLIST_H
