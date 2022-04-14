/** @file
 *  @brief SubMatch class for a local database.
 */
/* Copyright (C) 2006-2022 Olly Betts
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

#include "backends/databaseinternal.h"
#include "backends/leafpostlist.h"
#include "debuglog.h"
#include "extraweightpostlist.h"
#include "omassert.h"
#include "queryoptimiser.h"
#include "synonympostlist.h"
#include "api/termlist.h"
#include "weight/weightinternal.h"

#include "xapian/error.h"

#include <memory>
#include <map>
#include <string>

using namespace std;

/** Xapian::Weight subclass which adds laziness.
 *
 *  For terms from a wildcard when remote databases are involved, we need to
 *  delay calling init_() on the weight object until the stats for the terms
 *  from the wildcard have been collated.
 */
class LazyWeight : public Xapian::Weight {
    LeafPostList * pl;

    Xapian::Weight * real_wt;

    Xapian::Weight::Internal * stats;

    Xapian::termcount qlen;

    Xapian::termcount wqf;

    double factor;

    const Xapian::Database::Internal* shard;

    LazyWeight * clone() const;

    void init(double factor_);

  public:
    LazyWeight(LeafPostList * pl_,
	       Xapian::Weight * real_wt_,
	       Xapian::Weight::Internal * stats_,
	       Xapian::termcount qlen_,
	       Xapian::termcount wqf__,
	       double factor_,
	       const Xapian::Database::Internal* shard_)
	: pl(pl_),
	  real_wt(real_wt_),
	  stats(stats_),
	  qlen(qlen_),
	  wqf(wqf__),
	  factor(factor_),
	  shard(shard_)
    { }

    std::string name() const;

    std::string serialise() const;
    LazyWeight * unserialise(const std::string & serialised) const;

    double get_sumpart(Xapian::termcount wdf,
		       Xapian::termcount doclen,
		       Xapian::termcount uniqterms,
		       Xapian::termcount wdfdocmax) const;
    double get_maxpart() const;

    double get_sumextra(Xapian::termcount doclen,
			Xapian::termcount uniqterms,
			Xapian::termcount wdfdocmax) const;
    double get_maxextra() const;
};

LazyWeight *
LazyWeight::clone() const
{
    throw Xapian::InvalidOperationError("LazyWeight::clone()");
}

void
LazyWeight::init(double factor_)
{
    (void)factor_;
    throw Xapian::InvalidOperationError("LazyWeight::init()");
}

string
LazyWeight::name() const
{
    string desc = "LazyWeight(";
    desc += real_wt->name();
    desc += ")";
    return desc;
}

string
LazyWeight::serialise() const
{
    throw Xapian::InvalidOperationError("LazyWeight::serialise()");
}

LazyWeight *
LazyWeight::unserialise(const string &) const
{
    throw Xapian::InvalidOperationError("LazyWeight::unserialise()");
}

double
LazyWeight::get_sumpart(Xapian::termcount wdf,
			Xapian::termcount doclen,
			Xapian::termcount uniqterms,
			Xapian::termcount wdfdocmax) const
{
    (void)wdf;
    (void)doclen;
    (void)uniqterms;
    (void)wdfdocmax;
    throw Xapian::InvalidOperationError("LazyWeight::get_sumpart()");
}

double
LazyWeight::get_sumextra(Xapian::termcount doclen,
			 Xapian::termcount uniqterms,
			 Xapian::termcount wdfdocmax) const
{
    (void)doclen;
    (void)uniqterms;
    (void)wdfdocmax;
    throw Xapian::InvalidOperationError("LazyWeight::get_sumextra()");
}

double
LazyWeight::get_maxpart() const
{
    // This gets called first for the case we care about.
    return pl->resolve_lazy_termweight(real_wt, stats, qlen, wqf, factor, shard);
}

double
LazyWeight::get_maxextra() const
{
    throw Xapian::InvalidOperationError("LazyWeight::get_maxextra()");
}

PostList *
LocalSubMatch::get_postlist(PostListTree * matcher,
			    Xapian::termcount * total_subqs_ptr)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::get_postlist", matcher | total_subqs_ptr);

    if (query.empty() || db->get_doccount() == 0)
	RETURN(NULL); // MatchNothing

    // Build the postlist tree for the query.  This calls
    // LocalSubMatch::open_post_list() for each term in the query.
    PostList * pl;
    {
	QueryOptimiser opt(*db, *this, matcher, shard_index);
	double factor = wt_factory.is_bool_weight_() ? 0.0 : 1.0;
	pl = query.internal->postlist(&opt, factor);
	*total_subqs_ptr = opt.get_total_subqs();
    }

    if (pl) {
	unique_ptr<Xapian::Weight> extra_wt(wt_factory.clone());
	// Only uses term-independent stats.
	extra_wt->init_(*total_stats, qlen, db);
	if (extra_wt->get_maxextra() != 0.0) {
	    // There's a term-independent weight contribution, so we combine
	    // the postlist tree with an ExtraWeightPostList which adds in this
	    // contribution.
	    pl = new ExtraWeightPostList(pl, extra_wt.release(), matcher);
	}
    }

    RETURN(pl);
}

PostList *
LocalSubMatch::make_synonym_postlist(PostListTree* pltree,
				     PostList* or_pl,
				     double factor,
				     bool wdf_disjoint)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::make_synonym_postlist", pltree | or_pl | factor | wdf_disjoint);
    LOGVALUE(MATCH, or_pl->get_termfreq());
    unique_ptr<SynonymPostList> res(new SynonymPostList(or_pl, db, pltree,
							wdf_disjoint));
    unique_ptr<Xapian::Weight> wt(wt_factory.clone());

    // We shortcut an empty shard and avoid creating a postlist tree for it,
    // and all shards must be empty for collection_size to be zero.
    Assert(total_stats->collection_size);
    TermFreqs freqs = or_pl->estimate_termfreqs(*total_stats);
    wt->init_(*total_stats, qlen, factor,
	      freqs.termfreq, freqs.reltermfreq, freqs.collfreq, db);

    res->set_weight(wt.release());
    RETURN(res.release());
}

PostList *
LocalSubMatch::open_post_list(const string& term,
			      Xapian::termcount wqf,
			      double factor,
			      bool need_positions,
			      bool compound_weight,
			      QueryOptimiser * qopt,
			      bool lazy_weight)
{
    LOGCALL(MATCH, PostList *, "LocalSubMatch::open_post_list", term | wqf | factor | need_positions | qopt | lazy_weight);

    bool weighted = false;

    LeafPostList * pl = NULL;
    if (term.empty()) {
	Assert(!need_positions);
	pl = db->open_leaf_post_list(term, false);
    } else {
	weighted = (factor != 0.0);
	if (!need_positions) {
	    if ((!weighted && !compound_weight) ||
		!wt_factory.get_sumpart_needs_wdf_()) {
		Xapian::doccount sub_tf;
		db->get_freqs(term, &sub_tf, NULL);
		if (sub_tf == db->get_doccount()) {
		    // If we're not going to use the wdf or term positions, and
		    // the term indexes all documents, we can replace it with
		    // the MatchAll postlist, which is especially efficient if
		    // there are no gaps in the docids.
		    pl = db->open_leaf_post_list(string(), false);

		    // Set the term name so the postlist looks up the correct
		    // term frequencies - this is necessary if the weighting
		    // scheme needs collection frequency or reltermfreq
		    // (termfreq would be correct anyway since it's just the
		    // collection size in this case).
		    pl->set_term(term);
		}
	    }
	}

	if (!pl) {
	    const LeafPostList * hint = qopt->get_hint_postlist();
	    if (hint)
		pl = hint->open_nearby_postlist(term, need_positions);
	    if (!pl) {
		pl = db->open_leaf_post_list(term, need_positions);
	    }
	    qopt->set_hint_postlist(pl);
	}
    }

    if (lazy_weight) {
	auto res = total_stats->termfreqs.emplace(term, TermFreqs());
	if (res.second) {
	    // Term came from a wildcard, but the same term may be elsewhere
	    // in the query so only accumulate its TermFreqs if emplace()
	    // created a new element.
	    db->get_freqs(term,
			  &res.first->second.termfreq,
			  &res.first->second.collfreq);
	}
    }

    if (weighted) {
	Xapian::Weight * wt = wt_factory.clone();
	if (!lazy_weight) {
	    wt->init_(*total_stats, qlen, term, wqf, factor, db, pl);
	    if (pl->get_termfreq() > 0)
		total_stats->set_max_part(term, wt->get_maxpart());
	} else {
	    // Delay initialising the actual weight object, so that we can
	    // gather stats for the terms lazily expanded from a wildcard
	    // (needed for the remote database case).
	    wt = new LazyWeight(pl, wt, total_stats, qlen, wqf, factor, db);
	}
	pl->set_termweight(wt);
    }

    add_op(pl->get_termfreq());
    RETURN(pl);
}
