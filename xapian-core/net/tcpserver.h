/** @file
 *  @brief Generic TCP/IP socket based server base class.
 */
/* Copyright (C) 2007,2008 Olly Betts
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

#ifdef __WIN32__
# include "remoteconnection.h"
# define SOCKET_INITIALIZER_MIXIN : private WinsockInitializer
#else
# define SOCKET_INITIALIZER_MIXIN
#endif

#if defined __CYGWIN__ || defined __WIN32__
# include "safewindows.h" // Only for HANDLE!
#endif

#include <xapian/visibility.h>

#include <string>

/** TCP/IP socket based server for RemoteDatabase.
 *
 *  This class implements the server used by xapian-tcpsrv.
 */
class XAPIAN_VISIBILITY_DEFAULT TcpServer SOCKET_INITIALIZER_MIXIN {
    /// Don't allow assignment.
    void operator=(const TcpServer &);

    /// Don't allow copying.
    TcpServer(const TcpServer &);

#if defined __CYGWIN__ || defined __WIN32__
    /// Mutex to stop two TcpServers running on the same port.
    HANDLE mutex = NULL;
#endif

    /** The socket we're listening on. */
    int listen_socket;

    /** Create a listening socket ready to accept connections.
     *
     *  @param host	hostname or address to listen on or an empty string to
     *			accept connections on any interface.
     *  @param port	TCP port to listen on.
     *  @param tcp_nodelay	If true, enable TCP_NODELAY option.
     */
    static int get_listening_socket(const std::string & host, int port,
				    bool tcp_nodelay
#if defined __CYGWIN__ || defined __WIN32__
				    , HANDLE &mutex
#endif
	    );

  protected:
    /** Should we produce output when connections are made or lost? */
    bool verbose;

    /** Accept a connection and return the file descriptor for it. */
    int accept_connection();

  public:
    /** Construct a TcpServer and start listening for connections.
     *
     *  @param host	The hostname or address for the interface to listen on
     *			(or "" to listen on all interfaces).
     *  @param port	The TCP port number to listen on.
     *  @param tcp_nodelay	If true, enable TCP_NODELAY option.
     *	@param verbose	Should we produce output when connections are
     *			made or lost?
     */
    TcpServer(const std::string &host, int port, bool tcp_nodelay,
	      bool verbose);

    /** Destructor. */
    virtual ~TcpServer();

    /** Accept connections and service requests indefinitely.
     *
     *  This method runs the TcpServer as a daemon which accepts a connection
     *  and forks itself (or creates a new thread under Windows) to serve the
     *  request while continuing to listen for more connections.
     */
    void run();

    /** Accept a single connection, service requests on it, then stop.  */
    void run_once();

    /// Handle a single connection on an already connected socket.
    virtual void handle_one_connection(int socket) = 0;
};

#endif  // XAPIAN_INCLUDED_TCPSERVER_H
