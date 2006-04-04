/* omenquire.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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
#include "omdebug.h"

#include <xapian/document.h>
#include <xapian/enquire.h>
#include <xapian/error.h>
#include <xapian/errorhandler.h>
#include <xapian/expanddecider.h>
#include <xapian/termiterator.h>

#include "vectortermlist.h"

#include "rset.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "omenquireinternal.h"
#include "utils.h"

#include <vector>
#include "autoptr.h"
#include <algorithm>
#include <math.h>

using namespace std;

namespace Xapian {
   
ExpandDeciderFilterTerms::ExpandDeciderFilterTerms(TermIterator terms,
						   TermIterator termsend)
#ifndef __SUNPRO_CC
    : tset(terms, termsend)
{
}
#else
{
    // I'd prefer using an initialiser list for this, but it seems
    // that Solaris' CC doesn't like initialising a set with list
    // iterators or Xapian::TermIterators.
    while (terms != termsend) {
        tset.insert(*terms);
	++terms;
    }
}
#endif

int
ExpandDeciderFilterTerms::operator()(const string &tname) const
{
    /* Solaris CC returns an iterator from tset.find() const, and then
     * doesn't like comparing it to the const_iterator from end().
     * Therefore make sure we get a const_iterator to do the comparision.
     */
    set<string>::const_iterator i = tset.find(tname);
    return (i == tset.end());
}

ExpandDeciderAnd::ExpandDeciderAnd(const ExpandDecider *left_,
                                   const ExpandDecider *right_)
        : left(left_), right(right_) { }

int
ExpandDeciderAnd::operator()(const string &tname) const
{
    return ((*left)(tname)) && ((*right)(tname));
}

// Methods for Xapian::RSet

RSet::RSet() : internal(new RSet::Internal())
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
    internal->items.insert(did);
}

void
RSet::remove_document(Xapian::docid did)
{
    set<Xapian::docid>::iterator i = internal->items.find(did);
    if (i != internal->items.end()) internal->items.erase(i);
}

bool
RSet::contains(Xapian::docid did) const
{
    set<Xapian::docid>::iterator i = internal->items.find(did);
    return i != internal->items.end();
}

string
RSet::get_description() const
{
    DEBUGCALL(INTRO, string, "RSet::get_description", "");
    RETURN("RSet(" + internal->get_description() + ")");
}

string
RSet::Internal::to_string() const
{
    string result = om_tostring(items.size());

    set<Xapian::docid>::const_iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
	result += ' ';
	result += om_tostring(*i);
    }
    return result;
}

string
RSet::Internal::get_description() const
{
    DEBUGCALL(INTRO, string, "RSet::get_description", "");
    string description;

    set<Xapian::docid>::const_iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
	if (!description.empty()) description += ", ";
	description += om_tostring(*i);
    }

    description = "RSet(" + description + ")";

    RETURN(description);
}

namespace Internal {

// Methods for Xapian::MSetItem

string
MSetItem::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::MSetItem::get_description", "");
    string description;

    description = om_tostring(did) + ", " + om_tostring(wt) + ", " +
	    collapse_key;

    description = "Xapian::MSetItem(" + description + ")";

    RETURN(description);
}

}

// Methods for Xapian::MSet

MSet::MSet() : internal(new MSet::Internal())
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
    DEBUGAPICALL(void, "Xapian::MSet::fetch", beginiter << ", " << enditer);
    Assert(internal.get() != 0);
    if (beginiter != enditer)
	internal->fetch_items(beginiter.index, enditer.index - 1);
}

void
MSet::fetch(const MSetIterator & beginiter) const
{
    DEBUGAPICALL(void, "Xapian::MSet::fetch", beginiter);
    Assert(internal.get() != 0);
    internal->fetch_items(beginiter.index, beginiter.index);
}

void
MSet::fetch() const
{
    DEBUGAPICALL(void, "Xapian::MSet::fetch", "");
    Assert(internal.get() != 0);
    if (!internal->items.empty())
	internal->fetch_items(0, internal->items.size() - 1);
}

percent
MSet::convert_to_percent(Xapian::weight wt) const
{
    DEBUGAPICALL(Xapian::percent, "Xapian::MSet::convert_to_percent", wt);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(wt));
}

percent
MSet::convert_to_percent(const MSetIterator & it) const
{
    DEBUGAPICALL(Xapian::percent, "Xapian::MSet::convert_to_percent", it);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(it.get_weight()));
}

Xapian::doccount
MSet::get_termfreq(const string &tname) const
{
    DEBUGAPICALL(Xapian::doccount, "Xapian::MSet::get_termfreq", tname);
    map<string, Internal::TermFreqAndWeight>::const_iterator i;
    Assert(internal.get() != 0);
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw InvalidArgumentError("Term frequency of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termfreq);
}

Xapian::weight
MSet::get_termweight(const string &tname) const
{
    DEBUGAPICALL(Xapian::weight, "Xapian::MSet::get_termweight", tname);
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
    DEBUGCALL(INTRO, string, "Xapian::MSet::get_description", "");
    Assert(internal.get() != 0);
    RETURN("Xapian::MSet(" + internal->get_description() + ")");
}

percent
MSet::Internal::convert_to_percent_internal(Xapian::weight wt) const
{
    DEBUGAPICALL(Xapian::percent, "Xapian::MSet::Internal::convert_to_percent", wt);
    if (percent_factor == 0) RETURN(100);

    // FIXME: + 0.5 ???
    Xapian::percent pcent = static_cast<Xapian::percent>(wt * percent_factor);
    DEBUGLINE(API, "wt = " << wt << ", max_possible = "
	      << max_possible << " =>  pcent = " << pcent);
    if (pcent > 100) pcent = 100;
    if (pcent < 0) pcent = 0;
    if (pcent == 0 && wt > 0) pcent = 1;

    RETURN(pcent);
}

Document
MSet::Internal::get_doc_by_index(Xapian::doccount index) const
{
    DEBUGCALL(API, Document, "Xapian::MSet::Internal::Data::get_doc_by_index",
	      index);
    index += firstitem; 
    map<Xapian::doccount, Document>::const_iterator doc;
    doc = indexeddocs.find(index);
    if (doc != indexeddocs.end()) {
	RETURN(doc->second);
    }
    if (index < firstitem || index >= firstitem + items.size()) {
	throw RangeError("The mset returned from the match does not contain the document at index " + om_tostring(index));
    }
    fetch_items(index, index); // FIXME: this checks indexeddocs AGAIN!
    /* Actually read the fetched documents */
    read_docs();
    Assert(indexeddocs.find(index) != indexeddocs.end());
    Assert(indexeddocs.find(index)->first == index); // Paranoid assert
    RETURN(indexeddocs.find(index)->second);
}

void
MSet::Internal::fetch_items(Xapian::doccount first, Xapian::doccount last) const
{
    DEBUGAPICALL(void, "Xapian::MSet::Internal::Data::fetch_items",
		 first << ", " << last);
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

    description += "firstitem=" + om_tostring(firstitem) + ", " +
	    "matches_lower_bound=" + om_tostring(matches_lower_bound) + ", " +
	    "matches_estimated=" + om_tostring(matches_estimated) + ", " +
	    "matches_upper_bound=" + om_tostring(matches_upper_bound) + ", " +
	    "max_possible=" + om_tostring(max_possible) + ", " +
	    "max_attained=" + om_tostring(max_attained);

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
	DEBUGLINE(API, "stored doc at index " << *i << " is " << indexeddocs[*i]);
    }
    /* Clear list of requested but not fetched documents. */
    requested_docs.clear();
}

// Methods for Xapian::Internal::ESetItem

string
Xapian::Internal::ESetItem::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::Internal::ESetItem::get_description", "");
    RETURN("Xapian::Internal::ESetItem(" + tname + ", " + om_tostring(wt) + ")");
}

// Methods for Xapian::ESet

ESet::ESet() : internal(new Internal()) { }

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
    DEBUGCALL(INTRO, string, "Xapian::ESet::get_description", "");
    Assert(internal.get() != 0);
    RETURN("Xapian::ESet(" + internal->get_description() + ")");
}

//////////////////////////////////
// Methods for Xapian::ESet::Internal //
//////////////////////////////////

string
Xapian::ESet::Internal::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::ESet::Internal::get_description", "");
    string description = "ebound=" + om_tostring(ebound);

    for (vector<Xapian::Internal::ESetItem>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	description += ", " + i->get_description();
    }

    RETURN("Xapian::ESet::Internal(" + description + ")");
}

// Xapian::ESetIterator

const string &
ESetIterator::operator *() const
{
    return eset.internal->items[index].tname;
}

Xapian::weight
ESetIterator::get_weight() const
{
    return eset.internal->items[index].wt;
}

string
ESetIterator::get_description() const
{
    return "Xapian::ESetIterator(" + om_tostring(index) + ")";
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

Xapian::doccount
MSetIterator::get_collapse_count() const
{
    return mset.internal->items[index].collapse_count;
}

Xapian::percent
MSetIterator::get_percent() const
{
    DEBUGAPICALL(Xapian::percent, "MSetIterator::get_percent", "");
    RETURN(mset.internal->convert_to_percent_internal(get_weight()));
}

string
MSetIterator::get_description() const
{
    return "Xapian::MSetIterator(" + om_tostring(index) + ")";
}

// Methods for Xapian::Enquire::Internal

Enquire::Internal::Internal(const Database &db_, ErrorHandler * errorhandler_)
  : db(db_), query(), collapse_key(Xapian::valueno(-1)),
    order(Enquire::ASCENDING), percent_cutoff(0), weight_cutoff(0),
    sort_key(Xapian::valueno(-1)), sort_by(REL), sort_value_forward(true),
    bias_halflife(0), bias_weight(0), errorhandler(errorhandler_), weight(0)
{
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
			    const MatchDecider *mdecider) const
{
    DEBUGCALL(API, MSet, "Enquire::Internal::get_mset", first << ", "
	      << maxitems << ", " << check_at_least << ", " << rset << ", "
	      << mdecider);

    if (weight == 0) {
	weight = new BM25Weight;
    }

    MSet retval;
    if (rset == 0) {
	::MultiMatch match(db, query.internal.get(), qlen, RSet(), collapse_key,
		       percent_cutoff, weight_cutoff,
		       order, sort_key, sort_by, sort_value_forward,
		       bias_halflife, bias_weight, errorhandler,
		       new LocalStatsGatherer(), weight);
	// Run query and put results into supplied Xapian::MSet object.
	match.get_mset(first, maxitems, check_at_least, retval, mdecider);
    } else {
	::MultiMatch match(db, query.internal.get(), qlen, *rset, collapse_key,
		       percent_cutoff, weight_cutoff,
		       order, sort_key, sort_by, sort_value_forward,
		       bias_halflife, bias_weight, errorhandler,
		       new LocalStatsGatherer(), weight);
	// Run query and put results into supplied Xapian::MSet object.
	match.get_mset(first, maxitems, check_at_least, retval, mdecider);
    }

    Assert(weight->name() != "bool" || retval.get_max_possible() == 0);

    // The Xapian::MSet needs to have a pointer to ourselves, so that it can
    // retrieve the documents.  This is set here explicitly to avoid having
    // to pass it into the matcher, which gets messy particularly in the
    // networked case.
    retval.internal->enquire = this;

    return retval;
}

ESet
Enquire::Internal::get_eset(Xapian::termcount maxitems,
                    const RSet & rset, int flags, double k,
		    const ExpandDecider * edecider) const
{
    ESet retval;

    // FIXME: make expand and rseti take a refcntptr
    OmExpand expand(db);
    RSetI rseti(db, rset);

    DEBUGLINE(API, "rset size is " << rset.size());

    /* The AutoPtrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    AutoPtr<ExpandDecider> decider_noquery;
    AutoPtr<ExpandDecider> decider_andnoquery;
    ExpandDeciderAlways decider_always;

    if (!query.empty() && !(flags & Enquire::include_query_terms)) {
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
    } else if (edecider == 0) {
	edecider = &decider_always;
    }

    expand.expand(maxitems, retval, &rseti, edecider,
		  bool(flags & Enquire::use_exact_termfreq), k);

    return retval;
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
	//return TermIterator(NULL);

    // The ordered list of terms in the query.
    TermIterator qt = query.get_terms_begin();
    TermIterator qt_end = query.get_terms_end();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    map<string, unsigned int> tmap;
    unsigned int index = 1;
    for ( ; qt != qt_end; qt++) {
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
	docterms++;
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

void
Enquire::Internal::register_match_decider(const string &name,
	const MatchDecider *mdecider)
{
    if (mdecider) {
	mdecider_map[name] = mdecider;
    } else {
	mdecider_map.erase(name);
    }
}

// Methods of Xapian::Enquire

Enquire::Enquire(const Database &databases, ErrorHandler * errorhandler)
    : internal(new Internal(databases, errorhandler))
{
    DEBUGAPICALL(void, "Xapian::Enquire::Enquire", databases);
}

Enquire::~Enquire()
{
    DEBUGAPICALL(void, "Xapian::Enquire::~Enquire", "");
}

void
Enquire::set_query(const Query & query, termcount len)
{
    DEBUGAPICALL(void, "Xapian::Enquire::set_query", query << ", " << len);
    internal->set_query(query, len);
}

const Query &
Enquire::get_query()
{
    DEBUGAPICALL(const Xapian::Query &, "Xapian::Enquire::get_query", "");
    try {
	RETURN(internal->get_query());
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

void
Enquire::set_weighting_scheme(const Weight &weight_)
{
    DEBUGAPICALL(void, "Xapian::Enquire::set_weighting_scheme", "[Weight]");
    delete internal->weight;
    internal->weight = weight_.clone();
}

void
Enquire::set_collapse_key(Xapian::valueno collapse_key)
{
    internal->collapse_key = collapse_key;
}

void
Enquire::set_docid_order(Enquire::docid_order order)
{
    internal->order = order;
}

void
Enquire::set_sort_forward(bool sort_forward)
{
    Enquire::set_docid_order(sort_forward ? ASCENDING : DESCENDING);
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
Enquire::set_sort_by_value(Xapian::valueno sort_key, bool ascending)
{
    internal->sort_key = sort_key;
    internal->sort_by = Internal::VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_value_then_relevance(Xapian::valueno sort_key,
					  bool ascending)
{
    internal->sort_key = sort_key;
    internal->sort_by = Internal::VAL_REL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sort_by_relevance_then_value(Xapian::valueno sort_key,
					  bool ascending)
{
    internal->sort_key = sort_key;
    internal->sort_by = Internal::REL_VAL;
    internal->sort_value_forward = ascending;
}

void
Enquire::set_sorting(Xapian::valueno sort_key, int sort_bands,
		     bool sort_by_relevance)
{
    if (sort_bands > 1) {
	throw Xapian::UnimplementedError("sort bands are no longer supported");
    }
    if (sort_bands == 0 || sort_key == Xapian::valueno(-1)) {
	Enquire::set_sort_by_relevance();
    } else if (!sort_by_relevance) {
	Enquire::set_sort_by_value(sort_key);
    } else {
	Enquire::set_sort_by_value_then_relevance(sort_key);
    }
}

void
Enquire::set_bias(Xapian::weight bias_weight, time_t bias_halflife)
{
    internal->bias_weight = bias_weight;
    internal->bias_halflife = bias_halflife;
}

MSet
Enquire::get_mset(Xapian::doccount first, Xapian::doccount maxitems,
		  Xapian::doccount check_at_least, const RSet *rset,
		  const MatchDecider *mdecider) const
{
    // FIXME: display contents of pointer params, if they're not null.
    DEBUGAPICALL(Xapian::MSet, "Xapian::Enquire::get_mset", first << ", " <<
		 maxitems << ", " << check_at_least << ", " << rset << ", " <<
		 mdecider);

    try {
	RETURN(internal->get_mset(first, maxitems, check_at_least, rset,
				  mdecider));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

ESet
Enquire::get_eset(Xapian::termcount maxitems, const RSet & rset, int flags,
		    double k, const ExpandDecider * edecider) const
{
    // FIXME: display contents of pointer params and rset, if they're not
    // null.
    DEBUGAPICALL(Xapian::ESet, "Xapian::Enquire::get_eset", maxitems << ", " <<
		 rset << ", " << flags << ", " << k << ", " << edecider);

    try {
	// FIXME: this copies the eset too much: pass it in by reference?
	RETURN(internal->get_eset(maxitems, rset, flags, k, edecider));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

TermIterator
Enquire::get_matching_terms_begin(const MSetIterator &it) const
{
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", it);
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
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", did);
    try {
	RETURN(internal->get_matching_terms(did));
    } catch (Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

void
Enquire::register_match_decider(const string &name,
				  const MatchDecider *mdecider)
{
    internal->register_match_decider(name, mdecider);
}

string
Enquire::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::Enquire::get_description", "");
    RETURN("Xapian::Enquire(" + internal->get_description() + ")");
}

}
