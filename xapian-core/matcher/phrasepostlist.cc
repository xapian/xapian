/* phrasepostlist.cc: Return only items where terms are near or form a phrase
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2007,2008,2009 Olly Betts
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
#include "phrasepostlist.h"

#include "debuglog.h"
#include "positionlist.h"
#include "omassert.h"
#include "str.h"

#include <algorithm>

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PositionListCmpLt {
    public:
	/** Return true if and only if a is strictly shorter than b.
	 */
        bool operator()(const PositionList *a, const PositionList *b) {
            return a->get_size() < b->get_size();
        }
};


/** Check if terms occur sufficiently close together in the current doc
 */
bool
NearPostList::test_doc()
{
    LOGCALL(MATCH, bool, "NearPostList::test_doc", NO_ARGS);
    std::vector<PositionList *> plists;

    std::vector<PostList *>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	PositionList * p = (*i)->read_position_list();
	// If p is NULL, the backend doesn't support positionlists
	if (!p) return false;
	plists.push_back(p);
    }

    std::sort(plists.begin(), plists.end(), PositionListCmpLt());

    Xapian::termpos pos;
    do {
	plists[0]->next();
	if (plists[0]->at_end()) RETURN(false);
	pos = plists[0]->get_position();
    } while (!do_test(plists, 1, pos, pos));

    RETURN(true);
}

bool
NearPostList::do_test(std::vector<PositionList *> &plists, Xapian::termcount i,
		      Xapian::termcount min, Xapian::termcount max)
{
    LOGCALL(MATCH, bool, "NearPostList::do_test", plists | i | min | max);
    LOGLINE(MATCH, "docid = " << get_docid() << ", window = " << window);
    Xapian::termcount tmp = max + 1;
    // take care to avoid underflow
    if (window <= tmp) tmp -= window; else tmp = 0;
    plists[i]->skip_to(tmp);
    while (!plists[i]->at_end()) {
	Xapian::termpos pos = plists[i]->get_position();
	LOGLINE(MATCH, "[" << i << "]: " << max - window + 1 << " " << min <<
		       " " << pos << " " << max << " " << min + window - 1);
	if (pos > min + window - 1) RETURN(false);
	if (i + 1 == plists.size()) RETURN(true);
	if (pos < min) min = pos;
	else if (pos > max) max = pos;
	if (do_test(plists, i + 1, min, max)) RETURN(true);
	plists[i]->next();
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

    std::vector<PostList *>::const_iterator i = terms.begin();
    Xapian::termcount wdf = (*i)->get_wdf();
    for (; i != terms.end(); ++i) {
	wdf = std::min(wdf, (*i)->get_wdf());
    }

    // Ensure that we always return a wdf of at least 1, since we know there
    // was at least one occurrence of the phrase.
    return std::max(wdf, Xapian::termcount(1));
}

TermFreqs
NearPostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "NearPostList::get_termfreq_est_using_stats", stats);
    // No idea how to estimate this - FIXME
    TermFreqs result(source->get_termfreq_est_using_stats(stats));
    result.termfreq /= 2;
    result.reltermfreq /= 2;
    RETURN(result);
}

std::string
NearPostList::get_description() const
{
    return "(Near " + str(window) + " " + source->get_description() + ")";
}



/** Check if terms form a phrase in the current doc
 */
bool
PhrasePostList::test_doc()
{
    LOGCALL(MATCH, bool, "PhrasePostList::test_doc", NO_ARGS);
    std::vector<PositionList *> plists;

    std::vector<PostList *>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	PositionList * p = (*i)->read_position_list();
	// If p is NULL, the backend doesn't support positionlists
	if (!p) return false;
	p->index = i - terms.begin();
	plists.push_back(p);
    }

    std::sort(plists.begin(), plists.end(), PositionListCmpLt());

    Xapian::termpos pos;
    Xapian::termpos idx, min, max;
    do {
	plists[0]->next();
	if (plists[0]->at_end()) {
	    LOGLINE(MATCH, "--MISS--");
	    RETURN(false);
	}
	pos = plists[0]->get_position();
	idx = plists[0]->index;
	if (idx == 0) {
	    min = pos;
	} else {
	    min = pos + plists.size() - idx;
	    if (min > window) min -= window; else min = 0;
	}
	if (idx == plists.size() - 1) {
	    max = pos + 1;
	} else {
	    max = pos + window - idx;
	}
    } while (!do_test(plists, 1, min, max));
    LOGLINE(MATCH, "**HIT**");
    RETURN(true);
}

bool
PhrasePostList::do_test(std::vector<PositionList *> &plists, Xapian::termcount i,
			Xapian::termcount min, Xapian::termcount max)
{
    LOGCALL(MATCH, bool, "PhrasePostList::do_test", plists | i | min | max);
    LOGLINE(MATCH, "docid = " << get_docid() << ", window = " << window);
    Xapian::termpos idxi = plists[i]->index;
    LOGLINE(MATCH, "my idx in phrase is " << idxi);

    Xapian::termpos mymin = min + idxi;
    Xapian::termpos mymax = max - plists.size() + idxi;
    LOGLINE(MATCH, "MIN = " << mymin << " MAX = " << mymax);
    // FIXME: this is worst case O(n^2) where n = length of phrase
    // Can we do better?
    for (Xapian::termcount j = 0; j < i; j++) {
	Xapian::termpos idxj = plists[j]->index;
	if (idxj > idxi) {
	    Xapian::termpos tmp = plists[j]->get_position() + idxj - idxi;
	    LOGLINE(MATCH, "ABOVE " << tmp);
	    if (tmp < mymax) mymax = tmp;
	} else {
	    AssertRel(idxi, !=, idxj);
	    Xapian::termpos tmp = plists[j]->get_position() + idxi - idxj;
	    LOGLINE(MATCH, "BELOW " << tmp);
	    if (tmp > mymin) mymin = tmp;
	}
	LOGLINE(MATCH, "min = " << mymin << " max = " << mymax);
    }
    plists[i]->skip_to(mymin);

    while (!plists[i]->at_end()) {
	Xapian::termpos pos = plists[i]->get_position();
	LOGLINE(MATCH, " " << mymin << " " << pos << " " << mymax);
	if (pos > mymax) RETURN(false);
	if (i + 1 == plists.size()) RETURN(true);
	Xapian::termpos tmp = pos + window - idxi;
	if (tmp < max) max = tmp;
	tmp = pos + plists.size() - idxi;
	if (tmp > window) {
	    tmp -= window;
	    if (tmp > min) min = tmp;
	}
	if (do_test(plists, i + 1, min, max)) RETURN(true);
	plists[i]->next();
    }
    RETURN(false);
}

Xapian::termcount
PhrasePostList::get_wdf() const
{
    // Calculate an estimate for the wdf of a phrase postlist.
    //
    // We use the minimum wdf of a sub-postlist as our estimate.  See the
    // comment in NearPostList::get_wdf for justification of this estimate.
    //
    // We divide the value calculated for a NearPostList by 2, as a very rough
    // heuristic to represent the fact that the words must occur in order, and
    // phrases are therefore rarer than near matches.

    std::vector<PostList *>::const_iterator i = terms.begin();
    Xapian::termcount wdf = (*i)->get_wdf();
    for (; i != terms.end(); ++i) {
	wdf = std::min(wdf, (*i)->get_wdf());
    }

    // Ensure that we always return a wdf of at least 1, since we know there
    // was at least one occurrence of the phrase.
    return std::max(wdf / 2, Xapian::termcount(1));
}

TermFreqs
PhrasePostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "PhrasePostList::get_termfreq_est_using_stats", stats);
    // No idea how to estimate this - FIXME
    TermFreqs result(source->get_termfreq_est_using_stats(stats));
    result.termfreq /= 3;
    result.reltermfreq /= 3;
    RETURN(result);
}

std::string
PhrasePostList::get_description() const
{
    return "(Phrase " + str(window) + " "
	   + source->get_description() + ")";
}
