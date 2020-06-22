/** @file enquire.cc
 * @brief Xapian::Enquire class
 */
/* Copyright (C) 2009,2017 Olly Betts
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

#include "xapian/enquire.h"
#include "enquireinternal.h"

#include "expand/esetinternal.h"
#include "expand/expandweight.h"
#include "matcher/matcher.h"
#include "msetinternal.h"
#include "vectortermlist.h"
#include "weight/weightinternal.h"
#include "xapian/database.h"
#include "xapian/error.h"
#include "xapian/expanddecider.h"
#include "xapian/intrusive_ptr.h"
#include "xapian/keymaker.h"
#include "xapian/matchspy.h"
#include "xapian/query.h"
#include "xapian/rset.h"
#include "xapian/weight.h"

#include <memory>
#include <string>
#include <vector>

using namespace std;

[[noreturn]]
static void
throw_invalid_arg(const char* msg) {
    throw Xapian::InvalidArgumentError(msg);
}

namespace Xapian {

Enquire::Enquire(const Enquire&) = default;

Enquire&
Enquire::operator=(const Enquire&) = default;

Enquire::Enquire(Enquire&&) = default;

Enquire&
Enquire::operator=(Enquire&&) = default;

Enquire::Enquire(const Database& db) : internal(new Enquire::Internal(db)) {}

Enquire::~Enquire() {}

void
Enquire::set_query(const Query& query, termcount query_length)
{
    internal->query = query;
    internal->query_length = query_length;
}

const Query&
Enquire::get_query() const
{
    return internal->query;
}

void
Enquire::set_weighting_scheme(const Weight& weight)
{
    internal->weight.reset(weight.clone());
}

void
Enquire::set_docid_order(docid_order order)
{
    internal->order = order;
}

void
Enquire::set_sort_by_relevance()
{
    internal->sort_by = Internal::REL;
}

void
Enquire::set_sort_by_value(valueno sort_key, bool reverse)
{
    internal->sort_by = Internal::VAL;
    internal->sort_functor = NULL;
    internal->sort_key = sort_key;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_sort_by_key(KeyMaker* sorter, bool reverse)
{
    if (sorter == NULL) {
	throw_invalid_arg("Enquire::set_sort_by_key(): sorter cannot be NULL");
    }
    internal->sort_by = Internal::VAL;
    internal->sort_functor = sorter;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_sort_by_value_then_relevance(valueno sort_key, bool reverse)
{
    internal->sort_by = Internal::VAL_REL;
    internal->sort_functor = NULL;
    internal->sort_key = sort_key;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_sort_by_key_then_relevance(KeyMaker* sorter, bool reverse)
{
    if (sorter == NULL) {
	throw_invalid_arg("Enquire::set_sort_by_key_then_relevance(): "
			  "sorter cannot be NULL");
    }
    internal->sort_by = Internal::VAL_REL;
    internal->sort_functor = sorter;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_sort_by_relevance_then_value(valueno sort_key, bool reverse)
{
    internal->sort_by = Internal::REL_VAL;
    internal->sort_functor = NULL;
    internal->sort_key = sort_key;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_sort_by_relevance_then_key(KeyMaker* sorter, bool reverse)
{
    if (sorter == NULL) {
	throw_invalid_arg("Enquire::set_sort_by_relevance_then_key(): "
			  "sorter cannot be NULL");
    }
    internal->sort_by = Internal::REL_VAL;
    internal->sort_functor = sorter;
    internal->sort_val_reverse = reverse;
}

void
Enquire::set_collapse_key(valueno collapse_key, doccount collapse_max)
{
    internal->collapse_key = collapse_key;
    internal->collapse_max = collapse_max;
}

void
Enquire::set_cutoff(int percent_threshold, double weight_threshold)
{
    internal->percent_threshold = percent_threshold;
    internal->weight_threshold = weight_threshold;
}

void
Enquire::add_matchspy(MatchSpy* spy)
{
    using Xapian::Internal::opt_intrusive_ptr;
    if (spy == NULL)
	throw_invalid_arg("Enquire::add_matchspy(): spy cannot be NULL");
    internal->matchspies.push_back(opt_intrusive_ptr<MatchSpy>(spy));
}

void
Enquire::clear_matchspies()
{
    internal->matchspies.clear();
}

void
Enquire::set_time_limit(double time_limit)
{
    internal->time_limit = time_limit;
}

MSet
Enquire::get_mset(doccount first,
		  doccount maxitems,
		  doccount checkatleast,
		  const RSet* rset,
		  const MatchDecider* mdecider) const
{
    return internal->get_mset(first, maxitems, checkatleast, rset, mdecider);
}

TermIterator
Enquire::get_matching_terms_begin(docid did) const
{
    return internal->get_matching_terms_begin(did);
}

void
Enquire::set_expansion_scheme(const std::string &eweightname, double expand_k) const
{
    if (eweightname == "bo1") {
	internal->eweight = Enquire::Internal::EXPAND_BO1;
    } else if (eweightname == "trad") {
	internal->eweight = Enquire::Internal::EXPAND_TRAD;
    } else {
	throw_invalid_arg("Enquire::set_expansion_scheme(): eweightname must "
			  "be 'bo1' or 'trad'");
    }
    internal->expand_k = expand_k;
}

ESet
Enquire::get_eset(termcount maxitems,
		  const RSet& rset,
		  int flags,
		  const ExpandDecider* edecider,
		  double min_weight) const
{
    return internal->get_eset(maxitems, rset, flags, edecider, min_weight);
}

std::string
Enquire::get_description() const
{
    string desc = "Enquire(db=";
    desc += internal->db.get_description();
    if (!internal->query.empty()) {
	desc += ", query=";
	desc += internal->query.get_description();
    }
    desc += ')';
    return desc;
}

Enquire::Internal::Internal(const Database& db_)
    : db(db_) {}

MSet
Enquire::Internal::get_mset(doccount first,
			    doccount maxitems,
			    doccount checkatleast,
			    const RSet* rset,
			    const MatchDecider* mdecider) const
{
    if (query.empty()) {
	MSet mset;
	mset.internal->set_first(first);
	return mset;
    }

    if (percent_threshold && (sort_by == VAL || sort_by == VAL_REL)) {
	throw Xapian::UnimplementedError("Use of a percentage cutoff while "
					 "sorting primary by value isn't "
					 "currently supported");
    }

    // Lazily initialise weight to its default if necessary.
    if (!weight.get())
	weight.reset(new BM25Weight);

    // Lazily initialise query_length if it wasn't explicitly specified.
    if (query_length == 0) {
	query_length = query.get_length();
    }

    Xapian::doccount first_orig = first;
    {
	Xapian::doccount docs = db.get_doccount();
	first = min(first, docs);
	maxitems = min(maxitems, docs - first);
	checkatleast = min(checkatleast, docs);
	checkatleast = max(checkatleast, first + maxitems);
    }

    unique_ptr<Xapian::Weight::Internal> stats(new Xapian::Weight::Internal);
    ::Matcher match(db,
		    db.has_positions(),
		    query,
		    query_length,
		    rset,
		    *stats,
		    *weight,
		    (mdecider != NULL),
		    collapse_key,
		    collapse_max,
		    percent_threshold,
		    weight_threshold,
		    order,
		    sort_key,
		    sort_by,
		    sort_val_reverse,
		    time_limit,
		    matchspies);

    MSet mset = match.get_mset(first,
			       maxitems,
			       checkatleast,
			       *stats,
			       *weight,
			       mdecider,
			       sort_functor.get(),
			       collapse_key,
			       collapse_max,
			       percent_threshold,
			       weight_threshold,
			       order,
			       sort_key,
			       sort_by,
			       sort_val_reverse,
			       time_limit,
			       matchspies);

    if (first_orig != first && mset.internal.get()) {
	mset.internal->set_first(first_orig);
    }

    mset.internal->set_enquire(this);

    if (!mset.internal->get_stats()) {
	mset.internal->set_stats(stats.release());
    }

    return mset;
}

TermIterator
Enquire::Internal::get_matching_terms_begin(docid did) const
{
    if (query.empty())
	return TermIterator();

    struct term_and_pos {
	string term;
	Xapian::termpos pos;

	term_and_pos(const string& term_, Xapian::termpos pos_)
	    : term(term_), pos(pos_) {}
    };

    vector<term_and_pos> query_terms;
    Xapian::termpos pos = 1;
    for (auto t = query.get_terms_begin(); t != query.get_terms_end(); ++t) {
	query_terms.emplace_back(*t, pos++);
    }

    if (query_terms.empty())
	return TermIterator();

    // Reorder by term, secondary sort by position.
    sort(query_terms.begin(), query_terms.end(),
	 [](const term_and_pos& a, const term_and_pos& b) {
	     int cmp = a.term.compare(b.term);
	     return cmp ? cmp < 0 : a.pos < b.pos;
	 });

    // Loop through the query terms, skipping the document terms for each to
    // see which match, and shuffling down the matching ones.  Also discard
    // repeats, keeping the smallest position.
    size_t i = 0, j = 0;
    auto t = db.termlist_begin(did);
    do {
	const string& term = query_terms[i].term;
	if (j == 0 || term != query_terms[j - 1].term) {
	    t.skip_to(term);
	    if (t == db.termlist_end(did)) {
		break;
	    }

	    if (*t == term) {
		// Matched, so move down if necessary.
		if (i != j)
		    query_terms[j] = std::move(query_terms[i]);
		++j;
	    }
	}
    } while (++i != query_terms.size());

    // Truncate to leave just the matching terms.
    query_terms.erase(query_terms.begin() + j, query_terms.end());

    // Reorder by ascending query position.
    sort(query_terms.begin(), query_terms.end(),
	 [](const term_and_pos& a, const term_and_pos& b) {
	     return a.pos < b.pos;
	 });

    // Iterator adaptor to present query_terms as a container of just strings.
    struct Itor {
	vector<term_and_pos>::const_iterator it;

	explicit
	Itor(vector<term_and_pos>::const_iterator it_) : it(it_) {}

	const std::string& operator*() const {
	    return it->term;
	}

	Itor& operator++() {
	    ++it;
	    return *this;
	}

	Itor operator++(int) {
	    Itor retval = *this;
	    ++it;
	    return retval;
	}

	bool operator!=(const Itor& o) { return it != o.it; }
    };

    return TermIterator(new VectorTermList(Itor(query_terms.cbegin()),
					   Itor(query_terms.cend())));
}

ESet
Enquire::Internal::get_eset(termcount maxitems,
			    const RSet& rset,
			    int flags,
			    const ExpandDecider* edecider_,
			    double min_weight) const
{
    using Xapian::Internal::opt_intrusive_ptr;
    opt_intrusive_ptr<const ExpandDecider> edecider(edecider_);

    Xapian::ESet eset;

    if (maxitems == 0 || rset.empty()) {
	// Either we were asked for no results, or wouldn't produce any
	// because no documents were marked as relevant.
	return eset;
    }

    // Excluding query terms is a no-op without a query.
    if ((flags & Enquire::INCLUDE_QUERY_TERMS) == 0 && !query.empty()) {
	auto edft = new ExpandDeciderFilterTerms(query.get_terms_begin(),
						 query.get_terms_end());
	if (edecider.get() == NULL) {
	    edecider = edft->release();
	} else {
	    // Make sure ExpandDeciderFilterTerms doesn't leak if new throws.
	    opt_intrusive_ptr<const ExpandDecider> ptr(edft->release());
	    edecider = (new ExpandDeciderAnd(ptr.get(),
					     edecider.get()))->release();
	}
    }

    bool use_exact_termfreq = flags & Enquire::USE_EXACT_TERMFREQ;
    if (eweight == Enquire::Internal::EXPAND_BO1) {
	using Xapian::Internal::Bo1EWeight;
	Bo1EWeight bo1eweight(db, rset.size(), use_exact_termfreq);
	eset.internal->expand(maxitems, db, rset, edecider.get(), bo1eweight,
			      min_weight);
    } else {
	AssertEq(eweight, Enquire::Internal::EXPAND_TRAD);
	using Xapian::Internal::TradEWeight;
	TradEWeight tradeweight(db, rset.size(), use_exact_termfreq, expand_k);
	eset.internal->expand(maxitems, db, rset, edecider.get(), tradeweight,
			      min_weight);
    }

    return eset;
}

}
