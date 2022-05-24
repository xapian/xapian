/** @file
 * @brief PostList class implementing Query::OP_OR
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

#ifndef XAPIAN_INCLUDED_ORPOSTLIST_H
#define XAPIAN_INCLUDED_ORPOSTLIST_H

#include "backends/postlist.h"

class PostListTree;

/// PostList class implementing Query::OP_OR
class OrPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const OrPostList&) = delete;

    /// Don't allow copying.
    OrPostList(const OrPostList&) = delete;

    /** Left side.
     *
     *  We optimise assuming this side is more frequent, and so creators should
     *  try to set this side to the side with the higher estimated term
     *  frequency.
     */
    PostList* l;

    /// Right side.
    PostList* r;

    Xapian::docid l_did = 0;

    Xapian::docid r_did = 0;

    double l_max = 0;

    double r_max = 0;

    PostListTree* pltree;

    PostList* decay_to_and(Xapian::docid did,
			   double w_min,
			   bool* valid_ptr = NULL);

    PostList* decay_to_andmaybe(PostList* left,
				PostList* right,
				Xapian::docid did,
				double w_min,
				bool* valid_ptr = NULL);

  public:
    OrPostList(PostList* left, PostList* right,
	       PostListTree* pltree_, Xapian::doccount db_size);

    ~OrPostList() {
	delete l;
	delete r;
    }

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    bool at_end() const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;

    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_ORPOSTLIST_H
