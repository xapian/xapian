/** @file
 * @brief PostList class implementing unweighted Query::OP_OR
 */
/* Copyright 2017,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BOOLORPOSTLIST_H
#define XAPIAN_INCLUDED_BOOLORPOSTLIST_H

#include "backends/postlist.h"

/// PostList class implementing unweighted Query::OP_OR
class BoolOrPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const BoolOrPostList&) = delete;

    /// Don't allow copying.
    BoolOrPostList(const BoolOrPostList&) = delete;

    /// The current docid, or zero if we haven't started or are at_end.
    Xapian::docid did = 0;

    /// The number of sub-postlists.
    size_t n_kids;

    struct PostListAndDocID {
	PostList* pl;

	Xapian::docid did = 0;

	PostListAndDocID() : pl(nullptr) { }

	PostListAndDocID(PostList* pl_) : pl(pl_) { }

	bool operator>(const PostListAndDocID& o) const {
	    return did > o.did;
	}
    };

    /// Array of pointers to sub-postlists.
    PostListAndDocID* plist = nullptr;

    /** Helper to apply operation to all postlists matching current docid.
     *
     *  For each matching postlist this helper evaluates `func`, and
     *  accumulates the returned Xapian::termcount value.  Of the three current
     *  uses, two want to accumulate a value of this type, while the other
     *  doesn't need to accumulate anything.  This is an inlined template so
     *  the compiler's optimiser should be able to see the result of the
     *  accumulation isn't used and eliminate it.
     *
     *  This function makes use of the heap structure by walking the tree
     *  of the heap in a particular order, descending to any children which
     *  match the current docid (i.e. the docid of the tip of the heap) to
     *  visit every postlist matching the current docid (due to the heap
     *  property, for any such postlist all ancestors must also match the
     *  current docid).
     *
     *  At each step we do the first of the following which stays within the
     *  heap and matches the current docid:
     *
     *  * go down+right
     *  * go down+left
     *  * go left if we're the right descendent of our parent
     *  * go up until we can go left (or we reach the root where we stop)
     *
     *  This effectively recurses the tree, but only needs O(1) storage.  It
     *  requires O(n) in time where n is the number of postlists matching the
     *  current docid, but that's inherent as we need to call `func` n times.
     *
     *  Going down+right in preference to down+left simplifies the "go left"
     *  step a little because any right descendent must have a left sibling,
     *  but if the last entry in index order is a left descendent it doesn't
     *  have a right sibling.  It also makes it easy to implement an
     *  optimisation which can ascend multiple levels in one go if the
     *  compiler provides __builtin_ffs().
     */
    template<typename F>
    Xapian::termcount
    for_all_matches(F func) const
    {
	size_t i = 0;
	Xapian::termcount result = 0;
	AssertEq(plist[0].did, did);
	while (true) {
	    result += func(plist[i].pl);
	    // Children of i are (2 * i + 1) and (2 * i + 2).
	    size_t j = 2 * i + 2;
	    if (j < n_kids && plist[j].did == did) {
		// Down right.
		i = j;
		continue;
	    }
	    --j;
	    if (j < n_kids && plist[j].did == did) {
		// Down left.
		i = j;
		continue;
	    }
    try_left:
	    if (i < 2) break;
	    if ((i & 1) == 0 && plist[i - 1].did == did) {
		// Left.
		--i;
		continue;
	    }
	    // Up.
	    //
	    // We can ascend back up to our parent, plus any sequence of
	    // contiguous left branches up from our parent, with a neat
	    // bit-twiddling trick if we have __builtin_ffs() available.
#if HAVE_DECL___BUILTIN_FFS
	    ++i;
	    i >>= __builtin_ffs(i & ~1) - 1;
	    --i;
#else
	    // Fall-back to just ascending one level at a time, which is
	    // simple and probably similarly efficient to a portable
	    // fallback ffs() implementation which doesn't make use of
	    // specialised machine code instructions, since the distance
	    // we can ascend will be usually be short.
	    i = (i - 1) / 2;
#endif
	    goto try_left;
	}
	return result;
    }

  public:
    /** Construct from 2 random-access iterators to a container of PostList*,
     *  a pointer to the matcher, and the document collection size.
     */
    template<class RandomItor>
    BoolOrPostList(RandomItor pl_begin, RandomItor pl_end,
		   Xapian::doccount db_size)
	: n_kids(pl_end - pl_begin)
    {
	plist = new PostListAndDocID[n_kids];
	// This initialises all entries to have did 0, so all entries are
	// equal, which is a valid heap.
	std::copy(pl_begin, pl_end, plist);

	// We shortcut an empty shard and avoid creating a postlist tree for it.
	Assert(db_size);
	Assert(n_kids != 0);
	// We calculate the estimate assuming independence.  The simplest
	// way to calculate this seems to be a series of (n_kids - 1) pairwise
	// calculations, which gives the same answer regardless of the order.
	double scale = 1.0 / db_size;
	double P_est = plist[0].pl->get_termfreq() * scale;
	for (size_t i = 1; i < n_kids; ++i) {
	    double P_i = plist[i].pl->get_termfreq() * scale;
	    P_est += P_i - P_est * P_i;
	}
	termfreq = static_cast<Xapian::doccount>(P_est * db_size + 0.5);
    }

    ~BoolOrPostList();

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    bool at_end() const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    void get_docid_range(Xapian::docid& first, Xapian::docid& last) const;

    std::string get_description() const;

    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_BOOLORPOSTLIST_H
