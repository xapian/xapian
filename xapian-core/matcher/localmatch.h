/* localmatch.h: class for performing the match calculations on postlists
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_LOCALMATCH_H
#define OM_HGUARD_LOCALMATCH_H

#include "omqueryinternal.h"
#include "match.h"
#include "stats.h"
#include "rset.h"

class OmWeight;
class Database;
class Document;
namespace Xapian {
    class PostListIterator::Internal;
}
typedef Xapian::PostListIterator::Internal PostList;

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

	AutoPtr<OmWeight::Internal> statssource;

	bool is_prepared;

	/// Query to be run
	Xapian::Query::Internal users_query;

	const Database *db;

	/// RSet to be used (affects weightings)
	AutoPtr<RSet> rset;
    
	/// The size of the query (passed to OmWeight objects)
	om_doclength querysize;
    
	/// Weighting scheme object
	const OmWeight * wtscheme;

	/// The weights and termfreqs of terms in the query.
	map<string, Xapian::MSet::Internal::TermFreqAndWeight> term_info;


	PostList * build_xor_tree(vector<PostList *> &postlists,
				  MultiMatch *matcher);

	PostList * build_and_tree(vector<PostList *> &postlists,
				  MultiMatch *matcher);

	PostList * build_or_tree(vector<PostList *> &postlists,
				 MultiMatch *matcher);

	/// Make a postlist from a vector of query objects (AND or OR)
	PostList *postlist_from_queries(Xapian::Query::Internal::op_t op,
				const Xapian::Query::Internal::subquery_list &queries,
				om_termpos window,
				om_termcount elite_set_size,
				MultiMatch *matcher,
				bool is_bool);

	/// Make a postlist from a query object
	PostList *postlist_from_query(const Xapian::Query::Internal * query,
				      MultiMatch *matcher,
				      bool is_bool);

	void register_term(const string &tname) {
	    statssource->my_termfreq_is(tname, db->get_termfreq(tname));
	}

    public:
	LocalSubMatch(const Database *db_,
		      const Xapian::Query::Internal * query,
		      const OmRSet & omrset, StatsGatherer *gatherer,
		      const OmWeight *wtscheme_)
		: statssource(new LocalStatsSource(gatherer)),
		  is_prepared(false), users_query(*query), db(db_),
		  querysize(query->qlen), wtscheme(wtscheme_)
	{	    
	    DEBUGCALL(MATCH, void, "LocalSubMatch::LocalSubMatch",
		      db << ", " << query << ", " << omrset << ", " <<
		      gatherer << ", [wtscheme]");
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

	const map<string, Xapian::MSet::Internal::TermFreqAndWeight> get_term_info() const {
	    return term_info;
	}
};   

#endif /* OM_HGUARD_LOCALMATCH_H */
