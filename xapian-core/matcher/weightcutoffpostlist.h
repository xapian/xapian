/* weightcutoffpostlist.h: One postlist, with a cutoff at a specified weight
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

#ifndef OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H
#define OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H

#include "database.h"
#include "postlist.h"
#include "utils.h"
class MultiMatch;

/** A postlist consisting of all the entries in a postlist with a weight
 *  higher than a given cutoff.
 */
class WeightCutoffPostList : public PostList {
    private:
	PostList *pl;
	Xapian::weight cutoff;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
	MultiMatch *matcher;

    public:
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::weight get_maxweight() const;
	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;

        Xapian::weight recalc_maxweight();

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	PositionList *read_position_list();
	PositionList * open_position_list() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const;

        WeightCutoffPostList(PostList * pl_,
			     Xapian::weight cutoff_,
			     MultiMatch * matcher_);

        ~WeightCutoffPostList();
};

inline
WeightCutoffPostList::~WeightCutoffPostList()
{
    if (pl) delete pl;
}

inline Xapian::doccount
WeightCutoffPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "WeightCutoffPostList::get_termfreq_max", "");
    if (cutoff > pl->get_maxweight()) return 0;
    return pl->get_termfreq_max();
}

inline Xapian::doccount
WeightCutoffPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "WeightCutoffPostList::get_termfreq_min", "");
    if (cutoff == 0) return pl->get_termfreq_min();
    return 0u;
}

inline Xapian::doccount
WeightCutoffPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "WeightCutoffPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l xor r) = P(l) + P(r) - 2 . P(l) . P(r)
    double est = static_cast<double>(pl->get_termfreq_est());
    double maxwt = pl->get_maxweight();
    if (maxwt == 0) {
	if (cutoff > 0) est = 0;
    } else {
	double wtfrac = (maxwt - cutoff) / maxwt;
	if (wtfrac < 0.0) wtfrac = 0.0;
	if (wtfrac > 1.0) wtfrac = 1.0;
	Assert(wtfrac <= 1.0);
	est *= wtfrac;
    }

    return static_cast<Xapian::doccount> (est);
}

// only called if we are doing a probabilistic operation
inline Xapian::weight
WeightCutoffPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::get_maxweight", "");
    return pl->get_maxweight();
}

inline Xapian::docid
WeightCutoffPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "WeightCutoffPostList::get_docid", "");
    return pl->get_docid();
}

inline Xapian::weight
WeightCutoffPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::get_weight", "");
    return pl->get_weight();
}

inline Xapian::weight
WeightCutoffPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::recalc_maxweight", "");
    return pl->recalc_maxweight();
}

inline bool
WeightCutoffPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "WeightCutoffPostList::at_end", "");
    return pl->at_end();
}

inline std::string
WeightCutoffPostList::get_description() const
{
    return "(WeightCutoff " + om_tostring(cutoff) + " " +
	    pl->get_description() + " )";
}

inline PositionList *
WeightCutoffPostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "WeightCutoffPostList::read_position_list", "");
    return pl->read_position_list();
}

inline PositionList *
WeightCutoffPostList::open_position_list() const
{
    return pl->open_position_list();
}

inline Xapian::doclength
WeightCutoffPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "WeightCutoffPostList::get_doclength", "");
    return pl->get_doclength();
}

#endif /* OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H */
