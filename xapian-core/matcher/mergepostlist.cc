/* mergepostlist.cc: merge postlists from different databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2008,2009 Olly Betts
 * Copyright 2007,2009 Lemur Consulting Ltd
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
#include "multimatch.h"
#include "emptypostlist.h"
#include "mergepostlist.h"
#include "branchpostlist.h"
#include "omassert.h"
#include "omdebug.h"
#include "valuestreamdocument.h"
#include "xapian/errorhandler.h"

// NB don't prune - even with one sublist we still translate docids...

MergePostList::~MergePostList()
{
    DEBUGCALL(MATCH, void, "MergePostList::~MergePostList", "");
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	delete *i;
    }
}

PostList *
MergePostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MergePostList::next", w_min);
    LOGVALUE(MATCH, current);
    if (current == -1) current = 0;
    while (true) {
	// FIXME: should skip over Remote matchers which aren't ready yet
	// and come back to them later...
	try {
	    next_handling_prune(plists[current], w_min, matcher);
	    if (!plists[current]->at_end()) break;
	    ++current;
	    if (unsigned(current) >= plists.size()) break;
	    vsdoc.new_subdb(current);
	} catch (Xapian::Error & e) {
	    if (errorhandler) {
		LOGLINE(EXCEPTION, "Calling error handler in MergePostList::next().");
		(*errorhandler)(e);
		// Continue match without this sub-postlist.
		delete plists[current];
		plists[current] = new EmptyPostList;
	    } else {
		throw;
	    }
	}
	if (matcher) matcher->recalc_maxweight();
    }
    LOGVALUE(MATCH, current);
    RETURN(NULL);
}

PostList *
MergePostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "MergePostList::skip_to", did << ", " << w_min);
    (void)did;
    (void)w_min;
    // MergePostList doesn't return documents in docid order, so skip_to
    // isn't a meaningful operation.
    throw Xapian::InvalidOperationError("MergePostList doesn't support skip_to");
}

Xapian::termcount
MergePostList::get_wdf() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "MergePostList::get_wdf", "");
    RETURN(plists[current]->get_wdf());
}

Xapian::doccount
MergePostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_max", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_max();
    }
    return total;
}

Xapian::doccount
MergePostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_min", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_min();
    }
    return total;
}

Xapian::doccount
MergePostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_est", "");
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	total += (*i)->get_termfreq_est();
    }
    return total;
}

Xapian::docid
MergePostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "MergePostList::get_docid", "");
    Assert(current != -1);
    // FIXME: this needs fixing so we can prune plists - see MultiPostlist
    // for code which does this...
    RETURN((plists[current]->get_docid() - 1) * plists.size() + current + 1);
}

Xapian::weight
MergePostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::get_weight", "");
    Assert(current != -1);
    return plists[current]->get_weight();
}

const string *
MergePostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, string *, "MergePostList::get_collapse_key", "");
    Assert(current != -1);
    return plists[current]->get_collapse_key();
}

Xapian::weight
MergePostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::get_maxweight", "");
    return w_max;
}

Xapian::weight
MergePostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "MergePostList::recalc_maxweight", "");
    w_max = 0;
    vector<PostList *>::iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	try {
	    Xapian::weight w = (*i)->recalc_maxweight();
	    if (w > w_max) w_max = w;
	} catch (Xapian::Error & e) {
	    if (errorhandler) {
		LOGLINE(EXCEPTION, "Calling error handler in MergePostList::recalc_maxweight().");
		(*errorhandler)(e);

		if (current == i - plists.begin()) {
		    // Fatal error
		    throw;
		}
		// Continue match without this sub-postlist.
		delete (*i);
		*i = new EmptyPostList;
	    } else {
		throw;
	    }
	}
    }
    return w_max;
}

bool
MergePostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "MergePostList::at_end", "");
    Assert(current != -1);
    return unsigned(current) >= plists.size();    
}

string
MergePostList::get_description() const
{
    string desc = "( Merge ";
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	desc += (*i)->get_description() + " ";
    }
    return desc + ")";
}

Xapian::termcount
MergePostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "MergePostList::get_doclength", "");
    Assert(current != -1);
    return plists[current]->get_doclength();
}

Xapian::termcount
MergePostList::count_matching_subqs() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "MergePostList::count_matching_subqs", "");
    RETURN(plists[current]->count_matching_subqs());
}
