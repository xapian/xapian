/* @file win32_uuid.cc
 * @brief Provide UUID functions compatible with libuuid from e2fsprogs.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <config.h>

#include "win32_uuid.h"

#include <xapian/error.h>

void
uuid_generate(uuid_t uu)
{
    UUID uuid;
    if (UuidCreate(&uuid) != RPC_S_OK) {
	// Throw a DatabaseCreateError, since we can't make a UUID.  The
	// windows API documentation is a bit unclear about the situations in
	// which this can happen, but if this behaviour causes a problem, an
	// alternative would be to create a UUID ourselves somehow in this
	// situation.
	throw Xapian::DatabaseCreateError("Cannot create UUID");
    }
    memcpy(uu, &uuid, 16);
}

int
uuid_parse(const char * in, uuid_t uu)
{
    UUID uuid;
    if (UuidFromString((unsigned char*)in, &uuid) != RPC_S_OK)
	return -1;
    memcpy(uu, &uuid, 16);
    return 0;
}

void uuid_unparse_lower(const uuid_t uu, char *  out)
{
    UUID uuid;
    char *uuidstr;
    memcpy(&uuid, uu, 16);
    if (UuidToString(&uuid, (unsigned char **)(&uuidstr)) != RPC_S_OK)
	return;
    int uuidlen = strlen(uuidstr);
    strncpy( out, strlwr(uuidstr), uuidlen );
    RpcStringFree((unsigned char**)(&uuidstr));
}

void uuid_clear(uuid_t uu)
{
    memset(uu, 0x00, 16);
}

int uuid_is_null(const uuid_t uu)
{
    int i = 0;
    while (i < 16) {
	if (uu[i++])
	    return 0;
    }
    return 1;
}
