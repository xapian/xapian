/* weightcutoffpostlist.cc: Part of a postlist with a score greater than cutoff
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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

#include <config.h>
#include "weightcutoffpostlist.h"
#include "branchpostlist.h"
#include "database.h"
#include "utils.h"

WeightCutoffPostList::WeightCutoffPostList(PostList * pl_,
					   Xapian::weight cutoff_,
					   MultiMatch * matcher_)
	: pl(pl_), cutoff(cutoff_), matcher(matcher_)
{
    DEBUGCALL(MATCH, void, "WeightCutoffPostList", pl_ << ", " << cutoff_ << ", " << matcher_);
}

PostList *
WeightCutoffPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "WeightCutoffPostList::next", w_min);
    if (w_min < cutoff) w_min = cutoff;
    do {
	(void) next_handling_prune(pl, w_min, matcher);
    } while ((!pl->at_end()) && pl->get_weight() < w_min);
    RETURN(NULL);
}

PostList *
WeightCutoffPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "WeightCutoffPostList::skip_to", did << ", " << w_min);
    if (w_min < cutoff) w_min = cutoff;
    while (true) {
	// skip_to guarantees skipping to at least docid did, but not that
	// it moves forward past did, or that the weight parameter is taken
	// into account.
	(void) skip_to_handling_prune(pl, did, w_min, matcher);

	if (pl->at_end() || pl->get_weight() >= w_min) RETURN(NULL);
	did = pl->get_docid() + 1;
    }
}

inline
WeightCutoffPostList::~WeightCutoffPostList()
{
    if (pl) delete pl;
}

Xapian::doccount
WeightCutoffPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "WeightCutoffPostList::get_termfreq_max", "");
    if (cutoff > pl->get_maxweight()) return 0;
    return pl->get_termfreq_max();
}

Xapian::doccount
WeightCutoffPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "WeightCutoffPostList::get_termfreq_min", "");
    if (cutoff == 0) return pl->get_termfreq_min();
    return 0u;
}

Xapian::doccount
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
Xapian::weight
WeightCutoffPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::get_maxweight", "");
    return pl->get_maxweight();
}

Xapian::docid
WeightCutoffPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "WeightCutoffPostList::get_docid", "");
    return pl->get_docid();
}

Xapian::weight
WeightCutoffPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::get_weight", "");
    return pl->get_weight();
}

Xapian::weight
WeightCutoffPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "WeightCutoffPostList::recalc_maxweight", "");
    return pl->recalc_maxweight();
}

bool
WeightCutoffPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "WeightCutoffPostList::at_end", "");
    return pl->at_end();
}

std::string
WeightCutoffPostList::get_description() const
{
    return "(WeightCutoff " + om_tostring(cutoff) + " " +
	    pl->get_description() + " )";
}

PositionList *
WeightCutoffPostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "WeightCutoffPostList::read_position_list", "");
    return pl->read_position_list();
}

PositionList *
WeightCutoffPostList::open_position_list() const
{
    return pl->open_position_list();
}

Xapian::doclength
WeightCutoffPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "WeightCutoffPostList::get_doclength", "");
    return pl->get_doclength();
}
