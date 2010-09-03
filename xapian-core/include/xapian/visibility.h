/** @file visibility.h
 * @brief Define XAPIAN_VISIBILITY macro.
 */
// Copyright (C) 2007,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_VISIBILITY_H
#define XAPIAN_INCLUDED_VISIBILITY_H

#include "xapian/version.h"
#if defined XAPIAN_ENABLE_VISIBILITY && defined __GNUC__ && (__GNUC__ >= 4)
// GCC 3.4 has visibility support, but it's a bit buggy so we require 4.0.
# define XAPIAN_VISIBILITY_DEFAULT __attribute__((visibility("default")))
#else
# define XAPIAN_VISIBILITY_DEFAULT
#endif

#endif
