/* omenquire.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "omdebug.h"
#include "omdatabaseinternal.h"
#include "omdocumentinternal.h"

#include <xapian/error.h>
#include <xapian/enquire.h>
#include <xapian/output.h>
#include <xapian/termiterator.h>
#include <xapian/expanddecider.h>

#include "omtermlistiteratorinternal.h"

#include "rset.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "om/omdocument.h"
#include <xapian/errorhandler.h>
#include <xapian/enquire.h>
#include "omenquireinternal.h"
#include "utils.h"

#include <vector>
#include "autoptr.h"
#include <algorithm>
#include <math.h>

using namespace std;

Xapian::ExpandDeciderFilterTerms::ExpandDeciderFilterTerms(Xapian::TermIterator terms,
						       Xapian::TermIterator termsend)
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
Xapian::ExpandDeciderFilterTerms::operator()(const string &tname) const
{
    /* Solaris CC returns an iterator from tset.find() const, and then
     * doesn't like comparing it to the const_iterator from end().
     * Therefore make sure we get a const_iterator to do the comparision.
     */
    set<string>::const_iterator i = tset.find(tname);
    return (i == tset.end());
}

Xapian::ExpandDeciderAnd::ExpandDeciderAnd(const Xapian::ExpandDecider *left_,
                                       const Xapian::ExpandDecider *right_)
        : left(left_), right(right_) {}

int
Xapian::ExpandDeciderAnd::operator()(const string &tname) const
{
    return ((*left)(tname)) && ((*right)(tname));
}

////////////////////////
// Methods for OmRSet //
////////////////////////

OmRSet::OmRSet()
	: internal(new OmRSet::Internal())
{
}

OmRSet::OmRSet(const OmRSet &other)
	: internal(new OmRSet::Internal(*other.internal))
{
}

void
OmRSet::operator=(const OmRSet &other)
{
    *internal = *other.internal;
}

OmRSet::~OmRSet()
{
    delete internal;
}

om_doccount
OmRSet::size() const
{
    return internal->items.size();
}

bool
OmRSet::empty() const
{
    return internal->items.empty();
}

void
OmRSet::add_document(om_docid did)
{
    internal->items.insert(did);
}

void
OmRSet::remove_document(om_docid did)
{
    set<om_docid>::iterator i = internal->items.find(did);
    if (i != internal->items.end()) internal->items.erase(i);
}

bool
OmRSet::contains(om_docid did) const
{
    set<om_docid>::iterator i = internal->items.find(did);
    return i != internal->items.end();
}

string
OmRSet::get_description() const
{
    DEBUGCALL(INTRO, string, "OmRSet::get_description", "");
    RETURN("OmRSet(" + internal->get_description() + ")");
}

string
OmRSet::Internal::get_description() const
{
    DEBUGCALL(INTRO, string, "OmRSet::get_description", "");
    string description;

    for (set<om_docid>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (!description.empty()) description += ", ";
	description += om_tostring(*i);
    }

    description = "OmRSet(" + description + ")";

    RETURN(description);
}

////////////////////////////
// Methods for OmMSetItem //
////////////////////////////

string
OmMSetItem::get_description() const
{
    DEBUGCALL(INTRO, string, "OmMSetItem::get_description", "");
    string description;

    description = om_tostring(did) + ", " + om_tostring(wt) + ", " +
	    collapse_key;

    description = "OmMSetItem(" + description + ")";

    RETURN(description);
}

// Methods for Xapian::MSet

namespace Xapian {

MSet::MSet() : internal(new MSet::Internal())
{
}

MSet::MSet(MSet::Internal * internal_) : internal(internal_)
{
}

MSet::~MSet()
{
}

MSet::MSet(const Xapian::MSet & other) : internal(other.internal)
{
}

void
MSet::operator=(const Xapian::MSet &other)
{
    internal = other.internal;
}

void
MSet::fetch(const OmMSetIterator & beginiter,
	    const OmMSetIterator & enditer) const
{
    DEBUGAPICALL(void, "Xapian::MSet::fetch", beginiter << ", " << enditer);
    Assert(internal.get() != 0);

    // we have to convert the MSetIterators into vector iterators
    // FIXME: this is messy
    if (beginiter.internal == 0) return;

    vector<OmMSetItem>::const_iterator end_vecit;
    if (enditer.internal == 0)
	end_vecit = internal->items.end();
    else
	end_vecit = enditer.internal->it;

    internal->fetch_items(beginiter.get_rank(),
			  beginiter.internal->it, end_vecit);
}

void
MSet::fetch(const OmMSetIterator & beginiter) const
{
    DEBUGAPICALL(void, "Xapian::MSet::fetch", beginiter);
    Assert(internal.get() != 0);

    // we have to convert the MSetIterators into vector iterators
    // FIXME: this is messy
    if (beginiter.internal == 0) return;

    OmMSetIterator enditer = beginiter;
    enditer++;

    vector<OmMSetItem>::const_iterator end_vecit;
    if (enditer.internal == 0)
	end_vecit = internal->items.end();
    else
	end_vecit = enditer.internal->it;

    internal->fetch_items(beginiter.get_rank(),
			  beginiter.internal->it, end_vecit);
}

void
MSet::fetch() const
{
    DEBUGAPICALL(void, "Xapian::MSet::fetch", "");
    Assert(internal.get() != 0);
    internal->fetch_items(internal->firstitem, internal->items.begin(),
			  internal->items.end());
}

percent
MSet::convert_to_percent(om_weight wt) const
{
    DEBUGAPICALL(Xapian::percent, "Xapian::MSet::convert_to_percent", wt);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(wt));
}

percent
MSet::convert_to_percent(const OmMSetIterator & it) const
{
    DEBUGAPICALL(Xapian::percent, "Xapian::MSet::convert_to_percent", it);
    Assert(internal.get() != 0);
    RETURN(internal->convert_to_percent_internal(it.get_weight()));
}

om_doccount
MSet::get_termfreq(const string &tname) const
{
    DEBUGAPICALL(om_doccount, "Xapian::MSet::get_termfreq", tname);
    map<string, Internal::TermFreqAndWeight>::const_iterator i;
    Assert(internal.get() != 0);
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw Xapian::InvalidArgumentError("Term frequency of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termfreq);
}

om_weight
MSet::get_termweight(const string &tname) const
{
    DEBUGAPICALL(om_weight, "Xapian::MSet::get_termweight", tname);
    map<string, Internal::TermFreqAndWeight>::const_iterator i;
    Assert(internal.get() != 0);
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw Xapian::InvalidArgumentError("Term weight of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termweight);
}

om_doccount
MSet::get_firstitem() const
{
    Assert(internal.get() != 0);
    return internal->firstitem;
}

om_doccount
MSet::get_matches_lower_bound() const
{
    Assert(internal.get() != 0);
    return internal->matches_lower_bound;
}

om_doccount
MSet::get_matches_estimated() const
{
    Assert(internal.get() != 0);
    return internal->matches_estimated;
}

om_doccount
MSet::get_matches_upper_bound() const
{
    Assert(internal.get() != 0);
    return internal->matches_upper_bound;
}

om_weight
MSet::get_max_possible() const
{
    Assert(internal.get() != 0);
    return internal->max_possible;
}

om_weight
MSet::get_max_attained() const
{
    Assert(internal.get() != 0);
    return internal->max_attained;
}

om_doccount
MSet::size() const
{
    Assert(internal.get() != 0);
    return internal->items.size();
}

om_doccount
MSet::max_size() const
{
    Assert(internal.get() != 0);
    return om_doccount(-1); // FIXME: is there a better way to do this?
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

OmMSetIterator
MSet::begin() const
{
    Assert(internal.get() != 0);
    if (internal->items.empty()) return OmMSetIterator(NULL);

    return OmMSetIterator(new OmMSetIterator::Internal(
	    internal->items.begin(), internal->items.end(),
	    internal->firstitem, internal));
}

OmMSetIterator
MSet::end() const
{
    return OmMSetIterator(NULL);
}

OmMSetIterator
MSet::operator[](om_doccount i) const
{
    // Don't test 0 <= i - that gives a compiler warning if i is unsigned
    Assert(0 < (i + 1) && i < size());
    Assert(internal.get() != 0);
    return OmMSetIterator(new OmMSetIterator::Internal(
	internal->items.begin() + i,
	internal->items.end(),
	internal->firstitem + i,
	internal));
}

OmMSetIterator
MSet::back() const
{
    Assert(!empty());
    Assert(internal.get() != 0);
    return OmMSetIterator(new OmMSetIterator::Internal(
	internal->items.begin() + internal->items.size() - 1,
	internal->items.end(),
	internal->firstitem + internal->items.size() - 1,
	internal));
}

string
MSet::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::MSet::get_description", "");
    Assert(internal.get() != 0);
    RETURN("Xapian::MSet(" + internal->get_description() + ")");
}

percent
MSet::Internal::convert_to_percent_internal(om_weight wt) const
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

OmDocument
MSet::Internal::get_doc_by_rank(om_doccount rank) const
{
    DEBUGCALL(API, OmDocument, "Xapian::MSet::Internal::Data::get_doc_by_rank", rank);
    map<om_doccount, OmDocument>::const_iterator doc;
    doc = rankeddocs.find(rank);
    if (doc != rankeddocs.end()) {
	RETURN(doc->second);
    }
    if (rank < firstitem || rank >= firstitem + items.size()) {
	throw Xapian::RangeError("The mset returned from the match does not contain the document at rank " + om_tostring(rank));
    }
    fetch_items(rank,
		items.begin() + (rank - firstitem),
		items.begin() + (rank - firstitem) + 1);
    /* Actually read the fetched documents */
    read_docs();
    Assert(rankeddocs.find(rank) != rankeddocs.end());
    Assert(rankeddocs.find(rank)->first == rank); // Paranoid assert
    RETURN(rankeddocs.find(rank)->second);
}

void
MSet::Internal::fetch_items(
	om_doccount rank,
	vector<OmMSetItem>::const_iterator begin,
	vector<OmMSetItem>::const_iterator end) const
{
#ifdef MUS_DEBUG_VERBOSE
    unsigned int count = 0;
    for (vector<OmMSetItem>::const_iterator i = begin; i != end; i++)
	count++;
    DEBUGAPICALL(void, "Xapian::MSet::Internal::Data::fetch_items",
		 rank << ", " <<
		 "[" << count << " items]");
#endif
    if (enquire.get() == 0) {
	throw Xapian::InvalidOperationError("Can't fetch documents from an Mset which is not derived from a query.");
    }
    om_doccount currrank = rank;
    vector<OmMSetItem>::const_iterator i;
    for (i = begin; i != end; ++i, ++currrank) {
	map<om_doccount, OmDocument>::const_iterator doc;
	doc = rankeddocs.find(currrank);
	if (doc == rankeddocs.end()) {
	    /* We don't have the document cached */
	    set<om_doccount>::const_iterator s;
	    s = requested_docs.find(currrank);
	    if (s == requested_docs.end()) {
		/* We haven't even requested it yet - do so now. */
		enquire->request_doc(*i);
		requested_docs.insert(currrank);
	    }
	}
    }
}

string
MSet::Internal::get_description() const
{
    string description = "Xapian::MSet::Internal(";

    description = "firstitem=" + om_tostring(firstitem) + ", " +
	    "matches_lower_bound=" + om_tostring(matches_lower_bound) + ", " +
	    "matches_estimated=" + om_tostring(matches_estimated) + ", " +
	    "matches_upper_bound=" + om_tostring(matches_upper_bound) + ", " +
	    "max_possible=" + om_tostring(max_possible) + ", " +
	    "max_attained=" + om_tostring(max_attained);

    for (vector<OmMSetItem>::const_iterator i = items.begin();
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
    set<om_doccount>::const_iterator i;
    for (i = requested_docs.begin(); i != requested_docs.end(); ++i) {
	rankeddocs[*i] = enquire->read_doc(items[*i - firstitem]);
	DEBUGLINE(API, "stored doc at rank " << *i << " is " << rankeddocs[*i]);
    }
    /* Clear list of requested but not fetched documents. */
    requested_docs.clear();
}

}

////////////////////////////
// Methods for OmESetItem //
////////////////////////////

string
OmESetItem::get_description() const
{
    DEBUGCALL(INTRO, string, "OmESetItem::get_description", "");
    RETURN("OmESetItem(" + tname + ", " + om_tostring(wt) + ")");
}

////////////////////////
// Methods for OmESet //
////////////////////////

OmESet::OmESet() : internal(new OmESet::Internal()) {}

OmESet::~OmESet()
{
    delete internal;
}

OmESet::OmESet(const OmESet & other)
	: internal(new OmESet::Internal(*other.internal))
{
}

void
OmESet::operator=(const OmESet &other)
{
    *internal = *other.internal;
}

om_termcount
OmESet::get_ebound() const
{
    return internal->ebound;
}

om_termcount
OmESet::size() const
{
    return internal->items.size();
}

bool
OmESet::empty() const
{
    return internal->items.empty();
}

OmESetIterator
OmESet::begin() const
{
    if (internal->items.begin() == internal->items.end()) {
	return OmESetIterator(NULL);
    } else {
	return OmESetIterator(new OmESetIterator::Internal(internal->items.begin(),
							   internal->items.end()));
    }
}

OmESetIterator
OmESet::end() const
{
    return OmESetIterator(NULL);
}

string
OmESet::get_description() const
{
    DEBUGCALL(INTRO, string, "OmESet::get_description", "");
    RETURN("OmESet(" + internal->get_description() + ")");
}

//////////////////////////////////
// Methods for OmESet::Internal //
//////////////////////////////////

string
OmESet::Internal::get_description() const
{
    DEBUGCALL(INTRO, string, "OmESet::Internal::get_description", "");
    string description = "ebound=" + om_tostring(ebound);

    for (vector<OmESetItem>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	description += ", " + i->get_description();
    }

    RETURN("OmESet::Internal(" + description + ")");
}

// OmESetIterator

OmESetIterator::OmESetIterator(Internal *internal_) : internal(internal_)
{
}

OmESetIterator::OmESetIterator() : internal(0)
{
}

OmESetIterator::~OmESetIterator()
{
    delete internal;
}

OmESetIterator::OmESetIterator(const OmESetIterator &other)
    : internal((other.internal == 0) ? 0 :
	       new OmESetIterator::Internal(*(other.internal)))
{
}

void
OmESetIterator::operator=(const OmESetIterator &other)
{
    if (other.internal == 0) {
	delete internal;
	internal = 0;
    } else if (internal == 0) {
	internal = new OmESetIterator::Internal(other.internal->it,
						other.internal->end);
    } else {
	OmESetIterator::Internal temp(other.internal->it,
				      other.internal->end);
	swap(*internal, temp);
    }
}

OmESetIterator &
OmESetIterator::operator++()
{
    ++internal->it;
    if (internal->it == internal->end) {
	delete internal;
	internal = NULL;
    }
    return *this;
}

void
OmESetIterator::operator++(int)
{
    ++internal->it;
    if (internal->it == internal->end) {
	delete internal;
	internal = NULL;
    }
}

const string &
OmESetIterator::operator *() const
{
    return internal->it->tname;
}

om_weight
OmESetIterator::get_weight() const
{
    return internal->it->wt;
}

string
OmESetIterator::get_description() const
{
    return "OmESetIterator()"; // FIXME
}

bool
operator==(const OmESetIterator &a, const OmESetIterator &b)
{
    if (a.internal == b.internal) return true;
    if (!a.internal || !b.internal) return false;
    return (*(a.internal) == *(b.internal));
}

// OmMSetIterator

OmMSetIterator::OmMSetIterator(Internal *internal_) : internal(internal_)
{
}

OmMSetIterator::OmMSetIterator() : internal(0)
{
}

OmMSetIterator::~OmMSetIterator()
{
    delete internal;
}

OmMSetIterator::OmMSetIterator(const OmMSetIterator &other)
    : internal((other.internal == 0) ? 0 :
	       new OmMSetIterator::Internal(*(other.internal)))
{
}

void
OmMSetIterator::operator=(const OmMSetIterator &other)
{
    if (other.internal == 0) {
	delete internal;
	internal = 0;
    } else if (internal == 0) {
	internal = new OmMSetIterator::Internal(other.internal->it,
						other.internal->end,
						other.internal->currrank,
						other.internal->msetdata);
    } else {
	OmMSetIterator::Internal temp(other.internal->it,
				      other.internal->end,
				      other.internal->currrank,
				      other.internal->msetdata);
	swap(*internal, temp);
    }
}

OmMSetIterator &
OmMSetIterator::operator++()
{
    ++internal->it;
    ++internal->currrank;
    if (internal->it == internal->end) {
	delete internal;
	internal = NULL;
    }
    return *this;
}

void
OmMSetIterator::operator++(int)
{
    ++internal->it;
    ++internal->currrank;
    if (internal->it == internal->end) {
	delete internal;
	internal = NULL;
    }
}

om_docid
OmMSetIterator::operator *() const
{
    return internal->it->did;
}

OmDocument
OmMSetIterator::get_document() const
{
    return internal->msetdata->get_doc_by_rank(internal->currrank);
}

om_doccount
OmMSetIterator::get_rank() const
{
    return internal->currrank;
}

om_weight
OmMSetIterator::get_weight() const
{
    return internal->it->wt;
}

om_doccount
OmMSetIterator::get_collapse_count() const
{
    return internal->it->collapse_count;
}

Xapian::percent
OmMSetIterator::get_percent() const
{
    DEBUGAPICALL(Xapian::percent, "OmMSetIterator::get_percent", "");
    RETURN(internal->msetdata->convert_to_percent_internal(internal->it->wt));
}

string
OmMSetIterator::get_description() const
{
    return "OmMSetIterator()"; // FIXME
}

bool
operator==(const OmMSetIterator &a, const OmMSetIterator &b)
{
    if (a.internal == b.internal) return true;
    if (!a.internal || !b.internal) return false;
    return (*(a.internal) == *(b.internal));
}

// Methods for Xapian::Enquire::Internal

Xapian::Enquire::Internal::Internal(const OmDatabase &db_,
			    Xapian::ErrorHandler * errorhandler_)
  : db(db_), query(0), collapse_key(om_valueno(-1)), sort_forward(true), 
    percent_cutoff(0), weight_cutoff(0), sort_key(om_valueno(-1)),
    sort_bands(0), bias_halflife(0), bias_weight(0),
    errorhandler(errorhandler_), weight(0)
{
}

Xapian::Enquire::Internal::~Internal()
{
    delete query;
    query = 0;
    delete weight;
    weight = 0;
}

void
Xapian::Enquire::Internal::set_query(const Xapian::Query &query_)
{
    query = new Xapian::Query(query_);
}

const Xapian::Query &
Xapian::Enquire::Internal::get_query()
{
    if (query == 0) {
        throw Xapian::InvalidArgumentError("Can't get query before setting it");
    }
    return *query;
}

Xapian::MSet
Xapian::Enquire::Internal::get_mset(om_doccount first, om_doccount maxitems,
                    const OmRSet *omrset, const OmMatchDecider *mdecider) const
{
    DEBUGCALL(API, Xapian::MSet, "Enquire::Internal::get_mset", first << ", "
	      << maxitems << ", " << omrset << ", " << mdecider);
    if (query == 0) {
        throw Xapian::InvalidArgumentError("You must set a query before calling Xapian::Enquire::get_mset()");
    }

    // Set Rset
    OmRSet emptyrset;
    if (omrset == 0) {
	omrset = &emptyrset;
    }

    if (weight == 0) {
	weight = new BM25Weight;
    }

    // FIXME: make match take a refcntptr
    MultiMatch match(db, query->internal.get(), *omrset, collapse_key,
		     percent_cutoff, weight_cutoff,
		     sort_forward, sort_key, sort_bands,
		     bias_halflife, bias_weight, errorhandler,
		     AutoPtr<StatsGatherer>(new LocalStatsGatherer()), weight);

    // Run query and get results into supplied Xapian::MSet object
    Xapian::MSet retval;
    match.get_mset(first, maxitems, retval, mdecider);

    Assert(weight->name() != "bool" || retval.get_max_possible() == 0);

    // The Xapian::MSet needs to have a pointer to ourselves, so that it can
    // retrieve the documents.  This is set here explicitly to avoid having
    // to pass it into the matcher, which gets messy particularly in the
    // networked case.
    //Xapian::Internal::RefCntPtr<Xapian::Enquire::Internal> tmp(this);
    retval.internal->enquire = this;

    return retval;
}

OmESet
Xapian::Enquire::Internal::get_eset(om_termcount maxitems,
                    const OmRSet & omrset, int flags, double k,
		    const Xapian::ExpandDecider * edecider) const
{
    OmESet retval;

    // FIXME: make expand and rset take a refcntptr
    OmExpand expand(db);
    RSet rset(db, omrset);

    DEBUGLINE(API, "rset size is " << omrset.size());

    /* The AutoPtrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    AutoPtr<Xapian::ExpandDecider> decider_noquery;
    AutoPtr<Xapian::ExpandDecider> decider_andnoquery;
    Xapian::ExpandDeciderAlways decider_always;

    if (query != 0 && !(flags & Xapian::Enquire::include_query_terms)) {
	AutoPtr<Xapian::ExpandDecider> temp1(
	    new Xapian::ExpandDeciderFilterTerms(query->get_terms_begin(),
					   query->get_terms_end()));
        decider_noquery = temp1;

	if (edecider) {
	    AutoPtr<Xapian::ExpandDecider> temp2(
		new Xapian::ExpandDeciderAnd(decider_noquery.get(), edecider));
	    decider_andnoquery = temp2;
	    edecider = decider_andnoquery.get();
	} else {
	    edecider = decider_noquery.get();
	}
    } else if (edecider == 0) {
	edecider = &decider_always;
    }

    expand.expand(maxitems, retval, &rset, edecider,
		  bool(flags & Xapian::Enquire::use_exact_termfreq), k);

    return retval;
}

Xapian::TermIterator
Xapian::Enquire::Internal::get_matching_terms(om_docid did) const
{
    return calc_matching_terms(did);
}

Xapian::TermIterator
Xapian::Enquire::Internal::get_matching_terms(const OmMSetIterator &it) const
{
    // FIXME: take advantage of OmMSetIterator to ensure that database
    // doesn't get modified underneath us.
    return calc_matching_terms(*it);
}

string
Xapian::Enquire::Internal::get_description() const
{
    string description = db.get_description();
    if (query) description += ", " + query->get_description();
    return description;
}

// Private methods for Xapian::Enquire::Internal

void
Xapian::Enquire::Internal::request_doc(const OmMSetItem &item) const
{
    try {
	unsigned int multiplier = db.internal->databases.size();

	om_docid realdid = (item.did - 1) / multiplier + 1;
	om_doccount dbnumber = (item.did - 1) % multiplier;

	db.internal->databases[dbnumber]->request_document(realdid);
    } catch (Xapian::Error & e) {
	if (errorhandler) (*errorhandler)(e);
	throw;
    }
}

OmDocument
Xapian::Enquire::Internal::read_doc(const OmMSetItem &item) const
{
    try {
	unsigned int multiplier = db.internal->databases.size();

	om_docid realdid = (item.did - 1) / multiplier + 1;
	om_doccount dbnumber = (item.did - 1) % multiplier;

	::Document *doc = db.internal->databases[dbnumber]->collect_document(realdid);
	return OmDocument(new OmDocument::Internal(doc, db, item.did));
    } catch (Xapian::Error & e) {
	if (errorhandler) (*errorhandler)(e);
	throw;
    }
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

Xapian::TermIterator
Xapian::Enquire::Internal::calc_matching_terms(om_docid did) const
{
    if (query == 0) {
        throw Xapian::InvalidArgumentError("Can't get matching terms before setting query");
    }

    // the ordered list of terms in the query.
    Xapian::TermIterator qt = query->get_terms_begin();
    Xapian::TermIterator qt_end = query->get_terms_end();

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

    Xapian::TermIterator docterms = db.termlist_begin(did);
    Xapian::TermIterator docterms_end = db.termlist_end(did);
    while (docterms != docterms_end) {
	string term = *docterms;
        map<string, unsigned int>::iterator t = tmap.find(term);
        if (t != tmap.end()) matching_terms.push_back(term);
	docterms++;
    }

    // sort the resulting list by query position.
    sort(matching_terms.begin(), matching_terms.end(), ByQueryIndexCmp(tmap));

    return Xapian::TermIterator(new VectorTermList(matching_terms.begin(),
						   matching_terms.end()));
}

void
Xapian::Enquire::Internal::register_match_decider(const string &name,
	const OmMatchDecider *mdecider)
{
    if (mdecider) {
	mdecider_map[name] = mdecider;
    } else {
	mdecider_map.erase(name);
    }
}

// Methods of Xapian::Enquire

Xapian::Enquire::Enquire(const OmDatabase &databases,
			 Xapian::ErrorHandler * errorhandler)
    : internal(new Xapian::Enquire::Internal(databases, errorhandler))
{
    DEBUGAPICALL(void, "Xapian::Enquire::Enquire", databases);
}

Xapian::Enquire::~Enquire()
{
    DEBUGAPICALL(void, "Xapian::Enquire::~Enquire", "");
}

void
Xapian::Enquire::set_query(const Xapian::Query & query_)
{
    DEBUGAPICALL(void, "Xapian::Enquire::set_query", query_);
    try {
	internal->set_query(query_);
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

const Xapian::Query &
Xapian::Enquire::get_query()
{
    DEBUGAPICALL(const Xapian::Query &, "Xapian::Enquire::get_query", "");
    try {
	RETURN(internal->get_query());
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

void
Xapian::Enquire::set_weighting_scheme(const OmWeight &weight_)
{
    DEBUGAPICALL(void, "Xapian::Enquire::set_weighting_scheme", "[OmWeight]");
    delete internal->weight;
    internal->weight = weight_.clone();
}

void
Xapian::Enquire::set_collapse_key(om_valueno collapse_key)
{
    internal->collapse_key = collapse_key;
}

void
Xapian::Enquire::set_sort_forward(bool sort_forward)
{
    internal->sort_forward = sort_forward;
}

void
Xapian::Enquire::set_cutoff(Xapian::percent percent_cutoff, om_weight weight_cutoff)
{
    internal->percent_cutoff = percent_cutoff;
    internal->weight_cutoff = weight_cutoff;
}

void
Xapian::Enquire::set_sorting(om_valueno sort_key, int sort_bands)
{
    internal->sort_key = sort_key;
    internal->sort_bands = sort_bands;
}

void
Xapian::Enquire::set_bias(om_weight bias_weight, time_t bias_halflife)
{
    internal->bias_weight = bias_weight;
    internal->bias_halflife = bias_halflife;
}

Xapian::MSet
Xapian::Enquire::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
		    const OmMatchDecider *mdecider) const
{
    // FIXME: display contents of pointer params, if they're not null.
    DEBUGAPICALL(Xapian::MSet, "Xapian::Enquire::get_mset", first << ", " <<
		 maxitems << ", " << omrset << ", " << mdecider);

    try {
	RETURN(internal->get_mset(first, maxitems, omrset, mdecider));
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

OmESet
Xapian::Enquire::get_eset(om_termcount maxitems, const OmRSet & omrset, int flags,
		    double k, const Xapian::ExpandDecider * edecider) const
{
    // FIXME: display contents of pointer params and omrset, if they're not
    // null.
    DEBUGAPICALL(OmESet, "Xapian::Enquire::get_eset", maxitems << ", " <<
		 omrset << ", " << flags << ", " << k << ", " << edecider);

    try {
	// FIXME: this copies the eset too much: pass it in by reference?
	RETURN(internal->get_eset(maxitems, omrset, flags, k, edecider));
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

Xapian::TermIterator
Xapian::Enquire::get_matching_terms_begin(const OmMSetIterator &it) const
{
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", it);
    try {
	RETURN(internal->get_matching_terms(it));
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

Xapian::TermIterator
Xapian::Enquire::get_matching_terms_begin(om_docid did) const
{
    DEBUGAPICALL(Xapian::TermIterator, "Xapian::Enquire::get_matching_terms", did);
    try {
	RETURN(internal->get_matching_terms(did));
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

Xapian::TermIterator
Xapian::Enquire::get_matching_terms_end(const OmMSetIterator &/*it*/) const
{
    return Xapian::TermIterator(NULL);
}

Xapian::TermIterator
Xapian::Enquire::get_matching_terms_end(om_docid /*did*/) const
{
    return Xapian::TermIterator(NULL);
}

void
Xapian::Enquire::register_match_decider(const string &name,
				  const OmMatchDecider *mdecider)
{
    internal->register_match_decider(name, mdecider);
}

string
Xapian::Enquire::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::Enquire::get_description", "");
#if 1
    return "Xapian::Enquire()";
#else
    // Hmm, causing and handling exceptions is a somewhat suspect thing to be
    // doing in code which we want to be able to use for non-intrusive
    // tracing - FIXME
    try {
	RETURN("Xapian::Enquire(" + internal->get_description() + ")");
    } catch (Xapian::Error & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
#endif
}

