/** @file localsubmatch.cc
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006,2007,2009,2010,2014 Olly Betts
 * Copyright (C) 2007,2008,2009 Lemur Consulting Ltd
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

#include "localsubmatch.h"

#include "database.h"
#include "debuglog.h"
#include "extraweightpostlist.h"
#include "leafpostlist.h"
#include "omassert.h"
#include "omqueryinternal.h"
#include "queryoptimiser.h"
#include "synonympostlist.h"
#include "termlist.h"
#include "weightinternal.h"

#include "autoptr.h"
#include <map>
#include <string>

using namespace std;

bool
LocalSubMatch::prepare_match(bool nowait,
			     Xapian::Weight::Internal & total_stats)
{
    LOGCALL(MATCH, bool, "LocalSubMatch::prepare_match", nowait | total_stats);
    (void)nowait;
    Assert(db);
    Assert(query);
    total_stats.accumulate_stats(*db, rset);
    RETURN(true);
}

void
LocalSubMatch::start_match(Xapian::doccount first,
			   Xapian::doccount maxitems,
			   Xapian::doccount check_at_least,
			   const Xapian::Weight::Internal & total_stats)
{
    LOGCALL_VOID(MATCH, "LocalSubMatch::start_match", first | maxitems | check_at_least | total_stats);
    (void)first;
    (void)maxitems;
    (void)check_at_least;
    // Store a pointer to the total stats to use when building the Query tree.
    stats = &total_stats;
}

PostList *
LocalSubMatch::get_postlist_and_term_info(MultiMatch * matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> * termfreqandwts,
	Xapian::termcount * total_subqs_ptr)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::get_postlist_and_term_info", matcher | termfreqandwts | total_subqs_ptr);
    (void)matcher;
    term_info = termfreqandwts;

    // Build the postlist tree for the query.  This calls
    // LocalSubMatch::postlist_from_op_leaf_query() for each term in the query,
    // which builds term_info as a side effect.
    QueryOptimiser opt(*db, *this, matcher);
    PostList * pl = opt.optimise_query(query);
    *total_subqs_ptr = opt.get_total_subqueries();

    AutoPtr<Xapian::Weight> extra_wt(wt_factory->clone());
    extra_wt->init_(*stats, qlen);
    if (extra_wt->get_maxextra() != 0.0) {
	// There's a term-independent weight contribution, so we combine the
	// postlist tree with an ExtraWeightPostList which adds in this
	// contribution.
	pl = new ExtraWeightPostList(pl, extra_wt.release(), matcher);
    }

    RETURN(pl);
}

PostList *
LocalSubMatch::make_synonym_postlist(PostList * or_pl, MultiMatch * matcher,
				     double factor)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::make_synonym_postlist", or_pl | matcher | factor);
    LOGVALUE(MATCH, or_pl->get_termfreq_est());
    Xapian::termcount len_lb = db->get_doclength_lower_bound();
    AutoPtr<SynonymPostList> res(new SynonymPostList(or_pl, matcher, len_lb));
    AutoPtr<Xapian::Weight> wt(wt_factory->clone());

    TermFreqs freqs;
    // Avoid calling get_termfreq_est_using_stats() if the database is empty
    // so we don't need to special case that repeatedly when implementing it.
    // FIXME: it would be nicer to handle an empty database higher up, though
    // we need to catch the case where all the non-empty subdatabases have
    // failed, so we can't just push this right up to the start of get_mset().
    if (usual(stats->collection_size != 0)) {
	freqs = or_pl->get_termfreq_est_using_stats(*stats);
    }
    wt->init_(*stats, qlen, factor, freqs.termfreq, freqs.reltermfreq);

    res->set_weight(wt.release());
    RETURN(res.release());
}

PostList *
LocalSubMatch::postlist_from_op_leaf_query(const Xapian::Query::Internal *leaf,
					   double factor)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::postlist_from_op_leaf_query", leaf | factor);
    Assert(leaf);
    AssertEq(leaf->op, Xapian::Query::Internal::OP_LEAF);
    Assert(leaf->subqs.empty());
    const string & term = leaf->tname;
    bool boolean = (factor == 0.0);
    AutoPtr<Xapian::Weight> wt;
    if (!boolean) {
	wt.reset(wt_factory->clone());
	wt->init_(*stats, qlen, term, leaf->get_wqf(), factor);
    }

    if (term_info) {
	Xapian::doccount tf = stats->get_termfreq(term);
	using namespace Xapian;
	// Find existing entry for term, or else make a new one.
	map<string, MSet::Internal::TermFreqAndWeight>::iterator i;
	i = term_info->insert(
		make_pair(term, MSet::Internal::TermFreqAndWeight(tf))).first;
	if (!boolean)
	    i->second.termweight += wt->get_maxpart();
    }

    LeafPostList * pl = db->open_post_list(term);
    // The default for LeafPostList is to return 0 weight and maxweight which
    // is the same as boolean weighting.
    if (!boolean)
	pl->set_termweight(wt.release());
    RETURN(pl);
}
