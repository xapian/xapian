/* socketserver.h: class for socket-based server
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2006 Olly Betts
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

#ifndef OM_HGUARD_SOCKETSERVER_H
#define OM_HGUARD_SOCKETSERVER_H

//#define TIMING_PATCH // for webtop

#include <xapian/database.h>
#include "netserver.h"
#include "stats.h"
#include "autoptr.h"

class NetworkStatsGatherer;
class OmSocketLineBuf;

/** The base class of the network server object.
 *  A NetServer object is used by server programs to take care
 *  of a connection to a NetClient.
 */
class SocketServer : public NetServer {
    private:
	// disallow copies
	SocketServer(const SocketServer &);
	void operator=(const SocketServer &);

	/// The database we're associated with
	Xapian::Database db;

	/// The filedescriptors for talking to the remote end.
	int readfd;
	int writefd;

	/** The timeout before read/write operations give up and throw
	 *  and exception during a running transaction
	 */
	int msecs_active_timeout;

	/** The timeout before read/write operations give up and throw
	 *  and exception while waiting for a request from the client.
	 */
	int msecs_idle_timeout;

#ifdef TIMING_PATCH
	/// Timing mode.
	bool timing;

#endif /* TIMING_PATCH */
	/// The line buffer for doing the actual I/O
	AutoPtr<OmSocketLineBuf> buf;

	/// The various states of the conversation we can be in
	enum conv_states {
	    /// "Ready" state, waiting for a request.
	    conv_ready,
	    /// Got request, going to send local stats
	    conv_sendlocal,
	    /// Waiting for response with global stats
	    conv_getglobal,
	    /// Got stats, will next send the MSet.
	    conv_sendresult
	} conversation_state;

	/// The gatherer we can use to get local statistics
	NetworkStatsGatherer *gatherer;

	/// Locally cached global statistics
	Stats global_stats;

	/// Flag indicating that we have the global statistics
	bool have_global_stats;

	/// Registered weighting schemes
	map<string, Xapian::Weight *> wtschemes;

	/// Run the match conversation
	void run_match(const std::string &firstmessage);

	/// run the get term list conversation
	void run_gettermlist(const std::string &firstmessage);

	/// run the get document conversation
	void run_getdocument(const std::string &firstmessage);

	/// Handle a keepalive message
	void run_keepalive();

	/** Read a line of input from the buffer, and process
	 *  any special messages (eg 'X')
	 */
	std::string readline(int msecs_delay = 0);

	/// Write a line of output to the buffer
	void writeline(const std::string &message,
			      int msecs_delay = 0);

    public:
	/** Default constructor.
	 *  @param db		The database on which searches are done.
	 *  @param readfd_	The file descriptor for reading.
	 *  @param writefd_	The file descriptor for writing.  If
	 *                      missing or -1, then readfd_ will be used
	 *                      instead.
	 *  @param msecs_active_timeout_	The timeout (in milliseconds)
	 *  			used while waiting for data from the client
	 *  			during the handling of a request.
	 *  @param msecs_idle_timeout_		The timeout (in milliseconds)
	 *  			used while waiting for a request from the
	 *  			client while idle.
	 */
	SocketServer(const Xapian::Database &db,
		     int readfd_,
		     int writefd_ = -1,
		     int msecs_active_timeout_ = 10000,
#ifndef TIMING_PATCH
		     int msecs_idle_timeout_ = 60000);
#else /* TIMING_PATCH */
		     int msecs_idle_timeout_ = 60000,
		     bool timing_ = false);
#endif /* TIMING_PATCH */

	/** Default constructor.
	 *  @param db		The database on which searches are done.
	 *  @param buffer	OmSocketLineBuf already connected to remote end.
	 *  @param msecs_active_timeout_	The timeout (in milliseconds)
	 *  			used while waiting for data from the client
	 *  			during the handling of a request.
	 *  @param msecs_idle_timeout_		The timeout (in milliseconds)
	 *  			used while waiting for a request from the
	 *  			client while idle.
	 */
	SocketServer(const Xapian::Database &db,
		     AutoPtr<OmSocketLineBuf> buffer,
		     int msecs_active_timeout_ = 10000,
#ifndef TIMING_PATCH
		     int msecs_idle_timeout_ = 60000);
#else /* TIMING_PATCH */
		     int msecs_idle_timeout_ = 60000,
		     bool timing_ = false);
#endif /* TIMING_PATCH */

	/** Destructor. */
	~SocketServer();

	/** Send the local statistics to the remote gatherer.
	 *  The remote gatherer works out the global statistics from
	 *  this.
	 */
	void send_local_stats(const Stats &stats);

	/** Ask for the remote global statistics.
	 *  These are calculated from the contributed local statistics.
	 */
	Stats get_global_stats();

	/** Handle requests from a ProgClient until the connection
	 *  is closed.
	 */
	void run();

	/** Register a custom weighting scheme
	 */
	void register_weighting_scheme(const Xapian::Weight &wt) {
	    wtschemes[wt.name()] = wt.clone();
	}
};

#endif  /* OM_HGUARD_SOCKETSERVER_H */
