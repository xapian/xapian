/** @file
 *  @brief TCP/IP socket based RemoteDatabase implementation
 */
/* Copyright (C) 2008,2010,2024 Olly Betts
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

pair<int, string>
RemoteTcpClient::open_socket(string_view hostname, int port,
			     double timeout_connect)
{
    // Build a context string for use when constructing Xapian::NetworkError.
    string context{"remote:tcp("};
    context += hostname;
    context += ':';
    context += str(port);
    context += ')';
    return {TcpClient::open_socket(hostname, port, timeout_connect, true,
				   context),
	    context};
}

RemoteTcpClient::~RemoteTcpClient()
{
    try {
	do_close();
    } catch (...) {
    }
}
