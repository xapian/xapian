/* tcpserver.h: class for TCP/IP-based server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2006,2007 Olly Betts
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

#ifndef OM_HGUARD_TCPSERVER_H
#define OM_HGUARD_TCPSERVER_H

#include "remoteserver.h"

/** A TCP server class, which uses SocketServer.
 */
class TcpServer {
    private:
	// disallow copies
	TcpServer(const TcpServer &);
	void operator=(const TcpServer &);

	// If wdb is set, true; if db is set, false.
	bool writable;

	/// The database we're using.
	Xapian::Database db;

	/// The writable database we're using.
	Xapian::WritableDatabase wdb;

	/// The listening socket
	int listen_socket;

	/// The active timeout to use with network communications
	int msecs_active_timeout;

	/// The idle timeout to use with network communications
	int msecs_idle_timeout;

	/// Output informtive messages?
	bool verbose;

#ifdef __WIN32__
	/** Member which ensures that winsock is initialised.
	 *
	 *  As long as an instance of WinsockInitializer exists,
	 *  we can assume that winsock is initialised.
	 */
	WinsockInitializer winsock_initialiser;
#endif

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
	 *  @param port	The port on which to listen for connections.
	 *  @param msecs_active_timeout_	The timeout (in milliseconds)
	 *			used while waiting for data from the client
	 *			during the handling of a request.
	 *  @param msecs_idle_timeout_		The timeout (in milliseconds)
	 *			used while waiting for a request from the
	 *			client while idle.
	 */
	TcpServer(Xapian::Database db_, int port,
		  int msecs_normal_timeout_ = 10000,
		  int msecs_idle_timeout_ = 60000,
		  bool verbose_ = true);

	TcpServer(Xapian::WritableDatabase db_, int port,
		  int msecs_normal_timeout_ = 10000,
		  int msecs_idle_timeout_ = 60000,
		  bool verbose_ = true);

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

#ifdef __WIN32__
	/** Handle a single request on an already connected socket.
	 *  May be called by multiple threads.
	 */
	void handle_one_request(int socket);
#endif
};

#endif  /* OM_HGUARD_TCPSERVER_H */
