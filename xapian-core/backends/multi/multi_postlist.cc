/* multi_database.cc: interface to multiple database access
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

#include <stdio.h>

#include "omdebug.h"
#include "multi_postlist.h"
#include "multi_database.h"
#include "database_builder.h"

#include <list>

//////////////
// Postlist //
//////////////

MultiPostList::MultiPostList(std::vector<LeafPostList *> & pls,
			     RefCntPtr<const MultiDatabase> this_db_)
	: postlists(pls),
	  this_db(this_db_),
	  finished(false),
	  currdoc(0),
	  freq_initialised(false)
{
    multiplier = pls.size();
}


MultiPostList::~MultiPostList()
{
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	delete *i;
    }
    postlists.clear();
}

om_weight
MultiPostList::get_weight() const
{
    return postlists[(currdoc - 1) % multiplier]->get_weight();
}

om_doclength
MultiPostList::get_doclength() const
{
    return this_db->get_doclength(get_docid());
}

om_termcount
MultiPostList::get_wdf() const
{
    return postlists[(currdoc - 1) % multiplier]->get_wdf();
}

PositionList *
MultiPostList::get_position_list()
{
    return postlists[(currdoc - 1) % multiplier]->get_position_list();
}

PostList *
MultiPostList::next(om_weight w_min)
{
    DEBUGCALL(DB, PostList *, "MultiPostList::next", w_min);
    Assert(!at_end());

    om_docid newdoc = 0;
    om_docid offset = 1;
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	if (!(*i)->at_end()) {
	    om_docid id = ((*i)->get_docid() - 1) * multiplier + offset;
	    // Check if it needs to be advanced
	    if (currdoc >= id) {
		(*i)->next(w_min);
		if (!(*i)->at_end()) {
		    id = ((*i)->get_docid() - 1) * multiplier + offset;
		    if (newdoc == 0 || id < newdoc) newdoc = id;
		}
	    } else {
		if (newdoc == 0 || id < newdoc) newdoc = id;
	    }
	}
	offset++;
    }
    if (newdoc) {
	DEBUGLINE(DB, "MultiPostList::next() newdoc=" << newdoc <<
		  " (olddoc=" << currdoc << ")");
	currdoc = newdoc;
    } else {
	DEBUGLINE(DB, "MultiPostList::next() finished" <<
		  " (olddoc=" << currdoc << ")");
	finished = true;
    }
    RETURN(NULL);
}

PostList *
MultiPostList::skip_to(om_docid did, om_weight w_min)
{
    DEBUGCALL(DB, PostList *, "MultiPostList::skip_to", did << ", " << w_min);
    Assert(!at_end());
    om_docid newdoc = 0;
    om_docid offset = 1;
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {	
	Assert((realdid - 1) * multiplier + offset >= did);
	Assert((realdid - 1) * multiplier + offset < did + multiplier);
	if (!(*i)->at_end()) {
	    (*i)->skip_to(realdid, w_min);
	    if (!(*i)->at_end()) {
		om_docid id = ((*i)->get_docid() - 1) * multiplier + offset;
		if (newdoc == 0 || id < newdoc) newdoc = id;
	    }
	}
	offset++;
	if (offset == dbnumber) realdid--;
    }
    if (newdoc) {
	currdoc = newdoc;
    } else {
	finished = true;
    }
    RETURN(NULL);
}
