/* andpostlist.h: Return only items which are in both sublists
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

#ifndef OM_HGUARD_ANDPOSTLIST_H
#define OM_HGUARD_ANDPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"
#include "omdebug.h"

/** A postlist comprising two postlists ANDed together.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists.  The weight for a posting is the sum of the weights of
 *  the sub-postings.
 */
class AndPostList : public BranchPostList {
    protected:
        Xapian::docid head;
        Xapian::weight lmax, rmax;
    private:
	Xapian::doccount dbsize;

        void process_next_or_skip_to(Xapian::weight w_min, PostList *ret);
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
	 *
	 *  The doclength returned by each branch should be the same.
	 *  The default implementation is simply to return the result
	 *  returned by the left branch: the left branch is preferable
	 *  because this should be the fastest way to get to a leaf node.
	 */
	virtual Xapian::doclength get_doclength() const;

        AndPostList(PostList *left,
		    PostList *right,
		    MultiMatch *matcher_,
		    Xapian::doccount dbsize_,
		    bool replacement = false);
};

inline Xapian::doccount
AndPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_max", "");
    RETURN(std::min(l->get_termfreq_max(), r->get_termfreq_max()));
}

inline Xapian::doccount
AndPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_min", "");
    RETURN(0u);
}

inline Xapian::doccount
AndPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    RETURN(static_cast<Xapian::doccount> (lest * rest / dbsize));
}

inline Xapian::docid
AndPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "AndPostList::get_docid", "");
    RETURN(head);
}

// only called if we are doing a probabilistic AND
inline Xapian::weight
AndPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::get_weight", "");
    RETURN(l->get_weight() + r->get_weight());
}

// only called if we are doing a probabilistic operation
inline Xapian::weight
AndPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::get_maxweight", "");
    RETURN(lmax + rmax);
}

inline Xapian::weight
AndPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    RETURN(AndPostList::get_maxweight());
}

inline bool
AndPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndPostList::at_end", "");
    RETURN(head == 0);
}

inline std::string
AndPostList::get_description() const
{
    return "(" + l->get_description() + " And " + r->get_description() + ")";
}

inline Xapian::doclength
AndPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "AndPostList::get_doclength", "");
    Xapian::doclength doclength = l->get_doclength();
    DEBUGLINE(MATCH, "docid=" << head);
    AssertEqDouble(l->get_doclength(), r->get_doclength());
    RETURN(doclength);
}

#endif /* OM_HGUARD_ANDPOSTLIST_H */
