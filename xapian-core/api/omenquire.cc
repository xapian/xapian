/* omenquire.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2013,2014,2015,2016,2017 Olly Betts
 * Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2011, Action Without Borders
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
#include "xapian/enquire.h"

#include "xapian/document.h"
#include "xapian/error.h"
#include "xapian/expanddecider.h"
#include "xapian/matchspy.h"
#include "xapian/termiterator.h"
#include "xapian/weight.h"

#include "vectortermlist.h"

#include "backends/database.h"
#include "debuglog.h"
#include "expand/esetinternal.h"
#include "expand/expandweight.h"
#include "matcher/msetcmp.h"
#include "matcher/multimatch.h"
#include "msetinternal.h"
#include "omassert.h"
#include "api/omenquireinternal.h"
#include "roundestimate.h"
#include "str.h"
#include "weight/weightinternal.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <memory>
#include <vector>

using namespace std;

using Xapian::Internal::ExpandWeight;
using Xapian::Internal::Bo1EWeight;
using Xapian::Internal::TradEWeight;

namespace Xapian {

// Methods for Xapian::Enquire::Internal

Enquire::Internal::Internal(const Database &db_)
  : db(db_), query(), collapse_key(Xapian::BAD_VALUENO), collapse_max(0),
    order(Enquire::ASCENDING), percent_cutoff(0), weight_cutoff(0),
    sort_key(Xapian::BAD_VALUENO), sort_by(REL), sort_value_forward(true),
    sorter(), time_limit(0.0), weight(0),
    eweightname("trad"), expand_k(1.0)
{
    if (db.internal.empty()) {
	throw InvalidArgumentError("Can't make an Enquire object from an uninitialised Database object.");
    }
}

Enquire::Internal::~Internal()
{
    delete weight;
    weight = 0;
}

void
Enquire::Internal::set_query(const Query &query_, termcount qlen_)
{
    query = query_;
    qlen = qlen_ ? qlen_ : query.get_length();
}

const Query &
Enquire::Internal::get_query() const
{
    return query;
}

MSet
Enquire::Internal::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
			    Xapian::doccount check_at_least, const RSet *rset,
			    const MatchDecider *mdecider) const
{
    LOGCALL(MATCH, MSet, "Enquire::Internal::get_mset", first | maxitems | check_at_least | rset | mdecider);

    if (percent_cutoff && (sort_by == VAL || sort_by == VAL_REL)) {
	throw Xapian::UnimplementedError("Use of a percentage cutoff while sorting primary by value isn't currently supported");
    }

    if (weight == 0) {
	weight = new BM25Weight;
    }

    Xapian::doccount first_orig = first;
    {
	Xapian::doccount docs = db.get_doccount();
	first = min(first, docs);
	maxitems = min(maxitems, docs);
	check_at_least = min(check_at_least, docs);
	check_at_least = max(check_at_least, maxitems);
    }

    unique_ptr<Xapian::Weight::Internal> stats(new Xapian::Weight::Internal);
    ::MultiMatch match(db, query, qlen, rset,
		       collapse_max, collapse_key,
		       percent_cutoff, weight_cutoff,
		       order, sort_key, sort_by, sort_value_forward,
		       time_limit, *(stats.get()), weight, spies,
		       (sorter.get() != NULL),
		       (mdecider != NULL));
    // Run query and put results into supplied Xapian::MSet object.
    MSet retval;
    match.get_mset(first, maxitems, check_at_least, retval,
		   *(stats.get()), mdecider, sorter.get());
    if (first_orig != first && retval.internal.get()) {
	retval.internal->set_first(first_orig);
    }

    Assert(weight->name() != "bool" || retval.get_max_possible() == 0);

    // The Xapian::MSet needs to have a pointer to ourselves, so that it can
    // retrieve the documents.  This is set here explicitly to avoid having
    // to pass it into the matcher, which gets messy particularly in the
    // networked case.
    retval.internal->set_enquire(this);

    if (!retval.internal->get_stats()) {
	retval.internal->set_stats(stats.release());
    }

    RETURN(retval);
}

ESet
Enquire::Internal::get_eset(Xapian::termcount maxitems,
			    const RSet & rset, int flags,
			    const ExpandDecider * edecider_,
			    double min_wt) const
{
    LOGCALL(MATCH, ESet, "Enquire::Internal::get_eset", maxitems | rset | flags | edecider_ | min_wt);

    using Xapian::Internal::opt_intrusive_ptr;
    opt_intrusive_ptr<const ExpandDecider> edecider(edecider_);
    if (maxitems == 0 || rset.empty()) {
	// Either we were asked for no results, or wouldn't produce any
	// because no documents were marked as relevant.
	RETURN(ESet());
    }

    LOGVALUE(MATCH, rset.size());

    if (!query.empty() && !(flags & Enquire::INCLUDE_QUERY_TERMS)) {
	opt_intrusive_ptr<const ExpandDecider> decider_noquery(
	    (new ExpandDeciderFilterTerms(query.get_terms_begin(),
					  query.get_terms_end()))->release());
	if (edecider.get()) {
	    edecider = (new ExpandDeciderAnd(decider_noquery.get(),
					     edecider.get()))->release();
	} else {
	    edecider = decider_noquery;
	}
    }

    bool use_exact_termfreq(flags & Enquire::USE_EXACT_TERMFREQ);
    Xapian::ESet eset;

    if (eweightname == "bo1") {
	Bo1EWeight bo1eweight(db, rset.size(), use_exact_termfreq);
	eset.internal->expand(maxitems, db, rset, edecider.get(), bo1eweight,
			      min_wt);
    } else {
	TradEWeight tradeweight(db, rset.size(), use_exact_termfreq, expand_k);
	eset.internal->expand(maxitems, db, rset, edecider.get(), tradeweight,
			      min_wt);
    }

    RETURN(eset);
}

class ByQueryIndexCmp {
  private:
    typedef map<string, unsigned int> tmap_t;
    const tmap_t &tmap;

  public:
    explicit ByQueryIndexCmp(const tmap_t &tmap_) : tmap(tmap_) {}
    bool operator()(const string &left,
		    const string &right) const {
	tmap_t::const_iterator l, r;
	l = tmap.find(left);
	r = tmap.find(right);
	Assert((l != tmap.end()) && (r != tmap.end()));

	return l->second < r->second;
    }
};

TermIterator
Enquire::Internal::get_matching_terms(Xapian::docid did) const
{
    if (query.empty())
	return TermIterator();

    // The ordered list of terms in the query.
    TermIterator qt = query.get_terms_begin();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    map<string, unsigned int> tmap;
    unsigned int index = 1;
    for ( ; qt != query.get_terms_end(); ++qt) {
	if (tmap.find(*qt) == tmap.end())
	    tmap[*qt] = index++;
    }

    vector<string> matching_terms;

    TermIterator docterms = db.termlist_begin(did);
    TermIterator docterms_end = db.termlist_end(did);
    while (docterms != docterms_end) {
	string term = *docterms;
	map<string, unsigned int>::iterator t = tmap.find(term);
	if (t != tmap.end()) matching_terms.push_back(term);
	++docterms;
    }

    // sort the resulting list by query position.
    sort(matching_terms.begin(), matching_terms.end(), ByQueryIndexCmp(tmap));

    return TermIterator(new VectorTermList(matching_terms.begin(),
					   matching_terms.end()));
}

TermIterator
Enquire::Internal::get_matching_terms(const MSetIterator &it) const
{
    // FIXME: take advantage of MSetIterator to ensure that database
    // doesn't get modified underneath us.
    return get_matching_terms(*it);
}

Xapian::doccount
Enquire::Internal::get_termfreq(const string &tname) const
{
    return db.get_termfreq(tname);
}

string
Enquire::Internal::get_description() const
{
    string description = db.get_description();
    description += ", ";
    description += query.get_description();
    return description;
}

// Private methods for Xapian::Enquire::Internal

void
Enquire::Internal::request_doc(Xapian::docid did) const
{
    unsigned int multiplier = db.internal.size();

    Xapian::docid realdid = (did - 1) / multiplier + 1;
    Xapian::doccount dbnumber = (did - 1) % multiplier;

    db.internal[dbnumber]->request_document(realdid);
}

Document
Enquire::Internal::get_document(Xapian::docid did) const
{
    unsigned int multiplier = db.internal.size();

    Xapian::docid realdid = (did - 1) / multiplier + 1;
    Xapian::doccount dbnumber = (did - 1) % multiplier;

    // We know the doc exists, so open lazily.
    return Document(db.internal[dbnumber]->open_document(realdid, true));
}

// Methods of Xapian::Enquire

Enquire::Enquire(const Enquire & other) : internal(other.internal)
{
    LOGCALL_CTOR(API, "Enquire", other);
}

void
Enquire::operator=(const Enquire & other)
{
    LOGCALL_VOID(API, "Xapian::Enquire::operator=", other);
    internal = other.internal;
}

Enquire::Enquire(const Database &databases)
    : internal(new Internal(databases))
{
    LOGCALL_CTOR(API, "Enquire", databases);
}

Enquire::~Enquire()
{
    LOGCALL_DTOR(API, "Enquire");
}

void
Enquire::set_query(const Query & query, termcount len)
{
    LOGCALL_VOID(API, "Xapian::Enquire::set_query", query | len);
    internal->set_query(query, len);
}

const Query &
Enquire::get_query() const
{
    LOGCALL(API, const Xapian::Query &, "Xapian::Enquire::get_query", NO_ARGS);
    RETURN(internal->get_query());
}

void
Enquire::add_matchspy(MatchSpy * spy) {
    LOGCALL_VOID(API, "Xapian::Enquire::add_matchspy", spy);
    internal->spies.push_back(spy);
}

void
Enquire::clear_matchspies() {
    LOGCALL_VOID(API, "Xapian::Enquire::clear_matchspies", NO_ARGS);
    internal->spies.clear();
}

void
Enquire::set_weighting_scheme(const Weight &weight_)
{
    LOGCALL_VOID(API, "Xapian::Enquire::set_weighting_scheme", weight_);
    // Clone first in case doing so throws an exception.
    Weight * wt = weight_.clone();
    swap(wt, internal->weight);
    delete wt;
}

void
Enquire::set_expansion_scheme(const std::string &eweightname_, double expand_k_) const
{
    LOGCALL_VOID(API, "Xapian::Enquire::set_expansion_scheme", eweightname_ | expand_k_);

    if (eweightname_ != "bo1" && eweightname_ != "trad") {
	throw InvalidArgumentError("Invalid name for query expansion scheme.");
    }

    internal->eweightname = eweightname_;
    internal->expand_k = expand_k_;
}

void
Enquire::set_collapse_key(Xapian::valueno collapse_key, Xapian::doccount collapse_max)
{
    if (collapse_key == Xapian::BAD_VALUENO) collapse_max = 0;
    internal->collapse_key = collapse_key;
    internal->collapse_max = collapse_max;
}

void
Enquire::set_docid_order(Enquire::docid_order order)
{
    internal->order = order;
}

void
Enquire::set_cutoff(int percent_cutoff, double weight_cutoff)
{
    internal->percent_cutoff = percent_cutoff;
    internal->weight_cutoff = weight_cutoff;
}

void
Enquire::set_sort_by_relevance()
{
    internal->sort_by = Internal::REL;
}

void
Enquire::set_sort_by_value(valueno sort_key, bool ascending)
{
    internal->sorter = NULL;
    internal->sort_key = sort_key;
    internal->sort_by = Internal::VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_value_then_relevance(valueno sort_key, bool ascending)
{
    internal->sorter = NULL;
    internal->sort_key = sort_key;
    internal->sort_by = Internal::VAL_REL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_relevance_then_value(valueno sort_key, bool ascending)
{
    internal->sorter = NULL;
    internal->sort_key = sort_key;
    internal->sort_by = Internal::REL_VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_key(KeyMaker * sorter, bool ascending)
{
    if (sorter == NULL)
	throw InvalidArgumentError("sorter can't be NULL");
    internal->sorter = sorter;
    internal->sort_by = Internal::VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_key_then_relevance(KeyMaker * sorter, bool ascending)
{
    if (sorter == NULL)
	throw InvalidArgumentError("sorter can't be NULL");
    internal->sorter = sorter;
    internal->sort_by = Internal::VAL_REL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_relevance_then_key(KeyMaker * sorter, bool ascending)
{
    if (sorter == NULL)
	throw Xapian::InvalidArgumentError("sorter can't be NULL");
    internal->sorter = sorter;
    internal->sort_by = Internal::REL_VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_time_limit(double time_limit)
{
    internal->time_limit = time_limit;
}

MSet
Enquire::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		  Xapian::doccount check_at_least, const RSet *rset,
		  const MatchDecider *mdecider) const
{
    LOGCALL(API, Xapian::MSet, "Xapian::Enquire::get_mset", first | maxitems | check_at_least | rset | mdecider);
    RETURN(internal->get_mset(first, maxitems, check_at_least, rset, mdecider));
}

ESet
Enquire::get_eset(Xapian::termcount maxitems, const RSet & rset, int flags,
		  const ExpandDecider * edecider, double min_wt) const
{
    LOGCALL(API, Xapian::ESet, "Xapian::Enquire::get_eset", maxitems | rset | flags | edecider | min_wt);
    RETURN(internal->get_eset(maxitems, rset, flags, edecider, min_wt));
}

TermIterator
Enquire::get_matching_terms_begin(const MSetIterator &it) const
{
    LOGCALL(API, Xapian::TermIterator, "Xapian::Enquire::get_matching_terms_begin", it);
    RETURN(internal->get_matching_terms(it));
}

TermIterator
Enquire::get_matching_terms_begin(Xapian::docid did) const
{
    LOGCALL(API, Xapian::TermIterator, "Xapian::Enquire::get_matching_terms_begin", did);
    RETURN(internal->get_matching_terms(did));
}

string
Enquire::get_description() const
{
    return "Xapian::Enquire(" + internal->get_description() + ")";
}

}
