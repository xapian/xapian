/** @file
 * @brief class for TCP/IP-based remote server.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2010,2015 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "remotetcpserver.h"

#include <xapian/error.h>

#include "remoteserver.h"

#include <iostream>

using namespace std;

/// The RemoteTcpServer constructor, taking a database and a listening port.
RemoteTcpServer::RemoteTcpServer(const vector<std::string> &dbpaths_,
				 const std::string & host, int port,
				 double active_timeout_, double idle_timeout_,
				 bool writable_, bool verbose_)
    : TcpServer(host, port, true, verbose_),
      dbpaths(dbpaths_), writable(writable_),
      active_timeout(active_timeout_), idle_timeout(idle_timeout_)
{
}

void
RemoteTcpServer::handle_one_connection(int socket)
{
    try {
	RemoteServer sserv(dbpaths, socket, socket,
			   active_timeout, idle_timeout, writable);
	sserv.set_registry(reg);
	sserv.run();
    } catch (const Xapian::NetworkTimeoutError &e) {
	if (verbose)
	    cerr << "Connection timed out: " << e.get_description() << '\n';
    } catch (const Xapian::Error &e) {
	cerr << "Got exception " << e.get_description() << '\n';
    } catch (...) {
	// ignore other exceptions
    }
}
