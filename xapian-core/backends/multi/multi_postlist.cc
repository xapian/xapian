/* multi_postlist.cc: interface to multiple database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2007,2008,2009,2011 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "multi_postlist.h"

#include "debuglog.h"
#include "omassert.h"

#ifdef XAPIAN_ASSERTIONS_PARANOID
#include "xapian/database.h"
#endif

MultiPostList::MultiPostList(std::vector<LeafPostList *> & pls,
			     const Xapian::Database &this_db_)
	: postlists(pls),
	  this_db(this_db_),
	  finished(false),
	  currdoc(0)
{
    multiplier = pls.size();
}


MultiPostList::~MultiPostList()
{
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); ++i) {
	delete *i;
    }
    postlists.clear();
}

Xapian::doccount
MultiPostList::get_termfreq_min() const
{
    // Should never get called.
    Assert(false);
    return 0;
}

Xapian::doccount
MultiPostList::get_termfreq_max() const
{
    return MultiPostList::get_termfreq_min();
}

Xapian::doccount
MultiPostList::get_termfreq_est() const
{
    return MultiPostList::get_termfreq_min();
}

double
MultiPostList::get_maxweight() const
{
    return MultiPostList::get_weight();
}

double
MultiPostList::get_weight() const
{
    // Should never get called.
    Assert(false);
    return 0;
}

double
MultiPostList::recalc_maxweight()
{
    return MultiPostList::get_weight();
}

Xapian::docid
MultiPostList::get_docid() const
{
    LOGCALL(DB, Xapian::docid, "MultiPostList::get_docid", NO_ARGS);
    Assert(!at_end());
    Assert(currdoc != 0);
    RETURN(currdoc);
}

Xapian::termcount
MultiPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "MultiPostList::get_doclength", NO_ARGS);
    Assert(!at_end());
    Assert(currdoc != 0);
    Xapian::termcount result = postlists[(currdoc - 1) % multiplier]->get_doclength();
    AssertEqParanoid(result, this_db.get_doclength(get_docid()));
    RETURN(result);
}

Xapian::termcount
MultiPostList::get_unique_terms() const
{
    LOGCALL(DB, Xapian::termcount, "MultiPostList::get_unique_terms", NO_ARGS);
    Assert(!at_end());
    Assert(currdoc != 0);
    Xapian::termcount result = postlists[(currdoc - 1) % multiplier]->get_unique_terms();
    AssertEqParanoid(result, this_db.get_unique_terms(get_docid()));
    RETURN(result);
}

Xapian::termcount
MultiPostList::get_wdf() const
{
    return postlists[(currdoc - 1) % multiplier]->get_wdf();
}

PositionList *
MultiPostList::open_position_list() const
{
    return postlists[(currdoc - 1) % multiplier]->open_position_list();
}

PostList *
MultiPostList::next(double w_min)
{
    LOGCALL(DB, PostList *, "MultiPostList::next", w_min);
    Assert(!at_end());

    Xapian::docid newdoc = 0;
    Xapian::docid offset = 1;
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); ++i) {
	if (!(*i)->at_end()) {
	    Xapian::docid id = ((*i)->get_docid() - 1) * multiplier + offset;
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
	LOGLINE(DB, "MultiPostList::next() newdoc=" << newdoc <<
		    " (olddoc=" << currdoc << ")");
	currdoc = newdoc;
    } else {
	LOGLINE(DB, "MultiPostList::next() finished" <<
		    " (olddoc=" << currdoc << ")");
	finished = true;
    }
    RETURN(NULL);
}

PostList *
MultiPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(DB, PostList *, "MultiPostList::skip_to", did | w_min);
    Assert(!at_end());
    Xapian::docid newdoc = 0;
    Xapian::docid offset = 0;
    Xapian::docid realdid = (did - 1) / multiplier + 2;
    Xapian::doccount dbnumber = (did - 1) % multiplier;
    std::vector<LeafPostList *>::iterator i;
    for (i = postlists.begin(); i != postlists.end(); ++i) {
	if (offset == dbnumber) --realdid;
	++offset;
	Assert((realdid - 1) * multiplier + offset >= did);
	Assert((realdid - 1) * multiplier + offset < did + multiplier);
	if (!(*i)->at_end()) {
	    (*i)->skip_to(realdid, w_min);
	    if (!(*i)->at_end()) {
		Xapian::docid id = ((*i)->get_docid() - 1) * multiplier + offset;
		if (newdoc == 0 || id < newdoc) newdoc = id;
	    }
	}
    }
    if (newdoc) {
	currdoc = newdoc;
    } else {
	finished = true;
    }
    RETURN(NULL);
}

bool
MultiPostList::at_end() const
{
    return finished;
}

std::string
MultiPostList::get_description() const
{
    std::string desc;

    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); ++i) {
	if (!desc.empty()) desc += ',';
	desc += (*i)->get_description();
    }

    return desc;
}
