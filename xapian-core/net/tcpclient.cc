/** @file
 *  @brief Open a TCP connection to a server.
 */
/* Copyright 2007-2023 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>

#include "tcpclient.h"

#include <xapian/error.h>

#include "realtime.h"
#include "resolver.h"
#include "socket_utils.h"

#include "safefcntl.h"
#include "safesyssocket.h"

#ifdef HAVE_POLL_H
# include <poll.h>
#else
# include "safesysselect.h"
#endif

#include <cerrno>
#ifndef __WIN32__
# include <netinet/in.h>
# include <netinet/tcp.h>
#endif

using namespace std;

int
TcpClient::open_socket(const string& hostname, int port,
		       double timeout_connect, bool tcp_nodelay,
		       const string& context)
{
    int socketfd = -1;
    int connect_errno = 0;
    for (auto&& r : Resolver(hostname, port)) {
	int socktype = r.ai_socktype | SOCK_CLOEXEC;
#ifdef SOCK_NONBLOCK
	// Set the socket as non-blocking so we can implement a timeout on
	// the connection attempt by using select() or poll().  If
	// SOCK_NONBLOCK is available we can get socket() to do this, but
	// if not we call fcntl()/ioctlsocket() below to set it.
	socktype |= SOCK_NONBLOCK;
#endif
#ifdef SOCK_NOSIGPIPE
	// This seems to be needed on NetBSD to avoid testcase keepalive1
	// causing apitest to exit.  Not seen this on any other platform.
	socktype |= SOCK_NOSIGPIPE;
#endif
	int fd = socket(r.ai_family, socktype, r.ai_protocol);
	if (fd < 0)
	    continue;

#if !defined __WIN32__ && defined F_SETFD && defined FD_CLOEXEC
	// We can't use a preprocessor check on the *value* of SOCK_CLOEXEC as
	// on Linux SOCK_CLOEXEC is an enum, with '#define SOCK_CLOEXEC
	// SOCK_CLOEXEC' to allow '#ifdef SOCK_CLOEXEC' to work.
	if (SOCK_CLOEXEC == 0)
	    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

#ifndef SOCK_NONBLOCK
#ifdef __WIN32__
	int rc = [&]() {
	    ULONG on = 1;
	    return ioctlsocket(fd, FIONBIO, &on);
	}();
#define FLAG_NAME "FIONBIO"
#elif defined O_NONBLOCK
	int rc = fcntl(fd, F_SETFL, O_NONBLOCK);
#define FLAG_NAME "O_NONBLOCK"
#else
	int rc = fcntl(fd, F_SETFL, O_NDELAY);
#define FLAG_NAME "O_NDELAY"
#endif
	if (rc < 0) {
	    int saved_errno = socket_errno();
	    CLOSESOCKET(fd);
	    throw Xapian::NetworkError("Couldn't set " FLAG_NAME,
				       context,
				       saved_errno);
#undef FLAG_NAME
	}
#endif

	if (tcp_nodelay) {
	    int on = 1;
	    // 4th argument might need to be void* or char* - cast it to char*
	    // since C++ allows implicit conversion to void* but not from
	    // void*.
	    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			   reinterpret_cast<char*>(&on),
			   sizeof(on)) < 0) {
		int setsockopt_errno = socket_errno();
		CLOSESOCKET(fd);
		throw Xapian::NetworkError("Couldn't set TCP_NODELAY",
					   context,
					   setsockopt_errno);
	    }
	}

	int retval = connect(fd, r.ai_addr, r.ai_addrlen);
	if (retval == 0) {
	    socketfd = fd;
	    break;
	}

	int err = socket_errno();
	if (
#ifndef __WIN32__
	    err == EINPROGRESS
#else
	    err == WSAEWOULDBLOCK
#endif
	   ) {
	    // Wait for the socket to be writable or give an error, with a
	    // timeout.  FIXME: Reduce the timeout if we retry.
#ifdef HAVE_POLL
	    struct pollfd fds;
	    fds.fd = fd;
	    fds.events = POLLOUT;
	    do {
		retval = poll(&fds, 1, int(timeout_connect * 1000));
	    } while (retval < 0 && (errno == EINTR || errno == EAGAIN));
# define FUNC_NAME "poll()"
#else
	    fd_set fdset;
	    FD_ZERO(&fdset);
	    do {
		FD_SET(fd, &fdset);
		struct timeval tv;
		RealTime::to_timeval(timeout_connect, &tv);
		retval = select(fd + 1, 0, &fdset, 0, &tv);
	    } while (retval < 0 && (errno == EINTR || errno == EAGAIN));
# define FUNC_NAME "select()"
#endif

	    if (retval <= 0) {
		int saved_errno = errno;
		CLOSESOCKET(fd);
		if (retval < 0) {
		    throw Xapian::NetworkError("Couldn't connect ("
					       FUNC_NAME " on socket failed)",
					       context,
					       saved_errno);
#undef FUNC_NAME
		}
		throw Xapian::NetworkTimeoutError("Timed out waiting to "
						  "connect",
						  context,
						  ETIMEDOUT);
	    }

	    err = 0;
	    SOCKLEN_T len = sizeof(err);

	    // 4th argument might need to be void* or char* - cast it to char*
	    // since C++ allows implicit conversion to void* but not from void*.
	    retval = getsockopt(fd, SOL_SOCKET, SO_ERROR,
				reinterpret_cast<char*>(&err), &len);

	    if (retval < 0) {
		int getsockopt_errno = socket_errno();
		CLOSESOCKET(fd);
		throw Xapian::NetworkError("getsockopt failed",
					   context,
					   getsockopt_errno);
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

    if (socketfd < 0) {
	throw Xapian::NetworkError("connect failed",
				   context,
				   connect_errno);
    }

    // Set the socket to be blocking.
#ifndef __WIN32__
    fcntl(socketfd, F_SETFL, 0);
#else
    ULONG off = 0;
    ioctlsocket(socketfd, FIONBIO, &off);
#endif

    return socketfd;
}
