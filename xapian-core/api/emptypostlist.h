/** @file emptypostlist.h
 * @brief A PostList which contains no entries.
 */
/* Copyright (C) 2009,2011,2015,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_EMPTYPOSTLIST_H
#define XAPIAN_INCLUDED_EMPTYPOSTLIST_H

#include "postlist.h"

/// A PostList which contains no entries.
class EmptyPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const EmptyPostList &) = delete;

    /// Don't allow copying.
    EmptyPostList(const EmptyPostList &) = delete;

  public:
    /// Constructor.
    EmptyPostList() { }

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_max() const;
    Xapian::doccount get_termfreq_est() const;

    TermFreqs get_termfreq_est_using_stats(const Xapian::Weight::Internal &) const;

    Xapian::docid get_docid() const;
    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms) const;
    bool at_end() const;
    double recalc_maxweight();

    PostList * next(double w_min);
    PostList * skip_to(Xapian::docid, double w_min);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_EMPTYPOSTLIST_H
