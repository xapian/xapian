/** @file
 *  @brief Open a TCP connection to a server.
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

#ifndef XAPIAN_INCLUDED_TCPCLIENT_H
#define XAPIAN_INCLUDED_TCPCLIENT_H

#include <string>

namespace TcpClient {
    /** Attempt to open a TCP/IP socket connection to a server.
     *
     *  Connect to the server running on port @a port of host @a hostname.
     *  Give up trying to connect after @a timeout_connect seconds.
     */
    int open_socket(const std::string & hostname, int port,
		    double timeout_connect, bool tcp_nodelay);
}

#endif  // XAPIAN_INCLUDED_TCPCLIENT_H
