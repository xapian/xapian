/** @file safeuuid.h
 *  @brief #include <uuid/uuid.h>, with alternative implementation for windows.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2009,2010,2013 Olly Betts
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

#if defined __CYGWIN__ || defined __WIN32__
# include "common/win32_uuid.h"
#elif defined HAVE_UUID_UUID_H
# include <uuid/uuid.h>

// Some UUID libraries lack const qualifiers.  Solaris is a particular
// example which survives today (2009).  The libuuid from e2fsprogs (now in
// util-linux-ng) gained const qualifiers in 2001.  It's hard to cast uuid_t
// suitably as it is a typedef for an array in at least some implementations,
// but we probably can't safely assume that.  We don't need to pass const
// uuid_t, but we do need to pass const char *, so fix that with a macro
// wrapper for uuid_parse().
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

#else

// UUID API on FreeBSD, NetBSD and AIX.

# ifdef _AIX
/* AIX uses a byte typedef in its <uuid.h> which collides with ours, so use a
 * macro to rename theirs out of the way.
 */
#  define byte xapian_hack_aix_uuid_byte
#  include <uuid.h>
#  undef byte
# else
#  include <uuid.h>
# endif

# include <cstdlib>
# include <cstring>
# include <exception>

typedef unsigned char uuid_t_[16];

inline void
uuid_generate(uuid_t_ out) {
    uuid_t uuid;
    uint32_t status;
    uuid_create(&uuid, &status);
    if (status != uuid_s_ok) {
	// Can only be uuid_s_no_memory it seems.
	throw std::bad_alloc();
    }
    std::memcpy(out, &uuid, sizeof(uuid_t_));
}

inline int
uuid_parse(const char * in, uuid_t_ uu)
{
    uuid_t uuid;
    uint32_t status;
#ifdef _AIX
    // AIX takes unsigned char* not const char *.
    char * nonconst_in = const_cast<char*>(in);
    unsigned char * unsigned_in = reinterpret_cast<unsigned char *>(nonconst_in);
    uuid_from_string(unsigned_in, &uuid, &status);
#else
    uuid_from_string(in, &uuid, &status);
#endif
    if (status != uuid_s_ok)
	return -1;
    std::memcpy(uu, &uuid, sizeof(uuid_t_));
    return 0;
}

inline void
uuid_unparse_lower(const uuid_t_ uu, char * out)
{
    uuid_t uuid;
    uint32_t status;
    std::memcpy(&uuid, uu, sizeof(uuid_t));
#ifdef _AIX
    // AIX takes unsigned char* not char *.
    unsigned char * result;
#else
    char * result;
#endif
    uuid_to_string(&uuid, &result, &status);
    std::memcpy(out, result, 36);
    std::free(result);
    if (status != uuid_s_ok) {
	// Could be uuid_s_bad_version (FIXME) or uuid_s_no_memory it seems.
	throw std::bad_alloc();
    }
    /* Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
     * a-z by bitwise or with 0x20, and the others already have this bit set.
     */
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}

inline void
uuid_clear(uuid_t_ uu)
{
    std::memset(uu, 0, sizeof(uuid_t_));
}

inline int
uuid_is_null(const uuid_t_ uu)
{
    size_t i = 0;
    while (i < sizeof(uuid_t_)) {
	if (uu[i++])
	    return 0;
    }
    return 1;
}

// Hide incompatible uuid_t from <uuid.h>.
# define uuid_t uuid_t_

#endif

#endif // XAPIAN_INCLUDED_SAFEUUID_H
