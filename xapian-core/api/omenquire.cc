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
#include "leafmatch.h"
#include "multimatch.h"
#include "expand.h"
#include "database.h"
#include "database_builder.h"
#include <om/omdocument.h>

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
	  percent_cutoff(-1)
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

void OmMatchOptions::set_percentage_cutoff(int percent_)
{
    if (percent_ >=0 && percent_ <= 100) {
        percent_cutoff = percent_;
    } else {
        throw OmInvalidArgumentError("Percent cutoff must be in 0..100");
    }
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

/////////////////////////////////
// Internals of enquire system //
/////////////////////////////////

class OmEnquireInternal {
	mutable IRDatabase * database;
	OmDatabase dbdesc;
	
	/* This may need to be mutable in future so that it can be
	 * replaced by an optimised version.
	 */
	OmQuery * query;

    public:
	// pthread mutexes, if available.
	OmLock mutex;

	OmEnquireInternal(const OmDatabase &db);
	~OmEnquireInternal();

	void open_database() const;
	void set_query(const OmQuery & query_);
	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset,
			const OmMatchOptions *moptions,
			const OmMatchDecider *mdecider) const;
	OmESet get_eset(om_termcount maxitems,
			const OmRSet & omrset,
			const OmExpandOptions * eoptions,
			const OmExpandDecider * edecider) const;
	const OmDocument *get_doc(const OmMSetItem &mitem) const;
	const OmDocument *get_doc(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
	om_termname_list get_matching_terms(om_docid did) const;
};

//////////////////////////////////////////
// Inline methods for OmEnquireInternal //
//////////////////////////////////////////

OmEnquireInternal::OmEnquireInternal(const OmDatabase &db)
	: database(0), dbdesc(db), query(0)
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

// Open the database(s), if not already open.
void
OmEnquireInternal::open_database() const
{
    if(database == 0) {
	if(dbdesc.internal->params.size() == 0) {
	    throw OmInvalidArgumentError("Must specify at least one database");
	} else if(dbdesc.internal->params.size() == 1) {
	    database = DatabaseBuilder::create(dbdesc.internal->params.front());
	} else {
	    DatabaseBuilderParams multiparams(OM_DBTYPE_MULTI);
	    multiparams.subdbs = dbdesc.internal->params;
	    database = DatabaseBuilder::create(multiparams);
	}
    }
}

void
OmEnquireInternal::set_query(const OmQuery &query_)
{
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

    // Collection point for statistics
    StatsGatherer gatherer;

    // Set Database
#if 0
    vector<LeafMatch *> matchers;

    LeafMatch * temp = new LeafMatch(database, &gatherer);
    auto_ptr<LeafMatch> submatch(temp);
    matchers.push_back(temp);

    auto_ptr<Match> match(new MultiMatch(matchers));
#else
    auto_ptr<Match> match(new LeafMatch(database, &gatherer));
#endif

    // Set cutoff percent
    if (moptions->percent_cutoff > 0) {
        match->set_min_weight_percent(moptions->percent_cutoff);
    }

    // Set Rset
    // FIXME: should use an auto_ptr here
    RSet *rset = 0;
    if((omrset != 0) && (omrset->items.size() != 0)) {
	rset = new RSet(database, *omrset);
	match->set_rset(rset);
    }

    // FIXME: should be done by top match object.
    if(rset == 0) {
	gatherer.set_global_stats(0);
    } else {
	gatherer.set_global_stats(rset->get_rsize());
    }

    // Set options
    if(moptions->do_collapse) {
	match->set_collapse_key(moptions->collapse_key);
    }

    // Set Query
    match->set_query(query->internal);

    OmMSet retval;

    // Run query and get results into supplied OmMSet object
    if(query->is_bool()) {
	match->boolmatch(first, maxitems, retval.items);
	retval.mbound = retval.items.size();
	retval.max_possible = 1;
	if(retval.items.size() > 0) {
	    retval.max_attained = 1;
	} else {
	    retval.max_attained = 0;
	}
    } else {
	match->match(first, maxitems, retval.items,
		    msetcmp_forward, &(retval.mbound), &(retval.max_attained),
		    mdecider);

	// Get max weight for an item in the MSet
	retval.max_possible = match->get_max_weight();
    }

    // Store what the first item requested was, so that this information is
    // kept with the mset.
    retval.firstitem = first;

    // Do checks that the statistics got shared correctly.
    AssertParanoid(gatherer.get_stats()->collection_size ==
		   database->get_doccount());
    AssertParanoid((rset == 0 && gatherer.get_stats()->rset_size == 0) ||
		   (rset != 0 && gatherer.get_stats()->rset_size == rset->get_rsize()));

    // Clear up
    delete rset;

    return retval;
}

OmESet
OmEnquireInternal::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmExpandOptions * eoptions,
		    const OmExpandDecider * edecider) const
{
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

const OmDocument *
OmEnquireInternal::get_doc(om_docid did) const
{
    open_database();
    OmDocument *doc = database->open_document(did);

    return doc;
}

const OmDocument *
OmEnquireInternal::get_doc(const OmMSetItem &mitem) const
{
    open_database();
    return get_doc(mitem.did);
}

om_termname_list
OmEnquireInternal::get_matching_terms(const OmMSetItem &mitem) const
{
    // FIXME: take advantage of OmMSetItem to ensure that database
    // doesn't change underneath us.
    return get_matching_terms(mitem.did);
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
OmEnquireInternal::get_matching_terms(om_docid did) const
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

OmEnquire::OmEnquire(const OmDatabase &db)
{
    internal = new OmEnquireInternal(db);
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
    OmLockSentry locksentry(internal->mutex);
    internal->set_query(query_);
}

OmMSet
OmEnquire::get_mset(om_doccount first,
                    om_doccount maxitems,
                    const OmRSet *omrset,
                    const OmMatchOptions *moptions,
		    const OmMatchDecider *mdecider) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_mset(first, maxitems, omrset, moptions, mdecider);
}

OmESet
OmEnquire::get_eset(om_termcount maxitems,
                    const OmRSet & omrset,
	            const OmExpandOptions * eoptions,
		    const OmExpandDecider * edecider) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_eset(maxitems, omrset, eoptions, edecider);
}

const OmDocument *
OmEnquire::get_doc(om_docid did) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_doc(did);
}

const OmDocument *
OmEnquire::get_doc(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_doc(mitem);
}

om_termname_list
OmEnquire::get_matching_terms(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_matching_terms(mitem);
}

om_termname_list
OmEnquire::get_matching_terms(om_docid did) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_matching_terms(did);
}
