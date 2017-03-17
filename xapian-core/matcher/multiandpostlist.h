/** @file multiandpostlist.h
 * @brief N-way AND postlist
 */
/* Copyright (C) 2007,2009,2011,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTIANDPOSTLIST_H
#define XAPIAN_INCLUDED_MULTIANDPOSTLIST_H

#include "multimatch.h"
#include "omassert.h"
#include "api/postlist.h"

#include <algorithm>

/// N-way AND postlist.
class MultiAndPostList : public PostList {
    /** Comparison functor which orders PostList* by ascending
     *  get_termfreq_est(). */
    struct ComparePostListTermFreqAscending {
	/// Order by ascending get_termfreq_est().
	bool operator()(const PostList *a, const PostList *b) const {
	    return a->get_termfreq_est() < b->get_termfreq_est();
	}
    };

    /// Don't allow assignment.
    void operator=(const MultiAndPostList &);

    /// Don't allow copying.
    MultiAndPostList(const MultiAndPostList &);

    /// The current docid, or zero if we haven't started or are at_end.
    Xapian::docid did;

    /// The number of sub-postlists.
    size_t n_kids;

    /// Array of pointers to sub-postlists.
    PostList ** plist;

    /// Array of maximum weights for the sub-postlists.
    double * max_wt;

    /// Total maximum weight (== sum of max_wt values).
    double max_total;

    /// The number of documents in the database.
    Xapian::doccount db_size;

    /// Pointer to the matcher object, so we can report pruning.
    MultiMatch *matcher;

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
	    matcher->recalc_maxweight();
	}
    }

    /// Call skip_to on a sub-postlist n, and handle any pruning.
    void skip_to_helper(size_t n, Xapian::docid did_min, double w_min) {
	PostList * res = plist[n]->skip_to(did_min, new_min(w_min, n));
	if (res) {
	    delete plist[n];
	    plist[n] = res;
	    matcher->recalc_maxweight();
	}
    }

    /// Call check on a sub-postlist n, and handle any pruning.
    void check_helper(size_t n, Xapian::docid did_min, double w_min,
		      bool &valid) {
	PostList * res = plist[n]->check(did_min, new_min(w_min, n), valid);
	if (res) {
	    delete plist[n];
	    plist[n] = res;
	    matcher->recalc_maxweight();
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
    template <class RandomItor>
    MultiAndPostList(RandomItor pl_begin, RandomItor pl_end,
		     MultiMatch * matcher_, Xapian::doccount db_size_)
	: did(0), n_kids(pl_end - pl_begin), plist(NULL), max_wt(NULL),
	  max_total(0), db_size(db_size_), matcher(matcher_)
    {
	allocate_plist_and_max_wt();

	// Copy the postlists in ascending termfreq order, since it will
	// be more efficient to look at the shorter lists first, and skip
	// the longer lists based on those.
	std::partial_sort_copy(pl_begin, pl_end, plist, plist + n_kids,
			       ComparePostListTermFreqAscending());
    }

    /** Construct as the decay product of an OrPostList or AndMaybePostList. */
    MultiAndPostList(PostList *l, PostList *r,
		     double lmax, double rmax,
		     MultiMatch * matcher_, Xapian::doccount db_size_)
	: did(0), n_kids(2), plist(NULL), max_wt(NULL),
	  max_total(lmax + rmax), db_size(db_size_), matcher(matcher_)
    {
	// Even if we're the decay product of an OrPostList, we may want to
	// swap here, as the subqueries may also have decayed and so their
	// estimated termfreqs may have changed.
	if (l->get_termfreq_est() < r->get_termfreq_est()) {
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

    ~MultiAndPostList();

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

    Internal *next(double w_min);

    Internal *skip_to(Xapian::docid, double w_min);

    std::string get_description() const;

    /** get_wdf() for MultiAndPostlists returns the sum of the wdfs of the
     *  sub postlists.
     *
     *  The wdf isn't really meaningful in many situations, but if the lists
     *  are being combined as a synonym we want the sum of the wdfs, so we do
     *  that in general.
     */
    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_MULTIANDPOSTLIST_H
