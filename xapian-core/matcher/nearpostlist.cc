/** @file
 * @brief Return docs containing terms within a specified window.
 */
/* Copyright (C) 2006,2007,2009,2010,2011,2014,2015,2017 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "nearpostlist.h"

#include "debuglog.h"
#include "backends/positionlist.h"
#include "omassert.h"
#include "str.h"

#include <algorithm>
#include <vector>

using namespace std;

NearPostList::NearPostList(PostList *source_,
			   Xapian::termpos window_,
			   const vector<PostList*>::const_iterator &terms_begin,
			   const vector<PostList*>::const_iterator &terms_end)
    : SelectPostList(source_), window(window_), terms(terms_begin, terms_end)
{
    size_t n = terms.size();
    Assert(n > 1);
    poslists = new PositionList*[n];
}

NearPostList::~NearPostList()
{
    delete [] poslists;
}

struct TermCmp {
    bool operator()(const PostList * a, const PostList * b) const {
	return a->get_wdf() < b->get_wdf();
    }
};

struct Cmp {
    bool operator()(const PositionList * a, const PositionList * b) const {
	return a->get_position() > b->get_position();
    }
};

bool
NearPostList::test_doc()
{
    LOGCALL(MATCH, bool, "NearPostList::test_doc", NO_ARGS);

    // Sort to put least frequent terms first, to try to minimise the number of
    // position lists we need to read if there are no matches.
    //
    // We use wdf as a proxy for the length of the position lists, since we'd
    // need to read each position list to find its length and we're trying to
    // avoid having to read them all if we can.
    sort(terms.begin(), terms.end(), TermCmp());

    poslists[0] = terms[0]->read_position_list();
    if (!poslists[0]->next())
	RETURN(false);

    Xapian::termpos last = poslists[0]->get_position();
    PositionList ** end = poslists + 1;

    while (true) {
	if (last - poslists[0]->get_position() < window) {
	    if (size_t(end - poslists) != terms.size()) {
		// We haven't started all the position lists yet, so start the
		// next one.
		PositionList * posl =
		    terms[end - poslists]->read_position_list();
		if (last < window) {
		    if (!posl->next())
			RETURN(false);
		} else {
		    if (!posl->skip_to(last - window + 1))
			RETURN(false);
		}
		Xapian::termpos pos = posl->get_position();
		if (pos > last) last = pos;
		*end++ = posl;
		push_heap<PositionList **, Cmp>(poslists, end, Cmp());
		continue;
	    }

	    // We have all the terms within the specified window, but some may
	    // be at the same position (in particular if the same term is
	    // listed twice).  So we work through the terms in ascending
	    // position order, and each time we find one with a duplicate
	    // position, we advance it - if that pushes us out of the window,
	    // we return to the outer loop, otherwise we reinsert it into the
	    // heap at its new position and continue to look for duplicates
	    // we need to adjust.
	    PositionList ** i = end;
	    pop_heap<PositionList **, Cmp>(poslists, i, Cmp());
	    Xapian::termpos pos = (*--i)->get_position();
	    while (true) {
		pop_heap<PositionList **, Cmp>(poslists, i, Cmp());
		if ((*--i)->get_position() == pos) {
		    if (!(*i)->next())
			RETURN(false);
		    Xapian::termpos newpos = (*i)->get_position();
		    if (newpos - end[-1]->get_position() >= window) {
			// No longer fits in the window.
			last = newpos;
			break;
		    }
		    push_heap<PositionList **, Cmp>(poslists, ++i, Cmp());
		    continue;
		}
		pos = (*i)->get_position();
		if (i == poslists) {
		    Assert(pos - end[-1]->get_position() < window);
		    RETURN(true);
		}
	    }

	    make_heap<PositionList **, Cmp>(poslists, end, Cmp());
	    continue;
	}
	pop_heap<PositionList **, Cmp>(poslists, end, Cmp());
	if (!end[-1]->skip_to(last - window + 1))
	    break;
	last = max(last, end[-1]->get_position());
	push_heap<PositionList **, Cmp>(poslists, end, Cmp());
    }

    RETURN(false);
}

Xapian::termcount
NearPostList::get_wdf() const
{
    // Calculate an estimate for the wdf of a near postlist.
    //
    // The natural definition of the wdf for a near postlist is the number of
    // times the terms occur in a group within the windowsize in the document -
    // in other words, the number of times a routine like do_test() could find
    // a match, if it kept going after finding the first one.  However,
    // calculating this would be expensive (in CPU time at least - the position
    // lists are probably already read from disk), and the benefit is dubious
    // (given that the estimate here is probably only going to be used for
    // calculating the weight for synonym postlists, and it's not clear that
    // the above definition is exactly appropriate in this situation, anyway).
    //
    // Instead, we could do an estimate of this value, based on the lengths
    // of the position lists.  Rather than having to open the position lists,
    // we could use the wdfs, which will be the same value unless the wdfs have
    // been artificially inflated - in which case we probably want to use the
    // inflated value anyway (it will have been inflated to represent the fact
    // that this term is more important than others, so we should make use of
    // this hint).
    //
    // A reasonable way to calculate an estimate would be to consider the
    // document to be split into W (overlapping) windows, each 1 term apart,
    // and of the same length as the windowsize used for the phrase.  Then,
    // calculate the probability that each term is found in this window (as
    // Prob = wdf / doclen), multiply these probabilities to get the
    // probability that they're all found in the window, and then multiply by
    // the windowsize again to get an estimate.
    //
    // However, this requires the doclen, which won't always be available (ie,
    // if the weighting scheme doesn't use it, we won't have read it from
    // disk).
    //
    // Rather than force an access to disk to get the doclen, we use an even
    // rougher estimate: the minimum wdf in the sub-lists.  This is actually
    // the upper bound for the wdf (since the phrase can only occur the same
    // number of times as the least frequent of its constituent terms).
    //
    // If this estimate proves to give bad results, we can revisit this and try
    // a better approximation.
    vector<PostList *>::const_iterator i = terms.begin();
    Xapian::termcount wdf = (*i)->get_wdf();
    while (++i != terms.end()) {
	wdf = min(wdf, (*i)->get_wdf());
    }
    return wdf;
}

Xapian::doccount
NearPostList::get_termfreq_est() const
{
    // It's hard to estimate how many times the postlist will match as it
    // depends a lot on the terms and window, but usually it will occur
    // significantly less often than the individual terms.
    return source->get_termfreq_est() / 2;
}

TermFreqs
NearPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "NearPostList::get_termfreq_est_using_stats", stats);
    // No idea how to estimate this - do the same as get_termfreq_est() for
    // now.
    TermFreqs result(source->get_termfreq_est_using_stats(stats));
    result.termfreq /= 2;
    result.reltermfreq /= 2;
    result.collfreq /= 2;
    RETURN(result);
}

string
NearPostList::get_description() const
{
    string m = "(Near ";
    m += str(window);
    m += ' ';
    m += source->get_description();
    m += ")";
    return m;
}
