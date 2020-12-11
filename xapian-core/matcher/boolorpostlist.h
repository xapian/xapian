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
    Xapian::docid did;

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
    PostListAndDocID* plist;

    /** Total number of documents in the database. */
    Xapian::doccount db_size;

    /** Helper to apply operation to all postlists matching current docid.
     *
     *  This function makes use of the heap structure, descending to any
     *  children which match the current docid in an effectively recursive way
     *  which needs O(1) storage, and evaluating func for each of them.
     *
     *  There's support for accumulating a value of type Xapian::termcount,
     *  which is returned (of the three current uses, two want to accumulate a
     *  value of this type, while the other doesn't need to accumulate a
     *  value).
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
	    size_t j = 2 * i + 1;
	    if (j < n_kids && plist[j].did == did) {
		// Down left.
		i = j;
		continue;
	    }
	    if (j + 1 < n_kids && plist[j + 1].did == did) {
		// Down right.
		i = j + 1;
		continue;
	    }
    try_right:
	    if ((i & 1) && i + 1 < n_kids && plist[i + 1].did == did) {
		// Right.
		++i;
		continue;
	    }
	    // Up.
	    i = (i - 1) / 2;
	    if (i == 0) break;
	    goto try_right;
	}
	return result;
    }

  public:
    /** Construct from 2 random-access iterators to a container of PostList*,
     *  a pointer to the matcher, and the document collection size.
     */
    template<class RandomItor>
    BoolOrPostList(RandomItor pl_begin, RandomItor pl_end,
		   Xapian::doccount db_size_)
	: did(0), n_kids(pl_end - pl_begin), plist(NULL),
	  db_size(db_size_)
    {
	plist = new PostListAndDocID[n_kids];
	// This initialises all entries to have did 0, so all entries are
	// equal, which is a valid heap.
	std::copy(pl_begin, pl_end, plist);
    }

    ~BoolOrPostList();

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_max() const;

    Xapian::doccount get_termfreq_est() const;

    TermFreqs get_termfreq_est_using_stats(
	    const Xapian::Weight::Internal& stats) const;

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    bool at_end() const;

    double recalc_maxweight();

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    std::string get_description() const;

    Xapian::termcount get_wdf() const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);
};

#endif // XAPIAN_INCLUDED_BOOLORPOSTLIST_H
