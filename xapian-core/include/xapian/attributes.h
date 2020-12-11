/** @file
 * @brief Compiler attribute macros
 */
// Copyright (C) 2012,2013,2014,2015,2017 Olly Betts
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

#ifdef __GNUC__

// __attribute__((__const__)) is available at least as far back as GCC 2.95.
# define XAPIAN_CONST_FUNCTION __attribute__((__const__))
// __attribute__((__pure__)) is available from GCC 2.96 onwards.
# define XAPIAN_PURE_FUNCTION __attribute__((__pure__))

// We don't enable XAPIAN_NONNULL when building the library because that
// results in warnings when we check the parameter really isn't NULL, and we
// ought to still do that as (a) not all compilers support such annotations,
// and (b) even those that do don't actually prevent you from passing NULL.
# ifndef XAPIAN_LIB_BUILD
// __attribute__((__nonnull__(a,b,c))) is available from GCC 3.3 onwards, but
// seems to be buggy in GCC 4.8 so only enable it for versions after that.
// It's also supported by clang, which we have to check for separately as
// current versions pretend to be GCC 4.2.
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8) || \
      defined __clang__
#   define XAPIAN_NONNULL(LIST) __attribute__((__nonnull__ LIST))
#  endif
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

#ifndef XAPIAN_NONNULL
/** Annotate function parameters which should be non-NULL pointers.
 *
 *  If LIST isn't specified, all pointer parameters will be marked in this
 *  way (which is often sufficient):
 *
 *  int foo(const char* p) XAPIAN_NONNULL();
 *  int bar(char* p, const char* q) XAPIAN_NONNULL();
 *
 *  If there are other pointer parameters which can be NULL, then you need
 *  to specify a parenthesised list of the parameters to mark:
 *
 *  int foo(const char* p, int* maybenull) XAPIAN_NONNULL((1));
 *  int bar(char* p, void* maybenull, const char* q) XAPIAN_NONNULL((1,3));
 *
 *  NB In a non-class function, the first parameter is numbered 1, but in
 *  a non-static class method (which isn't a constructor) then the `this`
 *  pointer is implicitly counted as parameter 1, though this doesn't
 *  appear to be documented.  For confirmation see:
 *  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79961
 */
# define XAPIAN_NONNULL(LIST)
#endif

#endif // XAPIAN_INCLUDED_ATTRIBUTES_H
