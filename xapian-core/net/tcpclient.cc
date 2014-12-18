/* tcpclient.cc: Open a TCP connection to a server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2004,2005,2006,2007,2008,2010,2013 Olly Betts
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

#include "remoteconnection.h"
#include "tcpclient.h"
#include <xapian/error.h>

#include "safeerrno.h"
#include "safefcntl.h"
#include "safesysselect.h"
#include "socket_utils.h"

#include <cmath>
#include <cstring>
#ifndef __WIN32__
# include "safenetdb.h"
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/socket.h>
#else
# include "safewinsock2.h"
#endif

using namespace std;

int
TcpClient::open_socket(const std::string & hostname, int port,
		       double timeout_connect, bool tcp_nodelay)
{
    // FIXME: timeout on gethostbyname() ?
    struct hostent *host = gethostbyname(hostname.c_str());

    if (host == 0) {
	throw Xapian::NetworkError(std::string("Couldn't resolve host ") + hostname,
#ifdef __WIN32__
		socket_errno()
#else
		// "socket_errno()" is just errno on UNIX which is
		// inappropriate here - if gethostbyname() returns NULL an
		// error code is available in h_errno (with values
		// incompatible with errno).  On Linux at least, if h_errno
		// is < 0, then the error code *IS* in errno!
		(h_errno < 0 ? errno : -h_errno)
#endif
		);
    }

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
	throw Xapian::NetworkError("Couldn't create socket", socket_errno());
    }

    struct sockaddr_in remaddr;
    memset(&remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(port);
    memcpy(&remaddr.sin_addr, host->h_addr, host->h_length);

#ifdef __WIN32__
    ULONG enabled = 1;
    int rc = ioctlsocket(socketfd, FIONBIO, &enabled);
#define FLAG_NAME "FIONBIO"
#elif defined O_NONBLOCK
    int rc = fcntl(socketfd, F_SETFL, O_NONBLOCK);
#define FLAG_NAME "O_NONBLOCK"
#else
    int rc = fcntl(socketfd, F_SETFL, O_NDELAY);
#define FLAG_NAME "O_NDELAY"
#endif
    if (rc < 0) {
	int saved_errno = socket_errno(); // note down in case close hits an error
	close_fd_or_socket(socketfd);
	throw Xapian::NetworkError("Couldn't set "FLAG_NAME, saved_errno);
#undef FLAG_NAME
    }

    if (tcp_nodelay) {
	int optval = 1;
	// 4th argument might need to be void* or char* - cast it to char*
	// since C++ allows implicit conversion to void* but not from void*.
	if (setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
		       reinterpret_cast<char *>(&optval),
		       sizeof(optval)) < 0) {
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkError("Couldn't set TCP_NODELAY", saved_errno);
	}
    }

    int retval = connect(socketfd, reinterpret_cast<sockaddr *>(&remaddr),
			 sizeof(remaddr));

    if (retval < 0) {
#ifdef __WIN32__
	if (WSAGetLastError() != WSAEWOULDBLOCK) {
#else
	if (socket_errno() != EINPROGRESS) {
#endif
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkError("Couldn't connect (1)", saved_errno);
	}

	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(socketfd, &fdset);

	do {
	    // FIXME: Reduce the timeout if we retry on EINTR.
	    struct timeval tv;
	    tv.tv_sec = long(timeout_connect);
	    tv.tv_usec = long(std::fmod(timeout_connect, 1.0) * 1e6);

	    retval = select(socketfd + 1, 0, &fdset, &fdset, &tv);
	} while (retval < 0 && errno == EINTR);

	if (retval < 0) {
	    int saved_errno = errno;
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkError("Couldn't connect (2)", saved_errno);
	}

	if (retval <= 0) {
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkTimeoutError("Timed out waiting to connect", ETIMEDOUT);
	}

	int err = 0;
	SOCKLEN_T len = sizeof(err);

	// 4th argument might need to be void* or char* - cast it to char*
	// since C++ allows implicit conversion to void* but not from void*.
	retval = getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
			    reinterpret_cast<char *>(&err), &len);

	if (retval < 0) {
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkError("Couldn't get socket options", saved_errno);
	}
	if (err) {
	    close_fd_or_socket(socketfd);
	    throw Xapian::NetworkError("Couldn't connect (3)", err);
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
