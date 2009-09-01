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

# ifndef HAVE_UUID_UNPARSE_LOWER
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
