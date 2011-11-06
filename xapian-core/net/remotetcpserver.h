/** @file remotetcpserver.h
 *  @brief TCP/IP socket based server for RemoteDatabase.
 */
/* Copyright (C) 2007,2008,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REMOTETCPSERVER_H
#define XAPIAN_INCLUDED_REMOTETCPSERVER_H

#include "tcpserver.h"

#include <xapian/database.h>
#include <xapian/visibility.h>

#include <string>
#include <vector>

/** TCP/IP socket based server for RemoteDatabase.
 *
 *  This class implements the server used by xapian-tcpsrv.
 */
class XAPIAN_VISIBILITY_DEFAULT RemoteTcpServer : public TcpServer {
    /// Don't allow assignment.
    void operator=(const RemoteTcpServer &);

    /// Don't allow copying.
    RemoteTcpServer(const RemoteTcpServer &);

    /** Paths to the databases we will open.
     *
     *  Contains exactly one entry if writable, and at least one if not.
     */
    const std::vector<std::string> dbpaths;

    /** Is this a WritableDatabase? */
    bool writable;

    /** Timeout between messages during a single operation (in seconds). */
    double active_timeout;

    /** Timeout between operations (in seconds). */
    double idle_timeout;

    /** Accept a connection and return the filedescriptor for it. */
    int accept_connection();

  public:
    /** Construct a RemoteTcpServer for a Database and start listening for
     *  connections.
     *
     *  @param dbpaths_	The path(s) to the database(s) we should open.
     *  @param host	The hostname or address for the interface to listen on
     *			(or "" to listen on all interfaces).
     *  @param port	The TCP port number to listen on.
     *  @param active_timeout	Timeout between messages during a single
     *				operation (in seconds).
     *  @param idle_timeout	Timeout between operations (in seconds).
     *	@param writable		Should we open the DB for writing?
     *	@param verbose		Should we produce output when connections are
     *				made or lost?
     */
    RemoteTcpServer(const std::vector<std::string> &dbpaths_,
		    const std::string &host, int port,
		    double active_timeout, double idle_timeout,
		    bool writable, bool verbose);

    /** Handle a single connection on an already connected socket.
     *
     *  This method may be called by multiple threads.
     */
    void handle_one_connection(int socket);
};

#endif // XAPIAN_INCLUDED_REMOTETCPSERVER_H
