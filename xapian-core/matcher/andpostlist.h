/** @file
 * @brief N-way AND postlist
 */
/* Copyright (C) 2007,2009,2011,2017,2021 Olly Betts
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

#ifndef XAPIAN_INCLUDED_ANDPOSTLIST_H
#define XAPIAN_INCLUDED_ANDPOSTLIST_H

#include "backends/postlist.h"
#include "omassert.h"
#include "postlisttree.h"

#include <algorithm>

/// N-way AND postlist.
class AndPostList : public PostList {
    /** Comparison functor which orders PostList* by ascending
     *  get_termfreq(). */
    struct ComparePostListTermFreqAscending {
	/// Order by ascending get_termfreq().
	bool operator()(const PostList *a, const PostList *b) const {
	    return a->get_termfreq() < b->get_termfreq();
	}
    };

    /// Don't allow assignment.
    AndPostList& operator=(const AndPostList&) = delete;

    /// Don't allow copying.
    AndPostList(const AndPostList&) = delete;

    /// The current docid, or zero if we haven't started or are at_end.
    Xapian::docid did = 0;

    /// The number of sub-postlists.
    size_t n_kids;

    /// Array of pointers to sub-postlists.
    PostList** plist = nullptr;

    /// Array of maximum weights for the sub-postlists.
    double* max_wt = nullptr;

    /// Total maximum weight (== sum of max_wt values).
    double max_total = 0.0;

    /// Pointer to the matcher object, so we can report pruning.
    PostListTree *matcher;

    /// Calculate the new minimum weight for sub-postlist n.
    double new_min(double w_min, size_t n) {
	return w_min - (max_total - max_wt[n]);
    }

    /// Call next on a sub-postlist n, and handle any pruning.
    void next_helper(size_t n, double w_min) {
	PostList * res = plist[n]->next(new_min(w_min, n));
	if (res) {
	    delete plist[n];
	    plist[n] = res;
	    if (max_wt[n] > 0)
		matcher->force_recalc();
	}
    }

    /// Call skip_to on a sub-postlist n, and handle any pruning.
    void skip_to_helper(size_t n, Xapian::docid did_min, double w_min) {
	PostList * res = plist[n]->skip_to(did_min, new_min(w_min, n));
	if (res) {
	    delete plist[n];
	    plist[n] = res;
	    if (max_wt[n] > 0)
		matcher->force_recalc();
	}
    }

    /// Call check on a sub-postlist n, and handle any pruning.
    void check_helper(size_t n, Xapian::docid did_min, double w_min,
		      bool &valid) {
	PostList * res = plist[n]->check(did_min, new_min(w_min, n), valid);
	if (res) {
	    delete plist[n];
	    plist[n] = res;
	    if (max_wt[n] > 0)
		matcher->force_recalc();
	}
    }

    /** Allocate plist and max_wt arrays of @a n_kids each.
     *
     *  @exception  std::bad_alloc.
     */
    void allocate_plist_and_max_wt();

    /// Advance the sublists to the next match.
    PostList * find_next_match(double w_min);

  public:
    /** Construct from 2 random-access iterators to a container of PostList*,
     *  a pointer to the matcher, and the document collection size.
     */
    template<class RandomItor>
    AndPostList(RandomItor pl_begin, RandomItor pl_end,
		PostListTree* matcher_, Xapian::doccount db_size)
	: n_kids(pl_end - pl_begin), matcher(matcher_)
    {
	allocate_plist_and_max_wt();

	// Copy the postlists in ascending termfreq order, since it will
	// be more efficient to look at the shorter lists first, and skip
	// the longer lists based on those.
	std::partial_sort_copy(pl_begin, pl_end, plist, plist + n_kids,
			       ComparePostListTermFreqAscending());

	// We shortcut an empty shard and avoid creating a postlist tree for it.
	Assert(db_size);
	// We calculate the estimate assuming independence.  With this
	// assumption, the estimate is the product of the estimates for the
	// sub-postlists divided by db_size (n_kids - 1) times.
	double result = plist[0]->get_termfreq();
	for (size_t i = 1; i < n_kids; ++i) {
	    result = (result * plist[i]->get_termfreq()) / db_size;
	}
	termfreq = static_cast<Xapian::doccount>(result + 0.5);
    }

    /** Construct as the decay product of an OrPostList or AndMaybePostList. */
    AndPostList(PostList* l, PostList* r,
		double lmax, double rmax,
		PostListTree* matcher_, Xapian::doccount termfreq_)
	: n_kids(2), max_total(lmax + rmax), matcher(matcher_)
    {
	// Just copy the estimate from the PostList which decayed.
	termfreq = termfreq_;

	// Even if we're the decay product of an OrPostList, we may want to
	// swap here, as the subqueries may also have decayed and so their
	// estimated termfreqs may have changed.
	if (l->get_termfreq() < r->get_termfreq()) {
	    std::swap(l, r);
	    std::swap(lmax, rmax);
	}
	allocate_plist_and_max_wt();
	// Put the least frequent postlist first.
	plist[0] = r;
	plist[1] = l;
	max_wt[0] = rmax;
	max_wt[1] = lmax;
    }

    ~AndPostList();

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    bool at_end() const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid, double w_min);

    std::string get_description() const;

    /** Get the within-document frequency.
     *
     *  For AndPostlist returns the sum of the wdfs of the sub postlists.
     *
     *  The wdf isn't really meaningful in many situations, but if the lists
     *  are being combined as a synonym we want the sum of the wdfs, so we do
     *  that in general.
     */
    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_ANDPOSTLIST_H
