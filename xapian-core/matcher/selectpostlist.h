/** @file
 * @brief Base class for classes which filter another PostList
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

#ifndef XAPIAN_INCLUDED_SELECTPOSTLIST_H
#define XAPIAN_INCLUDED_SELECTPOSTLIST_H

#include "wrapperpostlist.h"

#include <cmath>

class PostListTree;

/// Base class for classes which filter another PostList
class SelectPostList : public WrapperPostList {
    /// Used to avoid calculating the weight twice for a given document.
    double cached_weight = -HUGE_VAL;

    PostListTree* pltree;

    /// Check if the current document is suitable.
    bool vet(double w_min);

  protected:
    /// Check if the current document should be selected.
    virtual bool test_doc() = 0;

  public:
    SelectPostList(PostList* pl_,
		   PostListTree* pltree_)
	: WrapperPostList(pl_), pltree(pltree_) {}

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    bool at_end() const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    Xapian::doccount get_termfreq_min() const;
};

#endif // XAPIAN_INCLUDED_SELECTPOSTLIST_H
