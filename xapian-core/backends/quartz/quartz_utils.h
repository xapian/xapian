/* quartz_utils.h: Generic functions for quartz
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_QUARTZ_UTILS_H
#define OM_HGUARD_QUARTZ_UTILS_H

#include "config.h"
#include <string>

/// Compile time assert a condition
#define CASSERT(a) {char assert[(a) ? 1 : -1];(void)assert;}

typedef unsigned char       om_byte;
typedef unsigned int        om_uint32;
typedef unsigned long long  om_uint64;
typedef int                 om_int32;
typedef long long           om_int64;

/** Reads an integer from a string starting at a given position.
 *
 *  @param source_ptr   A pointer to a pointer to the data to read.  The
 *                      character pointer will be updated to point to the
 *                      next character to read, or source_end if sooner.
 *  @param source_end   A pointer to the byte after the end of the data to
 *                      read the integer from.
 *  @param result       A pointer to a place to store the result.  If an
 *                      error occurs, the value stored in this location is
 *                      undefined.
 *
 *  @result True if an integer was successfully read.  False if the read
 *          failed.  Failure may either be due to the data running out (in
 *          which case *source_ptr will equal source_end), or due to the value
 *          read overflowing the size of result (in which case *source_ptr
 *          will point to wherever the value ends, despite the overflow).
 */
bool unpack_uint32(const char ** source_ptr,
		   const char * source_end,
		   om_uint32 * result);

/** Generates a packed representation of an integer.
 *
 *  @param value  The integer to represent.
 *
 *  @result       A string containing the representation of the integer.
 */
std::string pack_uint32(om_uint32 value);

#include "quartz_table_entries.h"
#include "om/omtypes.h"

/** Convert a document id to an OmKey.
 */
QuartzDbKey quartz_docid_to_key(om_docid did);

#endif /* OM_HGUARD_QUARTZ_UTILS_H */
