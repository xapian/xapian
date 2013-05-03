/** @file noreturn.h
 * @brief Define the XAPIAN_NORETURN macro.
 */
/* Copyright (C) 2007,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_NORETURN_H
#define XAPIAN_INCLUDED_NORETURN_H

// The macro needs to be applied to a declaration, so use it like so:
//
// XAPIAN_NORETURN(static void throw_read_only());
// static void
// throw_read_only()
// {
//     throw Xapian::InvalidOperationError("Server is read-only");
// }

// Allow the user to override XAPIAN_NORETURN on the compiler command line.
#ifndef XAPIAN_NORETURN
# if defined __GNUC__
// __attribute__((__noreturn__)) is supported by GCC 2.5 and later so there's
// no need to check the GCC version in use.
#  define XAPIAN_NORETURN(D) D __attribute__((__noreturn__))
# elif defined _MSC_VER && _MSC_VER >= 1300
// __declspec(noreturn) appears to be supported by MSVC 7.0 and later.
#  define XAPIAN_NORETURN(D) __declspec(noreturn) D
# else
#  define XAPIAN_NORETURN(D) D
# endif
#endif

#endif // XAPIAN_INCLUDED_NORETURN_H
