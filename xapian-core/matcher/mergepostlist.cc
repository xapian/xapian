/* mergepostlist.cc: merge postlists from different databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2008,2009,2011 Olly Betts
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
#include "mergepostlist.h"

#include "multimatch.h"
#include "api/emptypostlist.h"
#include "branchpostlist.h"
#include "debuglog.h"
#include "omassert.h"
#include "valuestreamdocument.h"
#include "xapian/errorhandler.h"

// NB don't prune - even with one sublist we still translate docids...

MergePostList::~MergePostList()
{
    LOGCALL_DTOR(MATCH, "MergePostList");
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	delete *i;
    }
}

PostList *
MergePostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "MergePostList::next", w_min);
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
MergePostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "MergePostList::skip_to", did | w_min);
    (void)did;
    (void)w_min;
    // MergePostList doesn't return documents in docid order, so skip_to
    // isn't a meaningful operation.
    throw Xapian::InvalidOperationError("MergePostList doesn't support skip_to");
}

Xapian::termcount
MergePostList::get_wdf() const
{
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::get_wdf", NO_ARGS);
    RETURN(plists[current]->get_wdf());
}

Xapian::doccount
MergePostList::get_termfreq_max() const
{
    LOGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_max", NO_ARGS);
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
    LOGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_min", NO_ARGS);
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
    LOGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_est", NO_ARGS);
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
    LOGCALL(MATCH, Xapian::docid, "MergePostList::get_docid", NO_ARGS);
    Assert(current != -1);
    // FIXME: this needs fixing so we can prune plists - see MultiPostlist
    // for code which does this...
    RETURN((plists[current]->get_docid() - 1) * plists.size() + current + 1);
}

double
MergePostList::get_weight() const
{
    LOGCALL(MATCH, double, "MergePostList::get_weight", NO_ARGS);
    Assert(current != -1);
    return plists[current]->get_weight();
}

const string *
MergePostList::get_collapse_key() const
{
    LOGCALL(MATCH, const string *, "MergePostList::get_collapse_key", NO_ARGS);
    Assert(current != -1);
    return plists[current]->get_collapse_key();
}

double
MergePostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "MergePostList::get_maxweight", NO_ARGS);
    return w_max;
}

double
MergePostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "MergePostList::recalc_maxweight", NO_ARGS);
    w_max = 0;
    vector<PostList *>::iterator i;
    for (i = plists.begin(); i != plists.end(); i++) {
	try {
	    double w = (*i)->recalc_maxweight();
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
    LOGCALL(MATCH, bool, "MergePostList::at_end", NO_ARGS);
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
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::get_doclength", NO_ARGS);
    Assert(current != -1);
    return plists[current]->get_doclength();
}

Xapian::termcount
MergePostList::count_matching_subqs() const
{
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::count_matching_subqs", NO_ARGS);
    RETURN(plists[current]->count_matching_subqs());
}
