/* xorpostlist.h: XOR of two posting lists
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

#ifndef OM_HGUARD_XORPOSTLIST_H
#define OM_HGUARD_XORPOSTLIST_H

#include "database.h"
#include "branchpostlist.h"

/** A postlist comprising two postlists XORed together.
 *
 *  This postlist returns a posting if and only if it is in one, and only
 *  one, of the sub-postlists.  The weight for a posting is the weight of
 *  the sub-posting.
 *
 *  This postlist is only known to be useful as a "decay product" of
 *  other postlists.  If you find an independent meaningful use, let us
 *  know...
 */
class XorPostList : public BranchPostList {
    private:
        Xapian::docid lhead, rhead;
        Xapian::weight lmax, rmax, minmax;

	Xapian::doccount dbsize;

        PostList *advance_to_next_match(Xapian::weight w_min);
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

        XorPostList(PostList * left_,
		    PostList * right_,
		    MultiMatch * matcher_,
		    Xapian::doccount dbsize_);
};

inline Xapian::doccount
XorPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_max", "");
    return l->get_termfreq_max() + r->get_termfreq_max();
}

inline Xapian::doccount
XorPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_min", "");
    // Min = freq_min(a or b) - freq_max(a and b)
    //     = max(a_min, b_min) - min(a_max, b_max)
    //     = min(b_min - a_max, a_min - b_max)
    Xapian::doccount r_min = r->get_termfreq_min();
    Xapian::doccount l_min = l->get_termfreq_min();
    Xapian::doccount r_max = r->get_termfreq_max();
    Xapian::doccount l_max = l->get_termfreq_max();
    Xapian::doccount termfreq_min = 0u;
    if (r_min > l_max)
	termfreq_min = r_min - l_max;
    if (l_min > r_max && (l_min - r_max) > termfreq_min)
	termfreq_min = l_min - r_max;

    return termfreq_min;
}

inline Xapian::doccount
XorPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l xor r) = P(l) + P(r) - 2 . P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    return static_cast<Xapian::doccount> (lest + rest - 2.0 * lest * rest / dbsize);
}

inline Xapian::docid
XorPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "XorPostList::get_docid", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    return std::min(lhead, rhead);
}

// only called if we are doing a probabilistic XOR
inline Xapian::weight
XorPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::get_weight", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    Assert(lhead > rhead);
    return r->get_weight();
}

// only called if we are doing a probabilistic operation
inline Xapian::weight
XorPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::get_maxweight", "");
    return std::max(lmax, rmax);
}

inline Xapian::weight
XorPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = std::min(lmax, rmax);
    return XorPostList::get_maxweight();
}

inline bool
XorPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "XorPostList::at_end", "");
    return lhead == 0;
}

inline std::string
XorPostList::get_description() const
{
    return "(" + l->get_description() + " Xor " + r->get_description() + ")";
}

inline Xapian::doclength
XorPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "XorPostList::get_doclength", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_doclength();
    Assert(lhead > rhead);
    return r->get_doclength();
}

#endif /* OM_HGUARD_XORPOSTLIST_H */
