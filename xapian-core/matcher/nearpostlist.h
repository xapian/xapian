/* nearpostlist.h: Return only items where the terms are near each other
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

#ifndef OM_HGUARD_NEARPOSTLIST_H
#define OM_HGUARD_NEARPOSTLIST_H

#include "database.h"
#include "andpostlist.h"
#include "omdebug.h"

class PosListBuffer;

/** A postlist comprising several postlists NEARed together.
 *
 *  This postlist returns a posting if and only if it is in all of the
 *  sub-postlists and all the terms occur within a specified distance of
 *  each other somewhere in the document.  The weight for a posting is the
 *  sum of the weights of the sub-postings.
 */
class NearPostList : public PostList {
    private:
        om_termpos window;
    	PostList *source; // Source of candidate documents
	vector<PostList *> terms;

        inline bool do_near(vector<PosListBuffer> &plists, om_termcount i,
			    om_termcount pos);
    	inline bool terms_near();
    public:
	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);

	// pass all these through to the underlying source PostList
	om_doccount get_termfreq() const { return source->get_termfreq(); }
	om_weight get_maxweight() const { return source->get_maxweight(); }
	om_docid get_docid() const { return source->get_docid(); }
	om_weight get_weight() const { return source->get_weight(); }
	om_doclength get_doclength() const { return source->get_doclength(); }
	om_weight recalc_maxweight() { return source->recalc_maxweight(); }
	PositionList * get_position_list() { return source->get_position_list(); }
	bool at_end() const { return source->at_end(); }

	string intro_term_description() const;

        NearPostList(PostList *source_, om_termpos window_,
		     vector<PostList *> terms_);
        ~NearPostList() { delete source; }
};

inline string
NearPostList::intro_term_description() const
{
    // FIXME: include window in desc?
    return "(Near " + source->intro_term_description() + ")";
}

#endif /* OM_HGUARD_NEARPOSTLIST_H */
