/* tcpserver.cc: class for TCP/IP-based server.
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

#include "tcpserver.h"
#include "database.h"
#include "stats.h"
#include "localmatch.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"
#include <unistd.h>
#include <memory>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <netdb.h>

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(OmRefCntPtr<MultiDatabase> db_,
		       int port)
	: SocketServer(db_, get_listening_socket(port))
{

}

int
TcpServer::get_listening_socket(int port)
{
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw OmNetworkError(string("socket: ") + strerror(errno));
    }

    int retval;

    {
	int optval = 1;
	retval = setsockopt(socketfd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    &optval,
			    sizeof(optval));
    }

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(string("setsockopt: ") + strerror(errno));
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    retval = bind(socketfd,
		      reinterpret_cast<sockaddr *>(&addr),
		      sizeof(addr));

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(string("bind: ") + strerror(errno));
    }

    // FIXME: backlog argument should perhaps be larger.
    retval = listen(socketfd, 1);

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(string("listen: ") + strerror(errno));
    }

    struct sockaddr_in remote_address;
    socklen_t remote_address_size = sizeof(remote_address);
    int con_socket = accept(socketfd,
			    reinterpret_cast<sockaddr *>(&remote_address),
			    &remote_address_size);

    if (con_socket < 0) {
	close(socketfd);
	throw OmNetworkError(string("accept: ") + strerror(errno));
    }

    if (remote_address_size != sizeof(remote_address)) {
	close(socketfd);
	throw OmNetworkError("accept: unexpected remote address size");
    }

    // FIXME: get this bit working, probably
#if 0
    struct hostent *hent =
	    gethostbyaddr(reinterpret_cast<char *>(&remote_address),
			  sizeof(remote_address),
			  AF_INET);

    if (hent == 0) {
	close(socketfd);
	close(con_socket);
	throw OmNetworkError(string("gethostbyaddr: ") +
			     hstrerror(h_errno));
    }

    cerr << "Connection from " << hent->h_name
	 << ", port " << remote_address.sin_port << endl;

#endif

    // FIXME: use a destructor for closing socketfd?
    close(socketfd);

    return con_socket;
}

TcpServer::~TcpServer()
{
}
