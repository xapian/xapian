/* orpostlist.cc: OR of two posting lists
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2003,2004,2007,2008,2009,2010,2012 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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
#include "orpostlist.h"

#include "debuglog.h"
#include "multiandpostlist.h"
#include "andmaybepostlist.h"
#include "omassert.h"

#include <algorithm>

OrPostList::OrPostList(PostList *left_,
		       PostList *right_,
		       MultiMatch *matcher_,
		       Xapian::doccount dbsize_)
	: BranchPostList(left_, right_, matcher_),
	  lhead(0), rhead(0), lvalid(false), rvalid(false),
	  lmax(0), rmax(0), minmax(0), dbsize(dbsize_)
{
    LOGCALL_CTOR(MATCH, "OrPostList", left_ | right_ | matcher_ | dbsize_);
    AssertRel(left_->get_termfreq_est(),>=,right_->get_termfreq_est());
}

PostList *
OrPostList::next(Xapian::weight w_min)
{
    LOGCALL(MATCH, PostList *, "OrPostList::next", w_min);
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		LOGLINE(MATCH, "OR -> AND");
		ret = new MultiAndPostList(l, r, lmax, rmax, matcher, dbsize);
		Xapian::docid newdocid = std::max(lhead, rhead);
		if (newdocid == 0 || (lvalid && rvalid && lhead == rhead)) {
		    ++newdocid;
		}
		skip_to_handling_prune(ret, newdocid, w_min, matcher);
	    } else {
		LOGLINE(MATCH, "OR -> AND MAYBE (1)");
		AndMaybePostList * ret2 =
		    new AndMaybePostList(r, l, matcher, dbsize, rhead, lhead);
		ret = ret2;
		// Advance the AndMaybePostList unless the old RHS postlist was
		// already ahead of the current docid.
		if (rhead <= lhead) {
		    next_handling_prune(ret, w_min, matcher);
		} else {
		    handle_prune(ret, ret2->sync_rhs(w_min));
		}
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    LOGLINE(MATCH, "OR -> AND MAYBE (2)");
	    AndMaybePostList * ret2 =
		    new AndMaybePostList(l, r, matcher, dbsize, lhead, rhead);
	    ret = ret2;
	    if (lhead <= rhead) {
		next_handling_prune(ret, w_min, matcher);
	    } else {
		handle_prune(ret, ret2->sync_rhs(w_min));
	    }
	}

	l = r = NULL;
	RETURN(ret);
    }

    bool ldry = false;
    bool rnext = !rvalid;

    if (!lvalid || lhead <= rhead) {
        if (lhead == rhead) rnext = true;
        next_handling_prune(l, w_min - rmax, matcher);
	lvalid = true;
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
        next_handling_prune(r, w_min - lmax, matcher);
	rvalid = true;
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }

    if (!ldry) {
	lhead = l->get_docid();
	RETURN(NULL);
    }

    PostList *ret = r;
    r = NULL;
    RETURN(ret);
}

PostList *
OrPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    LOGCALL(MATCH, PostList *, "OrPostList::skip_to", did | w_min);
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		LOGLINE(MATCH, "OR -> AND (in skip_to)");
		ret = new MultiAndPostList(l, r, lmax, rmax, matcher, dbsize);
		did = std::max(did, std::max(lhead, rhead));
	    } else {
		LOGLINE(MATCH, "OR -> AND MAYBE (in skip_to) (1)");
		AndMaybePostList * ret2 =
			new AndMaybePostList(r, l, matcher, dbsize, rhead, lhead);
		ret = ret2;
		handle_prune(ret, ret2->sync_rhs(w_min));
		did = std::max(did, rhead);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    LOGLINE(MATCH, "OR -> AND MAYBE (in skip_to) (2)");
	    AndMaybePostList * ret2 =
		    new AndMaybePostList(l, r, matcher, dbsize, lhead, rhead);
	    ret = ret2;
	    handle_prune(ret, ret2->sync_rhs(w_min));
	    did = std::max(did, lhead);
	}

	l = r = NULL;
	skip_to_handling_prune(ret, did, w_min, matcher);
	RETURN(ret);
    }

    bool ldry = false;
    if (lhead < did) {
	skip_to_handling_prune(l, did, w_min - rmax, matcher);
	lvalid = true;
	ldry = l->at_end();
    }

    if (rhead < did) {
	skip_to_handling_prune(r, did, w_min - lmax, matcher);
	rvalid = true;

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }

    if (!ldry) {
	lhead = l->get_docid();
	RETURN(NULL);
    }

    PostList *ret = r;
    r = NULL;
    RETURN(ret);
}

PostList *
OrPostList::check(Xapian::docid did, Xapian::weight w_min, bool &valid)
{
    LOGCALL(MATCH, PostList *, "OrPostList::check", did | w_min);
    if (w_min > minmax) {
	// we can replace the OR with another operator
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		LOGLINE(MATCH, "OR -> AND (in check)");
		ret = new MultiAndPostList(l, r, lmax, rmax, matcher, dbsize);
		did = std::max(did, std::max(lhead, rhead));
	    } else {
		LOGLINE(MATCH, "OR -> AND MAYBE (in check) (1)");
		AndMaybePostList * ret2 = new AndMaybePostList(r, l, matcher, dbsize, rhead, lhead);
		ret = ret2;
		handle_prune(ret, ret2->sync_rhs(w_min));
		did = std::max(did, rhead);
	    }
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    LOGLINE(MATCH, "OR -> AND MAYBE (in check) (2)");
	    AndMaybePostList * ret2 = new AndMaybePostList(l, r, matcher, dbsize, lhead, rhead);
	    ret = ret2;
	    handle_prune(ret, ret2->sync_rhs(w_min));
	    did = std::max(did, lhead);
	}

	l = r = NULL;
	check_handling_prune(ret, did, w_min, matcher, valid);
	RETURN(ret);
    }

    bool ldry = false;
    if (!lvalid || lhead < did) {
	lvalid = false;
	check_handling_prune(l, did, w_min - rmax, matcher, lvalid);
	ldry = l->at_end();
    }

    if (!rvalid || rhead <= did) {
	rvalid = false;
	check_handling_prune(r, did, w_min - lmax, matcher, rvalid);

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    valid = lvalid;
	    RETURN(ret);
	}
	if (rvalid) {
	    rhead = r->get_docid();
	} else {
	    rhead = did + 1;
	}
    }

    if (!ldry) {
	if (lvalid) {
	    lhead = l->get_docid();
	} else {
	    lhead = did + 1;
	}

	if (lhead < rhead) {
	    valid = lvalid;
	} else if (rhead < lhead) {
	    valid = rvalid;
	} else {
	    valid = lvalid || rvalid;
	}
	RETURN(NULL);
    }

    PostList *ret = r;
    r = NULL;
    valid = rvalid;
    RETURN(ret);
}

Xapian::doccount
OrPostList::get_termfreq_max() const
{
    LOGCALL(MATCH, Xapian::doccount, "OrPostList::get_termfreq_max", NO_ARGS);
    RETURN(std::min(l->get_termfreq_max() + r->get_termfreq_max(), dbsize));
}

Xapian::doccount
OrPostList::get_termfreq_min() const
{
    LOGCALL(MATCH, Xapian::doccount, "OrPostList::get_termfreq_min", NO_ARGS);
    RETURN(std::max(l->get_termfreq_min(), r->get_termfreq_min()));
}

Xapian::doccount
OrPostList::get_termfreq_est() const
{
    LOGCALL(MATCH, Xapian::doccount, "OrPostList::get_termfreq_est", NO_ARGS);
    if (rare(dbsize == 0))
	RETURN(0);
    // Estimate assuming independence:
    // P(l or r) = P(l) + P(r) - P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    double est = lest + rest - (lest * rest / dbsize);
    RETURN(static_cast<Xapian::doccount>(est + 0.5));
}

TermFreqs
OrPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const 
{
    LOGCALL(MATCH, TermFreqs, "OrPostList::get_termfreq_est_using_stats", stats);
    // Estimate assuming independence:
    // P(l or r) = P(l) + P(r) - P(l) . P(r)
    TermFreqs lfreqs(l->get_termfreq_est_using_stats(stats));
    TermFreqs rfreqs(r->get_termfreq_est_using_stats(stats));

    double freqest, relfreqest;

    // Our caller should have ensured this.
    Assert(stats.collection_size);

    freqest = lfreqs.termfreq + rfreqs.termfreq -
	    (lfreqs.termfreq * rfreqs.termfreq / stats.collection_size);

    if (stats.rset_size == 0) {
	relfreqest = 0;
    } else {
	relfreqest = lfreqs.reltermfreq + rfreqs.reltermfreq -
		(lfreqs.reltermfreq * rfreqs.reltermfreq / stats.rset_size);
    }

    RETURN(TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		     static_cast<Xapian::doccount>(relfreqest + 0.5)));
}

Xapian::docid
OrPostList::get_docid() const
{
    LOGCALL(MATCH, Xapian::docid, "OrPostList::get_docid", NO_ARGS);
    Assert(lhead != 0 && rhead != 0); // check we've started
    RETURN(std::min(lhead, rhead));
}

// only called if we are doing a probabilistic OR
Xapian::weight
OrPostList::get_weight() const
{
    LOGCALL(MATCH, Xapian::weight, "OrPostList::get_weight", NO_ARGS);
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) RETURN(l->get_weight());
    if (lhead > rhead) RETURN(r->get_weight());
    RETURN(l->get_weight() + r->get_weight());
}

// only called if we are doing a probabilistic operation
Xapian::weight
OrPostList::get_maxweight() const
{
    LOGCALL(MATCH, Xapian::weight, "OrPostList::get_maxweight", NO_ARGS);
    RETURN(lmax + rmax);
}

Xapian::weight
OrPostList::recalc_maxweight()
{
    LOGCALL(MATCH, Xapian::weight, "OrPostList::recalc_maxweight", NO_ARGS);
    // l and r cannot be NULL here, because the only place where they get set
    // to NULL is when the tree is decaying, and the OrPostList is then
    // immediately replaced.
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = std::min(lmax, rmax);
    RETURN(OrPostList::get_maxweight());
}

bool
OrPostList::at_end() const
{
    LOGCALL(MATCH, bool, "OrPostList::at_end", NO_ARGS);
    // Can never really happen - OrPostList next/skip_to autoprune
    AssertParanoid(!(l->at_end()) && !(r->at_end()));
    RETURN(false);
}

std::string
OrPostList::get_description() const
{
    return "(" + l->get_description() + " Or " + r->get_description() + ")";
}

Xapian::termcount
OrPostList::get_doclength() const
{
    LOGCALL(MATCH, Xapian::termcount, "OrPostList::get_doclength", NO_ARGS);
    Xapian::termcount doclength;

    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead > rhead) {
	doclength = r->get_doclength();
	LOGLINE(MATCH, "OrPostList::get_doclength() [right docid=" << rhead <<
		       "] = " << doclength);
    } else {
	doclength = l->get_doclength();
	LOGLINE(MATCH, "OrPostList::get_doclength() [left docid=" << lhead <<
	       	       "] = " << doclength);
    }

    RETURN(doclength);
}

Xapian::termcount
OrPostList::get_wdf() const
{
    LOGCALL(MATCH, Xapian::termcount, "OrPostList::get_wdf", NO_ARGS);
    if (lhead < rhead) RETURN(l->get_wdf());
    if (lhead > rhead) RETURN(r->get_wdf());
    RETURN(l->get_wdf() + r->get_wdf());
}

Xapian::termcount
OrPostList::count_matching_subqs() const
{
    LOGCALL(MATCH, Xapian::termcount, "OrPostList::count_matching_subqs", NO_ARGS);
    if (lhead < rhead) RETURN(l->count_matching_subqs());
    if (lhead > rhead) RETURN(r->count_matching_subqs());
    RETURN(l->count_matching_subqs() + r->count_matching_subqs());
}
