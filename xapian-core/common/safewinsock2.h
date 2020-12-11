/** @file
 * @brief #include <winsock2.h> but working around problems.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFEWINSOCK2_H
#define XAPIAN_INCLUDED_SAFEWINSOCK2_H

#ifndef __WIN32__
# error Including safewinsock2.h, but __WIN32__ is not defined!
#endif

// Include windows.h ourselves first to avoid problems with winsock2.h
// including it for us but without our workarounds.
#include "safewindows.h"

#include <winsock2.h>

#endif // XAPIAN_INCLUDED_SAFEWINSOCK2_H
