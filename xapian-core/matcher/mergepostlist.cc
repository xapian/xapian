/* mergepostlist.cc: merge postlists from different databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2008,2009,2011,2015,2016 Olly Betts
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

#include "xapian/error.h"

// NB don't prune - even with one sublist we still translate docids...

MergePostList::~MergePostList()
{
    LOGCALL_DTOR(MATCH, "MergePostList");
    std::vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); ++i) {
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
	next_handling_prune(plists[current], w_min, matcher);
	if (!plists[current]->at_end()) break;
	++current;
	if (unsigned(current) >= plists.size()) break;
	vsdoc.new_subdb(current);
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
    for (i = plists.begin(); i != plists.end(); ++i) {
	total += (*i)->get_termfreq_max();
    }
    RETURN(total);
}

Xapian::doccount
MergePostList::get_termfreq_min() const
{
    LOGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_min", NO_ARGS);
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); ++i) {
	total += (*i)->get_termfreq_min();
    }
    RETURN(total);
}

Xapian::doccount
MergePostList::get_termfreq_est() const
{
    LOGCALL(MATCH, Xapian::doccount, "MergePostList::get_termfreq_est", NO_ARGS);
    // sum of termfreqs for all children
    Xapian::doccount total = 0;
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); ++i) {
	total += (*i)->get_termfreq_est();
    }
    RETURN(total);
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
    RETURN(plists[current]->get_weight());
}

const string *
MergePostList::get_sort_key() const
{
    LOGCALL(MATCH, const string *, "MergePostList::get_sort_key", NO_ARGS);
    Assert(current != -1);
    RETURN(plists[current]->get_sort_key());
}

const string *
MergePostList::get_collapse_key() const
{
    LOGCALL(MATCH, const string *, "MergePostList::get_collapse_key", NO_ARGS);
    Assert(current != -1);
    RETURN(plists[current]->get_collapse_key());
}

double
MergePostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "MergePostList::get_maxweight", NO_ARGS);
    RETURN(w_max);
}

double
MergePostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "MergePostList::recalc_maxweight", NO_ARGS);
    w_max = 0;
    vector<PostList *>::iterator i;
    for (i = plists.begin(); i != plists.end(); ++i) {
	double w = (*i)->recalc_maxweight();
	if (w > w_max) w_max = w;
    }
    RETURN(w_max);
}

bool
MergePostList::at_end() const
{
    LOGCALL(MATCH, bool, "MergePostList::at_end", NO_ARGS);
    Assert(current != -1);
    RETURN(unsigned(current) >= plists.size());
}

string
MergePostList::get_description() const
{
    string desc = "( Merge ";
    vector<PostList *>::const_iterator i;
    for (i = plists.begin(); i != plists.end(); ++i) {
	desc += (*i)->get_description() + " ";
    }
    return desc + ")";
}

Xapian::termcount
MergePostList::get_doclength() const
{
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::get_doclength", NO_ARGS);
    Assert(current != -1);
    RETURN(plists[current]->get_doclength());
}

Xapian::termcount
MergePostList::get_unique_terms() const
{
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::get_unique_terms", NO_ARGS);
    Assert(current != -1);
    RETURN(plists[current]->get_unique_terms());
}

Xapian::termcount
MergePostList::count_matching_subqs() const
{
    LOGCALL(MATCH, Xapian::termcount, "MergePostList::count_matching_subqs", NO_ARGS);
    RETURN(plists[current]->count_matching_subqs());
}
