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

#include <om/omerror.h>
#include <om/omenquire.h>
#include <om/omoutput.h>
#include <om/omtermlistiterator.h>

#include "omtermlistiteratorinternal.h"

#include "rset.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "database_builder.h"
#include "om/omdocument.h"
#include "om/omerrorhandler.h"
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

om_percent
OmMSet::convert_to_percent(om_weight wt) const
{
    DEBUGAPICALL(om_percent, "OmMSet::convert_to_percent", wt);
    RETURN(internal->convert_to_percent_internal(wt));
}

om_percent
OmMSet::convert_to_percent(const OmMSetIterator & it) const
{
    DEBUGAPICALL(om_percent, "OmMSet::convert_to_percent", it);
    RETURN(internal->convert_to_percent_internal(it.get_weight()));
}

om_doccount
OmMSet::get_termfreq(const om_termname &tname) const
{
    DEBUGAPICALL(om_doccount, "OmMSet::get_termfreq", tname);
    std::map<om_termname, Internal::TermFreqAndWeight>::const_iterator i;
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term frequency of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termfreq);
}

om_weight
OmMSet::get_termweight(const om_termname &tname) const
{
    DEBUGAPICALL(om_weight, "OmMSet::get_termweight", tname);
    std::map<om_termname, Internal::TermFreqAndWeight>::const_iterator i;
    i = internal->termfreqandwts.find(tname);
    if (i == internal->termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term weight of `" + tname +
				     "' not available.");
    }
    RETURN(i->second.termweight);
}

om_doccount
OmMSet::get_firstitem() const
{
    return internal->firstitem;
}

om_doccount
OmMSet::get_matches_lower_bound() const
{
    return internal->matches_lower_bound;
}

om_doccount
OmMSet::get_matches_estimated() const
{
    return internal->matches_estimated;
}

om_doccount
OmMSet::get_matches_upper_bound() const
{
    return internal->matches_upper_bound;
}

om_weight
OmMSet::get_max_possible() const
{
    return internal->max_possible;
}

om_weight
OmMSet::get_max_attained() const
{
    return internal->max_attained;
}

om_doccount
OmMSet::size() const
{
    return internal->items.size();
}

om_doccount
OmMSet::max_size() const
{
    return om_doccount(-1); // FIXME: is there a better way to do this?
}

bool
OmMSet::empty() const
{
    return internal->items.empty();
}

void
OmMSet::swap(OmMSet & other)
{
    std::swap(internal, other.internal);
}

OmMSetIterator
OmMSet::begin() const
{
    if (internal->items.begin() == internal->items.end()) {
	return OmMSetIterator(NULL);
    } else {
	internal->calc_percent_factor();
	return OmMSetIterator(new OmMSetIterator::Internal(internal->items.begin(),
							   internal->items.end(),
							   internal->percent_factor));
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
    internal->calc_percent_factor();
    return OmMSetIterator(new OmMSetIterator::Internal(internal->items.begin() + i,
						       internal->items.end(),
						       internal->percent_factor));
}

OmMSetIterator
OmMSet::back() const
{
    Assert(!empty());
    internal->calc_percent_factor();
    return OmMSetIterator(new OmMSetIterator::Internal(internal->items.begin() + internal->items.size() - 1,
						       internal->items.end(),
						       internal->percent_factor));
}

std::string
OmMSet::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmMSet::get_description", "");
    RETURN("OmMSet(" + internal->get_description() + ")");
}

void
OmMSet::Internal::calc_percent_factor() const {
    if (!have_percent_factor) {
	if (max_possible == 0) percent_factor = 0;
	else percent_factor = 100 / max_possible;
	have_percent_factor = true;
    }
}

om_percent
OmMSet::Internal::convert_to_percent_internal(om_weight wt) const
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

std::string
OmMSet::Internal::get_description() const
{
    std::string description = "OmMSet::Internal(";

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
    return (a.internal->it == b.internal->it);
}

// OmMSetIterator

OmMSetIterator::OmMSetIterator(Internal *internal_) : internal(internal_)
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
						other.internal->percent_factor);
    } else {
	OmMSetIterator::Internal temp(other.internal->it,
				      other.internal->end,
				      other.internal->percent_factor);
	std::swap(*internal, temp);
    }
}

OmMSetIterator &
OmMSetIterator::operator++()
{
    ++internal->it;
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

om_weight
OmMSetIterator::get_weight() const
{
    return internal->it->wt;
}

om_percent
OmMSetIterator::get_percent() const
{
    DEBUGAPICALL(om_percent, "OmMSetIterator::get_percent", "");
    if (internal->percent_factor == 0) RETURN(100);

    om_percent pcent =
	static_cast<om_percent>(internal->it->wt * internal->percent_factor);
    DEBUGLINE(API, "wt = " << internal->it->wt << " =>  pcent = " << pcent);
    if (pcent > 100) pcent = 100;
    if (pcent < 0) pcent = 0;
    if (pcent == 0 && internal->it->wt > 0) pcent = 1;

    RETURN(pcent);
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
    return (a.internal->it == b.internal->it);
}

/////////////////////////////////////
// Methods for OmEnquire::Internal //
/////////////////////////////////////

OmEnquire::Internal::Internal(const OmDatabase &db_,
			      OmErrorHandler * errorhandler_)
  : db(db_), query(0), errorhandler(errorhandler_)
{
}

OmEnquire::Internal::~Internal()
{
    if(query != 0) {
	delete query;
	query = 0;
    }
}

void
OmEnquire::Internal::set_query(const OmQuery &query_)
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
OmEnquire::Internal::get_query()
{
    if (query == 0) {
        throw OmInvalidArgumentError("Can't get query before setting it");
    }
    return *query;
}

OmMSet
OmEnquire::Internal::get_mset(om_doccount first,
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

    OmMSet retval;
    // Run query and get results into supplied OmMSet object
    match.get_mset(first, maxitems, retval, mdecider, errorhandler);

    Assert(!(query->is_bool()) || retval.get_max_possible() == 0);

    return retval;
}

OmESet
OmEnquire::Internal::get_eset(om_termcount maxitems,
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

const OmDocument
OmEnquire::Internal::get_doc(om_docid did) const
{
    OmLockSentry locksentry(mutex);
    return read_doc(did);
}

const OmDocument
OmEnquire::Internal::get_doc(const OmMSetIterator &it) const
{
    OmLockSentry locksentry(mutex);
    // FIXME: take advantage of OmMSetIterator to ensure that database
    // doesn't get modified underneath us.
    return read_doc(*it);
}

const std::vector<OmDocument>
OmEnquire::Internal::get_docs(const OmMSetIterator &begin,
			      const OmMSetIterator &end) const
{
    OmLockSentry locksentry(mutex);

    OmDatabase::Internal * internal = OmDatabase::InternalInterface::get(db);
    unsigned int multiplier = internal->databases.size();

    for (OmMSetIterator i = begin; i != end; i++) {
	om_docid realdid = (*i - 1) / multiplier + 1;
	om_doccount dbnumber = (*i - 1) % multiplier;
	
	internal->databases[dbnumber]->request_document(realdid);
    }

    std::vector<OmDocument> docs;
    for (OmMSetIterator i = begin; i != end; i++) {
	om_docid realdid = (*i - 1) / multiplier + 1;
	om_doccount dbnumber = (*i - 1) % multiplier;
	
	Document *doc = internal->databases[dbnumber]->collect_document(realdid);
	docs.push_back(OmDocument(new OmDocument::Internal(doc, db, *i)));
    }

    return docs;
}

OmTermIterator
OmEnquire::Internal::get_matching_terms(om_docid did) const
{
    OmLockSentry locksentry(mutex);
    return calc_matching_terms(did);
}

OmTermIterator
OmEnquire::Internal::get_matching_terms(const OmMSetIterator &it) const
{
    OmLockSentry locksentry(mutex);
    // FIXME: take advantage of OmMSetIterator to ensure that database
    // doesn't get modified underneath us.
    return calc_matching_terms(*it);
}

std::string
OmEnquire::Internal::get_description() const
{
    std::string description = db.get_description();
    if (query) description += ", " + query->get_description();
    return description;
}

/////////////////////////////////////////////
// Private methods for OmEnquire::Internal //
/////////////////////////////////////////////

const OmDocument
OmEnquire::Internal::read_doc(om_docid did) const
{
    OmDatabase::Internal * internal = OmDatabase::InternalInterface::get(db);
    unsigned int multiplier = internal->databases.size();
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    Document *doc = internal->databases[dbnumber]->open_document(realdid);

    return OmDocument(new OmDocument::Internal(doc, db, did));
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
OmEnquire::Internal::calc_matching_terms(om_docid did) const
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
    internal = new OmEnquire::Internal(databases, errorhandler);
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
	internal->set_query(query_);
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

const OmQuery &
OmEnquire::get_query()
{
    DEBUGAPICALL(const OmQuery &, "OmEnquire::get_query", "");
    try {
	RETURN(internal->get_query());
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
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
	// FIXME: this copies the mset too much: pass it in by reference?
	RETURN(internal->get_mset(first, maxitems, omrset, moptions, mdecider));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
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
	RETURN(internal->get_eset(maxitems, omrset, eoptions, edecider));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

const OmDocument
OmEnquire::get_doc(om_docid did) const
{
    DEBUGAPICALL(const OmDocument, "OmEnquire::get_doc", did);
    try {
	RETURN(internal->get_doc(did));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

const OmDocument
OmEnquire::get_doc(const OmMSetIterator &it) const
{
    DEBUGAPICALL(const OmDocument, "OmEnquire::get_doc", it);
    try {
	RETURN(internal->get_doc(it));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

const std::vector<OmDocument>
OmEnquire::get_docs(const OmMSetIterator &begin,
		    const OmMSetIterator &end) const
{
    DEBUGAPICALL(const std::vector<OmDocument>,
		 "OmEnquire::get_docs", begin << ", " << end);
    try {
	// FIXME - debug info for return
	return(internal->get_docs(begin, end));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

OmTermIterator
OmEnquire::get_matching_terms_begin(const OmMSetIterator &it) const
{
    DEBUGAPICALL(OmTermIterator, "OmEnquire::get_matching_terms", it);
    try {
	RETURN(internal->get_matching_terms(it));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

OmTermIterator
OmEnquire::get_matching_terms_begin(om_docid did) const
{
    DEBUGAPICALL(OmTermIterator, "OmEnquire::get_matching_terms", did);
    try {
	RETURN(internal->get_matching_terms(did));
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
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
	RETURN("OmEnquire(" + internal->get_description() + ")");
    } catch (OmError & e) {
	if (internal->errorhandler) (*internal->errorhandler)(e);
	throw;
    }
}

