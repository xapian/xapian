/* localmatch.h: class for performing the match calculations on postlists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
class Document;
class PostList;

#include <vector>
using std::vector;
#include <map>
using std::map;
#include "autoptr.h"
#include "omdebug.h"

class LocalSubMatch : public SubMatch {
    private:
        // Prevent copying
        LocalSubMatch(const LocalSubMatch &);
        LocalSubMatch & operator=(const LocalSubMatch &);

	AutoPtr<StatsSource> statssource;
	
	bool is_prepared;

	/// Query to be run
	OmQuery::Internal users_query;

	const Database *db;

	/// RSet to be used (affects weightings)
	AutoPtr<RSet> rset;
    
	/// The size of the query (passed to IRWeight objects)
	om_doclength querysize;
    
	/// Stored match options object
	OmSettings opts;

	/// The weights and termfreqs of terms in the query.
	map<om_termname, OmMSet::Internal::Data::TermFreqAndWeight> term_info;


	PostList * build_xor_tree(vector<PostList *> &postlists,
				  MultiMatch *matcher);

	PostList * build_and_tree(vector<PostList *> &postlists,
				  MultiMatch *matcher);

	PostList * build_or_tree(vector<PostList *> &postlists,
				 MultiMatch *matcher);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList *postlist_from_queries(OmQuery::Internal::op_t op,
				const OmQuery::Internal::subquery_list &queries,
				om_termpos window,
				om_termcount elite_set_size,
				MultiMatch *matcher,
				bool is_bool);

	/// Make a postlist from a query object
	PostList *postlist_from_query(const OmQuery::Internal * query,
				      MultiMatch *matcher,
				      bool is_bool);

	void register_term(const om_termname &tname) {
	    statssource->my_termfreq_is(tname, db->get_termfreq(tname));
	}

	/// Make a weight - default argument is used for finding extra_weight
	IRWeight * mk_weight(const OmQuery::Internal *query = NULL);

    public:
	LocalSubMatch(const Database *db_, const OmQuery::Internal * query,
		      const OmRSet & omrset, const OmSettings &opts_,
		      StatsGatherer *gatherer)
		: statssource(new LocalStatsSource(gatherer)),
		  is_prepared(false), users_query(*query), db(db_),
		  querysize(query->qlen), opts(opts_)
	{	    
	    DEBUGCALL(MATCH, void, "LocalSubMatch::LocalSubMatch",
		      db << ", " << query << ", " << omrset << ", " <<
		      opts_ << ", " << gatherer << ", ");
	    AutoPtr<RSet> new_rset(new RSet(db, omrset));
	    rset = new_rset;

	    statssource->take_my_stats(db->get_doccount(), db->get_avlength());
	}

	~LocalSubMatch()
	{
	    DEBUGCALL(MATCH, void, "LocalSubMatch::~LocalSubMatch", "");
	}
	
	/// Calculate the statistics for the query
	bool prepare_match(bool nowait);

	PostList * get_postlist(om_doccount maxitems, MultiMatch *matcher);

	virtual Document * open_document(om_docid did) const {
	    return db->open_document(did);
	}

	const map<om_termname, OmMSet::Internal::Data::TermFreqAndWeight> get_term_info() const {
	    return term_info;
	}
};   

#endif /* OM_HGUARD_LOCALMATCH_H */
