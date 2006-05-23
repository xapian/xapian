/* net_database.h: C++ class definition for network database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_NET_DATABASE_H
#define OM_HGUARD_NET_DATABASE_H

#include <map>
#include <string>

#include "omqueryinternal.h"
#include "stats.h"
#include "networkstats.h"
#include "omenquireinternal.h"
#include "omdebug.h"
#include "database.h"

using std::map;

class RemoteSubMatch;
class PendingMSetPostList;

/** A network database.  This is a reference to a remote database, and is
 *  mainly used by a RemoteSubMatch object.
 *  Subclassed to produce a class which is used by NetworkMatch to communicate
 *  with remote matching processes.
 */
class NetworkDatabase : public Xapian::Database::Internal {
    friend class RemoteSubMatch;
    friend class PendingMSetPostList;
    private:
	/// Set up the connection, including swapping statistics.
	void initialise_link();

	// disallow copies
	NetworkDatabase(const NetworkDatabase &);
	void operator=(const NetworkDatabase &);

    protected:
	/// The StatsSource which deals with the statistics
	NetworkStatsSource *statssource;

    public:
	/** Constructor. */
	NetworkDatabase() : statssource(0) { }

	/** Destructor. */
	~NetworkDatabase();

	/// Not yet implemented.
	Xapian::doclength get_doclength(Xapian::docid did) const;

	/// Not yet implemented.
	Xapian::termcount get_collection_freq(const string & tname) const;

	/// Not yet implemented.
	bool has_positions() const;

	/// Not yet implemented.
	LeafPostList * do_open_post_list(const string & tname) const;

	LeafTermList * open_term_list(Xapian::docid did) const;

	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;

	// Dummy implementation.
	void reopen();

	/// Not yet implemented.
	PositionList * open_position_list(Xapian::docid did,
					const string & tname) const;

	Xapian::Document::Internal * collect_document(Xapian::docid did) const;

	/// Not yet implemented.
	TermList * open_allterms() const;

	// Introspection methods...
	NetworkDatabase * as_networkdatabase() { return this; }

	void register_statssource(NetworkStatsSource *statssource_) {
	    statssource = statssource_;
	}

	/** Wait for some input to be available.
	 *  wait_for_input() can be called to avoid having to loop
	 *  waiting for eg finish_query() to return.  wait_for_input()
	 *  returns when the next operation can proceed without waiting.
	 */
	virtual void wait_for_input() = 0;

	/** Set the query
	 *
	 * @param query_    The query.
	 * @param qlen      The query length.
	 * @param wtscheme  Weighting scheme.
	 * @param omrset_   The rset.
	 */
	virtual void set_query(const Xapian::Query::Internal *query_,
			       Xapian::termcount qlen,
			       Xapian::valueno collapse_key,
			       Xapian::Enquire::docid_order order,
			       Xapian::valueno sort_key,
			       Xapian::Enquire::Internal::sort_setting sort_by,
			       bool sort_value_forward,
			       int percent_cutoff, Xapian::weight weight_cutoff,
			       const Xapian::Weight *wtscheme,
			       const Xapian::RSet &omrset_) = 0;

	/** Read the remote statistics.
	 *  Returns true if the call succeeded, or false
	 *  if the remote end hasn't yet responded.
	 */
	virtual bool get_remote_stats(Stats &out) = 0;

	/** Signal to the remote end that this is the end of the query
	 *  specification phase.
	 */
	virtual bool finish_query() = 0;

	/** Send the global statistics down */
	virtual void send_global_stats(const Stats &stats) = 0;

	/** Do the actual MSet fetching */
	virtual bool get_mset(Xapian::doccount first, Xapian::doccount maxitems,
			      Xapian::MSet &mset) = 0;

	virtual void close_end_time() const = 0;

	virtual void next(Xapian::weight w_min, Xapian::docid &did, Xapian::weight &w,
			  string &value) = 0;

	virtual void skip_to(Xapian::docid new_did, Xapian::weight w_min, Xapian::docid &did,
			     Xapian::weight &w, string &value) = 0;

	/** The structure used to hold a termlist item */
	struct TermListItem {
	    string tname;
	    Xapian::doccount termfreq;
	    Xapian::termcount wdf;

	    vector<Xapian::termpos> positions;
	};

	/** Retrieve a remote termlist */
	virtual void get_tlist(Xapian::docid did,
			       vector<TermListItem> &items) const = 0;

	/** Retrieve a remote document */
	virtual void get_doc(Xapian::docid did, string &doc,
			     map<Xapian::valueno, string> &values) const = 0;

	/** Collect a remote document */
	virtual void collect_doc(Xapian::docid did, string &doc,
				 map<Xapian::valueno, string> &values) const = 0;
};

#endif /* OM_HGUARD_NET_DATABASE_H */
