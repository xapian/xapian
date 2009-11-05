/** @file safeuuid.h
 *  @brief #include <uuid/uuid.h>, with alternative implementation for windows.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFEUUID_H
#define XAPIAN_INCLUDED_SAFEUUID_H

#ifdef __WIN32__
# include "common/win32_uuid.h"
#else
# include <uuid/uuid.h>

// Some UUID libraries lack const qualifiers.  Solaris is a particular
// example which survives today (2009).  The libuuid from e2fsprogs gained
// const qualifiers in 2001.
# ifndef uuid_compare
#  define uuid_compare(UU1, UU2) \
     (uuid_compare)(*const_cast<uuid_t*>(&(UU1)), *const_cast<uuid_t*>(&(UU2)))
# endif
# ifndef uuid_copy
#  define uuid_copy(DST, SRC) (uuid_copy)((DST), *const_cast<uuid_t*>(&(SRC)))
# endif
# ifndef uuid_is_null
#  define uuid_is_null(UU) (uuid_is_null)(*const_cast<uuid_t*>(&(UU)))
# endif
# ifndef uuid_parse
#  define uuid_parse(IN, UU) (uuid_parse)(const_cast<char*>(IN), (UU))
# endif
# ifndef uuid_time
#  define uuid_time(UU, TV) (uuid_time)(*const_cast<uuid_t*>(&(UU)), (TV))
# endif
# ifndef uuid_unparse
#  define uuid_unparse(UU, O) (uuid_unparse)(*const_cast<uuid_t*>(&(UU)), (O))
# endif

# ifdef HAVE_UUID_UNPARSE_LOWER
#  ifndef uuid_unparse_lower
#   define uuid_unparse_lower(UU, O) \
      (uuid_unparse_lower)(*const_cast<uuid_t*>(&(UU)), (O))
#  endif
# else
/* Older versions of libuuid (such as that on CentOS 4.7) don't have
 * uuid_unparse_lower(), only uuid_unparse(). */
inline void uuid_unparse_lower(const uuid_t uu, char *out) {
    uuid_unparse(uu, out);
    /* Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
     * a-z by bitwise or with 0x20, and the others already have this bit set.
     */
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}
# endif

#endif

#endif // XAPIAN_INCLUDED_SAFEUUID_H
