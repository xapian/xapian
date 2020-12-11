/** @file
 * @brief Provide setenv() with compatibility versions
 */
/* Copyright (C) 2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SETENV_H
#define XAPIAN_INCLUDED_SETENV_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <stdlib.h>

#ifdef HAVE_SETENV
// setenv() provided by the system.
#elif defined HAVE__PUTENV_S
# if !HAVE_DECL__PUTENV_S
// Mingw 3.20 doesn't declare this, but it's in the Microsoft C runtime DLL.
extern "C" int _putenv_s(const char*, const char*);
# endif

// Use a lambda function to give us a block scope to use static_assert in
// while still being able to return a result.
# define setenv(NAME, VALUE, OVERWRITE) ([]() { \
    static_assert((OVERWRITE), "OVERWRITE must be non-zero constant"); \
    (void)(OVERWRITE); \
    return _putenv_s((NAME), (VALUE)) ? -1 : 0; \
})()

#endif

#endif // XAPIAN_INCLUDED_SETENV_H
