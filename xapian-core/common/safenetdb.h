/** @file safenetdb.h
 *  @brief #include <netdb.h>, with portability workarounds.
 */
/* Copyright (C) 2013 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFENETDB_H
#define XAPIAN_INCLUDED_SAFENETDB_H

#ifdef __WIN32__
# error "safenetdb.h not supported under __WIN32__"
#endif

#ifdef _AIX
# define _USE_IRS // Needed to get hstrerror().
#endif
#include <netdb.h>

#endif // XAPIAN_INCLUDED_SAFENETDB_H
