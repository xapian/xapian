/* omenquire.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "omdebug.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"
#include "omdatabaseinterface.h"
#include "omdocumentinternal.h"

#include "om/omerror.h"
#include "om/omenquire.h"
#include "om/omoutput.h"
#include "om/omtermlistiterator.h"
#include "om/omexpanddecider.h"

#include "omtermlistiteratorinternal.h"

#include "rset.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "database_builder.h"
#include "om/omdocument.h"
#include "om/omerrorhandler.h"
#include "om/omenquire.h"
#include "omenquireinternal.h"
#include "utils.h"

#include <vector>
#include "autoptr.h"
#include <algorithm>
#include <math.h>

OmExpandDeciderFilterTerms::OmExpandDeciderFilterTerms(OmTermIterator terms,
						       OmTermIterator termsend)
#if 1
    // the comment below suggests this may not work on Solaris CC -
    // but perhaps the issue is with the STL list iterator so let's
    // give it a go... [FIXME check and fix or remove these comments]
    : tset(terms, termsend)
{
}
#else
{
    // I'd prefer using an initialiser list for this, but it seems
    // that Solaris' CC doesn't like initialising a set with list
    // iterators.
    while (terms != termsend) {
        tset.insert(*terms);
	terms++;
    }
}
#endif

int
OmExpandDeciderFilterTerms::operator()(const om_termname &tname) const
{
    /* Solaris CC returns an iterator from tset.find() const, and then
     * doesn't like comparing it to the const_iterator from end().
     * Therefore make sure we get a const_iterator to do the comparision.
     */
    std::set<om_termname>::const_iterator i = tset.find(tname);
    return (i == tset.end());
}

OmExpandDeciderAnd::OmExpandDeciderAnd(const OmExpandDecider *left_,
                                       const OmExpandDecider *right_)
        : left(left_), right(right_) {}

int
OmExpandDeciderAnd::operator()(const om_termname &tname) const
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
    std::set<om_docid>::iterator i = internal->items.find(did);
    if (i != internal->items.end()) internal->items.erase(i);
}

bool
OmRSet::contains(om_docid did) const
{
    std::set<om_docid>::iterator i = internal->items.find(did);
    return i != internal->items.end();
}

std::string
OmRSet::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmRSet::get_description", "");
    RETURN("OmRSet(" + internal->get_description() + ")");
}

std::string
OmRSet::Internal::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmRSet::get_description", "");
    std::string description;

    for (std::set<om_docid>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (description != "") description += ", ";
	description += om_tostring(*i);
    }

    description = "OmRSet(" + description + ")";

    RETURN(description);
}

////////////////////////////
// Methods for OmMSetItem //
////////////////////////////

std::string
OmMSetItem::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmMSetItem::get_description", "");
    std::string description;

    description = om_tostring(did) + ", " + om_tostring(wt) + ", " +
	    collapse_key.get_description();

    description = "OmMSetItem(" + description + ")";

    RETURN(description);
}

////////////////////////
// Methods for OmMSet //
////////////////////////

OmMSet::OmMSet() : internal(new OmMSet::Internal())
{
}

OmMSet::OmMSet(OmMSet::Internal * internal_) : internal(internal_)
{
}

OmMSet::~OmMSet()
{
    delete internal;
}

OmMSet::OmMSet(const OmMSet & other)
	: internal(new OmMSet::Internal(*other.internal))
{
}

void
OmMSet::operator=(const OmMSet &other)
{
    *internal = *other.internal;
}

void
OmMSet::fetch(const OmMSetIterator & beginiter,
	      const OmMSetIterator & enditer) const
{
    DEBUGAPICALL(void, "OmMSet::fetch", beginiter << ", " << enditer);
    Assert(internal != 0);
    Assert(internal->data.get() != 0);

    // we have to convert the MSetIterators into vector iterators
    // FIXME: this is messy
    if (beginiter.internal == 0) return;

    std::vector<OmMSetItem>::const_iterator end_vecit;
    if (enditer.internal == 0) end_vecit = internal->data->items.end();
    else end_vecit = enditer.internal->it;

    internal->data->fetch_items(beginiter.get_rank(),
				beginiter.internal->it, end_vecit);
}

void
OmMSet::fetch(const OmMSetIterator & beginiter) const
{
    DEBUGAPICALL(void, "OmMSet::fetch", beginiter);
    Assert(internal != 0);
    Assert(internal->data.get() != 0);

    // we have to convert the MSetIterators into vector iterators
    // FIXME: this is messy
    if (beginiter.internal == 0) return;

    OmMSetIterator enditer = beginiter;
    enditer++;

    std::vector<OmMSetItem>::const_iterator end_vecit;
    if (enditer.internal == 0) end_vecit = internal->data->items.end();
    else end_vecit = enditer.internal->it;

    internal->data->fetch_items(beginiter.get_rank(),
				beginiter.internal->it, end_vecit);
}

void
OmMSet::fetch() const
{
    DEBUGAPICALL(void, "OmMSet::fetch", "");
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    internal->data->fetch_items(internal->data->firstitem,
				internal->data->items.begin(),
				internal->data->items.end());
}

om_percent
OmMSet::convert_to_percent(om_weight wt) const
{
    DEBUGAPICALL(om_percent, "OmMSet::convert_to_percent", wt);
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    RETURN(internal->data->convert_to_percent_internal(wt));
}

om_percent
OmMSet::convert_to_percent(const OmMSetIterator & it) const
{
    DEBUGAPICALL(om_percent, "OmMSet::convert_to_percent", it);
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    RETURN(internal->data->convert_to_percent_internal(it.get_weight()));
}

om_doccount
OmMSet::get_termfreq(const om_termname &tname) const
{
    DEBUGAPICALL(om_doccount, "OmMSet::get_termfreq", tname);
    std::map<om_termname, Internal::Data::TermFreqAndWeight>::const_iterator i;
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    i = internal->data->termfreqandwts.find(tname);
    if (i == internal->data->termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term frequency of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termfreq);
}

om_weight
OmMSet::get_termweight(const om_termname &tname) const
{
    DEBUGAPICALL(om_weight, "OmMSet::get_termweight", tname);
    std::map<om_termname, Internal::Data::TermFreqAndWeight>::const_iterator i;
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    i = internal->data->termfreqandwts.find(tname);
    if (i == internal->data->termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term weight of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termweight);
}

om_doccount
OmMSet::get_firstitem() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->firstitem;
}

om_doccount
OmMSet::get_matches_lower_bound() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->matches_lower_bound;
}

om_doccount
OmMSet::get_matches_estimated() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->matches_estimated;
}

om_doccount
OmMSet::get_matches_upper_bound() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->matches_upper_bound;
}

om_weight
OmMSet::get_max_possible() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->max_possible;
}

om_weight
OmMSet::get_max_attained() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->max_attained;
}

om_doccount
OmMSet::size() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->items.size();
}

om_doccount
OmMSet::max_size() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return om_doccount(-1); // FIXME: is there a better way to do this?
}

bool
OmMSet::empty() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return internal->data->items.empty();
}

void
OmMSet::swap(OmMSet & other)
{
    std::swap(internal, other.internal);
}

OmMSetIterator
OmMSet::begin() const
{
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    if (internal->data->items.begin() == internal->data->items.end()) {
	return OmMSetIterator(NULL);
    } else {
	return OmMSetIterator(new OmMSetIterator::Internal(
	    internal->data->items.begin(),
	    internal->data->items.end(),
	    internal->data->firstitem,
	    internal->data));
    }
}

OmMSetIterator
OmMSet::end() const
{
    return OmMSetIterator(NULL);
}

OmMSetIterator
OmMSet::operator[](om_doccount i) const
{
    Assert(0 <= i && i < size());
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return OmMSetIterator(new OmMSetIterator::Internal(
	internal->data->items.begin() + i,
	internal->data->items.end(),
	internal->data->firstitem + i,
	internal->data));
}

OmMSetIterator
OmMSet::back() const
{
    Assert(!empty());
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    return OmMSetIterator(new OmMSetIterator::Internal(
	internal->data->items.begin() + internal->data->items.size() - 1,
	internal->data->items.end(),
	internal->data->firstitem + internal->data->items.size() - 1,
	internal->data));
}

std::string
OmMSet::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmMSet::get_description", "");
    Assert(internal != 0);
    Assert(internal->data.get() != 0);
    RETURN("OmMSet(" + internal->data->get_description() + ")");
}

void
OmMSet::Internal::Data::calc_percent_factor() const {
    if (!have_percent_factor) {
	if (max_possible == 0) percent_factor = 0;
	else percent_factor = 100.0 / max_possible;
	DEBUGLINE(API, "OmMSet::Internal::calc_percent_factor(): max_possible = " << max_possible << " => percent_factor = " << percent_factor);
	have_percent_factor = true;
    }
}

om_percent
OmMSet::Internal::Data::convert_to_percent_internal(om_weight wt) const
{
    DEBUGAPICALL(om_percent, "OmMSet::Internal::convert_to_percent", wt);
    calc_percent_factor();

    if (percent_factor == 0) RETURN(100);

    om_percent pcent = static_cast<om_percent>(wt * percent_factor);
    DEBUGLINE(API, "wt = " << wt << ", max_possible = "
	      << max_possible << " =>  pcent = " << pcent);
    if (pcent > 100) pcent = 100;
    if (pcent < 0) pcent = 0;
    if (pcent == 0 && wt > 0) pcent = 1;

    RETURN(pcent);
}

OmDocument
OmMSet::Internal::Data::get_doc_by_rank(om_doccount rank) const
{
    DEBUGCALL(API, OmDocument, "OmMSet::Internal::Data::get_doc_by_rank", rank);
    std::map<om_doccount, OmDocument>::const_iterator doc;
    doc = rankeddocs.find(rank);
    if (doc == rankeddocs.end()) {
	if (rank < firstitem || rank >= firstitem + items.size()) {
	    throw OmRangeError("The mset returned from the match does not contain the document at rank " + om_tostring(rank));
	}
	fetch_items(rank,
		    items.begin() + (rank - firstitem),
		    items.begin() + (rank - firstitem) + 1);
	Assert(rankeddocs.find(rank) != rankeddocs.end());
	Assert(rankeddocs.find(rank)->first == rank); // Paranoid assert
	RETURN(rankeddocs.find(rank)->second);
    } else {
	RETURN(doc->second);
    }
}

void
OmMSet::Internal::Data::fetch_items(
	om_doccount rank,
	std::vector<OmMSetItem>::const_iterator begin,
	std::vector<OmMSetItem>::const_iterator end) const
{
#ifdef MUS_DEBUG_VERBOSE
    unsigned int count = 0;
    for (std::vector<OmMSetItem>::const_iterator i = begin; i != end; i++)
	count++;
    DEBUGAPICALL(void, "OmMSet::Internal::Data::fetch_items",
		 rank << ", " <<
		 "[" << count << " items]");
#endif
    if (enquire.get() == 0) {
	throw OmInvalidOperationError("Can't fetch documents from an Mset which is not derived from a query.");
    } else {
	// FIXME: don't refetch documents which are already in the cache - this
	// is already implemented for getting a single document (in
	// get_doc_by_rank()), but should perhaps be at this level.
	const std::vector<OmDocument> docs = enquire->read_docs(begin, end);
	DEBUGLINE(API, "fetch_items: read " << docs.size() << " docs");
	om_doccount currrank = rank;
	for(std::vector<OmDocument>::const_iterator i = docs.begin();
	    i != docs.end(); i++) {
	    DEBUGLINE(API, "new doc at rank " << currrank << " is " << *i);
	    DEBUGLINE(API, "old doc at rank " << currrank << " is " << rankeddocs[currrank]);
	    rankeddocs[currrank] = *i;
	    DEBUGLINE(API, "stored doc at rank " << currrank << " is " << rankeddocs[currrank]);
	    currrank++;
	}
    }
}

std::string
OmMSet::Internal::Data::get_description() const
{
    std::string description = "OmMSet::Internal::Data(";

    description = "firstitem=" + om_tostring(firstitem) + ", " +
	    "matches_lower_bound=" + om_tostring(matches_lower_bound) + ", " +
	    "matches_estimated=" + om_tostring(matches_estimated) + ", " +
	    "matches_upper_bound=" + om_tostring(matches_upper_bound) + ", " +
	    "max_possible=" + om_tostring(max_possible) + ", " +
	    "max_attained=" + om_tostring(max_attained);

    for (std::vector<OmMSetItem>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (description != "") description += ", ";
	description += i->get_description();
    }

    description = description + ")";

    return description;
}

////////////////////////////
// Methods for OmESetItem //
////////////////////////////

std::string
OmESetItem::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmESetItem::get_description", "");
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

std::string
OmESet::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmESet::get_description", "");
    RETURN("OmESet(" + internal->get_description() + ")");
}

//////////////////////////////////
// Methods for OmESet::Internal //
//////////////////////////////////

std::string
OmESet::Internal::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmESet::Internal::get_description", "");
    std::string description = "ebound=" + om_tostring(ebound);

    for (std::vector<OmESetItem>::const_iterator i = items.begin();
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
	std::swap(*internal, temp);
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

const om_termname &
OmESetIterator::operator *() const
{
    return internal->it->tname;
}

om_weight
OmESetIterator::get_weight() const
{
    return internal->it->wt;
}

std::string
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
	std::swap(*internal, temp);
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

om_percent
OmMSetIterator::get_percent() const
{
    DEBUGAPICALL(om_percent, "OmMSetIterator::get_percent", "");
    RETURN(internal->msetdata->convert_to_percent_internal(internal->it->wt));
}

std::string
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

///////////////////////////////////////////
// Methods for OmEnquire::Internal::Data //
///////////////////////////////////////////

OmEnquire::Internal::Data::Data(const OmDatabase &db_,
			      OmErrorHandler * errorhandler_)
  : db(db_), query(0), errorhandler(errorhandler_)
{
}

OmEnquire::Internal::Data::~Data()
{
    if(query != 0) {
	delete query;
	query = 0;
    }
}

void
OmEnquire::Internal::Data::set_query(const OmQuery &query_)
{
    OmLockSentry locksentry(mutex);
    if(query) {
	delete query;
	query = 0;
    }
    if (!query_.is_defined()) {
        throw OmInvalidArgumentError("Query must not be undefined");
    }
    query = new OmQuery(query_);
}

const OmQuery &
OmEnquire::Internal::Data::get_query()
{
    if (query == 0) {
        throw OmInvalidArgumentError("Can't get query before setting it");
    }
    return *query;
}

OmMSet
OmEnquire::Internal::Data::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmSettings *moptions,
		    const OmMatchDecider *mdecider) const
{
    OmLockSentry locksentry(mutex);
    if(query == 0) {
        throw OmInvalidArgumentError("You must set a query before calling OmEnquire::get_mset()");
    }
    Assert(query->is_defined());

    // Use default options if none supplied
    OmSettings default_settings;
    if (moptions == 0) {
        moptions = &default_settings;
    }

    // Set Rset
    OmRSet emptyrset;
    if(omrset == 0) {
	omrset = &emptyrset;
    }

    // FIXME: make match take a refcntptr
    //
    // Notes: when accessing query, we don't need to lock mutex, since it's
    // our own copy and we're locked ourselves
    MultiMatch match(db, query->internal, *omrset, *moptions);

    // Run query and get results into supplied OmMSet object
    OmMSet retval;
    match.get_mset(first, maxitems, retval, mdecider, errorhandler);

    Assert(!(query->is_bool()) || retval.get_max_possible() == 0);

    // The OmMSet needs to have a pointer to ourselves, so that it can
    // retrieve the documents.  This is set here explicitly to avoid having
    // to pass it into the matcher, which gets messy particularly in the
    // networked case.
    RefCntBase::RefCntPtrToThis tmp;
    retval.internal->data->enquire =
	    RefCntPtr<const OmEnquire::Internal::Data>(tmp, this);

    return retval;
}

OmESet
OmEnquire::Internal::Data::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmSettings * eoptions,
		    const OmExpandDecider * edecider) const
{
    OmLockSentry locksentry(mutex);
    OmESet retval;

    OmSettings defeoptions;
    if (eoptions == 0) {
        eoptions = &defeoptions;
    }

    // FIXME: make expand and rset take a refcntptr
    OmExpand expand(db);
    RSet rset(db, omrset);

    DEBUGLINE(API, "rset size is " << omrset.size());

    OmExpandDeciderAlways decider_always;
    if (edecider == 0) edecider = &decider_always;

    /* The AutoPtrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    AutoPtr<OmExpandDecider> decider_noquery;
    AutoPtr<OmExpandDecider> decider_andnoquery;

    if (query != 0 && !eoptions->get_bool("expand_use_query_terms", false)) {
	AutoPtr<OmExpandDecider> temp1(
	    new OmExpandDeciderFilterTerms(query->get_terms_begin(),
					   query->get_terms_end()));
        decider_noquery = temp1;

	AutoPtr<OmExpandDecider> temp2(
	    new OmExpandDeciderAnd(decider_noquery.get(),
				   edecider));
	decider_andnoquery = temp2;

        edecider = decider_andnoquery.get();
    }

    expand.expand(maxitems, retval, &rset, edecider,
		  eoptions->get_bool("expand_use_exact_termfreq", false));

    return retval;
}

OmTermIterator
OmEnquire::Internal::Data::get_matching_terms(om_docid did) const
{
    OmLockSentry locksentry(mutex);
    return calc_matching_terms(did);
}

OmTermIterator
OmEnquire::Internal::Data::get_matching_terms(const OmMSetIterator &it) const
{
    OmLockSentry locksentry(mutex);
    // FIXME: take advantage of OmMSetIterator to ensure that database
    // doesn't get modified underneath us.
    return calc_matching_terms(*it);
}

std::string
OmEnquire::Internal::Data::get_description() const
{
    std::string description = db.get_description();
    if (query) description += ", " + query->get_description();
    return description;
}

///////////////////////////////////////////////////
// Private methods for OmEnquire::Internal::Data //
///////////////////////////////////////////////////

const std::vector<OmDocument>
OmEnquire::Internal::Data::read_docs(
	std::vector<OmMSetItem>::const_iterator begin,
	std::vector<OmMSetItem>::const_iterator end) const
{
    OmLockSentry locksentry(mutex);
    try {
	OmDatabase::Internal * dbinternal =
		OmDatabase::InternalInterface::get(db);
	unsigned int multiplier = dbinternal->databases.size();

	for (std::vector<OmMSetItem>::const_iterator i = begin; i != end; i++) {
	    om_docid realdid = (i->did - 1) / multiplier + 1;
	    om_doccount dbnumber = (i->did - 1) % multiplier;

	    dbinternal->databases[dbnumber]->request_document(realdid);
	}

	std::vector<OmDocument> docs;
	for (std::vector<OmMSetItem>::const_iterator i = begin; i != end; i++) {
	    om_docid realdid = (i->did - 1) / multiplier + 1;
	    om_doccount dbnumber = (i->did - 1) % multiplier;

	    Document *doc = dbinternal->databases[dbnumber]->collect_document(realdid);
	    docs.push_back(OmDocument(new OmDocument::Internal(doc, db, i->did)));
	}

	return docs;
    } catch (OmError & e) {
	if (errorhandler) (*errorhandler)(e);
	throw;
    }
}


class ByQueryIndexCmp {
 private:
    typedef std::map<om_termname, unsigned int> tmap_t;
    const tmap_t &tmap;
 public:
    ByQueryIndexCmp(const tmap_t &tmap_) : tmap(tmap_) {};
    bool operator()(const om_termname &left,
		    const om_termname &right) const {
	tmap_t::const_iterator l, r;
	l = tmap.find(left);
	r = tmap.find(right);
	Assert((l != tmap.end()) && (r != tmap.end()));

	return l->second < r->second;
    }
};

OmTermIterator
OmEnquire::Internal::Data::calc_matching_terms(om_docid did) const
{
    if (query == 0) {
        throw OmInvalidArgumentError("Can't get matching terms before setting query");
    }
    Assert(query->is_defined());

    // the ordered list of terms in the query.
    OmTermIterator qt = query->get_terms_begin();
    OmTermIterator qt_end = query->get_terms_end();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    std::map<om_termname, unsigned int> tmap;
    unsigned int index = 1;
    for ( ; qt != qt_end; qt++) {
	if (tmap.find(*qt) == tmap.end())
	    tmap[*qt] = index++;
    }

    std::vector<om_termname> matching_terms;

    OmTermIterator docterms = db.termlist_begin(did);
    OmTermIterator docterms_end = db.termlist_end(did);
    while (docterms != docterms_end) {
	om_termname term = *docterms;
        std::map<om_termname, unsigned int>::iterator t = tmap.find(term);
        if (t != tmap.end()) matching_terms.push_back(term);
	docterms++;
    }

    // sort the resulting list by query position.
    sort(matching_terms.begin(), matching_terms.end(), ByQueryIndexCmp(tmap));

    return OmTermIterator(new OmTermIterator::Internal(
			      new VectorTermList(matching_terms.begin(),
						 matching_terms.end())));
}

//////////////////////////
// Methods of OmEnquire //
//////////////////////////

OmEnquire::OmEnquire(const OmDatabase &databases,
		     OmErrorHandler * errorhandler)
{
    DEBUGAPICALL(void, "OmEnquire::OmEnquire", databases);
    internal = new OmEnquire::Internal(
	new OmEnquire::Internal::Data(databases, errorhandler));
}

OmEnquire::~OmEnquire()
{
    DEBUGAPICALL(void, "OmEnquire::~OmEnquire", "");
    delete internal;
    internal = NULL;
}

void
OmEnquire::set_query(const OmQuery & query_)
{
    DEBUGAPICALL(void, "OmEnquire::set_query", query_);
    try {
	internal->data->set_query(query_);
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

const OmQuery &
OmEnquire::get_query()
{
    DEBUGAPICALL(const OmQuery &, "OmEnquire::get_query", "");
    try {
	RETURN(internal->data->get_query());
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

OmMSet
OmEnquire::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmSettings *moptions,
		    const OmMatchDecider *mdecider) const
{
    // FIXME: display contents of pointer params, if they're not null.
    DEBUGAPICALL(OmMSet, "OmEnquire::get_mset",
		 first << ", " <<
		 maxitems << ", " <<
		 omrset << ", " <<
		 moptions << ", " << mdecider);

    try {
	RETURN(internal->data->get_mset(first, maxitems, omrset, moptions, mdecider));
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

OmESet
OmEnquire::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmSettings * eoptions,
		    const OmExpandDecider * edecider) const
{
    // FIXME: display contents of pointer params and omrset, if they're not
    // null.
    DEBUGAPICALL(OmESet, "OmEnquire::get_eset",
		 maxitems << ", " <<
		 omrset << ", " <<
		 eoptions << ", " <<
		 edecider);

    try {
	// FIXME: this copies the eset too much: pass it in by reference?
	RETURN(internal->data->get_eset(maxitems, omrset, eoptions, edecider));
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

OmTermIterator
OmEnquire::get_matching_terms_begin(const OmMSetIterator &it) const
{
    DEBUGAPICALL(OmTermIterator, "OmEnquire::get_matching_terms", it);
    try {
	RETURN(internal->data->get_matching_terms(it));
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

OmTermIterator
OmEnquire::get_matching_terms_begin(om_docid did) const
{
    DEBUGAPICALL(OmTermIterator, "OmEnquire::get_matching_terms", did);
    try {
	RETURN(internal->data->get_matching_terms(did));
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

OmTermIterator
OmEnquire::get_matching_terms_end(const OmMSetIterator &it) const
{
    return OmTermIterator(NULL);
}

OmTermIterator
OmEnquire::get_matching_terms_end(om_docid did) const
{
    return OmTermIterator(NULL);
}

std::string
OmEnquire::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmEnquire::get_description", "");
    try {
	RETURN("OmEnquire(" + internal->data->get_description() + ")");
    } catch (OmError & e) {
	if (internal->data->errorhandler) (*internal->data->errorhandler)(e);
	throw;
    }
}

