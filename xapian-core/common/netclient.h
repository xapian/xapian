/* netclient.h: base class for network connection object
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

#ifndef OM_HGUARD_NETCLIENT_H
#define OM_HGUARD_NETCLIENT_H

#include <string>
#include "refcnt.h"
#include "irweight.h"
#include "omqueryinternal.h"
#include "stats.h"
#include "networkstats.h"

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
	NetClient() : statssource(0) {}
	/** Destructor. */
	virtual ~NetClient() {};

	void register_statssource(NetworkStatsSource *statssource_);

	/** Write some bytes to the remote process.
	 *  FIXME: These will be specialised into content-specific
	 *  writer and reader functions.
	 */
	virtual void write_data(std::string msg) = 0;

	/** Wait for some input to be available.
	 *  wait_for_input() can be called to avoid having to loop
	 *  waiting for eg finish_query() to return.  wait_for_input()
	 *  returns when the next operation can proceed without waiting.
	 */
	virtual void wait_for_input() = 0;

	/** Set the query */
	virtual void set_query(const OmQueryInternal *query_) = 0;

	/** Read the remote statistics.
	 *  Returns true if the call succeeded, or false
	 *  if the remote end hasn't yet responded.
	 */
	virtual bool get_remote_stats(Stats &out) = 0;

	/** Set the options to be used for this query.
	 *
	 * @param moptions_ The match options.
	 */
	virtual void set_options(const OmSettings &moptions_) = 0;

	/** Set the rset to be transmitted over the link.
	 *
	 * @param rset_  A pointer to the rset object.
	 */
	virtual void set_rset(const OmRSet &omrset_) = 0;

	/** Signal to the remote end that this is the end of the query
	 *  specification phase.
	 */
	virtual bool finish_query() = 0;

	/** Send the global statistics down */
	virtual void send_global_stats(const Stats &stats) = 0;

	/** Do the actual MSet fetching */
	virtual bool get_mset(om_doccount first,
			      om_doccount maxitems,
			      OmMSet &mset) = 0;

	/** The structure used to hold a termlist item */
	struct TermListItem {
	    om_termname tname;
	    om_doccount termfreq;
	    om_termcount wdf;

	    std::vector<om_termpos> positions;
	};

	/** Retrieve a remote termlist */
	virtual void get_tlist(om_docid did,
			       std::vector<TermListItem> &items) = 0;

	/** Retrieve a remote document */
	virtual void get_doc(om_docid did,
			     std::string &doc,
			     std::map<om_keyno, OmKey> &keys) = 0;

	/** Find out the max_weight */
	virtual om_weight get_max_weight() = 0;

	/** Find out the remote document count */
	virtual om_doccount get_doccount() = 0;

	/** Find out the remote average document length */
	virtual om_doclength get_avlength() = 0;

	/** Read some data from the remote process.
	 */
	virtual std::string read_data() = 0;

	/** Determine if any data is waiting to be read.
	 */
	virtual bool data_is_available() = 0;
};

inline void
NetClient::register_statssource(NetworkStatsSource *statssource_) {
    statssource = statssource_;
}

#endif  /* OM_HGUARD_NETCLIENT_H */
