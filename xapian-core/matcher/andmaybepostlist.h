/* andmaybepostlist.h: Merged postlist: items from one list, weights from both
 *
 * AND MAYBE of two posting lists
 * A AND MAYBE B is logically just A, but we keep B around for weight purposes
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
	Xapian::doccount dbsize; // only need in case we decay to an AndPostList
	Xapian::docid lhead, rhead;
	Xapian::weight lmax, rmax;

        PostList * process_next_or_skip_to(Xapian::weight w_min, PostList *ret);
    public:
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;
	Xapian::weight get_maxweight() const;

        Xapian::weight recalc_maxweight();

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const;

        AndMaybePostList(PostList *left_,
			 PostList *right_,
			 MultiMatch *matcher_,
			 Xapian::doccount dbsize_)
		: BranchPostList(left_, right_, matcher_),
		  dbsize(dbsize_), lhead(0), rhead(0)
	{
	    // lmax and rmax will get initialised by a recalc_maxweight
	}

	/// Constructor for use by decomposing OrPostList
        AndMaybePostList(PostList *left_,
			 PostList *right_,
			 MultiMatch *matcher_,
			 Xapian::doccount dbsize_,
			 Xapian::docid lhead_,
			 Xapian::docid rhead_)
		: BranchPostList(left_, right_, matcher_),
		  dbsize(dbsize_), lhead(lhead_), rhead(rhead_)
	{
	    // Initialise the maxweights from the kids so we can avoid forcing
	    // a full maxweight recalc
	    lmax = l->get_maxweight();
	    rmax = r->get_maxweight();
	}
};

inline Xapian::doccount
AndMaybePostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_max", "");
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_max());
}

inline Xapian::doccount
AndMaybePostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_min", "");
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_min());
}

inline Xapian::doccount
AndMaybePostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_est", "");
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_est());
}

inline Xapian::docid
AndMaybePostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "AndMaybePostList::get_docid", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    RETURN(lhead);
}

// only called if we are doing a probabilistic AND MAYBE
inline Xapian::weight
AndMaybePostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndMaybePostList::get_weight", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) RETURN(l->get_weight() + r->get_weight());
    RETURN(l->get_weight());
}

// only called if we are doing a probabilistic operation
inline Xapian::weight
AndMaybePostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndMaybePostList::get_maxweight", "");
    RETURN(lmax + rmax);
}

inline Xapian::weight
AndMaybePostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "AndMaybePostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    RETURN(AndMaybePostList::get_maxweight());
}

inline bool
AndMaybePostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndMaybePostList::at_end", "");
    RETURN(lhead == 0);
}

inline std::string
AndMaybePostList::get_description() const
{
    return "(" + l->get_description() + " AndMaybe " + r->get_description() +
	   ")";
}

inline Xapian::doclength
AndMaybePostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "AndMaybePostList::get_doclength", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead == rhead) AssertEqDouble(l->get_doclength(), r->get_doclength());
    RETURN(l->get_doclength());
}

#endif /* OM_HGUARD_ANDMAYBEPOSTLIST_H */
