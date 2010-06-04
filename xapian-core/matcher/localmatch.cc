/* localmatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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
#include "debuglog.h"
#include "extraweightpostlist.h"
#include "leafpostlist.h"
#include "omassert.h"
#include "omqueryinternal.h"
#include "queryoptimiser.h"
#include "rset.h"
#include "synonympostlist.h"
#include "weightinternal.h"

#include <cfloat>
#include <cmath>
#include <map>

using namespace std;

LocalSubMatch::LocalSubMatch(const Xapian::Database::Internal *db_,
	const Xapian::Query::Internal * query, Xapian::termcount qlen_,
	const Xapian::RSet & omrset_,
	const Xapian::Weight *wt_factory_)
	: orig_query(*query), qlen(qlen_), db(db_), omrset(omrset_),
	  wt_factory(wt_factory_), term_info(NULL)
{
    LOGCALL_VOID(MATCH, "LocalSubMatch::LocalSubMatch", db | query | qlen_ | omrset_ | wt_factory);
}

bool
LocalSubMatch::prepare_match(bool, Xapian::Weight::Internal & total_stats)
{
    LOGCALL(MATCH, bool, "LocalSubMatch::prepare_match", "[nowait]" | total_stats);
    Xapian::Weight::Internal my_stats;

    // Set the collection statistics.
    my_stats.total_length = db->get_total_length();
    my_stats.collection_size = db->get_doccount();

    RSetI rset(db, omrset);
    // Get the term-frequencies and relevant term-frequencies.
    Xapian::TermIterator titer = orig_query.get_terms();
    Xapian::TermIterator terms_end(NULL);
    for ( ; titer != terms_end; ++titer) {
	Assert(!(*titer).empty());
	my_stats.set_termfreq(*titer, db->get_termfreq(*titer));
	rset.will_want_reltermfreq(*titer);
    }
    rset.contribute_stats(my_stats);

    // Contribute the calculated statistics.
    total_stats += my_stats;
    RETURN(true);
}

void
LocalSubMatch::start_match(Xapian::doccount, Xapian::doccount,
			   Xapian::doccount,
			   const Xapian::Weight::Internal & total_stats)
{
    // Set the statistics for the whole collection.
    stats = &total_stats;
}

PostList *
LocalSubMatch::get_postlist_and_term_info(MultiMatch * matcher,
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> * termfreqandwts,
	Xapian::termcount * total_subqs_ptr)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::get_postlist_and_term_info", matcher | termfreqandwts | total_subqs_ptr);
    term_info = termfreqandwts;

    // Build the postlist tree for the query.  This calls
    // LocalSubMatch::postlist_from_op_leaf_query() for each term in the query,
    // which builds term_info as a side effect.
    QueryOptimiser opt(*db, *this, matcher);
    PostList * pl = opt.optimise_query(&orig_query);
    *total_subqs_ptr = opt.get_total_subqueries();

    // We only need an ExtraWeightPostList if there's an extra weight
    // contribution.
    AutoPtr<Xapian::Weight> extra_wt(wt_factory->clone());
    extra_wt->init_(*stats, qlen);
    if (extra_wt->get_maxextra() != 0.0) {
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
    AutoPtr<SynonymPostList> res(new SynonymPostList(or_pl, matcher));
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
LocalSubMatch::postlist_from_op_leaf_query(const Xapian::Query::Internal *query,
					   double factor)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::postlist_from_op_leaf_query", query | factor);
    Assert(query);
    AssertEq(query->op, Xapian::Query::Internal::OP_LEAF);
    Assert(query->subqs.empty());
    bool boolean = (factor == 0.0);
    AutoPtr<Xapian::Weight> wt;
    if (!boolean) {
	wt.reset(wt_factory->clone());
	wt->init_(*stats, qlen, query->tname, query->get_wqf(), factor);
    }

    if (term_info) {
	map<string, Xapian::MSet::Internal::TermFreqAndWeight>::iterator i;
	i = term_info->find(query->tname);
	if (i == term_info->end()) {
	    Xapian::doccount tf = stats->get_termfreq(query->tname);
	    Xapian::weight weight = boolean ? 0 : wt->get_maxpart();
	    Xapian::MSet::Internal::TermFreqAndWeight info(tf, weight);
	    LOGLINE(MATCH, "Setting term_info[" << query->tname << "] "
		    "to (" << tf << ", " << weight << ")");
	    term_info->insert(make_pair(query->tname, info));
	} else if (!boolean) {
	    i->second.termweight += wt->get_maxpart();
	    AssertEq(stats->get_termfreq(query->tname), i->second.termfreq);
	    LOGLINE(MATCH, "Increasing term_info[" << query->tname << "] "
		    "to (" << i->second.termfreq << ", " <<
		    i->second.termweight << ")");
	}
    }

    LeafPostList * pl = db->open_post_list(query->tname);
    // The default for LeafPostList is to return 0 weight and maxweight which
    // is the same as boolean weighting.
    if (!boolean) pl->set_termweight(wt.release());
    RETURN(pl);
}
