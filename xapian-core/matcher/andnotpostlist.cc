/* andnotpostlist.cc: Return items which are in A, unless they're in B
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2007,2009,2011,2012 Olly Betts
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
#include "andnotpostlist.h"

#include "debuglog.h"
#include "omassert.h"

PostList *
AndNotPostList::advance_to_next_match(double w_min, PostList *ret)
{
    LOGCALL(MATCH, PostList *, "AndNotPostList::advance_to_next_match", w_min | ret);
    handle_prune(l, ret);
    if (l->at_end()) {
	lhead = 0;
	RETURN(NULL);
    }
    lhead = l->get_docid();

    while (rhead <= lhead) {
	if (lhead == rhead) {
	    next_handling_prune(l, w_min, matcher);
	    if (l->at_end()) {
		lhead = 0;
		RETURN(NULL);
	    }
	    lhead = l->get_docid();
	}
	skip_to_handling_prune(r, lhead, 0, matcher);
	if (r->at_end()) {
	    ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }
    RETURN(NULL);
}

AndNotPostList::AndNotPostList(PostList *left_,
			       PostList *right_,
			       MultiMatch *matcher_,
			       Xapian::doccount dbsize_)
	: BranchPostList(left_, right_, matcher_),
	  lhead(0), rhead(0), dbsize(dbsize_)
{
    LOGCALL_CTOR(MATCH, "AndNotPostList", left_ | right_ | matcher_ | dbsize_);
}

PostList *
AndNotPostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "AndNotPostList::next", w_min);
    RETURN(advance_to_next_match(w_min, l->next(w_min)));
}

PostList *
AndNotPostList::sync_and_skip_to(Xapian::docid id,
				 double w_min,
				 Xapian::docid lh,
				 Xapian::docid rh)
{
    LOGCALL(MATCH, PostList *, "AndNotPostList::sync_and_skip_to", id | w_min | lh | rh);
    lhead = lh;
    rhead = rh;
    RETURN(skip_to(id, w_min));
}

PostList *
AndNotPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "AndNotPostList::skip_to", did | w_min);
    if (did <= lhead) RETURN(NULL);
    RETURN(advance_to_next_match(w_min, l->skip_to(did, w_min)));
}

Xapian::doccount
AndNotPostList::get_termfreq_max() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_max", NO_ARGS);
    // Max is when as many docs as possible on left, and none excluded.
    RETURN(l->get_termfreq_max());
}

Xapian::doccount
AndNotPostList::get_termfreq_min() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_min", NO_ARGS);
    // Min is when as few docs as possible on left, and maximum are excluded.
    Xapian::doccount l_min = l->get_termfreq_min();
    Xapian::doccount r_max = r->get_termfreq_max();
    if (l_min > r_max) RETURN(l_min - r_max);
    RETURN(0u);
}

Xapian::doccount
AndNotPostList::get_termfreq_est() const
{
    LOGCALL(MATCH, Xapian::doccount, "AndNotPostList::get_termfreq_est", NO_ARGS);
    if (rare(dbsize == 0))
	RETURN(0);
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    // P(l not r) = P(l) - P(l and r) = P(l) . ( 1 - P(r))
    double est = l->get_termfreq_est() *
	     (1.0 - double(r->get_termfreq_est()) / dbsize);
    RETURN(static_cast<Xapian::doccount>(est + 0.5));
}

TermFreqs
AndNotPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "AndNotPostList::get_termfreq_est_using_stats", stats);
    // Estimate assuming independence:
    // P(l and r) = P(l) . P(r)
    // P(l not r) = P(l) - P(l and r) = P(l) . ( 1 - P(r))
    TermFreqs lfreqs(l->get_termfreq_est_using_stats(stats));
    TermFreqs rfreqs(r->get_termfreq_est_using_stats(stats));

    double freqest, relfreqest, collfreqest;

    // Our caller should have ensured this.
    Assert(stats.collection_size);

    freqest = lfreqs.termfreq *
	    (1.0 - (double(rfreqs.termfreq) / stats.collection_size));

    collfreqest = lfreqs.collfreq *
	    (1.0 - (double(rfreqs.collfreq) / stats.total_term_count));

    if (stats.rset_size == 0) {
	relfreqest = 0;
    } else {
	relfreqest = lfreqs.reltermfreq *
		(1.0 - (double(rfreqs.reltermfreq) / stats.rset_size));
    }

    RETURN(TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		     static_cast<Xapian::doccount>(relfreqest + 0.5),
		     static_cast<Xapian::termcount>(collfreqest + 0.5)));
}

Xapian::docid
AndNotPostList::get_docid() const
{
    LOGCALL(MATCH, Xapian::docid, "AndNotPostList::get_docid", NO_ARGS);
    RETURN(lhead);
}

// only called if we are doing a probabilistic AND NOT
double
AndNotPostList::get_weight() const
{
    LOGCALL(MATCH, double, "AndNotPostList::get_weight", NO_ARGS);
    RETURN(l->get_weight());
}

// only called if we are doing a probabilistic AND NOT
double
AndNotPostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "AndNotPostList::get_maxweight", NO_ARGS);
    RETURN(l->get_maxweight());
}

double
AndNotPostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "AndNotPostList::recalc_maxweight", NO_ARGS);
    // l cannot be NULL here because it is only set to NULL when the tree
    // decays, so this can never be called at that point.
    RETURN(l->recalc_maxweight());
}

bool
AndNotPostList::at_end() const
{
    LOGCALL(MATCH, bool, "AndNotPostList::at_end", NO_ARGS);
    RETURN(lhead == 0);
}

std::string
AndNotPostList::get_description() const
{
    return "(" + l->get_description() + " AndNot " + r->get_description() + ")";
}

Xapian::termcount
AndNotPostList::get_doclength() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndNotPostList::get_doclength", NO_ARGS);
    RETURN(l->get_doclength());
}

Xapian::termcount
AndNotPostList::get_unique_terms() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndNotPostList::get_unique_terms", NO_ARGS);
    RETURN(l->get_unique_terms());
}

Xapian::termcount
AndNotPostList::get_wdf() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndNotPostList::get_wdf", NO_ARGS);
    RETURN(l->get_wdf());
}

Xapian::termcount
AndNotPostList::count_matching_subqs() const
{
    LOGCALL(MATCH, Xapian::termcount, "AndNotPostList::count_matching_subqs", NO_ARGS);
    RETURN(l->count_matching_subqs());
}
