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

NearPostList::NearPostList(PostList *source_, om_termpos window_,
			   vector<PostList *> terms_)
{
    source = source_;
    window = window_;
    terms = terms_;
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
    // FIXME: now sort plists...
    while (1) {
#if 0 // FIXME: recode...
        lposlist->next();
        if (lposlist->at_end()) break;
        om_termpos lpos = lposlist->get_position();
        om_termpos lthresh;
        if (order_matters)
	    lthresh = lpos + 1;
        else
	    lthresh = lpos - max_separation;
        rposlist->skip_to(lthresh);
        if (rposlist->at_end()) break;
        // FIXME: if terms are the same should we reject rpos == lpos???
        if (rposlist->get_position() <= lpos + max_separation) return true;
#endif
    }
    return false;
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
