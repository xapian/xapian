/* tcpserver.cc: class for TCP/IP-based server.
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

#include "config.h"
#include "tcpserver.h"
#include "database.h"
#include "stats.h"
#include "localmatch.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <memory>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <netdb.h>

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(OmRefCntPtr<MultiDatabase> db_,
		       int port_)
	: port(port_), db(db_), listen_socket(get_listening_socket(port_))
{

}

int
TcpServer::get_listening_socket(int port)
{
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw OmNetworkError(std::string("socket: ") + strerror(errno));
    }

    int retval;

    {
	int optval = 1;
	retval = setsockopt(socketfd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    (void *)&optval,
			    sizeof(optval));
    }

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(std::string("setsockopt: ") + strerror(errno));
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
	throw OmNetworkError(std::string("bind: ") + strerror(errno));
    }

    // FIXME: backlog argument should perhaps be larger.
    retval = listen(socketfd, 1);

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(std::string("listen: ") + strerror(errno));
    }
    return socketfd;
}

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif  /* HAVE_SOCKLEN_T */

int
TcpServer::get_connected_socket()
{
    struct sockaddr_in remote_address;
    socklen_t remote_address_size = sizeof(remote_address);
    int con_socket = accept(listen_socket,
			    reinterpret_cast<sockaddr *>(&remote_address),
			    &remote_address_size);

    if (con_socket < 0) {
	throw OmNetworkError(std::string("accept: ") + strerror(errno));
    }

    if (remote_address_size != sizeof(remote_address)) {
	throw OmNetworkError("accept: unexpected remote address size");
    }

    // Note: this bit is probably not threadsafe.
    struct in_addr address = remote_address.sin_addr;
    struct hostent *hent = gethostbyaddr(reinterpret_cast<char *>(&address),
					 sizeof(address),
					 AF_INET);

    if (hent == 0) {
	close(con_socket);
	throw OmNetworkError(std::string("gethostbyaddr: ") +
			     hstrerror(h_errno));
    }

    cout << "Connection from " << hent->h_name << ", port " <<
	    remote_address.sin_port << "\n";

    return con_socket;
}

TcpServer::~TcpServer()
{
    close(listen_socket);
}

void
TcpServer::run_once()
{
    SocketServer sserv(db, get_connected_socket());

    sserv.run();
}

void
TcpServer::run()
{
    while (1) {
	try {
	    run_once();
	} catch (...) {
	    // FIXME: better error handling.
	    std::cerr << "Caught exception." << "\n";
	}
    }
}
