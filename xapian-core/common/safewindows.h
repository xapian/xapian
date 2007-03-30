/* safewindows.h: #include <windows.h> without all the bloat and damage.
 *
 * Copyright (C) 2005 Olly Betts
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 */

#ifndef XAPIAN_INCLUDED_SAFEWINDOWS_H
#define XAPIAN_INCLUDED_SAFEWINDOWS_H

#if !defined __CYGWIN__ && !defined __WIN32__
# error Including safewindows.h, but neither __CYGWIN__ nor __WIN32__ defined!
#endif

// Prevent windows.h from defining min and max macros.
#ifndef NOMINMAX
# define NOMINMAX
#endif

// Prevent windows.h from including lots of obscure win32 api headers
// which we don't care about and will just slow down compilation and
// increase the risk of symbol collisions.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// FOF_NOERRORUI isn't defined by older versions of the mingw headers.
#ifndef FOF_NOERRORUI
# define FOF_NOERRORUI 1024
#endif

#ifdef XAPIAN_BUILD_BACKEND_REMOTE

// We also need the winsock headers for socket stuff.
#include "winsock2.h"

// Re-define some of the unix socket error contants to the winsock ones.
#define EADDRINUSE WSAEADDRINUSE
#define ETIMEDOUT WSAETIMEDOUT
#define EINPROGRESS WSAEINPROGRESS

typedef int socklen_t;

/** Class to initialise winsock and keep it initialised while we use it.
 *
 *  We need to get WinSock initialised before we use it, and make it clean up
 *  after we've finished using it.  This class performs this initialisation when
 *  constructed and cleans up when destructed.  Multiple instances of the class
 *  may be instantiated - windows keeps a count of the number of times that
 *  winsock has been initialised, and only performs the cleanup when the cleanup
 *  function has been called the same number of times.
 *
 *  Simply ensure that an instance of this class is initialised whenever we're
 *  doing socket handling.
 */
class WinsockInitializer {
public:
    WinsockInitializer() {
	WSADATA wsadata;
	int wsaerror = WSAStartup(MAKEWORD(2,2), &wsadata);
	// FIXME - should we check the returned information in wsadata to check
	// that we have a version of winsock which is recent enough for us?
	if (wsaerror != 0) {
	    throw Xapian::NetworkError("Failed to initialize winsock", "", wsaerror);
	}
    }
    ~WinsockInitializer() {
	WSACleanup();
    }
};

#endif

#endif // XAPIAN_INCLUDED_SAFEWINDOWS_H
