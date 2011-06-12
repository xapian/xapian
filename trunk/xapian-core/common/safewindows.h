/* safewindows.h: #include <windows.h> without all the bloat and damage.
 *
 * Copyright (C) 2005,2007 Olly Betts
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
#define NOGDI
#include <windows.h>

// FOF_NOERRORUI isn't defined by older versions of the mingw headers.
#ifndef FOF_NOERRORUI
# define FOF_NOERRORUI 1024
#endif

#endif // XAPIAN_INCLUDED_SAFEWINDOWS_H
