/* weightcutoffpostlist.h: One postlist, with a cutoff at a specified weight
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

#ifndef OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H
#define OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H

#include "postlist.h"

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

#endif /* OM_HGUARD_WEIGHTCUTOFFPOSTLIST_H */
