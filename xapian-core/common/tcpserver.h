/* tcpserver.h: class for TCP/IP-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_TCPGSERVER_H
#define OM_HGUARD_TCPGSERVER_H

#include "socketserver.h"
#include "database.h"
#include "multi_database.h"
#include "socketcommon.h"
#include <memory>

/** The base class of the network server object.
 *  A NetServer object is used by server programs to take care
 *  of a connection to a NetClient.
 */
class TcpServer : public SocketServer {
    private:
	// disallow copies
	TcpServer(const TcpServer &);
	void operator=(const TcpServer &);

	/** Open the listening socket and return a filedescriptor to
	 *  it.
	 *  
	 *  @param port		The local port to bind to.
	 */
	static int get_listening_socket(int port);
    public:
	/** Default constructor. */
	TcpServer(OmRefCntPtr<MultiDatabase> db, int port);

	/** Destructor. */
	~TcpServer();
};

#endif  /* OM_HGUARD_TCPGSERVER_H */
