/** @file replicatetcpclient.h
 *  @brief TCP/IP replication client class.
 */
/* Copyright (C) 2008,2010,2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REPLICATETCPCLIENT_H
#define XAPIAN_INCLUDED_REPLICATETCPCLIENT_H

#include "remoteconnection.h"

#include "xapian/visibility.h"
#include "replication.h"

#ifdef __WIN32__
# define SOCKET_INITIALIZER_MIXIN : private WinsockInitializer
#else
# define SOCKET_INITIALIZER_MIXIN
#endif

/// TCP/IP replication client class.
class XAPIAN_VISIBILITY_DEFAULT ReplicateTcpClient SOCKET_INITIALIZER_MIXIN {
    /// Don't allow assignment.
    void operator=(const ReplicateTcpClient &);

    /// Don't allow copying.
    ReplicateTcpClient(const ReplicateTcpClient &);

    /// The socket fd.
    int socket;

    /// Write-only connection to the server.
    RemoteConnection remconn;

    /** Attempt to open a TCP/IP socket connection to a replication server.
     *
     *  Connect to replication server running on port @a port of host @a hostname.
     *  Give up trying to connect after @a timeout_connect seconds.
     *
     *  Note: this method is called early on during class construction before
     *  any member variables or even the base class have been initialised.
     *  To help avoid accidentally trying to use member variables or call other
     *  methods which do, this method has been deliberately made "static".
     */
    static int open_socket(const std::string & hostname, int port,
			   double timeout_connect);

  public:
    /** Constructor.
     *
     *  Connect to replication server running on port @a port of host @a hostname.
     *  Give up trying to connect after @a timeout_connect seconds.
     *
     *  @param timeout_connect	 Timeout for trying to connect (in seconds).
     *  @param socket_timeout	 Socket timeout (in seconds); 0 for no timeout.
     */
    ReplicateTcpClient(const std::string & hostname, int port,
		       double timeout_connect, double socket_timeout);

    void update_from_master(const std::string & path,
			    const std::string & remotedb,
			    Xapian::ReplicationInfo & info,
			    double reader_close_time,
			    bool force_copy);

    /** Destructor. */
    ~ReplicateTcpClient();
};

#endif // XAPIAN_INCLUDED_REPLICATETCPCLIENT_H
