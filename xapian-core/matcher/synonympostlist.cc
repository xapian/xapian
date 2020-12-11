/** @file
 * @brief Combine subqueries, weighting as if they are synonyms
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2014,2016,2018 Olly Betts
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

#include "synonympostlist.h"

#include "branchpostlist.h"
#include "debuglog.h"
#include "omassert.h"

SynonymPostList::~SynonymPostList()
{
    delete wt;
    delete subtree;
}

void
SynonymPostList::set_weight(const Xapian::Weight * wt_)
{
    delete wt;
    wt = wt_;
    want_doclength = wt->get_sumpart_needs_doclength_();
    want_wdf = wt->get_sumpart_needs_wdf_();
    want_unique_terms = wt->get_sumpart_needs_uniqueterms_();
}

PostList *
SynonymPostList::next(double w_min)
{
    LOGCALL(MATCH, PostList *, "SynonymPostList::next", w_min);
    (void)w_min;
    next_handling_prune(subtree, 0, matcher);
    RETURN(NULL);
}

PostList *
SynonymPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "SynonymPostList::skip_to", did | w_min);
    (void)w_min;
    skip_to_handling_prune(subtree, did, 0, matcher);
    RETURN(NULL);
}

double
SynonymPostList::get_weight() const
{
    LOGCALL(MATCH, double, "SynonymPostList::get_weight", NO_ARGS);
    // The wdf returned can be higher than the doclength.  In particular, this
    // can currently occur if the query contains a term more than once; the wdf
    // of each occurrence is added up.
    //
    // However, it's reasonable for weighting algorithms to optimise by
    // assuming that get_wdf() will never return more than get_doclength(),
    // since the doclength is the sum of the wdfs.
    //
    // Therefore, we simply clamp the wdf value to the doclength, to ensure
    // that this is true.  Note that this requires the doclength to be
    // calculated even if the weight object doesn't want it.

    Xapian::termcount unique_terms = 0;
    if (want_unique_terms)
	unique_terms = get_unique_terms();
    if (want_wdf) {
	Xapian::termcount wdf = get_wdf();
	Xapian::termcount doclen = 0;
	if (want_doclength || (!wdf_disjoint && wdf > doclen_lower_bound)) {
	    doclen = get_doclength();
	    if (wdf > doclen) wdf = doclen;
	}
	double sumpart = wt->get_sumpart(wdf, doclen, unique_terms);
	AssertRel(sumpart, <=, wt->get_maxpart());
	RETURN(sumpart);
    }
    Xapian::termcount doclen = want_doclength ? get_doclength() : 0;
    RETURN(wt->get_sumpart(0, doclen, unique_terms));
}

double
SynonymPostList::get_maxweight() const
{
    LOGCALL(MATCH, double, "SynonymPostList::get_maxweight", NO_ARGS);
    RETURN(wt->get_maxpart());
}

double
SynonymPostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "SynonymPostList::recalc_maxweight", NO_ARGS);
    RETURN(SynonymPostList::get_maxweight());
}

Xapian::termcount
SynonymPostList::get_wdf() const {
    LOGCALL(MATCH, Xapian::termcount, "SynonymPostList::get_wdf", NO_ARGS);
    RETURN(subtree->get_wdf());
}

Xapian::doccount
SynonymPostList::get_termfreq_min() const {
    LOGCALL(MATCH, Xapian::doccount, "SynonymPostList::get_termfreq_min", NO_ARGS);
    RETURN(subtree->get_termfreq_min());
}

Xapian::doccount
SynonymPostList::get_termfreq_est() const {
    LOGCALL(MATCH, Xapian::doccount, "SynonymPostList::get_termfreq_est", NO_ARGS);
    RETURN(subtree->get_termfreq_est());
}

Xapian::doccount
SynonymPostList::get_termfreq_max() const {
    LOGCALL(MATCH, Xapian::doccount, "SynonymPostList::get_termfreq_max", NO_ARGS);
    RETURN(subtree->get_termfreq_max());
}

Xapian::docid
SynonymPostList::get_docid() const {
    LOGCALL(MATCH, Xapian::docid, "SynonymPostList::get_docid", NO_ARGS);
    RETURN(subtree->get_docid());
}

Xapian::termcount
SynonymPostList::get_doclength() const {
    LOGCALL(MATCH, Xapian::termcount, "SynonymPostList::get_doclength", NO_ARGS);
    RETURN(subtree->get_doclength());
}

Xapian::termcount
SynonymPostList::get_unique_terms() const {
    LOGCALL(MATCH, Xapian::termcount, "SynonymPostList::get_unique_terms", NO_ARGS);
    RETURN(subtree->get_unique_terms());
}

bool
SynonymPostList::at_end() const {
    LOGCALL(MATCH, bool, "SynonymPostList::at_end", NO_ARGS);
    RETURN(subtree->at_end());
}

Xapian::termcount
SynonymPostList::count_matching_subqs() const
{
    return 1;
}

std::string
SynonymPostList::get_description() const
{
    return "(Synonym " + subtree->get_description() + ")";
}
