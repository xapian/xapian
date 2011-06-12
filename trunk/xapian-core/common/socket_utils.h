/** @file  socket_utils.h
 *  @brief Socket handling utilities.
 */
/* Copyright (C) 2006,2007,2008 Olly Betts
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

#include "safeunistd.h"

#ifdef __WIN32__

#include "safewinsock2.h"

/// Convert an fd (which might be a socket) to a WIN32 HANDLE.
extern HANDLE fd_to_handle(int fd);

/// Close an fd, which might be a socket.
extern void close_fd_or_socket(int fd);
#else
// There's no distinction between sockets and other fds on UNIX.
inline void close_fd_or_socket(int fd) { close(fd); }
#endif

#endif // XAPIAN_INCLUDED_SOCKET_UTILS_H
