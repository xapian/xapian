/* mergepostlist.cc: MERGE of two posting lists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

// NB don't prune - even with one sublist we still translate docids...

#include <config.h>
#include "multimatch.h"
#include "mergepostlist.h"
#include "branchpostlist.h"
#include "omdebug.h"
#include <xapian/enquire.h>
#include <xapian/errorhandler.h>

MergePostList::MergePostList(std::vector<PostList *> plists_,
			     MultiMatch *matcher_,
			     Xapian::ErrorHandler * errorhandler_)
	: plists(plists_), current(-1), matcher(matcher_),
	  errorhandler(errorhandler_)
{
    DEBUGCALL(MATCH, void, "MergePostList::MergePostList", "std::vector<PostList *>");
}

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
    DEBUGLINE(MATCH, "current = " << current);
    if (current == -1) current = 0;
    do {
	// FIXME: should skip over Remote matchers which aren't ready yet
	// and come back to them later...
	try {
	    next_handling_prune(plists[current], w_min, matcher);
	    if (!plists[current]->at_end()) break;
	    current++;
	} catch (Xapian::Error & e) {
	    if (errorhandler) {
		DEBUGLINE(EXCEPTION, "Calling error handler in MergePostList::next().");
		(*errorhandler)(e);
		// Continue match without this sub-postlist.
		delete plists[current];
		AutoPtr<LeafPostList> lpl(new EmptyPostList);
		// give it a weighting object
		// FIXME: make it an EmptyWeight instead of BoolWeight
		lpl->set_termweight(new Xapian::BoolWeight());
		plists[current] = lpl.release();
	    } else {
		throw;
	    }
	}
    } while ((unsigned)current < plists.size());
    DEBUGLINE(MATCH, "current = " << current);
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
		DEBUGLINE(EXCEPTION, "Calling error handler in MergePostList::recalc_maxweight().");
		(*errorhandler)(e);

		if (current == i - plists.begin()) {
		    // Fatal error
		    throw;
		} 
		// Continue match without this sub-postlist.
		delete (*i);
		AutoPtr<LeafPostList> lpl(new EmptyPostList);
		// give it a weighting object
		// FIXME: make it an EmptyWeight instead of BoolWeight
		lpl->set_termweight(new Xapian::BoolWeight());
		*i = lpl.release();
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
    return (unsigned int)current >= plists.size();    
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

Xapian::doclength
MergePostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "MergePostList::get_doclength", "");
    Assert(current != -1);
    return plists[current]->get_doclength();
}

PositionList *
MergePostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "MergePostList::read_position_list", "");
    throw Xapian::UnimplementedError("MergePostList::read_position_list() unimplemented");
}

PositionList *
MergePostList::open_position_list() const
{
    throw Xapian::UnimplementedError("MergePostList::open_position_list() unimplemented");
}
