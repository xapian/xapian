/* multi_database.cc: interface to multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "omassert.h"
#include "multi_postlist.h"
#include "database_builder.h"

#include <string>
#include <vector>
#include <list>

//////////////
// Postlist //
//////////////

MultiPostList::MultiPostList(list<MultiPostListInternal> & pls)
	: postlists(pls), finished(false), currdoc(0),
	  freq_initialised(false)
{
}


MultiPostList::~MultiPostList()
{
    // Will be empty if we've got to the end of all lists,
    // but we might not have, so remove anything left.
    while(postlists.begin() != postlists.end()) {
	delete (*postlists.begin()).pl;
	postlists.erase(postlists.begin());
    }
}

weight MultiPostList::get_weight() const
{
    Assert(freq_initialised);

    weight wt = 0;
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    while(i != postlists.end()) {
	if((*i).currdoc == currdoc)
	    wt += (*i).pl->get_weight();
	i++;
    }
    return wt;
}

PostList * MultiPostList::next(weight w_min)
{
    Assert(!at_end());

    list<MultiPostListInternal>::iterator i = postlists.begin();
    while(i != postlists.end()) {
	// Check if it needs to be advanced
	if(currdoc >= (*i).currdoc) {
	    (*i).pl->next(w_min);
	    if((*i).pl->at_end()) {
		// Close sub-postlist
		delete (*i).pl;
		list<MultiPostListInternal>::iterator erase_iter = i;
		i++;
		postlists.erase(erase_iter);
		// erase iter is now invalid, but i is still valid because
		// i) this is a list, and ii) i wasn't pointing to a deleted
		// entry
		continue;
	    }
	    (*i).currdoc = ((*i).pl->get_docid() - 1) * (*i).multiplier +
		           (*i).offset;
	}
	i++;
    }
    if(postlists.size() == 0) {
	finished = true;
	return NULL;
    }

    i = postlists.begin();
    docid newdoc = (*i).currdoc;
    for(i++; i != postlists.end(); i++) {
	// Check if it might be the newdoc
	if((*i).currdoc < newdoc) newdoc = (*i).currdoc;
    }

    currdoc = newdoc;

    return NULL;
}

// FIXME - write implementation to use skip_to methods of sub-postlists
// for greater efficiency
PostList *
MultiPostList::skip_to(docid did, weight w_min)
{
    Assert(!at_end());
    while (!at_end() && currdoc < did) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
}
