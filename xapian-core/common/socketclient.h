/* socketclient.h: implementation of NetClient over a socket
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

#ifndef OM_HGUARD_SOCKETCLIENT_H
#define OM_HGUARD_SOCKETCLIENT_H

#include "netclient.h"
#include "socketcommon.h"
#include "rset.h"

/** An implementation of the NetClient interface using a program.
 *  ProgClient gets a socket by spawning a separate program, rather
 *  than connecting to a remote machine.
 */
class SocketClient : public NetClient {
    private:
	// disallow copies
	SocketClient(const SocketClient &);
	void operator=(const SocketClient &);

	/// The socket filedescriptor
	int socketfd;

	/// Whether or not to close the socket on destruction
	bool close_socket;

	/// The timeout value used in network communications,
	/// in milliseconds
	int msecs_timeout;

	/// The line buffer which does the I/O
	OmSocketLineBuf buf;

	/// The conversation state
	enum {
	    state_getquery,  // Accumulate the query and other info
	    state_sentquery, // Query has been sent, waiting for remote stats.
	    state_sendglobal,// Ready to send the global stats
	    state_getmset,   // Ready to call get_mset
	    state_getresult  // Waiting for result
	} conv_state;

	/// The remote document count, given at open.
	om_doccount doccount;

	/// The remote document avlength, given at open.
	om_doclength avlength;

	/// The current query, as a string
	std::string query_string;

	/// The remote statistics
	Stats remote_stats;

	/// If true, the remote_stats are valid
	bool remote_stats_valid;

	/// The global statistics ready to be sent to the remote end
	Stats global_stats;

	/// If true, the global_stats are valid
	bool global_stats_valid;

	/// The match options object
	OmSettings moptions;

	/// The current RSet.
	OmRSet omrset;

	/// Read the initial data sent at the start of connection
	void handle_hello(const std::string &s);

	/** Write the string and get an "OK" message back,
	 *  or else throw an exception
	 */
	void do_simple_transaction(std::string msg);

	/** Write the string to the stream and return the
	 *  reply.  Throw an exception if the reply is "ERROR".
	 */
	std::string do_transaction_with_result(std::string msg);

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_spawned_socket(std::string progname, std::string arg);

	om_weight minw; // minimum weight we're interested in
	
    protected:
	/** Constructor.  The constructor is protected so that raw instances
	 *  can't be created - a derived class must be instantiated which
	 *  has code in the constructor to open the socket.
	 *
	 *  @param socketfd_  	The socket used for the communications.
	 *
	 *  @param close_socket_ If true (the default), then the SocketClient
	 *                      destructor will finish the session and close
	 *                      the socket.  If false, the derived class is
	 *                      responsible for the socket, which is assumed
	 *                      to be closed in ~SocketClient.
	 *
	 *  @param msecs_timeout_ The timeout used with the network operations.
	 *                       Generally an OmNetworkTimeout exception will
	 *                       be thrown if the remote end doesn't respond
	 *                       for this length of time (in milliseconds).
	 */
	SocketClient(int socketfd_,
		     bool close_socket_ = true,
		     int msecs_timeout_ = 10000);

	/// functions which actually do the work
	std::string do_read();
	void do_write(std::string data);

	/// Close the socket
	void do_close();

	bool get_posting(om_docid &did, om_weight &w, OmKey &key);

    public:
	/** Destructor. */
	virtual ~SocketClient();

	/** Write some bytes to the process.
	 */
	void write_data(std::string msg);

	/** Wait for input to be available */
	void wait_for_input();

	/** Set the query
	 *
	 * @param query_ The query.
	 * @param moptions_ The match options.
	 * @param omrset_ The rset.
	 */
	void set_query(const OmQueryInternal *query_,
		       const OmSettings &moptions_, const OmRSet &omrset_);

	/** Get the remote stats */
	bool get_remote_stats(Stats &out);

	/** Signal the end of the query specification phase.
	 *  Returns true if the operation succeeded, or false
	 *  if part or all of it is pending on network I/O.
	 */
	bool finish_query();

	/** Send the global statistics */
	void send_global_stats(const Stats &stats);

	/** Do the actual MSet fetching */
	bool get_mset(om_doccount first, om_doccount maxitems, OmMSet &mset);

	bool open_postlist(om_doccount first, om_doccount maxitems,
			   om_doccount &termfreq, om_weight &maxw,
			   std::map<om_termname, OmMSet::TermFreqAndWeight> &term_info);

	void next(om_weight w_min, om_docid &did, om_weight &w, OmKey &key);
	void skip_to(om_docid new_did, om_weight w_min, om_docid &did, om_weight &w, OmKey &key);
	
	/** get the remote termlist */
	void get_tlist(om_docid did,
		       std::vector<NetClient::TermListItem> &items);

	/** Retrieve a remote document */
	void get_doc(om_docid did,
		     std::string &doc,
		     std::map<om_keyno, OmKey> &keys);

	/** Request a remote document */
	void request_doc(om_docid did);

	/** Collect a remote document */
	void collect_doc(om_docid did, std::string &doc,
			 std::map<om_keyno, OmKey> &keys);

	/** Get the document count. */
	om_doccount get_doccount();

	/** Find out the remote average document length */
	om_doclength get_avlength();

	/** Read some data from the process.
	 */
	std::string read_data();

	/** Determine if any data is waiting to be read.
	 */
	bool data_is_available();
};

#endif  /* OM_HGUARD_SOCKETCLIENT_H */
