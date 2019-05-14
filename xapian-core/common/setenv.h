/** @file setenv.h
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
// setenv() provided.
#elif defined HAVE__PUTENV_S
inline int
setenv(const char* name, const char* value, constexpr int overwrite)
{
    static_assert(overwrite, "overwrite non-zero");
    (void)overwrite;
#if !HAVE_DECL__PUTENV_S
    // Mingw 3.20
    errno_t _putenv_s(const char*, const char*);
#endif
    return _putenv_s(name, value) ? -1 : 0;
}
#endif

#endif // XAPIAN_INCLUDED_SETENV_H
