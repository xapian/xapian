/** @file autoptr.h
 * @brief Wrapper around standard unique_ptr or autoptr template.
 */
/* Copyright (C) 2009,2011,2014 Olly Betts
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

// We used to provide an implementation of auto_ptr called AutoPtr to work
// around deficiencies in the implementation with some compiler, possibly
// GCC 2.95 (it was added when that was the current GCC release).
//
// We no longer support building Xapian with such old versions of GCC, but
// we'd like to switch to unique_ptr (new in C++11) so we've kept the AutoPtr
// wrapper to (ab/re)use for this transition.
#ifndef AutoPtr
# include <memory>
# if __cplusplus >= 201103L
// Under C++11, use unique_ptr
#  define AutoPtr std::unique_ptr
# else
#  define AutoPtr std::auto_ptr
# endif
#endif
