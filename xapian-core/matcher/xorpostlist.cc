/* xorpostlist.cc: XOR of two posting lists
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2007,2008,2009 Olly Betts
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

#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "omassert.h"
#include "omdebug.h"

// for XOR we just pass w_min through unchanged since both branches matching
// doesn't cause a match

inline PostList *
XorPostList::advance_to_next_match()
{
    DEBUGCALL(MATCH, PostList *, "XorPostList::advance_to_next_match", "");
    while (rhead == lhead) {
	next_handling_prune(l, 0, matcher);
	next_handling_prune(r, 0, matcher);
	if (l->at_end()) {
	    if (r->at_end()) {
		lhead = 0;
		RETURN(NULL);
	    }
	    PostList *ret = r;
	    r = NULL;
	    RETURN(ret);
	}
	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	lhead = l->get_docid();
	rhead = r->get_docid();
    }
    RETURN(NULL);
}

XorPostList::XorPostList(PostList *left_,
			 PostList *right_,
			 MultiMatch *matcher_,
			 Xapian::doccount dbsize_)
	: BranchPostList(left_, right_, matcher_),
	  lhead(0),
	  rhead(0),
	  dbsize(dbsize_)
{
    DEBUGCALL(MATCH, void, "XorPostList", left_ << ", " << right_ << ", " << matcher_ << ", " << dbsize_);
}

PostList *
XorPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "XorPostList::next", w_min);
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		LOGLINE(MATCH, "XOR drops below w_min");
		// neither side is weighty enough, so run dry
		lhead = 0;
		RETURN(NULL);
	    }
	    LOGLINE(MATCH, "XOR -> AND NOT (1)");
	    ret = new AndNotPostList(r, l, matcher, dbsize);
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    LOGLINE(MATCH, "XOR -> AND NOT (2)");
	    ret = new AndNotPostList(l, r, matcher, dbsize);
	}

	l = r = NULL;
	next_handling_prune(ret, w_min, matcher);
	RETURN(ret);
    }

    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
	// lhead == rhead should only happen on first next
	if (lhead == rhead) rnext = true;
	next_handling_prune(l, 0, matcher);
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }

    if (rnext) {
	next_handling_prune(r, 0, matcher);
	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	RETURN(ret);
    }

    lhead = l->get_docid();
    RETURN(advance_to_next_match());
}

PostList *
XorPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "XorPostList::skip_to", did << ", " << w_min);
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret, *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		LOGLINE(MATCH, "XOR drops below w_min");
		// neither side is weighty enough, so run dry
		lhead = 0;
		RETURN(NULL);
	    }
	    LOGLINE(MATCH, "XOR -> AND NOT (in skip_to) (1)");
	    AndNotPostList *ret3 = new AndNotPostList(r, l, matcher, dbsize);
	    did = std::max(did, rhead);
	    ret2 = ret3->sync_and_skip_to(did, w_min, rhead, lhead);
	    ret = ret3;
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    LOGLINE(MATCH, "XOR -> AND NOT (in skip_to) (2)");
	    AndNotPostList *ret3 = new AndNotPostList(l, r, matcher, dbsize);
	    did = std::max(did, lhead);
	    ret2 = ret3->sync_and_skip_to(did, w_min, lhead, rhead);
	    ret = ret3;
	}

	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	RETURN(ret);
    }

    bool ldry = false;
    if (lhead < did) {
	skip_to_handling_prune(l, did, 0, matcher);
	ldry = l->at_end();
    }

    if (rhead < did) {
	skip_to_handling_prune(r, did, 0, matcher);

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    RETURN(ret);
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	RETURN(ret);
    }

    lhead = l->get_docid();
    RETURN(advance_to_next_match());
}

Xapian::doccount
XorPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_max", "");
    return l->get_termfreq_max() + r->get_termfreq_max();
}

Xapian::doccount
XorPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_min", "");
    // Min = freq_min(a or b) - freq_max(a and b)
    //     = max(a_min, b_min) - min(a_max, b_max)
    //     = min(b_min - a_max, a_min - b_max)
    Xapian::doccount r_min = r->get_termfreq_min();
    Xapian::doccount l_min = l->get_termfreq_min();
    Xapian::doccount r_max = r->get_termfreq_max();
    Xapian::doccount l_max = l->get_termfreq_max();
    Xapian::doccount termfreq_min = 0u;
    if (r_min > l_max)
	termfreq_min = r_min - l_max;
    if (l_min > r_max && (l_min - r_max) > termfreq_min)
	termfreq_min = l_min - r_max;

    return termfreq_min;
}

Xapian::doccount
XorPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, Xapian::doccount, "XorPostList::get_termfreq_est", "");
    // Estimate assuming independence:
    // P(l xor r) = P(l) + P(r) - 2 . P(l) . P(r)
    double lest = static_cast<double>(l->get_termfreq_est());
    double rest = static_cast<double>(r->get_termfreq_est());
    double est = lest + rest - (2.0 * lest * rest / dbsize);
    RETURN(static_cast<Xapian::doccount>(est + 0.5));
}

TermFreqs
XorPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const 
{
    LOGCALL(MATCH, TermFreqs,
	    "XorPostList::get_termfreq_est_using_stats", stats);
    // Estimate assuming independence:
    // P(l or r) = P(l) + P(r) - 2 . P(l) . P(r)
    TermFreqs lfreqs(l->get_termfreq_est_using_stats(stats));
    TermFreqs rfreqs(r->get_termfreq_est_using_stats(stats));

    double freqest, relfreqest;

    // Our caller should have ensured this.
    Assert(stats.collection_size);

    freqest = lfreqs.termfreq + rfreqs.termfreq
	    - (2.0 * lfreqs.termfreq * rfreqs.termfreq
	       / stats.collection_size);

    if (stats.rset_size == 0) {
	relfreqest = 0;
    } else {
	relfreqest = lfreqs.reltermfreq + rfreqs.reltermfreq
		- (2.0 * lfreqs.reltermfreq * rfreqs.reltermfreq
		   / stats.rset_size);
    }

    RETURN(TermFreqs(static_cast<Xapian::doccount>(freqest + 0.5),
		     static_cast<Xapian::doccount>(relfreqest + 0.5)));
}

Xapian::docid
XorPostList::get_docid() const
{
    DEBUGCALL(MATCH, Xapian::docid, "XorPostList::get_docid", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    return std::min(lhead, rhead);
}

// only called if we are doing a probabilistic XOR
Xapian::weight
XorPostList::get_weight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::get_weight", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_weight();
    Assert(lhead > rhead);
    return r->get_weight();
}

// only called if we are doing a probabilistic operation
Xapian::weight
XorPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::get_maxweight", "");
    return std::max(lmax, rmax);
}

Xapian::weight
XorPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, Xapian::weight, "XorPostList::recalc_maxweight", "");
    // l and r cannot be NULL here, because the only place where they get set
    // to NULL is when the tree is decaying, and the XorPostList is then
    // immediately replaced.
    lmax = l->recalc_maxweight();
    rmax = r->recalc_maxweight();
    minmax = std::min(lmax, rmax);
    return XorPostList::get_maxweight();
}

bool
XorPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "XorPostList::at_end", "");
    return lhead == 0;
}

std::string
XorPostList::get_description() const
{
    return "(" + l->get_description() + " Xor " + r->get_description() + ")";
}

Xapian::termcount
XorPostList::get_doclength() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "XorPostList::get_doclength", "");
    Assert(lhead != 0 && rhead != 0); // check we've started
    if (lhead < rhead) return l->get_doclength();
    Assert(lhead > rhead);
    return r->get_doclength();
}

Xapian::termcount
XorPostList::get_wdf() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "XorPostList::get_wdf", "");
    if (lhead < rhead) RETURN(l->get_wdf());
    RETURN(r->get_wdf());
}

Xapian::termcount
XorPostList::count_matching_subqs() const
{
    DEBUGCALL(MATCH, Xapian::termcount, "XorPostList::count_matching_subqs", "");
    if (lhead < rhead) RETURN(l->count_matching_subqs());
    RETURN(r->count_matching_subqs());
}
