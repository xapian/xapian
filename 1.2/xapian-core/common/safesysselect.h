/* safesysselect.h: #include <sys/select.h> with portability workarounds.
 *
 * Copyright (C) 2007,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFESYSSELECT_H
#define XAPIAN_INCLUDED_SAFESYSSELECT_H

#ifndef PACKAGE
# error You must #include <config.h> before #include "safesysselect.h"
#endif

#ifndef __WIN32__
# ifdef HAVE_SYS_SELECT_H
// According to POSIX 1003.1-2001.
#  include <sys/select.h>
# else
// According to earlier standards.
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>
# endif

// On Solaris FDSET uses memset but fails to prototype it.
# include <cstring>

#else
// Under __WIN32__, socket() returns the unsigned type SOCKET.  We can safely
// assign that to an int (it'll be a non-negative fd or INVALID_SOCKET, which
// will cast to -1 as an int), but when we use int in FD_SET, we get a warning
// about comparing signed and unsigned, so we add a wrapper to cast the fd
// argument of FD_SET to unsigned.

// select() and FD_SET() are defined in <winsock2.h>:
# include "safewinsock2.h"
inline void xapian_FD_SET_(int fd, fd_set *set) {
    FD_SET((unsigned)fd, set);
}
# ifdef FD_SET
#  undef FD_SET
# endif
# define FD_SET(FD,SET) xapian_FD_SET_(FD,SET)
#endif

#endif // XAPIAN_INCLUDED_SAFESYSSELECT_H
