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
#include "pack.h"

#include "omassert.h"
#include <string>

using namespace std;

/// Brass table extension.
#define BRASS_TABLE_EXTENSION "brass"

/// Default B-tree block size.
#define BRASS_DEFAULT_BLOCKSIZE 8192

namespace Brass {
    enum table_type {
	POSTLIST,
	RECORD,
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

class RootInfo {
    brass_block_t root;
    unsigned level;
    brass_tablesize_t num_entries;
    bool root_is_fake;
    bool sequential_mode;
    unsigned blocksize;
    brass_revision_number_t revision;
    std::string fl_serialised;

  public:
    void init(unsigned blocksize_) {
	AssertRel(blocksize_,>=,2048);
	root = 0;
	level = 0;
	num_entries = 0;
	root_is_fake = true;
	sequential_mode = true;
	blocksize = blocksize_;
	revision = 0; // FIXME
	fl_serialised.resize(0);
    }

    void serialise(std::string &s) const {
	pack_uint(s, root);
	unsigned val = level << 2;
	if (sequential_mode) val |= 0x02;
	if (root_is_fake) val |= 0x01;
	pack_uint(s, val);
	pack_uint(s, num_entries);
	pack_uint(s, blocksize >> 11);
	pack_uint(s, revision);
	pack_string(s, fl_serialised);
    }

    bool unserialise(const char ** p, const char * end) {
	unsigned val;
	if (!unpack_uint(p, end, &root) ||
	    !unpack_uint(p, end, &val) ||
	    !unpack_uint(p, end, &num_entries) ||
	    !unpack_uint(p, end, &blocksize) ||
	    !unpack_uint(p, end, &revision) ||
	    !unpack_string(p, end, fl_serialised)) return false;
	level = val >> 2;
	sequential_mode = val & 0x02;
	root_is_fake = val & 0x01;
	blocksize <<= 11;
	return true;
    }

    brass_block_t get_root() const { return root; }
    int get_level() const { return int(level); }
    brass_tablesize_t get_num_entries() const { return num_entries; }
    bool get_root_is_fake() const { return root_is_fake; }
    bool get_sequential_mode() const { return sequential_mode; }
    unsigned get_blocksize() const {
	AssertRel(blocksize,>=,2048);
	return blocksize;
    }
    brass_revision_number_t get_revision() const { return revision; }
    const std::string & get_free_list() const { return fl_serialised; }

    void set_level(int level_) { level = unsigned(level_); }
    void set_num_entries(brass_tablesize_t n) { num_entries = n; }
    void set_root_is_fake(bool f) { root_is_fake = f; }
    void set_sequential_mode(bool f) { sequential_mode = f; }
    void set_root(brass_block_t root_) { root = root_; }
    void set_blocksize(unsigned b) {
	AssertRel(b,>=,2048);
	blocksize = b; }
    void set_revision(brass_revision_number_t r) { revision = r; }
    void set_free_list(const std::string & s) { fl_serialised = s; }
};

#endif // XAPIAN_INCLUDED_BRASS_DEFS_H
