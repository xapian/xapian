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

#include <om/omerror.h>
#include <om/omenquire.h>
#include <om/omoutput.h>

#include "rset.h"
#include "localmatch.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "omdatabaseinterface.h"
#include "database_builder.h"
#include <om/omdocument.h>
#include "omdocumentparams.h"
#include "omenquireinternal.h"
#include "utils.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <math.h>

////////////////////////////////
// Methods for OmMatchOptions //
////////////////////////////////

OmMatchOptions::OmMatchOptions()
	: do_collapse(false),
	  collapse_key(0),
	  sort_forward(true),
	  percent_cutoff(-1),
	  max_or_terms(0)
{}

std::string
OmMatchOptions::get_description() const
{
    DEBUGAPICALL("OmMatchOptions::get_description", "");
    std::string description;
    if (do_collapse) {
	if (description != "") description += ", ";
	description += "collapse on key " + om_inttostring(collapse_key);
    }

    if (!sort_forward) {
	if (description != "") description += ", ";
	description += "reverse document order";
    }

    if (percent_cutoff != -1) {
	if (description != "") description += ", ";
	description += "cutoff at " + om_inttostring(percent_cutoff) + "%";
    }

    if (max_or_terms != 0) {
	if (description != "") description += ", ";
	description += "max_or_terms = " + om_inttostring(max_or_terms);
    }

    description = "OmMatchOptions(" + description + ")";

    DEBUGAPIRETURN(description);
    return description;
}

void
OmMatchOptions::set_collapse_key(om_keyno key_)
{
    DEBUGAPICALL("OmMatchOptions::set_collapse_key", key_);
    do_collapse = true;
    collapse_key = key_;
}

void
OmMatchOptions::set_no_collapse()
{
    DEBUGAPICALL("OmMatchOptions::set_no_collapse", "");
    do_collapse = false;
}

void
OmMatchOptions::set_sort_forward(bool forward_)
{
    DEBUGAPICALL("OmMatchOptions::set_sort_forward", forward_);
    sort_forward = forward_;
}

void
OmMatchOptions::set_percentage_cutoff(int percent_)
{
    DEBUGAPICALL("OmMatchOptions::set_percentage_cutoff", percent_);
    if (percent_ >=0 && percent_ <= 100) {
        percent_cutoff = percent_;
    } else {
        throw OmInvalidArgumentError("Percent cutoff must be in 0..100");
    }
}

void
OmMatchOptions::set_max_or_terms(om_termcount max_)
{
    DEBUGAPICALL("OmMatchOptions::set_max_or_terms", max_);
    max_or_terms = max_;
}

OmMSetCmp
OmMatchOptions::get_sort_comparator() const
{
    DEBUGAPICALL("OmMatchOptions::get_sort_comparator", "");
    if (sort_forward) {
	return OmMSetCmp(msetcmp_forward);
    } else {
	return OmMSetCmp(msetcmp_reverse);
    }
}


/////////////////////////////////
// Methods for OmExpandOptions //
/////////////////////////////////

OmExpandOptions::OmExpandOptions()
	: use_query_terms(false),
	  use_exact_termfreq(false)
{
    DEBUGAPICALL("OmExpandOptions::OmExpandOptions", "");
}

std::string
OmExpandOptions::get_description() const
{
    DEBUGAPICALL("OmExpandOptions::get_description", "");
    std::string description;

    if (use_query_terms) {
	if (description != "") description += ", ";
	description += "use query terms";
    }

    if (use_exact_termfreq) {
	if (description != "") description += ", ";
	description += "use exact term frequency";
    }

    description = "OmExpandOptions(" + description + ")";

    DEBUGAPIRETURN(description);
    return description;
}

void
OmExpandOptions::set_use_query_terms(bool use_query_terms_)
{
    DEBUGAPICALL("OmExpandOptions::set_use_query_terms", use_query_terms_);
    use_query_terms = use_query_terms_;
}

void
OmExpandOptions::set_use_exact_termfreq(bool use_exact_termfreq_)
{   
    DEBUGAPICALL("OmExpandOptions::set_use_exact_termfreq", use_exact_termfreq_);
    use_exact_termfreq = use_exact_termfreq_;
}


OmExpandDeciderFilterTerms::OmExpandDeciderFilterTerms(
                               const om_termname_list &terms)
{
    // I'd prefer using an initialiser list for this, but it seems
    // that Solaris' CC doesn't like initialising a set with list
    // iterators.
    om_termname_list::const_iterator i = terms.begin();
    while (i != terms.end()) {
        tset.insert(*i);
	i++;
    }
}

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

std::string
OmRSet::get_description() const
{
    DEBUGAPICALL("OmRSet::get_description", "");
    std::string description;

    for (std::set<om_docid>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (description != "") description += ", ";
	description += om_inttostring(*i);
    }

    description = "OmRSet(" + description + ")";

    DEBUGAPIRETURN(description);
    return description;
}

////////////////////////////
// Methods for OmMSetItem //
////////////////////////////

std::string
OmMSetItem::get_description() const
{
    DEBUGAPICALL("OmMSetItem::get_description", "");
    std::string description;

    description = om_inttostring(did) + ", " + doubletostring(wt) + ", " +
	    collapse_key.get_description();

    description = "OmMSetItem(" + description + ")";

    DEBUGAPIRETURN(description);
    return description;
}

////////////////////////
// Methods for OmMSet //
////////////////////////

int
OmMSet::convert_to_percent_internal(om_weight wt) const
{
    DEBUGAPICALL("OmMSet::convert_to_percent", wt);
    if(max_possible == 0) return 100;

    int pcent = (int) ceil(wt * 100 / max_possible);
    DebugMsg("wt = " << wt << ", max_possible = " << max_possible <<
	     " =>  pcent = " << pcent << endl);
    if(pcent > 100) pcent = 100;
    if(pcent < 0) pcent = 0;
    if(pcent == 0 && wt > 0) pcent = 1;

    DEBUGAPIRETURN(pcent);
    return pcent;
}

int
OmMSet::convert_to_percent(om_weight wt) const
{
    DEBUGAPICALL("OmMSet::convert_to_percent", wt);
    int pcent = convert_to_percent_internal(wt);
    DEBUGAPIRETURN(pcent);
    return pcent;
}

int
OmMSet::convert_to_percent(const OmMSetItem & item) const
{
    DEBUGAPICALL("OmMSet::convert_to_percent", item);
    int pcent = convert_to_percent_internal(item.wt);
    DEBUGAPIRETURN(pcent);
    return pcent;
}

om_doccount
OmMSet::get_termfreq(om_termname tname) const
{
    DEBUGAPICALL("OmMSet::get_termfreq", tname);
    std::map<om_termname, TermFreqAndWeight>::const_iterator i;
    i = termfreqandwts.find(tname);
    if(i == termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term frequency of `" + tname +
				     "' not available.");
    }
    DEBUGAPIRETURN(i->second.termfreq);
    return i->second.termfreq;
}

om_weight
OmMSet::get_termweight(om_termname tname) const
{
    DEBUGAPICALL("OmMSet::get_termweight", tname);
    std::map<om_termname, TermFreqAndWeight>::const_iterator i;
    i = termfreqandwts.find(tname);
    if(i == termfreqandwts.end()) {
	throw OmInvalidArgumentError("Term weight of `" + tname +
				     "' not available.");
    }
    DEBUGAPIRETURN(i->second.termweight);
    return i->second.termweight;
}

std::string
OmMSet::get_description() const
{
    DEBUGAPICALL("OmMSet::get_description", "");
    std::string description;

    description = "firstitem=" + om_inttostring(firstitem) + ", " +
	    "mbound=" + om_inttostring(mbound) + ", " +
	    "max_possible=" + doubletostring(max_possible) + ", " +
	    "max_attained=" + doubletostring(max_attained);

    for (std::vector<OmMSetItem>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (description != "") description += ", ";
	description += i->get_description();
    }

    description = "OmMSet(" + description + ")";

    DEBUGAPIRETURN(description);
    return description;
}

////////////////////////////
// Methods for OmESetItem //
////////////////////////////

std::string
OmESetItem::get_description() const
{
    DEBUGAPICALL("OmESetItem::get_description", "");
    std::string description;

    description = "OmESetItem(" + tname + ", " + doubletostring(wt) + ")";

    DEBUGAPIRETURN(description);
    return description;
}

////////////////////////
// Methods for OmESet //
////////////////////////

std::string
OmESet::get_description() const
{
    DEBUGAPICALL("OmESet::get_description", "");
    std::string description;

    description = "ebound=" + om_inttostring(ebound);

    for (std::vector<OmESetItem>::const_iterator i = items.begin();
	 i != items.end();
	 i++) {
	if (description != "") description += ", ";
	description += i->get_description();
    }

    description = "OmESet(" + description + ")";
    DEBUGAPIRETURN(description);
    return description;
}

///////////////////////////////////
// Methods for OmEnquireInternal //
///////////////////////////////////

OmEnquireInternal::OmEnquireInternal(const OmDatabaseGroup &databases)
  : database(OmDatabaseGroup::InternalInterface::get_multidatabase(databases)),
    query(0)
{
}

OmEnquireInternal::~OmEnquireInternal()
{
    if(query != 0) {
	delete query;
	query = 0;
    }
}

void
OmEnquireInternal::set_query(const OmQuery &query_)
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
OmEnquireInternal::get_query()
{
    if (query == 0) {
        throw OmInvalidArgumentError("Can't get query before setting it");
    }
    return *query;
}

OmMSet
OmEnquireInternal::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmMatchOptions *moptions,
		    const OmMatchDecider *mdecider) const
{
    OmLockSentry locksentry(mutex);
    if(query == 0) {
        throw OmInvalidArgumentError("You must set a query before calling OmEnquire::get_mset()");
    }
    Assert(query->is_defined());

    // Use default options if none supplied
    OmMatchOptions defmoptions;
    if (moptions == 0) {
        moptions = &defmoptions;
    }

    // Set Rset
    OmRSet emptyrset;
    if(omrset == 0) {
	omrset = &emptyrset;
    }

    // FIXME: make match take a refcntptr
    // FIXME: incorporate weighting scheme into match options
    //
    // Notes: when accessing query, we don't need to lock mutex, since its our
    // own copy and we're locked ourselves
    MultiMatch match(database.get(),
		     query->internal,
		     *omrset,
		     IRWeight::WTTYPE_BM25,
		     *moptions);

    OmMSet retval;
    // Run query and get results into supplied OmMSet object
    match.match(first, maxitems, retval, mdecider);

    Assert(!(query->is_bool()) || retval.max_possible == 0);

    // Store what the first item requested was, so that this information is
    // kept with the mset.

    return retval;
}

OmESet
OmEnquireInternal::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmExpandOptions * eoptions,
		    const OmExpandDecider * edecider) const
{
    OmLockSentry locksentry(mutex);
    OmESet retval;

    OmExpandOptions defeoptions;
    if (eoptions == 0) {
        eoptions = &defeoptions;
    }

    // FIXME: make expand and rset take a refcntptr
    OmExpand expand(database.get());
    RSet rset(database.get(), omrset);

    DebugMsg("rset size is " << omrset.items.size() << endl);

    OmExpandDeciderAlways decider_always;
    if (edecider == 0) edecider = &decider_always;

    /* The auto_ptrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    std::auto_ptr<OmExpandDecider> decider_noquery;
    std::auto_ptr<OmExpandDecider> decider_andnoquery;
    
    if (query != 0 && !eoptions->use_query_terms) {
	std::auto_ptr<OmExpandDecider> temp1(
	    new OmExpandDeciderFilterTerms(query->get_terms()));
        decider_noquery = temp1;
	
	std::auto_ptr<OmExpandDecider> temp2(
	    new OmExpandDeciderAnd(decider_noquery.get(),
				   edecider));
	decider_andnoquery = temp2;

        edecider = decider_andnoquery.get();
    }
    
    expand.expand(maxitems, retval, &rset, edecider,
		  eoptions->use_exact_termfreq);

    return retval;
}

const OmDocument
OmEnquireInternal::get_doc(om_docid did) const
{
    OmLockSentry locksentry(mutex);
    return read_doc(did);
}

const OmDocument
OmEnquireInternal::get_doc(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(mutex);
    // FIXME: take advantage of OmMSetItem to ensure that database
    // doesn't get modified underneath us.
    return read_doc(mitem.did);
}

om_termname_list
OmEnquireInternal::get_matching_terms(om_docid did) const
{
    OmLockSentry locksentry(mutex);
    return calc_matching_terms(did);
}

om_termname_list
OmEnquireInternal::get_matching_terms(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(mutex);
    // FIXME: take advantage of OmMSetItem to ensure that database
    // doesn't get modified underneath us.
    return calc_matching_terms(mitem.did);
}

std::string
OmEnquireInternal::get_description() const
{
    std::string description;
    /// \todo get description of the database
    //description = database->get_description();
    description = "Database()";
    if (query != 0) {
	description += ", " + query->get_description();
    }
    return description;
}

///////////////////////////////////////////
// Private methods for OmEnquireInternal //
///////////////////////////////////////////

const OmDocument
OmEnquireInternal::read_doc(om_docid did) const
{
    LeafDocument *doc = database->open_document(did);

    return OmDocument(OmDocumentParams(doc));
}


struct ByQueryIndexCmp {
    typedef std::map<om_termname, unsigned int> tmap_t;
    const tmap_t &tmap;
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

om_termname_list
OmEnquireInternal::calc_matching_terms(om_docid did) const
{
    if (query == 0) {
        throw OmInvalidArgumentError("Can't get matching terms before setting query");
    }
    Assert(query->is_defined());

    // the ordered list of terms in the query.
    om_termname_list query_terms = query->get_terms();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    std::map<om_termname, unsigned int> tmap;
    unsigned int index = 1;
    for (om_termname_list::const_iterator i = query_terms.begin();
	 i != query_terms.end();
	 ++i) {
	tmap[*i] = index++;
    }
    
    std::auto_ptr<TermList> docterms(database->open_term_list(did));
    
    /* next() must be called on a TermList before you can
     * do anything else with it.
     */
    docterms->next();

    std::vector<om_termname> matching_terms;

    while (!docterms->at_end()) {
        std::map<om_termname, unsigned int>::iterator t =
		tmap.find(docterms->get_termname());
        if (t != tmap.end()) {
	    matching_terms.push_back(docterms->get_termname());
	}
	docterms->next();
    }
    
    // sort the resulting list by query position.
    sort(matching_terms.begin(),
	 matching_terms.end(),
	 ByQueryIndexCmp(tmap));

    return om_termname_list(matching_terms.begin(), matching_terms.end());
}

//////////////////////////
// Methods of OmEnquire //
//////////////////////////

OmEnquire::OmEnquire(const OmDatabaseGroup &databases)
{
    DEBUGAPICALL("OmEnquire::OmEnquire", databases);
    internal = new OmEnquireInternal(databases);
}

OmEnquire::~OmEnquire()
{
    DEBUGAPICALL("OmEnquire::~OmEnquire", "");
    delete internal;
    internal = NULL;
}

void
OmEnquire::set_query(const OmQuery & query_)
{
    DEBUGAPICALL("OmEnquire::set_query", query_);
    internal->set_query(query_);
}

const OmQuery &
OmEnquire::get_query()
{
    DEBUGAPICALL("OmEnquire::get_query", "");
    DEBUGAPIRETURN(internal->get_query());
    return internal->get_query();
}

OmMSet
OmEnquire::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmMatchOptions *moptions,
		    const OmMatchDecider *mdecider) const
{
    // FIXME: display contents of pointer params, if they're not null.
    DEBUGAPICALL("OmEnquire::get_mset",
		 first << ", " <<
		 maxitems << ", " <<
		 omrset << ", " <<
		 moptions << ", " << mdecider);

    // FIXME: this copies the mset too much: pass it in by reference?
    OmMSet mset(internal->get_mset(first, maxitems, omrset, moptions, mdecider));

    DEBUGAPIRETURN(mset);
    return mset;
}

OmESet
OmEnquire::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmExpandOptions * eoptions,
		    const OmExpandDecider * edecider) const
{
    // FIXME: display contents of pointer params and omrset, if they're not
    // null.
    DEBUGAPICALL("OmEnquire::get_eset",
		 maxitems << ", " <<
		 omrset << ", " <<
		 eoptions << ", " <<
		 edecider);

    // FIXME: this copies the eset too much: pass it in by reference?
    OmESet eset(internal->get_eset(maxitems, omrset, eoptions, edecider));

    DEBUGAPIRETURN(eset);
    return eset;
}

const OmDocument
OmEnquire::get_doc(om_docid did) const
{
    DEBUGAPICALL("OmEnquire::get_doc", did);
    OmDocument doc(internal->get_doc(did));
    DEBUGAPIRETURN(doc);
    return doc;
}

const OmDocument
OmEnquire::get_doc(const OmMSetItem &mitem) const
{
    DEBUGAPICALL("OmEnquire::get_doc", mitem);
    OmDocument doc(internal->get_doc(mitem));
    DEBUGAPIRETURN(doc);
    return doc;
}

om_termname_list
OmEnquire::get_matching_terms(const OmMSetItem &mitem) const
{
    DEBUGAPICALL("OmEnquire::get_matching_terms", mitem);
    om_termname_list matching_terms(internal->get_matching_terms(mitem));
    DEBUGAPIRETURN(matching_terms);
    return matching_terms;
}

om_termname_list
OmEnquire::get_matching_terms(om_docid did) const
{
    DEBUGAPICALL("OmEnquire::get_matching_terms", did);
    om_termname_list matching_terms(internal->get_matching_terms(did));
    DEBUGAPIRETURN(matching_terms);
    return matching_terms;
}

std::string
OmEnquire::get_description() const
{
    DEBUGAPICALL("OmEnquire::get_description", "");
    std::string description;

    description = "OmEnquire(" + internal->get_description() + ")";

    DEBUGAPIRETURN(description);
    return description;
}

