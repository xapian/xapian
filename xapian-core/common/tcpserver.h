/* tcpserver.h: class for TCP/IP-based server.
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

#ifndef OM_HGUARD_TCPSERVER_H
#define OM_HGUARD_TCPSERVER_H

#include "socketserver.h"
#include "database.h"
#include "multi_database.h"
#include "socketcommon.h"
#include <memory>

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
	RefCntPtr<MultiDatabase> db;

	/// The listening socket
	int listen_socket;

	/// The timeout to use with network communications
	int msecs_timeout;

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
	/** Default constructor. */
	TcpServer(RefCntPtr<MultiDatabase> db_, int port_,
		  int msecs_timeout_ = 10000);

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
	
	/** Signal handlers
	 */
	static void on_SIGTERM(int);
	static void on_SIGCLD(int);
};

#endif  /* OM_HGUARD_TCPSERVER_H */
