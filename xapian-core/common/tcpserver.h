/* tcpserver.h: class for TCP/IP-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef OM_HGUARD_TCPSERVER_H
#define OM_HGUARD_TCPSERVER_H

#include "socketserver.h"
#include "socketcommon.h"

/** A TCP server class, which uses SocketServer.
 */
class TcpServer {
    private:
	// disallow copies
	TcpServer(const TcpServer &);
	void operator=(const TcpServer &);

	/// The listening port number.
	int port;

	/// The database we're using.
	Xapian::Database db;

	/// The listening socket
	int listen_socket;

	/// The active timeout to use with network communications
	int msecs_active_timeout;

	/// The idle timeout to use with network communications
	int msecs_idle_timeout;

	/// Output informtive messages?
	bool verbose;

#ifdef TIMING_PATCH
	/// Output timing stats?
	bool timing;

#endif /* TIMING_PATCH */
	/** Open the listening socket and return a filedescriptor to
	 *  it.
	 *
	 *  @param port	The port to bind the listening socket to.
	 */
	static int get_listening_socket(int port);

	/** Open the listening socket and return a filedescriptor to
	 *  it.
	 */
	int get_connected_socket();
    public:
	/** Default constructor.
	 *
	 *  @param db_		The database used for matches etc.
	 *  @param port_	The port on which to listen for connections.
	 *  @param msecs_active_timeout_	The timeout (in milliseconds)
	 *  			used while waiting for data from the client
	 *  			during the handling of a request.
	 *  @param msecs_idle_timeout_		The timeout (in milliseconds)
	 *  			used while waiting for a request from the
	 *  			client while idle.
	 */
	TcpServer(Xapian::Database db_, int port_,
		  int msecs_normal_timeout_ = 10000,
		  int msecs_idle_timeout_ = 60000,
#ifndef TIMING_PATCH
		  bool verbose_ = true);
#else /* TIMING_PATCH */
		  bool verbose_ = true, bool timing_ = false);
#endif /* TIMING_PATCH */

	/** Destructor. */
	~TcpServer();

	/** Start the serving session.
	 *
	 *  Continues to accept connections and serve them.
	 *  Currently this is done sequentially.
	 */
	void run();

	/** Handle one incoming connection and stop.
	 */
	void run_once();
};

#endif  /* OM_HGUARD_TCPSERVER_H */
