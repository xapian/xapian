/* omenquire.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "omassert.h"
#include "omlocks.h"
#include "omdatabaseinternal.h"

#include <om/omerror.h>
#include <om/omenquire.h>

#include "rset.h"
#include "localmatch.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "database_builder.h"
#include <om/omdocument.h>
#include "omdocumentparams.h"
#include "omenquireinternal.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <math.h>

////////////////////////////////
// Methods for OmMatchOptions //
////////////////////////////////

OmMatchOptions::OmMatchOptions()
	: do_collapse(false),
	  sort_forward(true),
	  percent_cutoff(-1),
	  max_or_terms(0)
{}

void
OmMatchOptions::set_collapse_key(om_keyno key_)
{
    do_collapse = true;
    collapse_key = key_;
}

void
OmMatchOptions::set_no_collapse()
{
    do_collapse = false;
}

void
OmMatchOptions::set_sort_forward(bool forward_)
{
    sort_forward = forward_;
}

void
OmMatchOptions::set_percentage_cutoff(int percent_)
{
    if (percent_ >=0 && percent_ <= 100) {
        percent_cutoff = percent_;
    } else {
        throw OmInvalidArgumentError("Percent cutoff must be in 0..100");
    }
}

void
OmMatchOptions::set_max_or_terms(om_termcount max_)
{
    max_or_terms = max_;
}


/////////////////////////////////
// Methods for OmExpandOptions //
/////////////////////////////////

OmExpandOptions::OmExpandOptions()
	: allow_query_terms(false)
{}

void
OmExpandOptions::use_query_terms(bool allow_query_terms_)
{
    allow_query_terms = allow_query_terms_;
}

OmExpandDeciderFilterTerms::OmExpandDeciderFilterTerms(
                               const om_termname_list &terms)
	: tset(terms.begin(), terms.end()) {}

int
OmExpandDeciderFilterTerms::operator()(const om_termname &tname) const
{
    return (tset.find(tname) == tset.end());
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
// Methods for OmMSet //
////////////////////////

int
OmMSet::convert_to_percent(om_weight wt) const
{
    if(max_possible == 0) return 100;

    int pcent = (int) ceil(wt * 100 / max_possible);
    DebugMsg("wt = " << wt << ", max_possible = " << max_possible <<
	     " =>  pcent = " << pcent << endl);
    if(pcent > 100) pcent = 100;
    if(pcent < 0) pcent = 0;
    if(pcent == 0 && wt > 0) pcent = 1;

    return pcent;
}

int
OmMSet::convert_to_percent(const OmMSetItem & item) const
{
    return OmMSet::convert_to_percent(item.wt);
}

///////////////////////////////////
// Methods for OmEnquireInternal //
///////////////////////////////////

OmEnquireInternal::OmEnquireInternal(const OmDatabaseGroup &databases)
	: database(0), dbdesc(databases), query(0)
{
}

OmEnquireInternal::~OmEnquireInternal()
{
    if(database != 0) {
	delete database;
	database = 0;
    }
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

    open_database();
    Assert(database != 0);

    // Use default options if none supplied
    OmMatchOptions defmoptions;
    if (moptions == 0) {
        moptions = &defmoptions;
    }

    // Set Database
    MultiMatch match(database);

    // Set options
    match.set_options(*moptions);

    // Set Rset
    if((omrset != 0) && (omrset->items.size() != 0)) {
	match.set_rset(auto_ptr<RSet>(new RSet(database, *omrset)));
    }

    // Set weighting scheme
    // FIXME: incorporate into setting options
    if(query->is_bool()) {
	match.set_weighting(IRWeight::WTTYPE_BOOL);
    } else {
	match.set_weighting(IRWeight::WTTYPE_BM25);
    }

    // Set Query
    match.set_query(query->internal);

    OmMSet retval;

    // Set sort order
    // FIXME: incorporate into setting options
    mset_cmp cmp;
    if(moptions->sort_forward) {
	cmp = msetcmp_forward;
    } else {
	cmp = msetcmp_reverse;
    }

    // Run query and get results into supplied OmMSet object
    match.match(first, maxitems, retval.items, cmp,
		&(retval.mbound), &(retval.max_attained),
		mdecider);

    // Get max weight for an item in the MSet
    retval.max_possible = match.get_max_weight();
    Assert(!(query->is_bool()) || retval.max_possible == 0);

    // Store what the first item requested was, so that this information is
    // kept with the mset.
    retval.firstitem = first;

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

    open_database();
    OmExpand expand(database);
    RSet rset(database, omrset);

    DebugMsg("rset size is " << rset.get_rsize() << endl);

    OmExpandDeciderAlways decider_always;
    if (edecider == 0) edecider = &decider_always;

    /* The auto_ptrs will clean up any dynamically allocated
     * expand deciders automatically.
     */
    auto_ptr<OmExpandDecider> decider_noquery;
    auto_ptr<OmExpandDecider> decider_andnoquery;
    
    if (query != 0 && !eoptions->allow_query_terms) {
	auto_ptr<OmExpandDecider> temp1(
	    new OmExpandDeciderFilterTerms(query->get_terms()));
        decider_noquery = temp1;
	
	auto_ptr<OmExpandDecider> temp2(
	    new OmExpandDeciderAnd(decider_noquery.get(),
				   edecider));
	decider_andnoquery = temp2;

        edecider = decider_andnoquery.get();
    }
    
    expand.expand(maxitems, retval, &rset, edecider);

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
    // doesn't change underneath us.
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
    // doesn't change underneath us.
    return calc_matching_terms(mitem.did);
}

///////////////////////////////////////////
// Private methods for OmEnquireInternal //
///////////////////////////////////////////

/// Open the database(s), if not already open.
void
OmEnquireInternal::open_database() const
{
    if(database == 0) {
	if(dbdesc.internal->params.size() == 0) {
	    throw OmInvalidArgumentError("Must specify at least one database");
	} else {
	    DatabaseBuilderParams multiparams(OM_DBTYPE_MULTI);
	    multiparams.subdbs = dbdesc.internal->params;
	    auto_ptr<IRDatabase> tempdb(DatabaseBuilder::create(multiparams));

	    // FIXME: we probably need a better way of getting a
	    // MultiDatabase to avoid the dynamic_cast.
	    database = dynamic_cast<MultiDatabase *>(tempdb.get());
	    
	    if (database == 0) {
		// it wasn't really a MultiDatabase...
		Assert(false);
	    } else {
		// it was good - don't let the auto_ptr delete it.
		tempdb.release();
	    }
	}
    }
}

const OmDocument
OmEnquireInternal::read_doc(om_docid did) const
{
    open_database();
    LeafDocument *doc = database->open_document(did);

    return OmDocument(OmDocumentParams(doc));
}


struct ByQueryIndexCmp {
    typedef map<om_termname, unsigned int> tmap_t;
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

    open_database();  // will throw if database not set.

    // the ordered list of terms in the query.
    om_termname_list query_terms = query->get_terms();

    // copy the list of query terms into a map for faster access.
    // FIXME: a hash would be faster than a map, if this becomes
    // a problem.
    map<om_termname, unsigned int> tmap;
    unsigned int index = 1;
    for (om_termname_list::const_iterator i = query_terms.begin();
	 i != query_terms.end();
	 ++i) {
	tmap[*i] = index++;
    }
    
    auto_ptr<TermList> docterms(database->open_term_list(did));
    
    /* next() must be called on a TermList before you can
     * do anything else with it.
     */
    docterms->next();

    vector<om_termname> matching_terms;

    while (!docterms->at_end()) {
        map<om_termname, unsigned int>::iterator t =
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

////////////////////////////////////////////
// Initialise and delete OmEnquire object //
////////////////////////////////////////////

OmEnquire::OmEnquire(const OmDatabaseGroup &databases)
{
    internal = new OmEnquireInternal(databases);
}

OmEnquire::~OmEnquire()
{
    delete internal;
    internal = NULL;
}

//////////////////
// Set database //
//////////////////

void
OmEnquire::set_query(const OmQuery & query_)
{
    internal->set_query(query_);
}

OmMSet
OmEnquire::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmMatchOptions *moptions,
		    const OmMatchDecider *mdecider) const
{
    return internal->get_mset(first, maxitems, omrset, moptions, mdecider);
}

OmESet
OmEnquire::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmExpandOptions * eoptions,
		    const OmExpandDecider * edecider) const
{
    return internal->get_eset(maxitems, omrset, eoptions, edecider);
}

const OmDocument
OmEnquire::get_doc(om_docid did) const
{
    return internal->get_doc(did);
}

const OmDocument
OmEnquire::get_doc(const OmMSetItem &mitem) const
{
    return internal->get_doc(mitem);
}

om_termname_list
OmEnquire::get_matching_terms(const OmMSetItem &mitem) const
{
    return internal->get_matching_terms(mitem);
}

om_termname_list
OmEnquire::get_matching_terms(om_docid did) const
{
    return internal->get_matching_terms(did);
}
