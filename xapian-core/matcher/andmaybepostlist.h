/** @file
 * @brief PostList class implementing Query::OP_AND_MAYBE
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

#ifndef XAPIAN_INCLUDED_ANDMAYBEPOSTLIST_H
#define XAPIAN_INCLUDED_ANDMAYBEPOSTLIST_H

#include "postlisttree.h"
#include "wrapperpostlist.h"

/// PostList class implementing Query::OP_AND_MAYBE
class AndMaybePostList : public WrapperPostList {
    /// Right-hand side of OP_MAYBE.
    PostList* r;

    /// Current docid from WrapperPostList's pl.
    Xapian::docid pl_did = 0;

    /// Current docid from @a r (or 0).
    Xapian::docid r_did = 0;

    /// Current max weight from WrapperPostList's pl.
    double pl_max;

    /// Current max weight from @a r.
    double r_max;

    /** Total number of documents in the database.
     *
     *  Only used if we decay to AND.
     */
    Xapian::doccount db_size;

    PostListTree* pltree;

    /// Does @a r match at the current position?
    bool maybe_matches() const { return pl_did == r_did; }

    PostList* decay_to_and(Xapian::docid did,
			   double w_min,
			   bool* valid_ptr = NULL);

  public:
    AndMaybePostList(PostList* left, PostList* right,
		     PostListTree* pltree_, Xapian::doccount db_size_)
	: WrapperPostList(left), r(right), db_size(db_size_), pltree(pltree_)
    {}

    /** Construct as decay product from OrPostList.
     *
     *  The first operation after such construction must be check() or
     *  skip_to().
     */
    AndMaybePostList(PostList* left,
		     PostList* right,
		     double lmax,
		     double rmax,
		     PostListTree* pltree_,
		     Xapian::doccount db_size_)
	: WrapperPostList(left), r(right), pl_max(lmax), r_max(rmax),
	  db_size(db_size_), pltree(pltree_)
    {
    }

    ~AndMaybePostList() { delete r; }

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;

    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_ANDMAYBEPOSTLIST_H
