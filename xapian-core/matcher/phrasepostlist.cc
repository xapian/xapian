/* phrasepostlist.cc: Return only items where terms are near or form a phrase
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
#include "phrasepostlist.h"
#include "positionlist.h"

#include <algorithm>

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PositionListCmpLt {
    public:
	/** Return true if and only if a is strictly shorter than b.
	 */
        bool operator()(const PositionList *a, const PositionList *b) {
            return a->get_size() < b->get_size();
        }
};


/** Check if terms occur sufficiently close together in the current doc
 */
bool
NearPostList::test_doc()
{
    DEBUGCALL(MATCH, bool, "NearPostList::test_doc", "");
    std::vector<PositionList *> plists;

    std::vector<PostList *>::iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	plists.push_back((*i)->read_position_list());
    }

    std::sort(plists.begin(), plists.end(), PositionListCmpLt());

    om_termpos pos;
    do {
	plists[0]->next();
	if (plists[0]->at_end()) RETURN(false);
	pos = plists[0]->get_position();
    } while (!do_test(plists, 1, pos, pos));

    RETURN(true);
}

bool
NearPostList::do_test(std::vector<PositionList *> &plists, om_termcount i,
		      om_termcount min, om_termcount max)
{
    DEBUGCALL(MATCH, bool, "NearPostList::do_test", "[plists], " << i << ", " << min << ", " << max);
    DEBUGLINE(MATCH, "docid = " << get_docid() << ", window = " << window);
    om_termcount tmp = max + 1;
    // take care to avoid underflow
    if (window <= tmp) tmp -= window; else tmp = 0;
    plists[i]->skip_to(tmp);
    while (!plists[i]->at_end()) {
	om_termpos pos = plists[i]->get_position();
	DEBUGLINE(MATCH, "[" << i << "]: " << max - window + 1 << " " << min
		  << " " << pos << " " << max << " " << min + window - 1);
	if (pos > min + window - 1) RETURN(false);
	if (i + 1 == plists.size()) RETURN(true);
	if (pos < min) min = pos;
	else if (pos > max) max = pos;
	if (do_test(plists, i + 1, min, max)) RETURN(true);
	plists[i]->next();
    }
    RETURN(false);
}



/** Check if terms form a phrase in the current doc
 */
bool
PhrasePostList::test_doc()
{
    DEBUGCALL(MATCH, bool, "PhrasePostList::test_doc", "");
    std::vector<PositionList *> plists;

    std::vector<PostList *>::iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	PositionList *p = (*i)->read_position_list();
	p->index = i - terms.begin();
	plists.push_back(p);
    }

    std::sort(plists.begin(), plists.end(), PositionListCmpLt());

    om_termpos pos;
    om_termpos idx, min;
    do {
	plists[0]->next();
	if (plists[0]->at_end()) {
	    DEBUGLINE(MATCH, "--MISS--");
	    RETURN(false);
	}
	pos = plists[0]->get_position();
	idx = plists[0]->index;
	min = pos + plists.size() - idx;
	if (min > window) min -= window; else min = 0;
    } while (!do_test(plists, 1, min, pos + window - idx));
    DEBUGLINE(MATCH, "**HIT**");
    RETURN(true);
}

bool
PhrasePostList::do_test(std::vector<PositionList *> &plists, om_termcount i,
			om_termcount min, om_termcount max)
{
    DEBUGCALL(MATCH, bool, "PhrasePostList::do_test", "[plists],  " << i << ", " << min << ", " << max);
    DEBUGLINE(MATCH, "docid = " << get_docid() << ", window = " << window);
    om_termpos idxi = plists[i]->index;
    DEBUGLINE(MATCH, "my idx in phrase is " << idxi);

    om_termpos mymin = min + idxi;
    om_termpos mymax = max - plists.size() + idxi;
    DEBUGLINE(MATCH, "MIN = " << mymin << " MAX = " << mymax);
    // FIXME: this is worst case O(n^2) where n = length of phrase
    // Can we do better?
    for (om_termcount j = 0; j < i; j++) {
	om_termpos idxj = plists[j]->index;
	if (idxj > idxi) {
	    om_termpos tmp = plists[j]->get_position() + idxj - idxi;
	    DEBUGLINE(MATCH, "ABOVE " << tmp);
	    if (tmp < mymax) mymax = tmp;
	} else {
	    Assert(idxi != idxj);
	    om_termpos tmp = plists[j]->get_position() + idxi - idxj;
	    DEBUGLINE(MATCH, "BELOW " << tmp);
	    if (tmp > mymin) mymin = tmp;
	}
	DEBUGLINE(MATCH, "min = " << mymin << " max = " << mymax);
    }
    plists[i]->skip_to(mymin);

    while (!plists[i]->at_end()) {
	om_termpos pos = plists[i]->get_position();
	DEBUGLINE(MATCH, " " << mymin << " " << pos << " " << mymax);
	if (pos > mymax) RETURN(false);
	if (i + 1 == plists.size()) RETURN(true);
	om_termpos tmp = pos + window - idxi;
	if (tmp < max) max = tmp;
	tmp = pos + plists.size() - idxi;
	if (tmp > window) {
	    tmp -= window;
	    if (tmp > min) min = tmp;
	}
	if (do_test(plists, i + 1, min, max)) RETURN(true);
	plists[i]->next();
    }
    RETURN(false);
}
