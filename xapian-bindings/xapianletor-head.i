%{
/** @file xapianletor-head.i
 * @brief Header for SWIG interface file for Xapian-letor.
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2011,2012,2013,2014,2015,2016,2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

// Disable any deprecation warnings for Xapian methods/functions/classes.
#define XAPIAN_DEPRECATED(D) D

#include <xapian-letor.h>

#include <string>
#include <vector>

using namespace std;

%}

using namespace std;

%include exception.i
%include stl.i

// Define these away for SWIG's parser.
#define XAPIAN_DEPRECATED(D) D
#define XAPIAN_DEPRECATED_EX(D) D
#define XAPIAN_DEPRECATED_CLASS
#define XAPIAN_DEPRECATED_CLASS_EX
#define XAPIAN_VISIBILITY_DEFAULT
#define XAPIAN_VISIBILITY_INTERNAL
#define XAPIAN_CONST_FUNCTION
#define XAPIAN_PURE_FUNCTION
#define XAPIAN_NOEXCEPT
#define XAPIAN_NOTHROW(D) D

// This works around buggy behaviour in SWIG's preprocessor, and only works
// because we currently only use XAPIAN_NONNULL() with an empty argument:
//
// https://github.com/swig/swig/pull/1111
//
// The correct version is:
// #define XAPIAN_NONNULL(L)
#define XAPIAN_NONNULL()

// Ignore these which SWIG seems to add pointless type entries for due them
// being used in the SWIG typemap for std::pair.
%ignore first_type;
%ignore second_type;
