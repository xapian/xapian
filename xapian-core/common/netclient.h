/* netclient.h: base class for network connection object
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

#ifndef OM_HGUARD_NETCLIENT_H
#define OM_HGUARD_NETCLIENT_H

#include <string>
#include "omrefcnt.h"
#include "irweight.h"
#include "omqueryinternal.h"
#include "stats.h"

/** The base class of the network interface.
 *  A NetClient object is used by NetworkMatch to communicate
 *  with remote matching processes.
 */
class NetClient : public OmRefCntBase {
    private:
	// disallow copies
	NetClient(const NetClient &);
	void operator=(const NetClient &);

    public:
	/** Default constructor. */
	NetClient() {}
	/** Destructor. */
	virtual ~NetClient() {};

	/** Write some bytes to the remote process.
	 *  FIXME: These will be specialised into content-specific
	 *  writer and reader functions.
	 */
	virtual void write_data(string msg) = 0;

	/** Set the weighting type */
	virtual void set_weighting(IRWeight::weight_type wt_type) = 0;

	/** Set the query */
	virtual void set_query(const OmQueryInternal *query_) = 0;

	/** Read the remote statistics */
	virtual Stats get_remote_stats() = 0;

	/** Do the actual MSet fetching */
	virtual void get_mset(om_doccount first,
			      om_doccount maxitems,
			      vector<OmMSetItem> &mset,
			      om_doccount *mbound,
			      om_weight *greatest_wt) = 0;

	/** Find out the max_weight */
	virtual om_weight get_max_weight() = 0;

	/** Read some data from the remote process.
	 */
	virtual std::string read_data() = 0;

	/** Determine if any data is waiting to be read.
	 */
	virtual bool data_is_available() = 0;
};

#endif  /* OM_HGUARD_NETCLIENT_H */
