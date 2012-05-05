/** @file replicatetcpserver.cc
 * @brief TCP/IP replication server class.
 */
/* Copyright (C) 2008,2010,2011 Olly Betts
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

#include "replicatetcpserver.h"

#include <xapian/error.h>
#include "api/replication.h"

using namespace std;

ReplicateTcpServer::ReplicateTcpServer(const string & host, int port,
				       const string & path_)
    : TcpServer(host, port, false, false), path(path_)
{
}

ReplicateTcpServer::~ReplicateTcpServer() {
}

void
ReplicateTcpServer::handle_one_connection(int socket)
{
    RemoteConnection client(socket, -1);
    try {
	// Read start_revision from the client.
	string start_revision;
	if (client.get_message(start_revision, 0.0) != 'R') {
	    throw Xapian::NetworkError("Bad replication client message");
	}

	// Read dbname from the client.
	string dbname;
	if (client.get_message(dbname, 0.0) != 'D') {
	    throw Xapian::NetworkError("Bad replication client message (2)");
	}
	if (dbname.find("..") != string::npos) {
	    throw Xapian::NetworkError("dbname contained '..'");
	}

	string dbpath(path);
	dbpath += '/';
	dbpath += dbname;
	Xapian::DatabaseMaster master(dbpath);
	master.write_changesets_to_fd(socket, start_revision, NULL);
    } catch (...) {
	// Ignore exceptions.
    }
}
