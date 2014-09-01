/** @file brass_defs.h
 * @brief Definitions, types, etc for use inside brass.
 */
/* Copyright (C) 2010,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_DEFS_H
#define XAPIAN_INCLUDED_BRASS_DEFS_H

#include "internaltypes.h"

/// Brass table extension.
#define BRASS_TABLE_EXTENSION "brass"

/// Default B-tree block size.
#define BRASS_DEFAULT_BLOCKSIZE 8192

namespace Brass {
    enum table_type {
	POSTLIST,
	DOCDATA,
	TERMLIST,
	POSITION,
	SPELLING,
	SYNONYM,
	MAX_
    };
}

/// A block number in a brass Btree file.
typedef uint4 brass_block_t;

/// The revision number of a brass database.
typedef uint4 brass_revision_number_t;

/// How many entries there are in a table.
typedef unsigned long long brass_tablesize_t;

#endif // XAPIAN_INCLUDED_BRASS_DEFS_H
