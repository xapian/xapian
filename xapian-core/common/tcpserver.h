/** @file tcpclient.h
 *  @brief TCP/IP socket based server for RemoteDatabase.
 */
/* Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_TCPSERVER_H
#define XAPIAN_INCLUDED_TCPSERVER_H

#include <xapian/database.h>
#include <xapian/visibility.h>

#ifdef __WIN32__
# include "remoteconnection.h"
# define SOCKET_INITIALIZER_MIXIN : private WinsockInitializer
#else
# define SOCKET_INITIALIZER_MIXIN
#endif

#if defined __CYGWIN__ || defined __WIN32__
# include "safewindows.h" // Only for HANDLE!
#endif

/** TCP/IP socket based server for RemoteDatabase.
 *
 *  This class implements the server used by xapian-tcpsrv.
 */
class XAPIAN_VISIBILITY_DEFAULT TcpServer SOCKET_INITIALIZER_MIXIN {
    /// Don't allow assignment.
    void operator=(const TcpServer &);

    /// Don't allow copying.
    TcpServer(const TcpServer &);

    /** Paths to the databases we will open.
     *
     *  Contains exactly one entry if writable, and at least one if not.
     */
    const std::vector<std::string> dbpaths;

    /** Is this a WritableDatabase? */
    bool writable;

#if defined __CYGWIN__ || defined __WIN32__
    /// Mutex to stop two TcpServers running on the same port.
    HANDLE mutex;
#endif

    /** The socket we're listening on. */
    int listen_socket;

    /** Timeout between messages during a single operation (in milliseconds). */
    int msecs_active_timeout;

    /** Timeout between operations (in milliseconds). */
    int msecs_idle_timeout;

    /** Should we produce output when connections are made or lost? */
    bool verbose;

    /** Create a listening socket ready to accept connections.
     *
     *  @param host	hostname or address to listen on or an empty string to
     *			accept connections on any interface.
     *  @param port	TCP port to listen on.
     */
    static int get_listening_socket(const std::string & host, int port
#if defined __CYGWIN__ || defined __WIN32__
				    , HANDLE &mutex
#endif
	    );

    /** Accept a connection and return the filedescriptor for it. */
    int accept_connection();

  public:
    /** Construct a TcpServer for a Database and start listening for
     *  connections.
     *
     *  @param dbpaths_	The path(s) to the database(s) we should open.
     *  @param host	The hostname or address for the interface to listen on
     *			(or "" to listen on all interfaces).
     *  @port		The TCP port number to listen on.
     *  @param msecs_active_timeout	Timeout between messages during a
     *					single operation (in milliseconds).
     *  @param msecs_idle_timeout	Timeout between operations (in
     *					milliseconds).
     *	@param writable		Should we open the DB for writing?
     *	@param verbose		Should we produce output when connections are
     *				made or lost?
     */
    TcpServer(const std::vector<std::string> &dbpaths_,
	      const std::string &host, int port,
	      int msecs_active_timeout,
	      int msecs_idle_timeout,
	      bool writable,
	      bool verbose);

    /** Destructor. */
    ~TcpServer();

    /** Accept connections and service requests indefinitely.
     *
     *  This method runs the TcpServer as a daemon which accepts a connection
     *  and forks itself (or creates a new thread under Windows) to serve the
     *  request while continuing to listen for more connections.
     */
    void run();

    /** Accept a single connection, service requests on it, then stop.  */
    void run_once();

    /** Handle a single connection on an already connected socket.
     *
     *  @a socket will be closed before returning.  This method may be called
     *  by multiple threads.
     */
    void handle_one_connection(int socket);
};

#endif  // XAPIAN_INCLUDED_TCPSERVER_H
