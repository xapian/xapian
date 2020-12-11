/** @file
 * @brief PostList which adds on a term-independent weight contribution
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

#ifndef XAPIAN_INCLUDED_EXTRAWEIGHTPOSTLIST_H
#define XAPIAN_INCLUDED_EXTRAWEIGHTPOSTLIST_H

#include "wrapperpostlist.h"

namespace Xapian {
class Weight;
}

class PostListTree;

/// PostList which adds on a term-independent weight contribution
class ExtraWeightPostList : public WrapperPostList {
    /// Don't allow assignment.
    void operator=(const ExtraWeightPostList&) = delete;

    /// Don't allow copying.
    ExtraWeightPostList(const ExtraWeightPostList&) = delete;

    Xapian::Weight* weight;

    PostListTree* pltree;

    double max_extra;

  public:
    ExtraWeightPostList(PostList* pl_,
			Xapian::Weight* weight_,
			PostListTree* pltree_)
	: WrapperPostList(pl_),
	  weight(weight_),
	  pltree(pltree_),
	  max_extra(weight->get_maxextra()) {}

    ~ExtraWeightPostList() {
	delete weight;
    }

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid, double w_min);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_EXTRAWEIGHTPOSTLIST_H
