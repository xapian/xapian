/** @file safesyssocket.h
 * @brief #include <sys/socket.h> with portability workarounds.
 */
/* Copyright (C) 2012,2013,2014 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SAFESYSSOCKET_H
#define XAPIAN_INCLUDED_SAFESYSSOCKET_H

#ifndef __WIN32__
// Some older BSDs require sys/types.h to be included first.
# include <sys/types.h>
# include <sys/socket.h>
#else
# include "safewinsock2.h"
#endif

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC 0
#else
// On Linux at least, sometimes SOCK_CLOEXEC is defined but the kernel doesn't
// handle it in socket() or socketpair():

# include "safeerrno.h"

inline int socket_(int domain, int type, int protocol) {
    // Usually type is passed a constant, so we'll collapse to one branch or
    // the other here.  The case where SOCK_CLOEXEC == 0 is handled suitably.
    if (type & SOCK_CLOEXEC) {
	int save_errno = errno;
	int r = socket(domain, type, protocol);
	if (r != 0 && errno == EINVAL) {
	    errno = save_errno;
	    r = socket(domain, type &~ SOCK_CLOEXEC, protocol);
	}
	return r;
    } else {
	return socket(domain, type, protocol);
    }
}

inline int socketpair_(int domain, int type, int protocol, int *sv) {
    // Usually type is passed a constant, so we'll collapse to one branch or
    // the other here.  The case where SOCK_CLOEXEC == 0 is handled suitably.
    if (type & SOCK_CLOEXEC) {
	int save_errno = errno;
	int r = socketpair(domain, type, protocol, sv);
	if (r != 0 && errno == EINVAL) {
	    errno = save_errno;
	    r = socketpair(domain, type &~ SOCK_CLOEXEC, protocol, sv);
	}
	return r;
    } else {
	return socketpair(domain, type, protocol, sv);
    }
}

# ifdef socket
#  undef socket
# endif
# define socket(D,T,P) socket_(D,T,P)
# ifdef socketpair
#  undef socketpair
# endif
# define socketpair(D,T,P,S) socketpair_(D,T,P,S)
#endif

#endif // XAPIAN_INCLUDED_SAFESYSSOCKET_H
