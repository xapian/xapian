/* andmaybepostlist.h: Merged postlist: items from one list, weights from both
 *
 * AND MAYBE of two posting lists
 * A AND MAYBE B is logically just A, but we keep B around for weight purposes
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */


#ifndef OM_HGUARD_ANDMAYBEPOSTLIST_H
#define OM_HGUARD_ANDMAYBEPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"

/** A postlist with weights modified by another postlist.
 *
 *  This postlist returns a posting if and only if it is in the left
 *  sub-postlist.
 *
 *  If the posting does not occur in the right postlist, the weight for the
 *  posting is simply that in the left postlist.  If the posting occurs in
 *  both postlists, the weight for the posting is the sum of the weights in
 *  the sub-postlists.
 *
 *  This type of postlist is useful for specifying a set of terms which
 *  must appear in the query result: these terms can be specified as the
 *  left hand argument, with the rest of the query being on the right hand
 *  side, and having the effect of modifying the weights.
 *
 *  The postlist is also used as a "decay product" of other postlist types
 *  during the match process: when a postlist can no longer cause a
 *  document to enter the mset on its own, but can influence relative
 *  rankings, it may be combined using one of these.
 */
class AndMaybePostList : public BranchPostList {
    private:
	om_doccount dbsize; // only need in case we decay to an AndPostList
	om_docid lhead, rhead;
	om_weight lmax, rmax;

        PostList * process_next_or_skip_to(om_weight w_min, PostList *ret);
    public:
	om_doccount get_termfreq_max() const;
	om_doccount get_termfreq_min() const;
	om_doccount get_termfreq_est() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

        AndMaybePostList(PostList *left_,
			 PostList *right_,
			 MultiMatch *matcher_,
			 om_doccount dbsize_)
		: BranchPostList(left_, right_, matcher_),
		  dbsize(dbsize_), lhead(0), rhead(0)
	{
	    // lmax and rmax will get initialised by a recalc_maxweight
	}

	/// Constructor for use by decomposing OrPostList
        AndMaybePostList(PostList *left_,
			 PostList *right_,
			 MultiMatch *matcher_,
			 om_doccount dbsize_,
			 om_docid lhead_,
			 om_docid rhead_)
		: BranchPostList(left_, right_, matcher_),
		  dbsize(dbsize_), lhead(lhead_), rhead(rhead_)
	{
	    // Initialise the maxweights from the kids so we can avoid forcing
	    // a full maxweight recalc
	    lmax = l->get_maxweight();
	    rmax = r->get_maxweight();
	}
};

inline om_doccount
AndMaybePostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, om_doccount, "AndMaybePostList::get_termfreq_max", "");
    // Termfreq is exactly that of left hand branch.
    return l->get_termfreq_max();
}

inline om_doccount
AndMaybePostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, om_doccount, "AndMaybePostList::get_termfreq_min", "");
    // Termfreq is exactly that of left hand branch.
    return l->get_termfreq_min();
}

inline om_doccount
AndMaybePostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, om_doccount, "AndMaybePostList::get_termfreq_est", "");
    // Termfreq is exactly that of left hand branch.
    return l->get_termfreq_est();
}

inline om_docid
AndMaybePostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "AndMaybePostList::get_docid", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    return lhead;
}

// only called if we are doing a probabilistic AND MAYBE
inline om_weight
AndMaybePostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "AndMaybePostList::get_weight", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) return l->get_weight() + r->get_weight();
    return l->get_weight();
}

// only called if we are doing a probabilistic operation
inline om_weight
AndMaybePostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "AndMaybePostList::get_maxweight", "");
    return lmax + rmax;
}

inline om_weight
AndMaybePostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "AndMaybePostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    return AndMaybePostList::get_maxweight();
}

inline bool
AndMaybePostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndMaybePostList::at_end", "");
    return lhead == 0;
}

inline std::string
AndMaybePostList::get_description() const
{
    return "(" + l->get_description() + " AndMaybe " + r->get_description() +
	   ")";
}

inline om_doclength
AndMaybePostList::get_doclength() const
{
    DEBUGCALL(MATCH, om_doclength, "AndMaybePostList::get_doclength", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) AssertEqDouble(l->get_doclength(), r->get_doclength());
    return l->get_doclength();
}

#endif /* OM_HGUARD_ANDMAYBEPOSTLIST_H */
