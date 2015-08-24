/* omenquire.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
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
#include "xapian/errorhandler.h"
#include "xapian/expanddecider.h"
#include "xapian/termiterator.h"
#include "xapian/weight.h"

#include "vectortermlist.h"

#include "database.h"
#include "debuglog.h"
#include "esetinternal.h"
#include "expandweight.h"
#include "multimatch.h"
#include "omenquireinternal.h"
#include "str.h"
#include "weightinternal.h"

#include <algorithm>
#include "autoptr.h"
#include <cfloat>
#include <cmath>
#include <vector>

using namespace std;

using Xapian::Internal::ExpandWeight;

namespace Xapian {

MatchDecider::~MatchDecider() { }

// Methods for Xapian::RSet

RSet::RSet() : internal(new RSet::Internal)
{
}

RSet::RSet(const RSet &other) : internal(other.internal)
{
}

void
RSet::operator=(const RSet &other)
{
    internal = other.internal;
}

RSet::~RSet()
{
}

Xapian::doccount
RSet::size() const
{
    return internal->items.size();
}

bool
RSet::empty() const
{
    return internal->items.empty();
}

void
RSet::add_document(Xapian::docid did)
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 not valid");
    internal->items.insert(did);
}

void
RSet::remove_document(Xapian::docid did)
{
    internal->items.erase(did);
}

bool
RSet::contains(Xapian::docid did) const
{
    return internal->items.find(did) != internal->items.end();
}

string
RSet::get_description() const
{
    return "RSet(" + internal->get_description() + ")";
}

string
RSet::Internal::get_description() const
{
    string description("RSet::Internal(");

    set<Xapian::docid>::const_iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
	if (i != items.begin()) description += ", ";
	description += str(*i);
    }

    description += ')';

    return description;
}

namespace Internal {

// Methods for Xapian::MSetItem

string
MSetItem::get_description() const
{
    string description;

    description = str(did) + ", " + str(wt) + ", " +
	    collapse_key;

    description = "Xapian::MSetItem(" + description + ")";

    return description;
}

}

// Methods for Xapian::MSet

MSet::MSet() : internal(new MSet::Internal)
{
}

MSet::MSet(MSet::Internal * internal_) : internal(internal_)
{
}

MSet::~MSet()
{
}

MSet::MSet(const MSet & other) : internal(other.internal)
{
}

void
MSet::operator=(const MSet &other)
{
    internal = other.internal;
}

void
MSet::fetch(const MSetIterator & beginiter, const MSetIterator & enditer) const
{
    LOGCALL_VOID(API, "Xapian::MSet::fetch", beginiter | enditer);
    Assert(internal.get() != 0);
    if (beginiter != enditer)
	internal->fetch_items(beginiter.index, enditer.index - 1);
}

void
MSet::fetch(const MSetIterator & beginiter) const
{
    LOGCALL_VOID(API, "Xapian::MSet::fetch", beginiter);
    Assert(internal.get() != 0);
    internal->fetch_items(beginiter.index, beginiter.index);
}

void
MSet::fetch() const
{
    LOGCALL_VOID(API, "Xapian::MSet::fetch", NO_ARGS);
    Assert(internal.get() != 0);
    if (!internal->items.empty())
	internal->fetch_items(0, internal->items.size() - 1);
}

percent
MSet::convert_to_percent(Xapian::weight wt) const
{
    LOGCALL(API, Xapian::percent, "Xapian::MSet::convert_to_percent", wt);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(wt));
}

percent
MSet::convert_to_percent(const MSetIterator & it) const
{
    LOGCALL(API, Xapian::percent, "Xapian::MSet::convert_to_percent", it);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(it.get_weight()));
}

Xapian::doccount
MSet::get_termfreq(const string &tname) const
{
    LOGCALL(API, Xapian::doccount, "Xapian::MSet::get_termfreq", tname);
    map<string, Internal::TermFreqAndWeight>::const_iterator i;
    Assert(internal.get() != 0);
    i = internal->termfreqandwts.find(tname);
    if (i != internal->termfreqandwts.end()) {
	RETURN(i->second.termfreq);
    }
    if (internal->enquire.get() == 0) {
	throw InvalidOperationError("Can't get termfreq from an MSet which is not derived from a query.");
    }
    RETURN(internal->enquire->get_termfreq(tname));
}

Xapian::weight
MSet::get_termweight(const string &tname) const
{
    LOGCALL(API, Xapian::weight, "Xapian::MSet::get_termweight", tname);
    map<string, Internal::TermFreqAndWeight>::const_iterator i;
    Assert(internal.get() != 0);
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw InvalidArgumentError("Term weight of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termweight);
}

Xapian::doccount
MSet::get_firstitem() const
{
    Assert(internal.get() != 0);
    return internal->firstitem;
}

Xapian::doccount
MSet::get_matches_lower_bound() const
{
    Assert(internal.get() != 0);
    return internal->matches_lower_bound;
}

Xapian::doccount
MSet::get_matches_estimated() const
{
    Assert(internal.get() != 0);
    return internal->matches_estimated;
}

Xapian::doccount
MSet::get_matches_upper_bound() const
{
    Assert(internal.get() != 0);
    return internal->matches_upper_bound;
}

Xapian::doccount
MSet::get_uncollapsed_matches_lower_bound() const
{
    Assert(internal.get() != 0);
    return internal->uncollapsed_lower_bound;
}

Xapian::doccount
MSet::get_uncollapsed_matches_estimated() const
{
    Assert(internal.get() != 0);
    return internal->uncollapsed_estimated;
}

Xapian::doccount
MSet::get_uncollapsed_matches_upper_bound() const
{
    Assert(internal.get() != 0);
    return internal->uncollapsed_upper_bound;
}

Xapian::weight
MSet::get_max_possible() const
{
    Assert(internal.get() != 0);
    return internal->max_possible;
}

Xapian::weight
MSet::get_max_attained() const
{
    Assert(internal.get() != 0);
    return internal->max_attained;
}

Xapian::doccount
MSet::size() const
{
    Assert(internal.get() != 0);
    return internal->items.size();
}

bool
MSet::empty() const
{
    Assert(internal.get() != 0);
    return internal->items.empty();
}

void
MSet::swap(MSet & other)
{
    std::swap(internal, other.internal);
}

MSetIterator
MSet::begin() const
{
    return MSetIterator(0, *this);
}

MSetIterator
MSet::end() const
{
    Assert(internal.get() != 0);
    return MSetIterator(internal->items.size(), *this);
}

MSetIterator
MSet::operator[](Xapian::doccount i) const
{
    // Don't test 0 <= i - that gives a compiler warning if i is unsigned
    Assert(0 < (i + 1) && i < size());
    return MSetIterator(i, *this);
}

MSetIterator
MSet::back() const
{
    Assert(!empty());
    Assert(internal.get() != 0);
    return MSetIterator(internal->items.size() - 1, *this);
}

string
MSet::get_description() const
{
    Assert(internal.get() != 0);
    return "Xapian::MSet(" + internal->get_description() + ")";
}

percent
MSet::Internal::convert_to_percent_internal(Xapian::weight wt) const
{
    LOGCALL(MATCH, Xapian::percent, "Xapian::MSet::Internal::convert_to_percent_internal", wt);
    if (percent_factor == 0) RETURN(100);

    // Excess precision on x86 can result in a difference here.
    double v = wt * percent_factor + 100.0 * DBL_EPSILON;
    Xapian::percent pcent = static_cast<Xapian::percent>(v);
    LOGLINE(MATCH, "wt = " << wt << ", max_possible = " << max_possible <<
		   " =>  pcent = " << pcent);
    if (pcent > 100) pcent = 100;
    if (pcent < 0) pcent = 0;
    if (pcent == 0 && wt > 0) pcent = 1;

    RETURN(pcent);
}

Document
MSet::Internal::get_doc_by_index(Xapian::doccount index) const
{
    LOGCALL(MATCH, Document, "Xapian::MSet::Internal::get_doc_by_index", index);
    index += firstitem; 
    map<Xapian::doccount, Document>::const_iterator doc;
    doc = indexeddocs.find(index);
    if (doc != indexeddocs.end()) {
	RETURN(doc->second);
    }
    if (index < firstitem || index >= firstitem + items.size()) {
	throw RangeError("The mset returned from the match does not contain the document at index " + str(index));
    }
    Assert(enquire.get());
    if (!requested_docs.empty()) {
	// There's already a pending request, so handle that.
	read_docs();
	// Maybe we just fetched the doc we want.
	doc = indexeddocs.find(index);
	if (doc != indexeddocs.end()) {
	    RETURN(doc->second);
	}
    }

    // Don't cache unless fetch() was called by the API user.
    enquire->request_doc(items[index - firstitem]);
    RETURN(enquire->read_doc(items[index - firstitem]));
}

void
MSet::Internal::fetch_items(Xapian::doccount first, Xapian::doccount last) const
{
    LOGCALL_VOID(MATCH, "Xapian::MSet::Internal::fetch_items", first | last);
    if (enquire.get() == 0) {
	throw InvalidOperationError("Can't fetch documents from an MSet which is not derived from a query.");
    }
    for (Xapian::doccount i = first; i <= last; ++i) {
	map<Xapian::doccount, Document>::const_iterator doc;
	doc = indexeddocs.find(i);
	if (doc == indexeddocs.end()) {
	    /* We don't have the document cached */
	    set<Xapian::doccount>::const_iterator s;
	    s = requested_docs.find(i);
	    if (s == requested_docs.end()) {
		/* We haven't even requested it yet - do so now. */
		enquire->request_doc(items[i - firstitem]);
		requested_docs.insert(i);
	    }
	}
    }
}

string
MSet::Internal::get_description() const
{
    string description = "Xapian::MSet::Internal(";

    description += "firstitem=" + str(firstitem) + ", " +
	    "matches_lower_bound=" + str(matches_lower_bound) + ", " +
	    "matches_estimated=" + str(matches_estimated) + ", " +
	    "matches_upper_bound=" + str(matches_upper_bound) + ", " +
	    "max_possible=" + str(max_possible) + ", " +
	    "max_attained=" + str(max_attained);

    for (vector<Xapian::Internal::MSetItem>::const_iterator i = items.begin();
	 i != items.end(); ++i) {
	if (!description.empty()) description += ", ";
	description += i->get_description();
    }

    description += ")";

    return description;
}

void
MSet::Internal::read_docs() const
{
    set<Xapian::doccount>::const_iterator i;
    for (i = requested_docs.begin(); i != requested_docs.end(); ++i) {
	indexeddocs[*i] = enquire->read_doc(items[*i - firstitem]);
	LOGLINE(MATCH, "stored doc at index " << *i << " is " << indexeddocs[*i]);
    }
    /* Clear list of requested but not fetched documents. */
    requested_docs.clear();
}

// Methods for Xapian::ESet

ESet::ESet() : internal(new Internal) { }

ESet::~ESet()
{
}

ESet::ESet(const ESet & other) : internal(other.internal)
{
}

void
ESet::operator=(const ESet &other)
{
    internal = other.internal;
}

Xapian::termcount
ESet::get_ebound() const
{
    return internal->ebound;
}

Xapian::termcount
ESet::size() const
{
    return internal->items.size();
}

bool
ESet::empty() const
{
    return internal->items.empty();
}

void
ESet::swap(ESet & other)
{
    std::swap(internal, other.internal);
}

ESetIterator
ESet::begin() const
{
    return ESetIterator(0, *this);
}

ESetIterator
ESet::end() const
{
    Assert(internal.get() != 0);
    return ESetIterator(internal->items.size(), *this);
}

ESetIterator
ESet::operator[](Xapian::termcount i) const
{
    // Don't test 0 <= i - that gives a compiler warning if i is unsigned
    Assert(0 < (i + 1) && i < size());
    return ESetIterator(i, *this);
}

ESetIterator
ESet::back() const
{
    Assert(!empty());
    Assert(internal.get() != 0);
    return ESetIterator(internal->items.size() - 1, *this);
}

string
ESet::get_description() const
{
    Assert(internal.get() != 0);
    return "Xapian::ESet(" + internal->get_description() + ")";
}

// Xapian::ESetIterator

const string &
ESetIterator::operator *() const
{
    return eset.internal->items[index].term;
}

Xapian::weight
ESetIterator::get_weight() const
{
    return eset.internal->items[index].wt;
}

string
ESetIterator::get_description() const
{
    return "Xapian::ESetIterator(" + str(index) + ")";
}

// MSetIterator

Xapian::docid
MSetIterator::operator *() const
{
    return mset.internal->items[index].did;
}

Document
MSetIterator::get_document() const
{
    return mset.internal->get_doc_by_index(index);
}

Xapian::weight
MSetIterator::get_weight() const
{
    return mset.internal->items[index].wt;
}

std::string
MSetIterator::get_collapse_key() const
{
    return mset.internal->items[index].collapse_key;
}

Xapian::doccount
MSetIterator::get_collapse_count() const
{
    return mset.internal->items[index].collapse_count;
}

Xapian::percent
MSetIterator::get_percent() const
{
    LOGCALL(API, Xapian::percent, "MSetIterator::get_percent", NO_ARGS);
    RETURN(mset.internal->convert_to_percent_internal(get_weight()));
}

string
MSetIterator::get_description() const
{
    return "Xapian::MSetIterator(" + str(index) + ")";
}

// Methods for Xapian::Enquire::Internal

Enquire::Internal::Internal(const Database &db_, ErrorHandler * errorhandler_)
  : db(db_), query(), collapse_key(Xapian::BAD_VALUENO), collapse_max(0),
    order(Enquire::ASCENDING), percent_cutoff(0), weight_cutoff(0),
    sort_key(Xapian::BAD_VALUENO), sort_by(REL), sort_value_forward(true),
    sorter(0), errorhandler(errorhandler_), weight(0)
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
Enquire::Internal::get_query()
{
    return query;
}

MSet
Enquire::Internal::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
			    Xapian::doccount check_at_least, const RSet *rset,
			    const MatchDecider *mdecider,
			    const MatchDecider *matchspy_legacy) const
{
    LOGCALL(MATCH, MSet, "Enquire::Internal::get_mset", first | maxitems | check_at_least | rset | mdecider | matchspy_legacy);

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

    Xapian::Weight::Internal stats;
    ::MultiMatch match(db, query.internal.get(), qlen, rset,
		       collapse_max, collapse_key,
		       percent_cutoff, weight_cutoff,
		       order, sort_key, sort_by, sort_value_forward,
		       errorhandler, stats, weight, spies,
		       (sorter != NULL),
		       (mdecider != NULL || matchspy_legacy != NULL));
    // Run query and put results into supplied Xapian::MSet object.
    MSet retval;
    match.get_mset(first, maxitems, check_at_least, retval,
		   stats, mdecider, matchspy_legacy, sorter);
    if (first_orig != first && retval.internal.get()) {
	retval.internal->firstitem = first_orig;
    }

    Assert(weight->name() != "bool" || retval.get_max_possible() == 0);

    // The Xapian::MSet needs to have a pointer to ourselves, so that it can
    // retrieve the documents.  This is set here explicitly to avoid having
    // to pass it into the matcher, which gets messy particularly in the
    // networked case.
    retval.internal->enquire = this;

    RETURN(retval);
}

ESet
Enquire::Internal::get_eset(Xapian::termcount maxitems,
                    const RSet & rset, int flags, double k,
		    const ExpandDecider * edecider, Xapian::weight min_wt) const
{
    LOGCALL(MATCH, ESet, "Enquire::Internal::get_eset", maxitems | rset | flags | k | edecider | min_wt);

    if (maxitems == 0 || rset.empty()) {
	// Either we were asked for no results, or wouldn't produce any
	// because no documents were marked as relevant.
	RETURN(ESet());
    }

    LOGVALUE(MATCH, rset.size());

    /* The AutoPtrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    AutoPtr<ExpandDecider> decider_noquery;
    AutoPtr<ExpandDecider> decider_andnoquery;

    if (!query.empty() && !(flags & Enquire::INCLUDE_QUERY_TERMS)) {
	AutoPtr<ExpandDecider> temp1(
	    new ExpandDeciderFilterTerms(query.get_terms_begin(),
					 query.get_terms_end()));
        decider_noquery = temp1;

	if (edecider) {
	    AutoPtr<ExpandDecider> temp2(
		new ExpandDeciderAnd(decider_noquery.get(), edecider));
	    decider_andnoquery = temp2;
	    edecider = decider_andnoquery.get();
	} else {
	    edecider = decider_noquery.get();
	}
    }

    bool use_exact_termfreq(flags & Enquire::USE_EXACT_TERMFREQ);
    ExpandWeight eweight(db, rset.size(), use_exact_termfreq, k);

    Xapian::ESet eset;
    eset.internal->expand(maxitems, db, rset, edecider, eweight, min_wt);
    RETURN(eset);
}

class ByQueryIndexCmp {
 private:
    typedef map<string, unsigned int> tmap_t;
    const tmap_t &tmap;
 public:
    ByQueryIndexCmp(const tmap_t &tmap_) : tmap(tmap_) {}
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
	throw Xapian::InvalidArgumentError("get_matching_terms with empty query");
	//return TermIterator();

    // The ordered list of terms in the query.
    TermIterator qt = query.get_terms_begin();
    TermIterator qt_end = query.get_terms_end();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    map<string, unsigned int> tmap;
    unsigned int index = 1;
    for ( ; qt != qt_end; ++qt) {
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
Enquire::Internal::request_doc(const Xapian::Internal::MSetItem &item) const
{
    try {
	unsigned int multiplier = db.internal.size();

	Xapian::docid realdid = (item.did - 1) / multiplier + 1;
	Xapian::doccount dbnumber = (item.did - 1) % multiplier;

	db.internal[dbnumber]->request_document(realdid);
    } catch (Error & e) {
	if (errorhandler) (*errorhandler)(e);
	throw;
    }
}

Document
Enquire::Internal::read_doc(const Xapian::Internal::MSetItem &item) const
{
    try {
	unsigned int multiplier = db.internal.size();

	Xapian::docid realdid = (item.did - 1) / multiplier + 1;
	Xapian::doccount dbnumber = (item.did - 1) % multiplier;

	Xapian::Document::Internal *doc;
	doc = db.internal[dbnumber]->collect_document(realdid);
	return Document(doc);
    } catch (Error & e) {
	if (errorhandler) (*errorhandler)(e);
	throw;
    }
}

// Methods of Xapian::Enquire

Enquire::Enquire(const Enquire & other) : internal(other.internal)
{
    LOGCALL_VOID(API, "Xapian::Enquire::Enquire", other);
}

void
Enquire::operator=(const Enquire & other)
{
    LOGCALL_VOID(API, "Xapian::Enquire::operator=", other);
    internal = other.internal;
}

Enquire::Enquire(const Database &databases, ErrorHandler * errorhandler)
    : internal(new Internal(databases, errorhandler))
{
    LOGCALL_VOID(API, "Xapian::Enquire::Enquire", databases);
}

Enquire::~Enquire()
{
    LOGCALL_VOID(API, "Xapian::Enquire::~Enquire", NO_ARGS);
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
    try {
	RETURN(internal->get_query());
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

void
Enquire::add_matchspy(MatchSpy * spy) {
    LOGCALL_VOID(API, "Xapian::Enquire::add_matchspy", spy);
    internal->spies.push_back(spy);
}

void
Enquire::clear_matchspies() {
    LOGCALL(API, const Xapian::Query &, "Xapian::Enquire::clear_matchspies", NO_ARGS);
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
Enquire::set_cutoff(Xapian::percent percent_cutoff, Xapian::weight weight_cutoff)
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

MSet
Enquire::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		  Xapian::doccount check_at_least, const RSet *rset,
		  const MatchDecider *mdecider,
		  const MatchDecider *matchspy) const
{
    LOGCALL(API, Xapian::MSet, "Xapian::Enquire::get_mset", first | maxitems | check_at_least | rset | mdecider | matchspy);

    try {
	RETURN(internal->get_mset(first, maxitems, check_at_least, rset,
				  mdecider, matchspy));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

MSet
Enquire::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		  Xapian::doccount check_at_least, const RSet *rset,
		  const MatchDecider *mdecider) const
{
    LOGCALL(API, Xapian::MSet, "Xapian::Enquire::get_mset", first | maxitems | check_at_least | rset | mdecider);

    try {
	RETURN(internal->get_mset(first, maxitems, check_at_least, rset,
				  mdecider, NULL));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

ESet
Enquire::get_eset(Xapian::termcount maxitems, const RSet & rset, int flags,
		  double k, const ExpandDecider * edecider) const
{
    LOGCALL(API, Xapian::ESet, "Xapian::Enquire::get_eset", maxitems | rset | flags | k | edecider);

    try {
	RETURN(internal->get_eset(maxitems, rset, flags, k, edecider, 0));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

ESet
Enquire::get_eset(Xapian::termcount maxitems, const RSet & rset, int flags,
		  double k, const ExpandDecider * edecider, Xapian::weight min_wt) const
{
    LOGCALL(API, Xapian::ESet, "Xapian::Enquire::get_eset", maxitems | rset | flags | k | edecider | min_wt);

    try {
	RETURN(internal->get_eset(maxitems, rset, flags, k, edecider, min_wt));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

TermIterator
Enquire::get_matching_terms_begin(const MSetIterator &it) const
{
    LOGCALL(API, Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", it);
    try {
	RETURN(internal->get_matching_terms(it));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

TermIterator
Enquire::get_matching_terms_begin(Xapian::docid did) const
{
    LOGCALL(API, Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", did);
    try {
	RETURN(internal->get_matching_terms(did));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

string
Enquire::get_description() const
{
    return "Xapian::Enquire(" + internal->get_description() + ")";
}

}
