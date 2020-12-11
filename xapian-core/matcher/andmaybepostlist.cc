/** @file
 * @brief Merged postlist; items from one list, weights from both
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2008,2009,2011,2017 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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
#include "andmaybepostlist.h"

#include "debuglog.h"
#include "multiandpostlist.h"
#include "omassert.h"

PostList *
AndMaybePostList::process_next_or_skip_to(double w_min, PostList *ret)
{
    LOGCALL(MATCH, PostList *, "AndMaybePostList::process_next_or_skip_to", w_min | ret);
    handle_prune(l, ret);
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	RETURN(NULL);
    }

    lhead = l->get_docid();
    if (lhead <= rhead) RETURN(NULL);

    bool valid;
    check_handling_prune(r, lhead, w_min - lmax, matcher, valid);
    if (r->at_end()) {
	PostList *tmp = l;
	l = NULL;
	RETURN(tmp);
    }
    if (valid) {
	rhead = r->get_docid();
    } else {
	rhead = 0;
    }
    RETURN(NULL);
}

PostList *
AndMaybePostList::sync_rhs(double w_min)
{
    LOGCALL(MATCH, PostList *, "AndMaybePostList::sync_rhs", w_min);
    bool valid;
    check_handling_prune(r, lhead, w_min - lmax, matcher, valid);
    if (r->at_end()) {
	PostList *tmp = l;
	l = NULL;
	RETURN(tmp);
    }
    if (valid) {
	rhead = r->get_docid();
    } else {
	rhead = 0;
    }
    RETURN(NULL);
}

PostList *
AndMaybePostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "AndMaybePostList::next", w_min);
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	LOGLINE(MATCH, "AND MAYBE -> AND");
	ret = new MultiAndPostList(l, r, lmax, rmax, matcher, dbsize);
	l = r = NULL;
	skip_to_handling_prune(ret, std::max(lhead, rhead) + 1, w_min, matcher);
	RETURN(ret);
    }
    RETURN(process_next_or_skip_to(w_min, l->next(w_min - rmax)));
}

PostList *
AndMaybePostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "AndMaybePostList::skip_to", did | w_min);
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	LOGLINE(MATCH, "AND MAYBE -> AND (in skip_to)");
	ret = new MultiAndPostList(l, r, lmax, rmax, matcher, dbsize);
	did = std::max(did, std::max(lhead, rhead));
	l = r = NULL;
	skip_to_handling_prune(ret, did, w_min, matcher);
	RETURN(ret);
    }

    // exit if we're already past the skip point (or at it)
    if (did <= lhead) RETURN(NULL);

    RETURN(process_next_or_skip_to(w_min, l->skip_to(did, w_min - rmax)));
}

Xapian::doccount
AndMaybePostList::get_termfreq_max() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_max", NO_ARGS);
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_max());
}

Xapian::doccount
AndMaybePostList::get_termfreq_min() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_min", NO_ARGS);
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_min());
}

Xapian::doccount
AndMaybePostList::get_termfreq_est() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndMaybePostList::get_termfreq_est", NO_ARGS);
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_est());
}

TermFreqs
AndMaybePostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "AndMaybePostList::get_termfreq_est_using_stats", stats);
    // Termfreq is exactly that of left hand branch.
    RETURN(l->get_termfreq_est_using_stats(stats));
}

Xapian::docid
AndMaybePostList::get_docid() const
{
    LOGCALL(MATCH, Xapian::docid, "AndMaybePostList::get_docid", NO_ARGS);
    Assert(lhead != 0); // check we've started
    RETURN(lhead);
}

// only called if we are doing a probabilistic AND MAYBE
double
AndMaybePostList::get_weight() const
{
    LOGCALL(MATCH, double, "AndMaybePostList::get_weight", NO_ARGS);
    Assert(lhead != 0); // check we've started
    if (lhead == rhead) RETURN(l->get_weight() + r->get_weight());
    RETURN(l->get_weight());
}

// only called if we are doing a probabilistic operation
double
AndMaybePostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "AndMaybePostList::get_maxweight", NO_ARGS);
    RETURN(lmax + rmax);
}

double
AndMaybePostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "AndMaybePostList::recalc_maxweight", NO_ARGS);
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    RETURN(AndMaybePostList::get_maxweight());
}

bool
AndMaybePostList::at_end() const
{
    LOGCALL(MATCH, bool, "AndMaybePostList::at_end", NO_ARGS);
    RETURN(lhead == 0);
}

std::string
AndMaybePostList::get_description() const
{
    return "(" + l->get_description() + " AndMaybe " + r->get_description() +
	   ")";
}

Xapian::termcount
AndMaybePostList::get_doclength() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndMaybePostList::get_doclength", NO_ARGS);
    Assert(lhead != 0); // check we've started
    if (lhead == rhead) AssertEq(l->get_doclength(), r->get_doclength());
    RETURN(l->get_doclength());
}

Xapian::termcount
AndMaybePostList::get_unique_terms() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndMaybePostList::get_unique_terms", NO_ARGS);
    Assert(lhead != 0); // check we've started
    if (lhead == rhead) AssertEq(l->get_unique_terms(), r->get_unique_terms());
    RETURN(l->get_unique_terms());
}

Xapian::termcount
AndMaybePostList::get_wdf() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndMaybePostList::get_wdf", NO_ARGS);
    if (lhead == rhead) RETURN(l->get_wdf() + r->get_wdf());
    RETURN(l->get_wdf());
}

Xapian::termcount
AndMaybePostList::count_matching_subqs() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndMaybePostList::count_matching_subqs", NO_ARGS);
    if (lhead == rhead)
	RETURN(l->count_matching_subqs() + r->count_matching_subqs());
    RETURN(l->count_matching_subqs());
}

void
AndMaybePostList::gather_position_lists(OrPositionList* orposlist)
{
    l->gather_position_lists(orposlist);
    if (lhead == rhead) r->gather_position_lists(orposlist);
}
