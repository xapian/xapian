/* nearpostlist.cc: Return only items where the terms are near each other
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

#include "nearpostlist.h"

#include <algorithm>

class PosListBuffer {
    private:
	PositionList *pl;
	om_termpos cache;
    public:
	PosListBuffer(PositionList *poslist) : pl(poslist) {
	    cache = 0;
	}
	om_termcount get_size() const { return pl->get_size(); }
	om_termpos get_position() const {
	    if (cache != 0) return cache;
	    return pl->get_position();
	}
	void next() {
	    if (cache != 0) {
		cache = 0;
		return;
	    }
	    pl->next();
	}
	void skip_to(om_termpos termpos) {
	    if (termpos < 1) termpos = 1;
	    if (cache != 0) {
		if (termpos <= cache) return;
		cache = 0;
	    }
	    pl->skip_to(termpos);
	}
	void pushback(om_termpos termpos) {
	    Assert(cache == 0);
	    cache = termpos;
	}
	bool at_end() const {
	    return cache == 0 && pl->at_end();
	}
};

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PosListBufferCmpLt {
    public:
	/** Return true if and only if a is strictly shorter than b.
	 */
        bool operator()(const PosListBuffer &a, const PosListBuffer &b) {
            return a.get_size() < b.get_size();
        }
};

NearPostList::NearPostList(PostList *source_, om_termpos window_,
			   vector<PostList *> terms_)
{
    source = source_;
    window = window_;
    terms = terms_;
}

inline bool NearPostList::do_near(vector<PosListBuffer> &plists,
				  om_termcount i, om_termcount pos)
{
    plists[i].skip_to(pos - window + i);
    while (1) {
	om_termcount min = pos;
	om_termcount max = pos;
	pos = plists[i].get_position();
	for (om_termcount j = 0; j < i; j++) {
	    om_termcount tmp = plists[j].get_position();
	    if (tmp < min) min = tmp;
	    if (tmp > max) max = tmp;
	}
	if (pos > min + window - i) {
	    plists[i].pushback(pos);
	    return false;
	}
	if (i + 1 == plists.size() || do_near(plists, i + 1, pos)) {
	    return true;
	}
	plists[i].next();
    }
}

inline bool
NearPostList::terms_near()
{
    // check if NEAR criterion is satisfied
    vector<PosListBuffer> plists;
    vector<PostList *>::iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	plists.push_back(PosListBuffer((*i)->get_position_list()));
    }
    sort(plists.begin(), plists.end(), PosListBufferCmpLt());
     
    while (1) {
	plists[0].next();
	if (plists[0].at_end()) return false;	
	if (do_near(plists, 1, plists[0].get_position())) return true;
    }
}

PostList *
NearPostList::next(om_weight w_min)
{
    do {
        source->next(w_min);
    } while (!source->at_end() && !terms_near());
    return NULL;
}

PostList *
NearPostList::skip_to(om_docid did, om_weight w_min)
{
    if (did > get_docid()) {
	source->skip_to(did, w_min);
        if (!source->at_end() && !terms_near()) this->next(w_min);
    }
    return NULL;
}
