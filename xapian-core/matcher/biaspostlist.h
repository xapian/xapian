/* biaspostlist.h: add on extra weight based on functor
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_BIASPOSTLIST_H
#define OM_HGUARD_BIASPOSTLIST_H

#include <time.h>
#include <math.h>
#include <xapian/database.h>
#include "postlist.h"
#include "multimatch.h"

// DOCID_BASED doesn't require a key lookup so is more efficient
// and works if documents are added at a reasonably steady rate
//
// FIXME: this stuff need extracting so the user can write their
// own by subclassing a virtual base class...

class OmBiasFunctor {
    private:
#ifndef DOCID_BASED
	time_t now;
	Xapian::Database db;
#else /* DOCID_BASED */
	Xapian::docid max_id;
#endif /* DOCID_BASED */
	Xapian::weight max_w;
	double K; // factor in exponential decay
    public:
	OmBiasFunctor(const Xapian::Database &db_, Xapian::weight max_w_, double halflife)
#ifndef DOCID_BASED
	    : now(time(NULL)), db(db_), max_w(max_w_),
	      K(log(0.5) / fabs(halflife)) 
#else /* DOCID_BASED */
	    : max_id(db_.get_doccount()), max_w(max_w_)
#endif /* DOCID_BASED */
	    {}

	Xapian::weight get_maxweight() {
	    return max_w; 
	}

	Xapian::weight get_weight(Xapian::docid id) {
#ifndef DOCID_BASED
	    string value = db.get_document(id).get_value(0);
	    time_t t = atoi(value);
	    if (t >= now) return max_w;
#else /* DOCID_BASED */
	    if (id >= max_id) return max_w;
#endif /* DOCID_BASED */
	    // the same story but 2 days old gets half the extra weight
#ifndef DOCID_BASED
	    return max_w * exp(K * (now - t));
#else /* DOCID_BASED */
	    return max_w * exp(K * (max_id - id));
#endif /* DOCID_BASED */
	}
};

/// A postlist which adds on an extra weight contribution from a functor
class BiasPostList : public PostList {
    private:
        // Prevent copying
        BiasPostList(const BiasPostList &);
        BiasPostList & operator=(const BiasPostList &);

	PostList *pl;
	Xapian::Database db;
	OmBiasFunctor *bias;
	MultiMatch *matcher;
	Xapian::weight max_bias;
	Xapian::weight w;

    public:
	Xapian::doccount get_termfreq_max() const { return pl->get_termfreq_max(); }
	Xapian::doccount get_termfreq_min() const { return pl->get_termfreq_min(); }
	Xapian::doccount get_termfreq_est() const { return pl->get_termfreq_est(); }

	Xapian::docid get_docid() const { return pl->get_docid(); }

	Xapian::weight get_weight() const {
	    return w + bias->get_weight(pl->get_docid());
	}

	Xapian::weight get_maxweight() const {
	    return pl->get_maxweight() + max_bias;
	}

        Xapian::weight recalc_maxweight() {
	    return pl->recalc_maxweight() + max_bias;
	}

	PostList *next(Xapian::weight w_min) {
	    do {
		PostList *p = pl->next(w_min - max_bias);
		if (p) {
		    delete pl;
		    pl = p;
		    if (matcher) matcher->recalc_maxweight();
		}
		if (pl->at_end()) break;
		w = pl->get_weight();
	    } while (w + max_bias < w_min);
	    return NULL;
	}
	    
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min) {
	    do {
		PostList *p = pl->skip_to(did, w_min - max_bias);
		if (p) {
		    delete pl;
		    pl = p;
		    if (matcher) matcher->recalc_maxweight();
		}
		if (pl->at_end()) break;
		w = pl->get_weight();
	    } while (w + max_bias < w_min);
	    return NULL;
	}

	bool at_end() const { return pl->at_end(); }

	string get_description() const {
	    return "( Bias " + pl->get_description() + " )";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const {
	    return pl->get_doclength();
	}

	virtual PositionList * read_position_list() {
	    return pl->read_position_list();
	}

	virtual PositionList * open_position_list() const {
	    return pl->open_position_list();
	}

	BiasPostList(PostList *pl_, const Xapian::Database &db_, OmBiasFunctor *bias_,
		     MultiMatch *matcher_)
		: pl(pl_), db(db_), bias(bias_), matcher(matcher_),
		  max_bias(bias->get_maxweight())
	{ }

	~BiasPostList() {
	    delete pl;
	    delete bias;
	}
};

#endif /* OM_HGUARD_BIASPOSTLIST_H */
