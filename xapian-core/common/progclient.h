/* progclient.h: implementation of NetClient which spawns a program.
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

#ifndef OM_HGUARD_PROGCLIENT_H
#define OM_HGUARD_PROGCLIENT_H

#include "netclient.h"
#include "progcommon.h"

/** An implementation of the NetClient interface using a program.
 *  ProgClient gets a socket by spawning a separate program, rather
 *  than connecting to a remote machine.
 */
class ProgClient : public NetClient {
    private:
	// disallow copies
	ProgClient(const ProgClient &);
	void operator=(const ProgClient &);

	/// The socket filedescriptor
	int socketfd;

	/// The line buffer which does the I/O
	OmLineBuf buf;

	/// The conversation state
	enum {
	    state_getquery,  // Accumulate the query and other info
	    state_getmset    // Ready to call get_mset
	} conv_state;

	/// The weighting type to be used, as a string
	string wt_string;

	/// The current query, as a string
	string query_string;

	/// The remote statistics
	Stats remote_stats;

	/// If true, the remote_stats are valid
	bool remote_stats_valid;

	/// The global statistics ready to be sent to the remote end
	Stats global_stats;

	/// If true, the global_stats are valid
	bool global_stats_valid;

	/// The max weight from the remote match
	om_weight remote_maxweight;

	/// functions which actually do the work
	string do_read();
	void do_write(string data);

	/** Write the string and get an "OK" message back,
	 *  or else throw an exception
	 */
	void do_simple_transaction(string msg);

	/** Write the string to the stream and return the
	 *  reply.  Throw an exception if the reply is "ERROR".
	 */
	string do_transaction_with_result(string msg);

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_spawned_socket(string progname, string arg);

    public:
	/** Constructor.
	 *
	 *  @param progname The name of the program to run.
	 */
	ProgClient(string progname, string arg);

	/** Destructor. */
	~ProgClient();

	/** Write some bytes to the process.
	 */
	void write_data(string msg);

	/** Set the weighting type */
	void set_weighting(IRWeight::weight_type wt_type);

	/** Set the query */
	void set_query(const OmQueryInternal *query_);

	/** Get the remote stats */
	Stats get_remote_stats();

	/** Signal the end of the query specification phase.
	 */
	void finish_query();

	/** Send the global statistics */
	void send_global_stats(const Stats &stats);

	/** Do the actual MSet fetching */
	void get_mset(om_doccount first,
		      om_doccount maxitems,
		      vector<OmMSetItem> &mset,
		      om_doccount *mbound,
		      om_weight *greatest_wt);

	/** Find the max weight */
	om_weight get_max_weight();

	/** Read some data from the process.
	 */
	string read_data();

	/** Determine if any data is waiting to be read.
	 */
	bool data_is_available();
};

#endif  /* OM_HGUARD_PROGCLIENT_H */
