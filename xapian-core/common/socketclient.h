/* socketclient.h: implementation of NetClient over a socket
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

#ifndef OM_HGUARD_SOCKETCLIENT_H
#define OM_HGUARD_SOCKETCLIENT_H

#include "netclient.h"
#include "socketcommon.h"
#include "rset.h"
#include <deque>
using std::deque;
#include "omtime.h"

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
	Xapian::doccount doccount;

	/// The remote document avlength, given at open.
	Xapian::doclength avlength;

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

	/// A string serialisation of the weighting scheme
	std::string wtstring;

	/// A string serialisation of various match options
	std::string optstring;

	/// The current RSet.
	Xapian::RSet omrset;

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_spawned_socket(string progname, string arg);

	/// minimum weight we're interested in
	Xapian::weight minw;

	/// The context to return with any error messages
	string context;

	/// The queue of requested docids, in the right order
	deque<Xapian::docid> requested_docs;
	/* This would be a queue<Xapian::docid>, but that conflicts with
	 * some networking headers on Solaris.  Maybe when the 
	 * namespace actually works properly it can go back. */

	/** The number of times each requested document has been
	 *  requested.  This avoids multiple fetches of the same docuemnt,
	 *  as well as cases where the document is requested twice,
	 *  but removed from the cache on the first collect(), causing
	 *  an error on the second.
	 */
	map<Xapian::docid, unsigned int> request_count;

	struct cached_doc {
	    string data;
	    map<Xapian::valueno, string> values;
	    int users;  // number of clients wanting to retrieve this document
	};
	/** A store of the undecoded documents we've collected from the
	 *  other end
	 */
	map<Xapian::docid, cached_doc> collected_docs;

	void get_requested_docs();

    protected:
	/** Constructor.  The constructor is protected so that raw instances
	 *  can't be created - a derived class must be instantiated which
	 *  has code in the constructor to open the socket.
	 *
	 *  @param socketfd_  	The socket used for the communications.
	 *  @param msecs_timeout_ The timeout used with the network operations.
	 *                       Generally a Xapian::NetworkTimeout exception will
	 *                       be thrown if the remote end doesn't respond
	 *                       for this length of time (in milliseconds).
	 *  @param context_     The context to return with any error messages.
	 *  @param close_socket_ If true (the default), then the SocketClient
	 *                      destructor will finish the session and close
	 *                      the socket.  If false, the derived class is
	 *                      responsible for the socket, which is assumed
	 *                      to be closed in ~SocketClient.
	 */
	SocketClient(int socketfd_, int msecs_timeout_, string context_,
		     bool close_socket_ = true);

	/// functions which actually do the work
	string do_read();
	void do_write(string data);

	/// Close the socket
	void do_close();

	bool get_posting(Xapian::docid &did, Xapian::weight &w, string &value);

	/// The timeout value used in network communications, in milliseconds
	int msecs_timeout;

	/** The time at which the current operation will (eg a full
	 *  match) will time out.
	 */
	OmTime end_time;

	/// Whether the timeout is valid
	bool end_time_set;

    public:
	/// Initialise end_time to current time + msecs_timeout
	void init_end_time();

	/// Clear end_time
	void close_end_time();

	/** Destructor. */
	virtual ~SocketClient();

	/** Send a keep-alive signal */
	void keep_alive();

	/** Wait for input to be available */
	void wait_for_input();

	/** Set the query
	 *
	 * @param query_ The query.
	 * @param wtscheme Weighting scheme.
	 * @param omrset_ The rset.
	 */
	void set_query(const Xapian::Query::Internal *query_,
		       Xapian::valueno collapse_key, bool sort_forward,
		       int percent_cutoff, Xapian::weight weight_cutoff,
		       const Xapian::Weight *wtscheme,
		       const Xapian::RSet &omrset_);

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
	bool get_mset(Xapian::doccount first, Xapian::doccount maxitems, Xapian::MSet &mset);

	void next(Xapian::weight w_min, Xapian::docid &did, Xapian::weight &w, string &value);
	void skip_to(Xapian::docid new_did, Xapian::weight w_min, Xapian::docid &did, Xapian::weight &w, string &value);
	
	/** get the remote termlist */
	void get_tlist(Xapian::docid did,
		       vector<NetClient::TermListItem> &items);

	/** Retrieve a remote document */
	void get_doc(Xapian::docid did,
		     string &doc,
		     map<Xapian::valueno, string> &values);

	/** Request a remote document */
	void request_doc(Xapian::docid did);

	/** Collect a remote document */
	void collect_doc(Xapian::docid did, string &doc,
			 map<Xapian::valueno, string> &values);

	/** Get the document count. */
	Xapian::doccount get_doccount() const;

	/** Find out the remote average document length */
	Xapian::doclength get_avlength() const;

	/// Find out if term exists
	virtual bool term_exists(const string & tname);

	/// Find frequency of term
	virtual Xapian::doccount get_termfreq(const string & tname);

	/** Determine if any data is waiting to be read.
	 */
	bool data_is_available();
};

#endif  /* OM_HGUARD_SOCKETCLIENT_H */
