/* multi_postlist.h: C++ class definition for multiple database access
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

#ifndef OM_HGUARD_MULTI_POSTLIST_H
#define OM_HGUARD_MULTI_POSTLIST_H

#include "utils.h"
#include "omdebug.h"
#include "leafpostlist.h"
#include <stdlib.h>
#include <vector>

class MultiPostList : public LeafPostList {
    friend class MultiDatabase;
    private:
	std::vector<LeafPostList *> postlists;

	RefCntPtr<const MultiDatabase> this_db;

	bool   finished;
	om_docid  currdoc;

	om_termname tname;
	mutable bool freq_initialised;
	mutable om_doccount termfreq;

	om_weight termweight;

	om_doccount multiplier;

	MultiPostList(std::vector<LeafPostList *> & pls,
		      RefCntPtr<const MultiDatabase> this_db_);
    public:
	~MultiPostList();

	void set_termweight(const IRWeight * wt); // Sets term weight

	om_doccount get_termfreq() const;

	om_docid  get_docid() const;     // Gets current docid
	om_weight get_weight() const;    // Gets current weight
	om_doclength get_doclength() const; // Get length of current document
        om_termcount get_wdf() const;	    // Within Document Frequency
	PositionList *get_position_list();
	PostList *next(om_weight w_min);          // Moves to next docid
	PostList *skip_to(om_docid did, om_weight w_min);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

inline void
MultiPostList::set_termweight(const IRWeight * wt)
{
    // Set in base class, so that get_maxweight() works
    LeafPostList::set_termweight(wt);
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	(*i)->set_termweight(wt);
    }
}

inline om_doccount
MultiPostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;
    DEBUGLINE(DB, "Calculating multiple term frequencies");

    // Calculate and remember the termfreq
    termfreq = 0;
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	termfreq += (*i)->get_termfreq();
    }

    freq_initialised = true;
    return termfreq;
}

inline om_docid
MultiPostList::get_docid() const
{
    DEBUGCALL(DB, om_docid, "MultiPostList::get_docid", "");
    Assert(!at_end());
    Assert(currdoc != 0);
    RETURN(currdoc);
}

inline bool
MultiPostList::at_end() const
{
    return finished;
}

inline std::string
MultiPostList::get_description() const
{
    std::string desc = "[";

    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	desc += (*i)->get_description();
	if (i != postlists.end()) desc += ",";
    }

    return desc + "]:" + om_tostring(get_termfreq());
}

#endif /* OM_HGUARD_MULTI_POSTLIST_H */
