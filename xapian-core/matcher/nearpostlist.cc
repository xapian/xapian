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

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PosListCmpLt {
    public:
	/** Return true if and only if a is strictly shorter than b.
	 */
        bool operator()(const PositionList *a, const PositionList *b) {
            return a->get_size() < b->get_size();
        }
};

NearPostList::NearPostList(PostList *source_, om_termpos window_,
			   vector<PostList *> terms_)
{
    source = source_;
    window = window_;
    terms = terms_;
}

// FIXME: need to push back last pos before we back-up
inline bool NearPostList::do_near(vector<PositionList *> &plists,
				  om_termcount i, om_termcount max)
{
    plists[i]->skip_to(max - window + i);
    while (1) {
	om_termcount pos = plists[i]->get_position();
	om_termcount j;
	om_termcount min = max;
	for (j = 0; j < i; j++) {
	    om_termcount tmp = plists[j]->get_position();
	    if (tmp < min) min = tmp;
	}
	if (pos > min + window - i) return false;
	if (pos > max) max = pos;
	if (do_near(plists, i + 1, max)) return true;
	plists[i]->next();
    }
}

inline bool
NearPostList::terms_near()
{
    // check if NEAR criterion is satisfied
    vector<PositionList *> plists;
    vector<PostList *>::iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	plists.push_back((*i)->get_position_list());
    }
    sort(plists.begin(), plists.end(), PosListCmpLt());
    Assert((*plists.begin())->get_size() <= (*plists.end())->get_size());
    plists[0]->next();
    om_termcount pos = plists[0]->get_position();
    return do_near(plists, 1, pos); 
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
