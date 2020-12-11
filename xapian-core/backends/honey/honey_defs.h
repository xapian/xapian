/** @file
 * @brief Definitions, types, etc for use inside honey.
 */
/* Copyright (C) 2010,2014,2015,2017,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_DEFS_H
#define XAPIAN_INCLUDED_HONEY_DEFS_H

#include "internaltypes.h"

#define SST_SEARCH

/// Honey table extension.
#define HONEY_TABLE_EXTENSION "honey"

/** Minimum size to pad a honey table to.
 *
 *  Having this minimum size means we can usually know from the size that a
 *  stub database file isn't a single file database.
 */
#define HONEY_MIN_DB_SIZE 2048

/// Maximum key length.
#define HONEY_MAX_KEY_LENGTH 255

/** Maximum size of a postlist chunk in bytes.
 *
 *  This isn't a hard maximum, but we won't exceed it by much.
 *
 *  FIXME: 2000 is what glass uses, but we should probably tune this.
 */
#define HONEY_POSTLIST_CHUNK_MAX 2000

// Maximum size of a document length chunk in bytes.
#define HONEY_DOCLEN_CHUNK_MAX 2017

// HONEY_DOCLEN_CHUNK_MAX should be one more than a
// multiple of 12 so for widths 1,2,3,4 we can fix the
// initial byte which indicates the width for the chunk
// plus an exact number of entries.
static_assert((HONEY_DOCLEN_CHUNK_MAX - 1) % 12 == 0,
	      "HONEY_DOCLEN_CHUNK_MAX should be (12 * x + 1)");

/** The largest docid value supported by honey.
 *
 *  The disk format supports 64-bit docids, but if Xapian::docid is narrower
 *  then it's the largest value supported by the type that matters here.
 */
#define HONEY_MAX_DOCID Xapian::docid(0xffffffffffffffff)

namespace Honey {

enum table_type {
    POSTLIST,
    DOCDATA,
    TERMLIST,
    POSITION,
    SPELLING,
    SYNONYM,
    MAX_
};

/// Postlist key second bytes when first byte is zero.
enum {
    KEY_USER_METADATA = 0x00,
    KEY_VALUE_STATS = 0x01,
    KEY_VALUE_STATS_HI = 0x08,
    KEY_VALUE_CHUNK = 0x09,
    KEY_VALUE_CHUNK_HI = 0xe1, // (0xe1 for slots > 26)
    /* 0xe2-0xe6 inclusive unused currently. */
    /* 0xe7-0xee inclusive reserved for doc max wdf chunks. */
    /* 0xef-0xf6 inclusive reserved for unique terms chunks. */
    KEY_DOCLEN_CHUNK = 0xf7,
    KEY_DOCLEN_CHUNK_HI = 0xfe,
    KEY_POSTING_CHUNK = 0xff
};

#define KEY_DOCLEN_PREFIX "\0\xf7"

static_assert(((KEY_VALUE_CHUNK_HI - KEY_VALUE_CHUNK) & 0x07) == 0,
	      "No wasted values");

}

/// A block number in a honey Btree file.
typedef uint4 honey_block_t;

/// The revision number of a honey database.
typedef uint4 honey_revision_number_t;

/// How many entries there are in a table.
typedef unsigned long long honey_tablesize_t;

#endif // XAPIAN_INCLUDED_HONEY_DEFS_H
