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
// const qualifiers in 2001.  It's hard to cast uuid_t suitably as it is a
// typedef for an array in at least some implementations, but we probably can't
// safely assume that.  We don't need to pass const uuid_t, but we do need to
// pass const char *, so fix that with a macro wrapper for uuid_parse().
# ifndef uuid_parse
#  define uuid_parse(IN, UU) (uuid_parse)(const_cast<char*>(IN), (UU))
# endif

# ifndef HAVE_UUID_UNPARSE_LOWER
/* Older versions of libuuid (such as that on CentOS 4.7) don't have
 * uuid_unparse_lower(), only uuid_unparse().  NB uu parameter not const
 * as uuid_unparse may take a non-const uuid_t parameter. */
inline void uuid_unparse_lower(uuid_t uu, char *out) {
    uuid_unparse(uu, out);
    /* Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
     * a-z by bitwise or with 0x20, and the others already have this bit set.
     */
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}
# endif

#endif

#endif // XAPIAN_INCLUDED_SAFEUUID_H
