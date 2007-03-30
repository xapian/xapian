/* tcpclient.cc: implementation of NetClient which connects to a remote server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2004,2005,2006 Olly Betts
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

#include "safeerrno.h"
#include "safefcntl.h"

#include "tcpclient.h"
#include <xapian/error.h>

#include <string.h>
#ifdef __WIN32__
# define SOCKOPT_OPTIONS_TYPE char *
#else
# define SOCKOPT_OPTIONS_TYPE void *
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# else
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>
# endif
# include <string.h> // Solaris needs this as FDSET uses memset but fails to prototype it.
#endif


#include "utils.h"

TcpClient::TcpClient(std::string hostname, int port, int msecs_timeout_, int msecs_timeout_connect_)
	: RemoteDatabase(get_remote_socket(hostname, port, msecs_timeout_connect_),
			 msecs_timeout_, get_tcpcontext(hostname, port))
{

}

std::string
TcpClient::get_tcpcontext(std::string hostname, int port)
{
    return "remote:tcp(" + hostname + ":" + om_tostring(port) + ")";
}

int
TcpClient::get_remote_socket(std::string hostname,
			     int port,
			     int msecs_timeout_connect_)
{
    // Note: can't use RemoteDatabase::timeout because it won't yet have
    // been initialised.

    // FIXME: timeout on gethostbyname() ?
    struct hostent *host = gethostbyname(hostname.c_str());

    if (host == 0) {
	throw Xapian::NetworkError(std::string("Couldn't resolve host ") +
			     hostname, get_tcpcontext(hostname, port), socket_errno());
    }

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw Xapian::NetworkError("Couldn't create socket", get_tcpcontext(hostname, port), socket_errno());
    }

    struct sockaddr_in remaddr;
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(port);
    memcpy(&remaddr.sin_addr, host->h_addr, sizeof(remaddr.sin_addr));

#ifdef __WIN32__
    ULONG enabled = 1;
    int rc = ioctlsocket(socketfd, FIONBIO, &enabled);
#else
    int rc = fcntl(socketfd, F_SETFL, O_NDELAY);
#endif
    if (rc < 0) {
	int saved_errno = socket_errno(); // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("Couldn't set O_NDELAY", get_tcpcontext(hostname,  port), saved_errno);
    }

    {
	int optval = 1;
	if (setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
		       reinterpret_cast<SOCKOPT_OPTIONS_TYPE>(&optval),
		       sizeof(optval)) < 0) {
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close(socketfd);
	    throw Xapian::NetworkError("Couldn't set TCP_NODELAY", get_tcpcontext(hostname,  port), saved_errno);
	}
    }

    int retval = connect(socketfd, reinterpret_cast<sockaddr *>(&remaddr),
			 sizeof(remaddr));

    if (retval < 0) {
#ifdef __WIN32__
	if (socket_errno() != WSAEWOULDBLOCK) {
#else
	if (socket_errno() != EINPROGRESS) {
#endif
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close(socketfd);
	    throw Xapian::NetworkError("Couldn't connect", get_tcpcontext(hostname, port), saved_errno);
	}

	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(socketfd, &fdset);

	struct timeval tv;
	tv.tv_sec = msecs_timeout_connect_ / 1000;
	tv.tv_usec = msecs_timeout_connect_ % 1000 * 1000;

	retval = select(socketfd + 1, 0, &fdset, &fdset, &tv);

	if (retval == 0) {
	    close(socketfd);
	    throw Xapian::NetworkTimeoutError("Couldn't connect", get_tcpcontext(hostname, port), ETIMEDOUT);
	}

	int err = 0;
	SOCKLEN_T len = sizeof(err);

	/* On Solaris 5.6, the fourth argument is char *. */
	retval = getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
			    reinterpret_cast<SOCKOPT_OPTIONS_TYPE>(&err), &len);

	if (retval < 0) {
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close(socketfd);
	    throw Xapian::NetworkError("Couldn't get socket options", get_tcpcontext(hostname, port), saved_errno);
	}
	if (err) {
	    close(socketfd);
	    throw Xapian::NetworkError("Couldn't connect", get_tcpcontext(hostname, port), err);
	}
    }

#ifdef __WIN32__
    enabled = 0;
    ioctlsocket(socketfd, FIONBIO, &enabled);
#else
    fcntl(socketfd, F_SETFL, 0);
#endif
    return socketfd;
}

TcpClient::~TcpClient()
{
    do_close();
}
