/** @file leafpostlist.h
 * @brief Abstract base class for leaf postlists.
 */
/* Copyright (C) 2007,2009 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_LEAFPOSTLIST_H
#define XAPIAN_INCLUDED_LEAFPOSTLIST_H

#include "postlist.h"

#include <string>

namespace Xapian {
    class Weight;
}

/** Abstract base class for leaf postlists.
 *
 *  This class provides the following features in addition to the PostList
 *  class:
 */
class LeafPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const LeafPostList &);

    /// Don't allow copying.
    LeafPostList(const LeafPostList &);

  protected:
    const Xapian::Weight * weight;

    bool need_doclength;

    /// The term name for this postlist (empty for an alldocs postlist).
    std::string term;

    /// Only constructable as a base class for derived classes.
    explicit LeafPostList(const std::string & term_)
	: weight(0), need_doclength(false), term(term_) { }

  public:
    ~LeafPostList();

    /** Set the weighting scheme to use during matching.
     *
     *  If this isn't called, get_weight() and get_maxweight() will both
     *  return 0.
     *
     *  You should not call this more than once on a particular object.
     *
     *  @param weight_	The weighting object to use.  Must not be NULL.
     */
    void set_termweight(const Xapian::Weight * weight_);

    /** Return the exact term frequency.
     *
     *  Leaf postlists have an exact termfreq, which get_termfreq_min(),
     *  get_termfreq_max(), and get_termfreq_est() all report.
     */
    virtual Xapian::doccount get_termfreq() const = 0;

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_max() const;
    Xapian::doccount get_termfreq_est() const;

    Xapian::weight get_maxweight() const;
    Xapian::weight get_weight() const;
    Xapian::weight recalc_maxweight();

    TermFreqs get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const;

    Xapian::termcount count_matching_subqs() const;
};

#endif // XAPIAN_INCLUDED_LEAFPOSTLIST_H
