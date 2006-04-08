/* andpostlist.cc: Return only items which are in both sublists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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

#include <config.h>
#include "andpostlist.h"
#include "omdebug.h"

inline void
AndPostList::process_next_or_skip_to(Xapian::weight w_min, PostList *ret)
{
    DEBUGCALL(MATCH, void, "AndPostList::process_next_or_skip_to",
	      w_min << ", " << ret);
    head = 0;
    handle_prune(r, ret);
    DEBUGLINE(MATCH, "r at_end = " << r->at_end());
    if (r->at_end()) return;

    // r has just been advanced by next or skip_to so must be > head
    // (and head is the current position of l)
    Xapian::docid rhead = r->get_docid();
    DEBUGLINE(MATCH, "rhead " << rhead);
    DEBUGLINE(MATCH, "w_min " << w_min << " rmax " << rmax);
    skip_to_handling_prune(l, rhead, w_min - rmax, matcher);
    DEBUGLINE(MATCH, "l at_end = " << l->at_end());
    if (l->at_end()) return;

    Xapian::docid lhead = l->get_docid();
    DEBUGLINE(MATCH, "lhead " << lhead);

    while (lhead != rhead) {
	if (lhead < rhead) {
	    // FIXME: CSE these w_min values?
	    // But note that lmax and rmax may change on recalc_maxweight...
	    skip_to_handling_prune(l, rhead, w_min - rmax, matcher);
	    DEBUGLINE(MATCH, "l at_end = " << l->at_end());
	    if (l->at_end()) {
		head = 0;
		return;
	    }
	    lhead = l->get_docid();
	    DEBUGLINE(MATCH, "lhead " << lhead);
	} else {
	    skip_to_handling_prune(r, lhead, w_min - lmax, matcher);
	    DEBUGLINE(MATCH, "r at_end = " << r->at_end());
	    if (r->at_end()) {
		head = 0;
		return;
	    }
	    rhead = r->get_docid();
	    DEBUGLINE(MATCH, "rhead " << rhead);
	}
    }

    head = lhead;
    return;
}

AndPostList::AndPostList(PostList *left_, PostList *right_,
			 MultiMatch *matcher_,
			 Xapian::doccount dbsize_,
			 bool replacement)
	: BranchPostList(left_, right_, matcher_), head(0), lmax(0), rmax(0),
	  dbsize(dbsize_)
{
    DEBUGCALL(MATCH, void, "AndPostList", left_ << ", " << right_ << ", " << matcher_ << ", " << dbsize_ << ", " << replacement);
    if (replacement) {
	// Initialise the maxweights from the kids so we can avoid forcing
	// a full maxweight recalc
	lmax = l->get_maxweight();
	rmax = r->get_maxweight();
    }
}

PostList *
AndPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndPostList::next", w_min);
    process_next_or_skip_to(w_min, r->next(w_min - lmax));
    RETURN(NULL);
}

PostList *
AndPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndPostList::skip_to", did << ", " << w_min);
    if (did > head)
	process_next_or_skip_to(w_min, r->skip_to(did, w_min - lmax));
    RETURN(NULL);
}

Xapian::doccount
AndPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_max", "");
    RETURN(std::min(l->get_termfreq_max(), r->get_termfreq_max()));
}

Xapian::doccount
AndPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_min", "");
    RETURN(0u);
}

Xapian::doccount
AndPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "AndPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    RETURN(static_cast<Xapian::doccount> (lest * rest / dbsize));
}

Xapian::docid
AndPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "AndPostList::get_docid", "");
    RETURN(head);
}

// only called if we are doing a probabilistic AND
Xapian::weight
AndPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::get_weight", "");
    RETURN(l->get_weight() + r->get_weight());
}

// only called if we are doing a probabilistic operation
Xapian::weight
AndPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::get_maxweight", "");
    RETURN(lmax + rmax);
}

Xapian::weight
AndPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "AndPostList::recalc_maxweight", "");
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    RETURN(AndPostList::get_maxweight());
}

bool
AndPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "AndPostList::at_end", "");
    RETURN(head == 0);
}

std::string
AndPostList::get_description() const
{
    return "(" + l->get_description() + " And " + r->get_description() + ")";
}

Xapian::doclength
AndPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::doclength, "AndPostList::get_doclength", "");
    Xapian::doclength doclength = l->get_doclength();
    DEBUGLINE(MATCH, "docid=" << head);
    AssertEqDouble(l->get_doclength(), r->get_doclength());
    RETURN(doclength);
}
