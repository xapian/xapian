/* tcpclient.cc: implementation of NetClient which connects to a remote server.
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
#include "tcpclient.h"
#include "om/omerror.h"

#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

TcpClient::TcpClient(std::string hostname, int port, int msecs_timeout)
	: SocketClient(get_remote_socket(hostname, port), true, msecs_timeout)
{

}

int
TcpClient::get_remote_socket(std::string hostname, int port)
{
    struct hostent *host = gethostbyname(hostname.c_str());

    if (host == 0) {
	throw OmNetworkError(std::string("Couldn't resolve host ") +
			     hostname);
    }

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw OmNetworkError(std::string("socket: ") + strerror(errno));
    }

    struct sockaddr_in remaddr;
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(port);
    memcpy(&remaddr.sin_addr, host->h_addr, sizeof(remaddr.sin_addr));

    int retval = connect(socketfd,
			 reinterpret_cast<sockaddr *>(&remaddr),
			 sizeof(remaddr));

    if (retval < 0) {
	close(socketfd);
	throw OmNetworkError(std::string("connect: ") + strerror(errno));
    }

    return socketfd;
}

TcpClient::~TcpClient()
{
}
