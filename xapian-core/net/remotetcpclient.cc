/** @file remotetcpclient.cc
 *  @brief TCP/IP socket based RemoteDatabase implementation
 */
/* Copyright (C) 2008,2010 Olly Betts
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

#include <config.h>

#include "remotetcpclient.h"

#include <xapian/error.h>

#include "str.h"
#include "tcpclient.h"

using namespace std;

int
RemoteTcpClient::open_socket(const string & hostname, int port,
			     double timeout_connect)
{
    // If TcpClient::open_socket() throws, fill in the context.
    try {
	return TcpClient::open_socket(hostname, port, timeout_connect, true);
    } catch (const Xapian::NetworkTimeoutError & e) {
	throw Xapian::NetworkTimeoutError(e.get_msg(), get_tcpcontext(hostname, port),
					  e.get_error_string());
    } catch (const Xapian::NetworkError & e) {
	throw Xapian::NetworkError(e.get_msg(), get_tcpcontext(hostname, port),
				   e.get_error_string());
    }
}

string
RemoteTcpClient::get_tcpcontext(const string & hostname, int port)
{
    string result("remote:tcp(");
    result += hostname;
    result += ':';
    result += str(port);
    result += ')';
    return result;
}

RemoteTcpClient::~RemoteTcpClient()
{
    do_close();
}
