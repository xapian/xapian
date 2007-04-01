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

#ifdef __WIN32__
# include "remoteconnection.h"
# define SOCKET_INITIALIZER_MIXIN : private WinsockInitializer
#else
# define SOCKET_INITIALIZER_MIXIN
#endif

/** TCP/IP socket based server for RemoteDatabase.
 *
 *  This class implements the server used by xapian-tcpsrv.
 */
class TcpServer SOCKET_INITIALIZER_MIXIN {
    /// Don't allow assignment.
    void operator=(const TcpServer &);

    /// Don't allow copying.
    TcpServer(const TcpServer &);

    /** Is this a WritableDatabase?
     *
     *  If true, the wdb member is used.  If false, the db member is.
     */
    bool writable;

    /** If writable is false, this is the database we're using. */
    Xapian::Database db;
    
    /** If writable is true, this is the database we're using. */
    Xapian::WritableDatabase wdb;

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
    static int get_listening_socket(const std::string & host, int port);

    /** Accept a connection and return the filedescriptor for it. */
    int accept_connection();

  public:
    /** Construct a TcpServer for a Database and start listening for
     *  connections.
     *
     *  @param db_	The Database to provide remote access to.
     *  @param host	The hostname or address for the interface to listen on
     *			(or "" to listen on all interfaces).
     *  @port		The TCP port number to listen on.
     *  @param msecs_active_timeout	Timeout between messages during a
     *					single operation (in milliseconds)
     *					(default 10000).
     *  @param msecs_idle_timeout	Timeout between operations (in
     *					milliseconds) (default 60000).
     *	@param verbose_		Should we produce output when connections are
     *				made or lost? (default true).
     */
    TcpServer(Xapian::Database db_, const std::string &host, int port,
	      int msecs_normal_timeout_ = 10000,
	      int msecs_idle_timeout_ = 60000,
	      bool verbose_ = true);

    /** Construct a TcpServer for a WritableDatabase and start listening for
     *  connections.
     *
     *  @param db_	The WritableDatabase to provide remote access to.
     *  @param host	The hostname or address for the interface to listen on
     *			(or "" to listen on all interfaces).
     *  @port		The TCP port number to listen on.
     *  @param msecs_active_timeout	Timeout between messages during a
     *					single operation (in milliseconds)
     *					(default 10000).
     *  @param msecs_idle_timeout	Timeout between operations (in
     *					milliseconds) (default 60000).
     *	@param verbose_		Should we produce output when connections are
     *				made or lost? (default true).
     */
    TcpServer(Xapian::WritableDatabase db_, const std::string &host, int port,
	      int msecs_normal_timeout_ = 10000,
	      int msecs_idle_timeout_ = 60000,
	      bool verbose_ = true);

    /** Destructor. */
    ~TcpServer();

    /** Accept connections and service requests indefinitely.
     *
     *  This method runs the TcpServer as a daemon which accepts a connection
     *  and forks itself to server the request while continuing to listen for
     *  more connections.
     */
    void run();

    /** Handle a single connection, service the request, then stop. */
    void run_once();

#ifdef __WIN32__
    /** Handle a single request on an already connected socket.
     *
     *  May be called by multiple threads.
     */
    void handle_one_request(int socket);
#endif
};

#endif  // XAPIAN_INCLUDED_TCPSERVER_H
