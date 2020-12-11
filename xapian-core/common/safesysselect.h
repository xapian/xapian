/** @file
 * @brief #include <sys/select.h> with portability workarounds.
 */
/* Copyright (C) 2007,2011,2018 Olly Betts
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

// On Solaris FD_SET uses memset but fails to prototype it.
# include <cstring>

#else

// select() and FD_SET() are defined in <winsock2.h>:
//
// The FD_SET macro expects an unsigned type (SOCKET) for the fd and passing
// an int can result in a warning about comparing signed and unsigned, so we
// add a wrapper to cast the fd argument of FD_SET to unsigned.
# include "safewinsock2.h"
inline void xapian_FD_SET_(int fd, fd_set *set) {
    FD_SET(unsigned(fd), set);
}
# ifdef FD_SET
#  undef FD_SET
# endif
# define FD_SET(FD,SET) xapian_FD_SET_(FD,SET)
#endif

#endif // XAPIAN_INCLUDED_SAFESYSSELECT_H
