/* multi_postlist.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

#ifndef _multi_postlist_h_
#define _multi_postlist_h_

#include "utils.h"
#include "omassert.h"
#include "dbpostlist.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiPostListInternal {
    public:
	DBPostList * pl;
	docid currdoc;

	doccount offset;
	doccount multiplier;

	MultiPostListInternal(DBPostList * pl_new,
			      doccount off,
			      doccount mult)
		: pl(pl_new), currdoc(0), offset(off), multiplier(mult) {}
};

class MultiPostList : public virtual DBPostList {
    friend class MultiDatabase;
    private:
	list<MultiPostListInternal> postlists;

	bool   finished;
	docid  currdoc;

	termname tname;
	mutable bool freq_initialised;
	mutable doccount termfreq;

	weight termweight;

	MultiPostList(list<MultiPostListInternal> &);
    public:
	~MultiPostList();

	void set_termweight(const IRWeight *); // Sets term weight

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	PostList *next(weight);          // Moves to next docid
	PostList *skip_to(docid, weight);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	string intro_term_description() const;
};

inline void
MultiPostList::set_termweight(const IRWeight * wt)
{
    // Set in base class, so that get_maxweight() works
    DBPostList::set_termweight(wt);
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    while(i != postlists.end()) {
	(*i).pl->set_termweight(wt);
	i++;
    }
}

inline doccount
MultiPostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;
#ifdef DEBUG
    cout << "Calculating multiple term frequencies" << endl;
#endif /* DEBUG */

    // Calculate and remember the termfreq
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    termfreq = 0;
    while(i != postlists.end()) {
	termfreq += (*i).pl->get_termfreq();
	i++;
    }

    freq_initialised = true;
    return termfreq;
}

inline docid
MultiPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
#ifdef DEBUG
    //cout << this << ":DocID " << currdoc << endl;
#endif /* DEBUG */
    return currdoc;
}

inline bool
MultiPostList::at_end() const
{
    return finished;
}

inline string
MultiPostList::intro_term_description() const
{
    string desc = "[";

    // Calculate and remember the termfreq
    list<MultiPostListInternal>::const_iterator i;
    for(i = postlists.begin(); i != postlists.end(); i++) {
	if(desc != "[") desc += ",";
	desc += (*i).pl->intro_term_description();
    }

    return desc + "]:" + inttostring(get_termfreq());
}

#endif /* _multi_postlist_h_ */
