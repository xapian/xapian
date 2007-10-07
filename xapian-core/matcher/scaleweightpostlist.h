/** @file scaleweightpostlist.h
 * @brief Return documents from a subquery with weights multiplied by a double.
 */
/* Copyright 2007 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_SCALEWEIGHTPOSTLIST_H
#define XAPIAN_INCLUDED_SCALEWEIGHTPOSTLIST_H

#include "database.h"
#include "postlist.h"

class MultiMatch;

class ScaleWeightPostList : public PostList {
    /** The sub-postlist. */
    PostList *source;

    /** The multiplier to apply to the weights. */
    double multiplier;

    /** The object which is using this postlist to perform
     *  a match.  This object needs to be notified when the
     *  tree changes such that the maximum weights need to be
     *  recalculated.
     */
    MultiMatch *matcher;

    /// Disallow copying.
    ScaleWeightPostList(const ScaleWeightPostList &);

    /// Disallow assignment.
    void operator=(const ScaleWeightPostList &);

  public:
    ScaleWeightPostList(PostList *source_, double multiplier_,
			MultiMatch *matcher_)
	: source(source_), multiplier(multiplier_), matcher(matcher_) {}
    ~ScaleWeightPostList() { delete source; }

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;
    Xapian::weight get_maxweight() const;
    Xapian::docid get_docid() const;
    Xapian::weight get_weight() const;
    Xapian::doclength get_doclength() const;
    Xapian::weight recalc_maxweight();
    PositionList * read_position_list();
    PositionList * open_position_list() const;
    PostList * next(Xapian::weight w_min);
    PostList * skip_to(Xapian::docid did, Xapian::weight w_min);
    bool at_end() const;
    string get_description() const;
};

#endif /* XAPIAN_INCLUDED_SCALEWEIGHTPOSTLIST_H */
