/* multi_postlist.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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
    friend class Xapian::Database;
    private:
	std::vector<LeafPostList *> postlists;

	const Xapian::Database &this_db;

	bool   finished;
	Xapian::docid  currdoc;

	string tname;
	mutable bool freq_initialised;
	mutable Xapian::doccount termfreq;

	mutable bool collfreq_initialised;
	mutable Xapian::termcount collfreq;

	Xapian::weight termweight;

	Xapian::doccount multiplier;

	MultiPostList(std::vector<LeafPostList *> & pls,
		      const Xapian::Database &this_db_);
    public:
	~MultiPostList();

	void set_termweight(const Xapian::Weight * wt); // Sets term weight

	Xapian::doccount get_termfreq() const;
	Xapian::termcount get_collection_freq() const;

	Xapian::docid  get_docid() const;     // Gets current docid
	Xapian::doclength get_doclength() const; // Get length of current document
        Xapian::termcount get_wdf() const;	    // Within Document Frequency
	PositionList *read_position_list();
	PositionList * open_position_list() const;
	PostList *next(Xapian::weight w_min);          // Moves to next docid
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

inline void
MultiPostList::set_termweight(const Xapian::Weight * wt)
{
    // Set in base class, so that get_maxweight() works
    LeafPostList::set_termweight(wt);
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	(*i)->set_termweight(wt);
    }
}

inline Xapian::doccount
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

inline Xapian::termcount
MultiPostList::get_collection_freq() const
{
    if(collfreq_initialised) return collfreq;
    DEBUGLINE(DB, "Calculating multiple term frequencies");

    // Calculate and remember the collfreq
    collfreq = 0;
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	collfreq += (*i)->get_collection_freq();
    }

    collfreq_initialised = true;
    return collfreq;
}

inline Xapian::docid
MultiPostList::get_docid() const
{
    DEBUGCALL(DB, Xapian::docid, "MultiPostList::get_docid", "");
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
