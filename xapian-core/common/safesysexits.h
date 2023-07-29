/** @file
 * @brief #include <sysexits.h> with portability workarounds.
 */
/* Copyright (C) 2023 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFESYSEXITS_H
#define XAPIAN_INCLUDED_SAFESYSEXITS_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#ifdef HAVE_SYSEXITS_H
# include <sysexits.h>
#endif

#ifndef EX_UNAVAILABLE
# define EX_UNAVAILABLE 69
#endif
#ifndef EX_OSERR
# define EX_OSERR 71
#endif
#ifndef EX_TEMPFAIL
# define EX_TEMPFAIL 75
#endif
#ifndef EX_NOPERM
# define EX_NOPERM 77
#endif

#endif // XAPIAN_INCLUDED_SAFESYSEXITS_H
