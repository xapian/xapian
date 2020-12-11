/** @file
 *  @brief TCP/IP socket based RemoteDatabase implementation
 */
/* Copyright (C) 2007,2008,2010,2011,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REMOTETCPCLIENT_H
#define XAPIAN_INCLUDED_REMOTETCPCLIENT_H

#include "backends/remote/remote-database.h"

#ifdef __WIN32__
# define SOCKET_INITIALIZER_MIXIN private WinsockInitializer,
#else
# define SOCKET_INITIALIZER_MIXIN
#endif

/** TCP/IP socket based RemoteDatabase implementation.
 *
 *  Connects via TCP/IP to an instance of xapian-tcpsrv.
 */
class RemoteTcpClient : SOCKET_INITIALIZER_MIXIN public RemoteDatabase {
    /// Don't allow assignment.
    void operator=(const RemoteTcpClient &);

    /// Don't allow copying.
    RemoteTcpClient(const RemoteTcpClient &);

    /** Attempt to open a TCP/IP socket connection to xapian-tcpsrv.
     *
     *  Connect to xapian-tcpsrv running on port @a port of host @a hostname.
     *  Give up trying to connect after @a timeout_connect seconds.
     *
     *  Note: this method is called early on during class construction before
     *  any member variables or even the base class have been initialised.
     *  To help avoid accidentally trying to use member variables or call other
     *  methods which do, this method has been deliberately made "static".
     */
    static int open_socket(const std::string & hostname, int port,
			   double timeout_connect);

    /** Get a context string for use when constructing Xapian::NetworkError.
     *
     *  Note: this method is used from constructors so has been made static to
     *  avoid problems with trying to use uninitialised member variables.  In
     *  particular, it can't be made a virtual method of the base class.
     */
    static std::string get_tcpcontext(const std::string & hostname, int port);

  public:
    /** Constructor.
     *
     *  Attempts to open a TCP/IP connection to xapian-tcpsrv running on port
     *  @a port of host @a hostname.
     *
     *  @param timeout_connect	Timeout for trying to connect (in seconds).
     *  @param timeout		Timeout during communication after successfully
     *				connecting (in seconds).
     *	@param writable		Is this a WritableDatabase?
     *	@param flags		Xapian::DB_RETRY_LOCK or 0.
     */
    RemoteTcpClient(const std::string & hostname, int port,
		    double timeout_, double timeout_connect, bool writable,
		    int flags)
	: RemoteDatabase(open_socket(hostname, port, timeout_connect),
			 timeout_, get_tcpcontext(hostname, port),
			 writable, flags) { }

    /** Destructor. */
    ~RemoteTcpClient();
};

#endif  // XAPIAN_INCLUDED_REMOTETCPCLIENT_H
