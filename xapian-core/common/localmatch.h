/* localmatch.h: class for performing the match calculations on postlists
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

#ifndef OM_HGUARD_LOCALMATCH_H
#define OM_HGUARD_LOCALMATCH_H

#include "om/omenquire.h"
#include "om/omsettings.h"
#include "omqueryinternal.h"
#include "match.h"
#include "stats.h"
#include "rset.h"

class IRWeight;
class Database;
class LeafDocument;
class PostList;

#include <vector>
#include <map>
#include "autoptr.h"

class SubMatch : public RefCntBase {
    public:
	/** Name of weighting scheme to use.
	 *  This may differ from the requested scheme if, for example,
	 *  the query is pure boolean.
	 */
	string weighting_scheme;

	/// The size of the query (passed to IRWeight objects)
	om_doclength querysize;
    
	/// Stored match options object
	OmSettings mopts;

	StatsSource * statssource;
	
	SubMatch(const OmQueryInternal * query, const OmSettings &mopts_,
		 StatsSource * statssource_)
	    : querysize(query->qlen), mopts(mopts_), statssource(statssource_)
	{
	    // Check that we have a valid query to run
	    if (!query->isdefined) {
		throw OmInvalidArgumentError("Query is not defined.");
	    }
	    
	    // If query is boolean, set weighting to boolean
	    if (query->is_bool()) {
		weighting_scheme = "bool";
	    } else {
		weighting_scheme = mopts.get("match_weighting_scheme", "bm25");
	    }	    
	}

	virtual ~SubMatch() {
	    delete statssource;
	}

	///////////////////////////////////////////////////////////////////
	// Prepare to do the match
	// =======================

	/** Prepare to perform the match operation.
	 *  This must be called with a return value of true before
	 *  get_postlist().  It can be called more
	 *  than once.  If nowait is true, the operation has only succeeded
	 *  when the return value is true.
	 *
	 *  @param nowait	If true, then return as soon as
	 *  			possible even if the operation hasn't
	 *  			been completed.  If it hasn't, then
	 *  			the return value will be false.  The
	 *  			caller should retry until prepare_match
	 *  			returns true, or throws an exception to
	 *  			indicate an error.
	 *
	 *  @return  If nowait is true, and the match is being performed
	 *           over a network connection, and the result isn't
	 *           immediately available, this method returns false.
	 *           In all other circumstances it will return true.
	 */
	virtual bool prepare_match(bool nowait) = 0;

	/// Make a weight - default argument is used for finding extra_weight
	IRWeight * mk_weight(const OmQueryInternal *query = NULL);

	virtual PostList * get_postlist(om_doccount maxitems) = 0;

	virtual LeafDocument * open_document(om_docid did) const = 0;

	virtual const std::map<om_termname, OmMSet::TermFreqAndWeight> get_term_info() const = 0;
};

class LocalSubMatch : public SubMatch {
    private:
	bool is_prepared;

	/// Query to be run
	OmQueryInternal users_query;

	const Database *db;
	PostList *postlist;

	/// RSet to be used (affects weightings)
	AutoPtr<RSet> rset;
    
	/** Optional limit on number of terms to OR together.
	 *  If zero, there is no limit.
	 */
	om_termcount max_or_terms;

	/// The weights and termfreqs of terms in the query.
	std::map<om_termname, OmMSet::TermFreqAndWeight> term_info;


	PostList * build_and_tree(std::vector<PostList *> &postlists);

	PostList * build_or_tree(std::vector<PostList *> &postlists);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList *postlist_from_queries(OmQuery::op op,
				const std::vector<OmQueryInternal *> & queries,
				om_termcount window);

	/// Make a postlist from a query object
	PostList *postlist_from_query(const OmQueryInternal * query);

	void register_term(const om_termname &tname) {
	    statssource->my_termfreq_is(tname, db->get_termfreq(tname));
	}

    public:
	LocalSubMatch(const Database *db_, const OmQueryInternal * query,
		      const OmRSet & omrset, const OmSettings &mopts_,
		      StatsGatherer *gatherer)
		: SubMatch(query, mopts_, new LocalStatsSource),
		  is_prepared(false), users_query(*query), db(db_)
	{	    
	    AutoPtr<RSet> new_rset(new RSet(db, omrset));
	    rset = new_rset;

	    max_or_terms = mopts.get_int("match_max_or_terms", 0);

	    statssource->my_collection_size_is(db->get_doccount());
	    statssource->my_average_length_is(db->get_avlength());
	    statssource->connect_to_gatherer(gatherer);
	}

	/// Calculate the statistics for the query
	bool prepare_match(bool nowait);

	PostList * get_postlist(om_doccount maxitems);

	virtual LeafDocument * open_document(om_docid did) const {
	    return db->open_document(did);
	}

	const std::map<om_termname, OmMSet::TermFreqAndWeight> get_term_info() const {
	    return term_info;
	}
};   

////////////////////////////////////////////////////////////////////////////
// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b
// (By default, equally weighted items will be returned in reverse
// document id number.)

typedef bool (* mset_cmp)(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_forward(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_reverse(const OmMSetItem &, const OmMSetItem &);

#endif /* OM_HGUARD_LOCALMATCH_H */
