/* @file win32_uuid.h
 * @brief Provide UUID functions compatible with libuuid from util-linux-ng.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_WIN32_UUID_H
#define XAPIAN_INCLUDED_WIN32_UUID_H

#if !defined __CYGWIN__ && !defined __WIN32__
# error Including win32_uuid.h, but neither __CYGWIN__ nor __WIN32__ defined!
#endif

#include "safewindows.h"
#include <rpc.h>

// Unfortunately Windows defines uuid_t as GUID, so we redefine it to match the
// Unix definition.
#undef uuid_t
typedef unsigned char uuid_t[16];

void uuid_generate(uuid_t uu);

int uuid_parse(const char * in, uuid_t uu);

void uuid_unparse_lower(const uuid_t uu, char * out);

void uuid_clear(uuid_t uu);

int uuid_is_null(const uuid_t uu);

#endif /* XAPIAN_INCLUDED_WIN32_UUID_H */
