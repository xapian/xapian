/* tcpclient.cc: Open a TCP connection to a server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2004,2005,2006,2007,2008,2010,2012,2013,2015,2017 Olly Betts
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

#include "tcpclient.h"

#include "remoteconnection.h"
#include "resolver.h"
#include "str.h"
#include <xapian/error.h>

#include "realtime.h"
#include "safeerrno.h"
#include "safefcntl.h"
#include "safenetdb.h"
#include "safesysselect.h"
#include "safesyssocket.h"
#include "socket_utils.h"

#include <cmath>
#include <cstring>
#ifndef __WIN32__
# include <netinet/in.h>
# include <netinet/tcp.h>
#endif

using namespace std;

int
TcpClient::open_socket(const std::string & hostname, int port,
		       double timeout_connect, bool tcp_nodelay)
{
    int socketfd = -1;
    int connect_errno = 0;
    for (auto&& r : Resolver(hostname, port)) {
	int socktype = r.ai_socktype | SOCK_CLOEXEC;
	int fd = socket(r.ai_family, socktype, r.ai_protocol);
	if (fd == -1)
	    continue;

#if !defined __WIN32__ && defined F_SETFD && defined FD_CLOEXEC
	// We can't use a preprocessor check on the *value* of SOCK_CLOEXEC as
	// on Linux SOCK_CLOEXEC is an enum, with '#define SOCK_CLOEXEC
	// SOCK_CLOEXEC' to allow '#ifdef SOCK_CLOEXEC' to work.
	if (SOCK_CLOEXEC == 0)
	    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

#ifdef __WIN32__
	ULONG enabled = 1;
	int rc = ioctlsocket(fd, FIONBIO, &enabled);
#define FLAG_NAME "FIONBIO"
#elif defined O_NONBLOCK
	int rc = fcntl(fd, F_SETFL, O_NONBLOCK);
#define FLAG_NAME "O_NONBLOCK"
#else
	int rc = fcntl(fd, F_SETFL, O_NDELAY);
#define FLAG_NAME "O_NDELAY"
#endif
	if (rc < 0) {
	    int saved_errno = socket_errno(); // note down in case close hits an error
	    close_fd_or_socket(fd);
	    throw Xapian::NetworkError("Couldn't set " FLAG_NAME, saved_errno);
#undef FLAG_NAME
	}

	if (tcp_nodelay) {
	    int optval = 1;
	    // 4th argument might need to be void* or char* - cast it to char*
	    // since C++ allows implicit conversion to void* but not from
	    // void*.
	    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			   reinterpret_cast<char *>(&optval),
			   sizeof(optval)) < 0) {
		int saved_errno = socket_errno(); // note down in case close hits an error
		close_fd_or_socket(fd);
		throw Xapian::NetworkError("Couldn't set TCP_NODELAY", saved_errno);
	    }
	}

	int retval = connect(fd, r.ai_addr, r.ai_addrlen);
	if (retval == 0) {
	    socketfd = fd;
	    break;
	}

	int err = socket_errno();
	if (
#ifdef __WIN32__
	    WSAGetLastError() == WSAEWOULDBLOCK
#else
	    err == EINPROGRESS
#endif
	    ) {
	    // Wait for the socket to be writable, with a timeout.
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    FD_SET(fd, &fdset);

	    do {
		// FIXME: Reduce the timeout if we retry on EINTR.
		struct timeval tv;
		RealTime::to_timeval(timeout_connect, &tv);
		retval = select(fd + 1, 0, &fdset, &fdset, &tv);
	    } while (retval < 0 && errno == EINTR);

	    if (retval <= 0) {
		int saved_errno = errno;
		close_fd_or_socket(fd);
		if (retval < 0)
		    throw Xapian::NetworkError("Couldn't connect (select() on socket failed)",
					       saved_errno);
		throw Xapian::NetworkTimeoutError("Timed out waiting to connect", ETIMEDOUT);
	    }

	    err = 0;
	    SOCKLEN_T len = sizeof(err);

	    // 4th argument might need to be void* or char* - cast it to char*
	    // since C++ allows implicit conversion to void* but not from void*.
	    retval = getsockopt(fd, SOL_SOCKET, SO_ERROR,
				reinterpret_cast<char *>(&err), &len);

	    if (retval < 0) {
		int saved_errno = socket_errno(); // note down in case close hits an error
		close_fd_or_socket(fd);
		throw Xapian::NetworkError("Couldn't get socket options", saved_errno);
	    }
	    if (err == 0) {
		// Connected successfully.
		socketfd = fd;
		break;
	    }
	}

	// Note down the error code for the first address we try, which seems
	// likely to be more helpful than the last in the case where they
	// differ.
	if (connect_errno == 0)
	    connect_errno = err;

	// Failed to connect.
	CLOSESOCKET(fd);
    }

    if (socketfd == -1) {
	throw Xapian::NetworkError("Couldn't connect", connect_errno);
    }

#ifdef __WIN32__
    ULONG enabled = 0;
    ioctlsocket(socketfd, FIONBIO, &enabled);
#else
    fcntl(socketfd, F_SETFL, 0);
#endif
    return socketfd;
}
