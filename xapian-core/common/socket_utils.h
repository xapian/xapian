/** @file
 *  @brief Socket handling utilities.
 */
/* Copyright (C) 2006-2023 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SOCKET_UTILS_H
#define XAPIAN_INCLUDED_SOCKET_UTILS_H

#include "safenetdb.h"
#include "safeunistd.h"

#include <cerrno>

#ifdef __WIN32__

# include "safewinsock2.h"

# include <xapian/error.h>

/// Convert an fd (which might be a socket) to a WIN32 HANDLE.
extern HANDLE fd_to_handle(int fd);

/// Close an fd, which might be a socket.
extern void close_fd_or_socket(int fd);

/** Class to initialise winsock and keep it initialised while we use it.
 *
 *  We need to get WinSock initialised before we use it, and make it clean up
 *  after we've finished using it.  This class performs this initialisation when
 *  constructed and cleans up when destructed.  Multiple instances of the class
 *  may be instantiated - windows keeps a count of the number of times that
 *  WSAStartup has been successfully called and only performs the actual cleanup
 *  when WSACleanup has been called the same number of times.
 *
 *  Simply ensure that an instance of this class is initialised whenever we're
 *  doing socket handling.  This class can be used as a mixin class (just
 *  inherit from it) or instantiated as a class member or local variable).
 */
struct WinsockInitializer {
    WinsockInitializer() {
	WSADATA wsadata;
	int wsaerror = WSAStartup(MAKEWORD(2, 2), &wsadata);
	// FIXME - should we check the returned information in wsadata to check
	// that we have a version of winsock which is recent enough for us?

	if (wsaerror != 0) {
	    throw Xapian::NetworkError("Failed to initialize winsock", wsaerror);
	}
    }

    ~WinsockInitializer() {
	WSACleanup();
    }
};

/** Get the errno value of the last error to occur due to a socket operation.
 *
 *  This is specific to the calling thread.
 *
 *  This is needed because some platforms (Windows) separate errors due to
 *  socket operations from other errors.  On platforms which don't do this,
 *  the return value will be the value of errno.
 */
inline int socket_errno() {
    int wsa_err = WSAGetLastError();
    switch (wsa_err) {
# ifdef EADDRINUSE
	case WSAEADDRINUSE: return EADDRINUSE;
# endif
# ifdef ETIMEDOUT
	case WSAETIMEDOUT: return ETIMEDOUT;
# endif
# ifdef EINPROGRESS
	case WSAEINPROGRESS: return EINPROGRESS;
# endif
	default: return wsa_err;
    }
}

/* Newer compilers define these, in which case we map to those already defined
 * values in socket_errno() above.
 */
# ifndef EADDRINUSE
#  define EADDRINUSE WSAEADDRINUSE
# endif
# ifndef ETIMEDOUT
#  define ETIMEDOUT WSAETIMEDOUT
# endif
# ifndef EINPROGRESS
#  define EINPROGRESS WSAEINPROGRESS
# endif

// We must call closesocket() (instead of just close()) under __WIN32__ or
// else the socket remains in the CLOSE_WAIT state.
# define CLOSESOCKET(S) closesocket(S)

#else

// For INET_ADDRSTRLEN and INET6_ADDRSTRLEN.
#include <arpa/inet.h>

// There's no distinction between sockets and other fds on UNIX.
inline void close_fd_or_socket(int fd) { close(fd); }

inline int socket_errno() { return errno; }

# define CLOSESOCKET(S) close(S)

#endif

/** Attempt to set socket-level timeouts.
 *
 *  These aren't supported by all platforms, and some platforms allow them to
 *  set but ignore them, so we can't easily report failure.
 *
 *  Also sets SO_KEEPALIVE (if supported), which should ensure a stuck
 *  connection will eventually time out, though it may take up to ~2 hours.
 */
void set_socket_timeouts(int fd, double timeout);

constexpr size_t PRETTY_IP6_LEN =
    (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

int pretty_ip6(const void* p, char* buf);

#endif // XAPIAN_INCLUDED_SOCKET_UTILS_H
