/* extraweightpostlist.h: add on extra weight contribution
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_EXTRAWEIGHTPOSTLIST_H
#define OM_HGUARD_EXTRAWEIGHTPOSTLIST_H

/// A postlist which adds on an extra weight contribution
class ExtraWeightPostList : public PostList {
    private:
	PostList * pl;
	IRWeight * wt;
	om_weight max_weight;

    public:
	om_doccount get_termfreq() const { return pl->get_termfreq(); }

	om_docid  get_docid() const { return pl->get_docid(); }

	om_weight get_weight() const {
// FIXME don't have db here... DEBUGLINE(MATCH, "db.get_doclength(" << did << ") == " << db.get_doclength(did));
	    DEBUGLINE(MATCH, "pl->get_doclength() == " << pl->get_doclength());
// FIXME don;t have db here... AssertEqDouble(db.get_doclength(did), pl->get_doclength());
	    return pl->get_weight() + wt->get_sumextra(pl->get_doclength());
	}

	const OmKey * get_collapse_key() const {
	    return pl->get_collapse_key();
	}

	om_weight get_maxweight() const {
	    return pl->get_maxweight() + max_weight;
	}

        om_weight recalc_maxweight() {
	    return pl->recalc_maxweight() + max_weight;
	}

	PostList *next(om_weight w_min) {
	    return pl->next(w_min - max_weight);
	}
	    
	PostList *skip_to(om_docid did, om_weight w_min) {
	    return pl->skip_to(did, w_min - max_weight);
	}

	bool at_end() const { return pl->at_end(); }

	std::string get_description() const {
	    return "( ExtraWeight " + pl->get_description() + " )";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const {
	    return pl->get_doclength();
	}

	virtual PositionList * get_position_list() {
	    return pl->get_position_list();
	}

        ExtraWeightPostList(PostList * pl_, IRWeight *wt_)
	    : pl(pl_), wt(wt_), max_weight(wt->get_maxextra())
	{ }

	~ExtraWeightPostList() {
	    delete pl;
	    delete wt;
	}
};

#endif /* OM_HGUARD_EXTRAWEIGHTPOSTLIST_H */
