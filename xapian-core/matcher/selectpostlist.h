/* selectpostlist.h: Parent class for classes which only return selected docs
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

#ifndef OM_HGUARD_SELECTPOSTLIST_H
#define OM_HGUARD_SELECTPOSTLIST_H

#include "database.h"
#include "postlist.h"
#include "omdebug.h"

/** A postlist parent class for classes which only return selected docs
 *  from a source postlist (e.g. NEAR and PHRASE)
 */
class SelectPostList : public PostList {
    protected:
	PostList *source;

	/** Subclasses should override test_doc() with a method which returns
	 *  true if a document meets the appropriate criterion, false in not
	 */
    	virtual bool test_doc() = 0;
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

	std::string get_description() const;

	SelectPostList(PostList *source_) : source(source_) { }
        ~SelectPostList() { delete source; }
};

inline std::string
SelectPostList::get_description() const
{
    return "(Select " + source->get_description() + ")";
}

#endif /* OM_HGUARD_SELECTPOSTLIST_H */
