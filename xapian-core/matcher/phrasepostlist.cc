/* phrasepostlist.cc: Return only items where terms are near or form a phrase
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

#include "phrasepostlist.h"

#include <algorithm>

class PosListBuffer {
    private:
	PositionList *pl;
	om_termpos cache;
    public:
	om_termpos index; // only needed to phrase
	PosListBuffer(PositionList *poslist, om_termpos index_) : pl(poslist) {
	    cache = 0;
	    index = index_;
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

bool
NearOrPhrasePostList::test_doc()
{
    // check if criterion is satisfied
    vector<PosListBuffer> plists;

    vector<PostList *>::iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	plists.push_back(PosListBuffer((*i)->get_position_list(),
				       i - terms.begin()));
    }

    sort(plists.begin(), plists.end(), PosListBufferCmpLt());
     
    om_termpos pos;
    do {
	plists[0].next();
	if (plists[0].at_end()) return false;
	pos = plists[0].get_position();
    } while (!do_test(plists, 1, pos, pos));

    return true;
}


bool
NearPostList::do_test(vector<PosListBuffer> &plists, om_termcount i,
		      om_termcount min, om_termcount max)
{
    plists[i].skip_to(max - window + i);
    while (1) {
	om_termpos pos = plists[i].get_position();
	if (pos < min) {
	    min = pos;
	} else if (pos > min + window - i) {	    
	    plists[i].pushback(pos);
	    return false;
	}
	if (i + 1 == plists.size()) return true;
	if (pos > max) max = pos;
	if (do_test(plists, i + 1, min, max)) return true;
	plists[i].next();
    }
}


bool
PhrasePostList::do_test(vector<PosListBuffer> &plists, om_termcount i,
			om_termcount min, om_termcount max)
{
    // max passes pos, min unused as parameter
    om_termpos idxi = plists[i].index;
    min = max + plists[i].get_size() - 1 - i - window;
    max = max - i + window;
    for (int j = 0; j < i; j++) {
	om_termpos idxj = plists[j].index;
	if (idxj > idxi) {
	    om_termpos tmp = plists[j].get_position() - idxj + idxi;
	    if (tmp < max) max = tmp;
	} else {
	    Assert(idxi != idxj);
	    om_termpos tmp = plists[j].get_position() + idxj - idxi;
	    if (tmp > min) min = tmp;
	}
    }
    plists[i].skip_to(min);
    while (1) {
	om_termpos pos = plists[i].get_position();
	if (pos > max) {	    
	    plists[i].pushback(pos);
	    return false;
	}
	if (i + 1 == plists.size()) return true;
	if (do_test(plists, i + 1, 0, pos)) return true;
	plists[i].next();
    }
}
