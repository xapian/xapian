/** @file attributes.h
 * @brief Compiler attribute macros
 */
// Copyright (C) 2012,2013,2014 Olly Betts
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef XAPIAN_INCLUDED_ATTRIBUTES_H
#define XAPIAN_INCLUDED_ATTRIBUTES_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/attributes.h> directly; include <xapian.h> instead."
#endif

#ifdef __GNUC__
// __attribute__((__const__)) is available at least as far back as GCC 2.95.
# define XAPIAN_CONST_FUNCTION __attribute__((__const__))
// __attribute__((__pure__)) is available from GCC 2.96 onwards.
# define XAPIAN_PURE_FUNCTION __attribute__((__pure__))
// __attribute__((__nothrow__)) is available from GCC 3.3 onwards.
# if __GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)
#  define XAPIAN_NOTHROW(D) D __attribute__((__nothrow__))
# endif
#else
/** A function which does not examine any values except its arguments and has
 *  no effects except its return value.
 *
 *  This means the compiler can perform CSE (common subexpression elimination)
 *  on calls to such a function with the same arguments, and also completely
 *  eliminate calls to this function when the return value isn't used.
 */
# define XAPIAN_CONST_FUNCTION

/** Like XAPIAN_CONST_FUNCTION, but such a function can also examine global
 *  memory, perhaps via pointer or reference parameters.
 */
# define XAPIAN_PURE_FUNCTION
#endif

#ifdef _MSC_VER
# define XAPIAN_NOTHROW(D) __declspec(nothrow) D
#endif

#ifndef XAPIAN_NOTHROW
# if __cplusplus >= 201103L
// C++11
#  define XAPIAN_NOTHROW(D) D noexcept(true)
# else
/** A function or method which will never throw an exception. */
#  define XAPIAN_NOTHROW(D) D
# endif
#endif

#endif // XAPIAN_INCLUDED_ATTRIBUTES_H
