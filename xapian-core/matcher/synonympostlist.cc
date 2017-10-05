/** @file synonympostlist.cc
 * @brief Combine subqueries, weighting as if they are synonyms
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2014,2016,2017 Olly Betts
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

#include "debuglog.h"
#include "omassert.h"

using namespace std;

SynonymPostList::~SynonymPostList()
{
    delete wt;
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
    RETURN(WrapperPostList::next(0.0));
}

PostList *
SynonymPostList::skip_to(Xapian::docid did, double w_min)
{
    LOGCALL(MATCH, PostList *, "SynonymPostList::skip_to", did | w_min);
    (void)w_min;
    RETURN(WrapperPostList::skip_to(did, 0.0));
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

    Xapian::docid did = 0;
    Xapian::termcount unique_terms = 0;
    Xapian::termcount doclen = 0;
    if (want_unique_terms) {
	did = pl->get_docid();
	unique_terms = db->get_unique_terms(did);
    }
    if (want_wdf) {
	Xapian::termcount wdf = get_wdf();
	if (want_doclength || wdf > doclen_lower_bound) {
	    if (did == 0)
		did = pl->get_docid();
	    doclen = db->get_doclength(did);
	    if (wdf > doclen) wdf = doclen;
	}
	double sumpart = wt->get_sumpart(wdf, doclen, unique_terms);
	AssertRel(sumpart, <=, wt->get_maxpart());
	RETURN(sumpart);
    }
    if (want_doclength) {
	if (did == 0)
	    did = pl->get_docid();
	doclen = db->get_doclength(did);
    }
    RETURN(wt->get_sumpart(0, doclen, unique_terms));
}

double
SynonymPostList::recalc_maxweight()
{
    LOGCALL(MATCH, double, "SynonymPostList::recalc_maxweight", NO_ARGS);

    // Call recalc_maxweight on the subtree once, to ensure that the maxweights
    // are initialised.
    if (!have_calculated_subtree_maxweights) {
	pl->recalc_maxweight();
	have_calculated_subtree_maxweights = true;
    }
    RETURN(wt->get_maxpart());
}

Xapian::termcount
SynonymPostList::count_matching_subqs() const
{
    return 1;
}

string
SynonymPostList::get_description() const
{
    string desc = "SynonymPostList(";
    desc += pl->get_description();
    desc += ')';
    return desc;
}
