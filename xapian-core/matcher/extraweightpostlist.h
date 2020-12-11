/** @file
 * @brief add on extra weight contribution
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
 * Copyright 2003,2004,2007,2009,2011,2014 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_EXTRAWEIGHTPOSTLIST_H
#define OM_HGUARD_EXTRAWEIGHTPOSTLIST_H

#include "multimatch.h"
#include "omassert.h"

namespace Xapian {
    class Weight;
}

/// A postlist which adds on an extra weight contribution
class ExtraWeightPostList : public PostList {
    private:
	PostList * pl;
	Xapian::Weight * wt;
	MultiMatch * matcher;
	double max_weight;

    public:
	Xapian::doccount get_termfreq_max() const {
	    return pl->get_termfreq_max();
	}
	Xapian::doccount get_termfreq_min() const {
	    return pl->get_termfreq_min();
	}
	Xapian::doccount get_termfreq_est() const {
	    return pl->get_termfreq_est();
	}

	Xapian::docid get_docid() const { return pl->get_docid(); }

	double get_weight() const {
	    /* Second parameter of get_sumextra is number of unique terms in doc, which has been put
	     * to maintain consistency with get_sumpart, As of now none of weighting scheme is using
	     * it. Current 0 is being passed, change it to pl->get_unique_terms() in case you
	     * need access uniq_terms. */
	    double sumextra = wt->get_sumextra(pl->get_doclength(), 0);
	    AssertRel(sumextra, <=, max_weight);
	    return pl->get_weight() + sumextra;
	}

	double get_maxweight() const {
	    return pl->get_maxweight() + max_weight;
	}

	double recalc_maxweight() {
	    return pl->recalc_maxweight() + max_weight;
	}

	PostList *next(double w_min) {
	    PostList *p = pl->next(w_min - max_weight);
	    if (p) {
		delete pl;
		pl = p;
		if (matcher) matcher->recalc_maxweight();
	    }
	    return NULL;
	}

	PostList *skip_to(Xapian::docid did, double w_min) {
	    PostList *p = pl->skip_to(did, w_min - max_weight);
	    if (p) {
		delete pl;
		pl = p;
		if (matcher) matcher->recalc_maxweight();
	    }
	    return NULL;
	}

	bool at_end() const { return pl->at_end(); }

	std::string get_description() const {
	    return "( ExtraWeight " + pl->get_description() + " )";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::termcount get_doclength() const {
	    return pl->get_doclength();
	}

	virtual Xapian::termcount get_unique_terms() const {
	    return pl->get_unique_terms();
	}

	ExtraWeightPostList(PostList * pl_, Xapian::Weight *wt_,
			    MultiMatch *matcher_)
	    : pl(pl_), wt(wt_), matcher(matcher_),
	      max_weight(wt->get_maxextra())
	{ }

	~ExtraWeightPostList() {
	    delete pl;
	    delete wt;
	}

	Xapian::termcount count_matching_subqs() const {
	    return pl->count_matching_subqs();
	}
};

#endif /* OM_HGUARD_EXTRAWEIGHTPOSTLIST_H */
