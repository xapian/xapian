/* weightcutoffpostlist.h: One postlist, with a cutoff at a specified weight
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
	om_weight cutoff;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
	MultiMatch *matcher;

    public:
	om_doccount get_termfreq_max() const;
	om_doccount get_termfreq_min() const;
	om_doccount get_termfreq_est() const;

	om_weight get_maxweight() const;
	om_docid  get_docid() const;
	om_weight get_weight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	PositionList *read_position_list();
	AutoPtr<PositionList> open_position_list() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

        WeightCutoffPostList(PostList * pl_,
			     om_weight cutoff_,
			     MultiMatch * matcher_);

        ~WeightCutoffPostList();
};

inline
WeightCutoffPostList::~WeightCutoffPostList()
{
    if (pl) delete pl;
}

inline om_doccount
WeightCutoffPostList::get_termfreq_max() const
{
    if (cutoff > pl->get_maxweight()) return 0;
    return pl->get_termfreq_max();
}

inline om_doccount
WeightCutoffPostList::get_termfreq_min() const
{
    if (cutoff == 0) return pl->get_termfreq_min();
    return 0u;
}

inline om_doccount
WeightCutoffPostList::get_termfreq_est() const
{
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

    return static_cast<om_doccount> (est);
}

// only called if we are doing a probabilistic operation
inline om_weight
WeightCutoffPostList::get_maxweight() const
{
    return pl->get_maxweight();
}

inline om_docid
WeightCutoffPostList::get_docid() const
{
    return pl->get_docid();
}

inline om_weight
WeightCutoffPostList::get_weight() const
{
    return pl->get_weight();
}

inline om_weight
WeightCutoffPostList::recalc_maxweight()
{
    return pl->recalc_maxweight();
}

inline bool
WeightCutoffPostList::at_end() const
{
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
    return pl->read_position_list();
}

inline AutoPtr<PositionList>
WeightCutoffPostList::open_position_list() const
{
    return pl->open_position_list();
}

inline om_doclength
WeightCutoffPostList::get_doclength() const
{
    return pl->get_doclength();
}

#endif /* OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H */
