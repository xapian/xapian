/* orpostlist.h: OR of two posting lists
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

#ifndef OM_HGUARD_ORPOSTLIST_H
#define OM_HGUARD_ORPOSTLIST_H

#include "branchpostlist.h"
#include <algorithm>

class PostList;

/** A postlist comprising two postlists ORed together.
 *
 *  This postlist returns a posting if it is in either of the sub-postlists.
 *  The weight for a posting is the sum of the weights of the sub-postings,
 *  if both exist, or the sum of the single sub-posting which exists
 *  otherwise.
 */
class OrPostList : public BranchPostList {
    private:
        om_docid lhead, rhead;
        om_weight lmax, rmax, minmax;
	om_doccount dbsize;
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
	 *
	 *  This is obtained by asking the subpostlist which contains the
	 *  current document for the document length.  If both subpostlists
	 *  are valid, the left one is asked.
	 */
	virtual om_doclength get_doclength() const;

        OrPostList(PostList * left_,
		   PostList * right_,
		   MultiMatch * matcher_,
		   om_doccount dbsize_);
};

inline om_doccount
OrPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, om_doccount, "OrPostList::get_termfreq_max", "");
    return std::min(l->get_termfreq_max() + r->get_termfreq_max(), dbsize);
}

inline om_doccount
OrPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, om_doccount, "OrPostList::get_termfreq_min", "");
    return std::max(l->get_termfreq_min(), r->get_termfreq_min());
}

inline om_doccount
OrPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, om_doccount, "OrPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l or r) = P(l) + P(r) - P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    return static_cast<om_doccount> (lest + rest - lest * rest / dbsize);
}

inline om_docid
OrPostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "OrPostList::get_docid", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    return std::min(lhead, rhead);
}

// only called if we are doing a probabilistic OR
inline om_weight
OrPostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "OrPostList::get_weight", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    if (lhead > rhead) return r->get_weight();
    return l->get_weight() + r->get_weight();
}

// only called if we are doing a probabilistic operation
inline om_weight
OrPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "OrPostList::get_maxweight", "");
    return lmax + rmax;
}

inline om_weight
OrPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "OrPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = std::min(lmax, rmax);
    return OrPostList::get_maxweight();
}

inline bool
OrPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "OrPostList::at_end", "");
    // Can never really happen - OrPostList next/skip_to autoprune
    AssertParanoid(!(l->at_end()) && !(r->at_end()));
    return false;
}

inline std::string
OrPostList::get_description() const
{
    return "(" + l->get_description() + " Or " + r->get_description() + ")";
}

inline om_doclength
OrPostList::get_doclength() const
{
    DEBUGCALL(MATCH, om_doclength, "OrPostList::get_doclength", "");
    om_doclength doclength;

    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead > rhead) {
	doclength = r->get_doclength();
	DEBUGLINE(MATCH, "OrPostList::get_doclength() [right docid=" 
		  << rhead << "] = " << doclength);
    } else {
	doclength = l->get_doclength();
	DEBUGLINE(MATCH, "OrPostList::get_doclength() [left docid="
		  << lhead << "] = " << doclength);
    }

    return doclength;
}

#endif /* OM_HGUARD_ORPOSTLIST_H */
