/** @file
 * @brief PostList class implementing Query::OP_AND_NOT
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

#ifndef XAPIAN_INCLUDED_ANDNOTPOSTLIST_H
#define XAPIAN_INCLUDED_ANDNOTPOSTLIST_H

#include "wrapperpostlist.h"

class PostListTree;

/// PostList class implementing Query::OP_AND_NOT
class AndNotPostList : public WrapperPostList {
    /// Right-hand side of OP_NOT.
    PostList* r;

    /// Current docid from r (or 0).
    Xapian::docid r_did = 0;

  public:
    AndNotPostList(PostList* left, PostList* right, Xapian::doccount db_size)
	: WrapperPostList(left), r(right)
    {
	// We shortcut an empty shard and avoid creating a postlist tree for
	// it.
	Assert(db_size);
	// We calculate the estimate assuming independence.  With this
	// assumption, the estimate is the product of the estimates for the
	// sub-postlists (for the right side this is inverted by subtracting
	// from db_size), divided by db_size.
	double result = pl->get_termfreq();
	result = (result * (db_size - r->get_termfreq())) / db_size;
	termfreq = static_cast<Xapian::doccount>(result + 0.5);
    }

    ~AndNotPostList() { delete r; }

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_ANDNOTPOSTLIST_H
