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
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

TcpClient::TcpClient(std::string hostname, int port, int msecs_timeout_)
	: SocketClient(get_remote_socket(hostname, port, msecs_timeout_),
		       msecs_timeout_,
		       true)
{

}

int
TcpClient::get_remote_socket(std::string hostname,
			     int port,
			     int msecs_timeout_)
{
    // Note: can't use the msecs_timeout member in SocketClient because this
    // hasn't yet been initialised.

    // FIXME: timeout on gethostbyname() ?
    struct hostent *host = gethostbyname(hostname.c_str());

    if (host == 0) {
	throw OmNetworkError(std::string("Couldn't resolve host ") +
			     hostname);
    }

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw OmNetworkError(std::string("Couldn't make socket: ") + strerror(errno));
    }

    struct sockaddr_in remaddr;
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(port);
    memcpy(&remaddr.sin_addr, host->h_addr, sizeof(remaddr.sin_addr));

    fcntl(socketfd, F_SETFL, O_NDELAY);

    int retval = connect(socketfd, reinterpret_cast<sockaddr *>(&remaddr),
			 sizeof(remaddr));

    if (retval < 0) {
	if (errno != EINPROGRESS) {
	    close(socketfd);
	    throw OmNetworkError(std::string("Can't connect socket: ") + strerror(errno));
	}

	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(socketfd, &fdset);

	struct timeval tv;
	tv.tv_sec = msecs_timeout_ / 1000;
	tv.tv_usec = msecs_timeout_ % 1000 * 1000;

	retval = select(socketfd + 1, 0, &fdset, 0, &tv);
	
	if (retval == 0) {
	    close(socketfd);
	    throw OmNetworkTimeoutError("Can't connect socket: timed out");
	}

	int err = 0;
	socklen_t len = sizeof(err);
	retval = getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &err, &len);
	
	if (retval < 0) {
	    err = errno;
	    close(socketfd);
	    throw OmNetworkError(std::string("Couldn't get socket options: ") + strerror(err));
	}
	if (err) {
	    close(socketfd);
	    throw OmNetworkError(std::string("Connect failed: ") + strerror(err));
	}
    }

    fcntl(socketfd, F_SETFL, 0);

    return socketfd;
}

TcpClient::~TcpClient()
{
}
