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
	DebugMsg((*i)->intro_term_description() << " " << i - terms.begin()
		 << endl);
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
    DebugMsg("NearPostList::do_test([...], " << i << ", " << min << ", "
	     << max << ")\ndocid = " << get_docid() << ", window = "
	     << window << endl);
    om_termcount tmp = max + 1;
    // take care to avoid underflow
    if (window <= tmp) tmp -= window; else tmp = 0;
    plists[i].skip_to(tmp);
    while (!plists[i].at_end()) {
	om_termpos pos = plists[i].get_position();
	DebugMsg("[" << i << "]: " << max - window + 1 << " " << min << " "
		 << pos << " " << max << " " << min + window - 1 << endl);
	if (pos > min + window - 1) {
	    plists[i].pushback(pos);
	    return false;
	}
	if (i + 1 == plists.size()) return true; 
	if (pos < min) min = pos;
	else if (pos > max) max = pos;
	if (do_test(plists, i + 1, min, max)) return true;
	plists[i].next();
    }
    return false;
}


bool
PhrasePostList::do_test(vector<PosListBuffer> &plists, om_termcount i,
			om_termcount min, om_termcount max)
{
    DebugMsg("PhrasePostList::do_test([...], " << i << ", " << min << ", "
	     << max << ")\ndocid = " << get_docid() << ", window = "
	     << window << endl);
    // max passes pos, min unused as parameter
    om_termpos idxi = plists[i].index;
    DebugMsg("my idx in phrase is " << idxi << endl);
    min = max + 1;
    // take care to avoid underflow
    if (window <= min) min -= window; else min = 0;
    max = max + window - 1;
    DebugMsg("MIN = " << min << " MAX = " << max << endl);
    // FIXME: this is worst case O(n^2) where n = length of phrase
    // We should be able to do better...
    for (om_termcount j = 0; j < i; j++) {
	om_termpos idxj = plists[j].index;
	if (idxj > idxi) {
	    om_termpos tmp = plists[j].get_position() + idxj - idxi;
	    DebugMsg("ABOVE " << tmp << endl);
	    if (tmp < max) max = tmp;
	} else {
	    Assert(idxi != idxj);
	    om_termpos tmp = plists[j].get_position() + idxi - idxj;
	    DebugMsg("BELOW " << tmp << endl);
	    if (tmp > min) min = tmp;
	}
	DebugMsg("min = " << min << " max = " << max << endl);
    }
    plists[i].skip_to(min);
    while (!plists[i].at_end()) {
	om_termpos pos = plists[i].get_position();
	DebugMsg(" " << min << " " << pos << " " << max << endl);
	if (pos > max) {	    
	    plists[i].pushback(pos);
	    return false;
	}
	if (i + 1 == plists.size()) return true;
	if (do_test(plists, i + 1, 0, pos)) return true;
	plists[i].next();
    }
    return false;
}
