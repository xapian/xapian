/* localmatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

#include "localmatch.h"

#include "autoptr.h"
#include "extraweightpostlist.h"
#include "leafpostlist.h"
#include "omdebug.h"
#include "omqueryinternal.h"
#include "queryoptimiser.h"
#include "scaleweight.h"

#include <cfloat>
#include <cmath>
#include <map>

LocalSubMatch::LocalSubMatch(const Xapian::Database::Internal *db_,
	const Xapian::Query::Internal * query, Xapian::termcount qlen_,
	const Xapian::RSet & omrset, StatsGatherer *gatherer_,
	const Xapian::Weight *wt_factory_)
	: gatherer(gatherer_), statssource(gatherer_),
	  orig_query(*query), qlen(qlen_), db(db_),
	  rset(db, omrset), wt_factory(wt_factory_)
{
    DEBUGCALL(MATCH, void, "LocalSubMatch::LocalSubMatch",
	      db << ", " << query << ", " << qlen_ << ", " << omrset << ", " <<
	      gatherer_ << ", [wt_factory]");

    statssource.take_my_stats(db->get_doccount(), db->get_avlength());
}

void
LocalSubMatch::register_term(const string &tname)
{
    if (tname.empty()) {
	statssource.my_termfreq_is(tname, db->get_doccount());
    } else {
	statssource.my_termfreq_is(tname, db->get_termfreq(tname));
    }
}

bool
LocalSubMatch::prepare_match(bool /*nowait*/)
{
    DEBUGCALL(MATCH, bool, "LocalSubMatch::prepare_match", "/*nowait*/");
    Xapian::TermIterator terms = orig_query.get_terms();
    Xapian::TermIterator terms_end(NULL);
    for ( ; terms != terms_end; ++terms) {
	// MULTI
	register_term(*terms);
	rset.will_want_reltermfreq(*terms);
    }

    rset.give_stats_to_statssource(&statssource);
    RETURN(true);
}

void
LocalSubMatch::start_match(Xapian::doccount, Xapian::doccount, Xapian::doccount)
{
    // Nothing to do here for a local match.
}

PostList *
LocalSubMatch::get_postlist_and_term_info(MultiMatch * matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> * termfreqandwts)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::get_postlist_and_term_info",
	      matcher << ", [termfreqandwts]");

    // First, get the statistics into the statssource.
    statssource.set_total_stats(gatherer->get_stats());

    // Build the postlist tree for the query.  This calls
    // LocalSubMatch::postlist_from_op_leaf_query() for each term in the query,
    // which builds term_info as a side effect.
    QueryOptimiser opt(*db, *this, matcher);
    PostList * pl = opt.optimise_query(&orig_query);
    if (termfreqandwts) *termfreqandwts = term_info;

    // We only need an ExtraWeightPostList if there's an extra weight
    // contribution.
    AutoPtr<Xapian::Weight> extra_wt;
    extra_wt = wt_factory->create(&statssource, qlen, 1, "");
    if (extra_wt->get_maxextra() != 0.0) {
	pl = new ExtraWeightPostList(pl, extra_wt.release(), matcher);
    }

    RETURN(pl);
}

PostList *
LocalSubMatch::postlist_from_op_leaf_query(const Xapian::Query::Internal *query,
					   double factor)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::postlist_from_op_leaf_query",
	      query << ", " << factor);
    Assert(query);
    AssertEq(query->op, Xapian::Query::Internal::OP_LEAF);
    Assert(query->subqs.empty());
    bool boolean = (factor == 0.0);
    AutoPtr<Xapian::Weight> wt;
    if (!boolean) {
	// FIXME:
	// pass factor to Weight::create() - and have a shim class for classes
	// which don't understand it...
	Xapian::termcount wqf = query->wqf;
	wt = wt_factory->create(&statssource, qlen, wqf, query->tname);
	if (fabs(factor - 1.0) > DBL_EPSILON) {
	    wt = new ScaleWeight(wt.release(), factor);
	}
    }

    map<string, Xapian::MSet::Internal::TermFreqAndWeight>::iterator i;
    i = term_info.find(query->tname);
    if (i == term_info.end()) {
	Xapian::doccount tf = statssource.get_total_termfreq(query->tname);
	Xapian::weight weight = boolean ? 0 : wt->get_maxpart();
	Xapian::MSet::Internal::TermFreqAndWeight info(tf, weight);
	term_info.insert(make_pair(query->tname, info));
    } else if (!boolean) {
	i->second.termweight += wt->get_maxpart();
    }

    LeafPostList * pl = db->open_post_list(query->tname);
    // The default for LeafPostList is to return 0 weight and maxweight which
    // is the same as boolean weighting.
    if (!boolean) pl->set_termweight(wt.release());
    RETURN(pl);
}
