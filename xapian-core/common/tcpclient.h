/* tcpclient.h: implementation of NetClient which opens a TCP/IP socket
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_TCPCLIENT_H
#define OM_HGUARD_TCPCLIENT_H

#include "socketclient.h"
#include "socketcommon.h"

/** An implementation of the NetClient interface using a program.
 *  TcpClient opens a TCP/IP socket to a remote machine.
 */
class TcpClient : public SocketClient {
    private:
	// disallow copies
	TcpClient(const TcpClient &);
	void operator=(const TcpClient &);

	/** Spawn a program and return a filedescriptor of
	 *  the local end of a socket to it.
	 */
	int get_remote_socket(std::string hostname, int port);

    public:
	/** Constructor.
	 *
	 *  @param hostname The name of the remote host
	 *  @param port	    The TCP port to connect to.
	 */
	TcpClient(std::string hostname, int port);

	/** Destructor. */
	~TcpClient();
};

#endif  /* OM_HGUARD_TCPCLIENT_H */
