/** @file safesyssocket.h
 * @brief #include <sys/socket.h> with portability workarounds.
 */
/* Copyright (C) 2012 Olly Betts
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
#endif

#endif // XAPIAN_INCLUDED_SAFESYSSOCKET_H
