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

/** A postlist comprising two postlists NEARed together.
 *
 *  This postlist returns a posting if and only if it is in both of the
 *  sub-postlists and the terms are near each other.  The weight for a
 *  posting is the sum of the weights of the sub-postings.
 */
class NearPostList : public AndPostList {
    private:
        om_termpos max_separation;
        bool order_matters;

	inline bool terms_near();
    public:
	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);

	string intro_term_description() const;

        NearPostList(PostList *left,
		     PostList *right,
		     LocalMatch *matcher_,
		     om_termpos max_separation,
		     bool order_matters = false);
};

inline string
NearPostList::intro_term_description() const
{
    // FIXME: include max_separation and order_matters in desc?
    return "(" + l->intro_term_description() + " Near " +
	    r->intro_term_description() + ")";
}

#endif /* OM_HGUARD_NEARPOSTLIST_H */
