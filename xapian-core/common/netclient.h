/* netclient.h: base class for network connection object
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

#ifndef OM_HGUARD_NETCLIENT_H
#define OM_HGUARD_NETCLIENT_H

#include <string>
#include "refcnt.h"
#include "omqueryinternal.h"
#include "stats.h"
#include "networkstats.h"
#include "omenquireinternal.h"

using std::map;

/** The base class of the network interface.
 *  A NetClient object is used by NetworkMatch to communicate
 *  with remote matching processes.
 */
class NetClient : public RefCntBase {
    private:
	// disallow copies
	NetClient(const NetClient &);
	void operator=(const NetClient &);

    protected:
	/// The StatsSource which deals with the statistics
	NetworkStatsSource *statssource;

    public:
	/** Default constructor. */
	NetClient() : statssource(0) { }

	/** Destructor. */
	virtual ~NetClient() { }

	virtual void keep_alive() = 0;

	void register_statssource(NetworkStatsSource *statssource_);

	/** Wait for some input to be available.
	 *  wait_for_input() can be called to avoid having to loop
	 *  waiting for eg finish_query() to return.  wait_for_input()
	 *  returns when the next operation can proceed without waiting.
	 */
	virtual void wait_for_input() = 0;

	/** Set the query
	 *
	 * @param query_ The query.
	 * @param wtscheme Weighting scheme.
	 * @param omrset_ The rset.
	 */
	virtual void set_query(const Xapian::Query::Internal *query_,
			       om_valueno collapse_key, bool sort_forward,
			       int percent_cutoff, om_weight weight_cutoff,
			       const Xapian::Weight *wtscheme,
			       const OmRSet &omrset_) = 0;

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
	virtual bool get_mset(om_doccount first, om_doccount maxitems,
			      Xapian::MSet &mset) = 0;

	virtual void close_end_time() = 0;
	
	virtual void next(om_weight w_min, om_docid &did, om_weight &w,
			  string &value) = 0;

	virtual void skip_to(om_docid new_did, om_weight w_min, om_docid &did,
			     om_weight &w, string &value) = 0;

	/** The structure used to hold a termlist item */
	struct TermListItem {
	    string tname;
	    om_doccount termfreq;
	    om_termcount wdf;

	    vector<om_termpos> positions;
	};

	/** Retrieve a remote termlist */
	virtual void get_tlist(om_docid did,
			       vector<TermListItem> &items) = 0;

	/** Retrieve a remote document */
	virtual void get_doc(om_docid did, string &doc,
			     map<om_valueno, string> &values) = 0;

	/** Request a remote document */
	virtual void request_doc(om_docid did) = 0;

	/** Collect a remote document */
	virtual void collect_doc(om_docid did, string &doc,
				 map<om_valueno, string> &values) = 0;

	/** Find out the remote document count */
	virtual om_doccount get_doccount() const = 0;

	/** Find out the remote average document length */
	virtual om_doclength get_avlength() const = 0;

	/// Find out if term exists
	virtual bool term_exists(const string & tname) = 0;

	/// Find frequency of term
	virtual om_doccount get_termfreq(const string & tname) = 0;

	/** Determine if any data is waiting to be read.  */
	virtual bool data_is_available() = 0;
};

inline void
NetClient::register_statssource(NetworkStatsSource *statssource_) {
    statssource = statssource_;
}

#endif  /* OM_HGUARD_NETCLIENT_H */
