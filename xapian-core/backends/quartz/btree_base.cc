/* btree_base.cc: Btree base file implementation
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

#include "btree_base.h"
#include "quartz_utils.h"
#include "utils.h"
#include "om/omerror.h"
#include <errno.h>

/************ Base file parameters ************/

/** This is the current description of the base file format:
 *
 * Numbers are (unless mentioned otherwise) stored in the variable
 * length format used by pack_uint() - that is 7 bits at a time in
 * a byte, starting with lower-order bits, and setting the high bit
 * on all bytes before the last one.
 *
 * The format consists of a sequence of numbers in this order:
 *
 * REVISION
 * FORMAT	will be = 1 for the current format.  If this value is
 * 		higher then it is a different format which we
 * 		doesn't yet understand, so we bomb out.  If it's lower,
 * 		then it depends if we have backwards-compatibility code
 * 		implemented.
 * BLOCK_SIZE
 * ROOT
 * LEVEL
 * BIT_MAP_SIZE
 * ITEM_COUNT
 * LAST_BLOCK
 * HAVE_FAKEROOT
 * REVISION2	A second copy of the revision number, for consistency checks.
 */
#define CURR_FORMAT 1U

#if 0

/** This is the description of the old base file format: IGNORE */
#define B_SIZE          80  /* 2 bytes */

#define B_FORMAT         2  /* 1 byte - spare; 256 possible styles ... */
#define B_REVISION       3  /* 4 bytes */
#define B_BLOCK_SIZE     7  /* 4 bytes */
#define B_ROOT          11  /* 4 bytes */
#define B_LEVEL         15  /* 4 bytes */
#define B_BIT_MAP_SIZE  19  /* 4 bytes */
#define B_ITEM_COUNT    23  /* 4 bytes */
#define B_LAST_BLOCK    27  /* 4 bytes */
#define B_HAVE_FAKEROOT 31  /* 1 byte - boolean */

        /* 31 to 75 are spare */

#define B_REVISION2     (B_SIZE - 4)
#endif

Btree_base::Btree_base()
	: revision(0),
	  block_size(0),
	  root(0),
	  level(0),
	  bit_map_size(0),
	  item_count(0),
	  last_block(0),
	  have_fakeroot(false)
{
}

Btree_base::Btree_base(const std::string &name_, char ch)
	: revision(0),
	  block_size(0),
	  root(0),
	  level(0),
	  bit_map_size(0),
	  item_count(0),
	  last_block(0),
	  have_fakeroot(false)
{
    std::string err_msg;
    if (!read(name_, ch, err_msg)) {
	throw OmOpeningError(err_msg);
    }
}

Btree_base::Btree_base(const Btree_base &other)
	: revision(other.revision),
	  block_size(other.block_size),
	  root(other.root),
	  level(other.level),
	  bit_map_size(other.bit_map_size),
	  item_count(other.item_count),
	  last_block(other.last_block),
	  have_fakeroot(other.have_fakeroot)
{
}

void
Btree_base::operator=(const Btree_base &other)
{
    revision = other.revision;
    block_size = other.block_size;
    root = other.root;
    level = other.level;
    bit_map_size = other.bit_map_size;
    item_count = other.item_count;
    last_block = other.last_block;
    have_fakeroot = other.have_fakeroot;
}

Btree_base::~Btree_base()
{
}

bool
Btree_base::read(const std::string & name, char ch, std::string &err_msg)
{
    int h = sys_open_to_read_no_except(name + "base" + ch);
    fdcloser closefd(h);
    if ( ! valid_handle(h)) {
	err_msg += "Couldn't open " + name + "base" +
		ch + ": " + strerror(errno) + "\n";
	return false;
    }
    std::string buf(sys_read_all_bytes(h, 1024));

    const char *start = buf.data();
    const char *end = start + buf.length();

    if (!unpack_uint(&start, end, &revision)) {
	err_msg += "Unable to read revision number from " +
		    name + "base" + ch + "\n";
	return false;
    }
    uint4 format;
    if (!unpack_uint(&start, end, &format)) {
	err_msg += "Unable to read revision number from " +
		    name + "base" + ch + "\n";
	return false;
    }
    if (format != CURR_FORMAT) {
	err_msg += "Bad base file format " + om_tostring(format) + " in " +
		    name + "base" + ch + "\n";
	return false;
    }
    if (!unpack_uint(&start, end, &block_size)) {
	err_msg += "Couldn't read block_size from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    uint4 unsigned_temp;
    if (!unpack_uint(&start, end, &unsigned_temp)) {
	err_msg += "Couldn't read root from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    root = unsigned_temp;
    if (!unpack_uint(&start, end, &unsigned_temp)) {
	err_msg += "Couldn't read level from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    level = unsigned_temp;
    if (!unpack_uint(&start, end, &unsigned_temp)) {
	err_msg += "Couldn't read bit_map_size from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    bit_map_size = unsigned_temp;
    if (!unpack_uint(&start, end, &unsigned_temp)) {
	err_msg += "Couldn't read item_count from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    item_count = unsigned_temp;
    if (!unpack_uint(&start, end, &unsigned_temp)) {
	err_msg += "Couldn't read last_block from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    last_block = unsigned_temp;
    uint4 temp_have_fakeroot;
    if (!unpack_uint(&start, end, &temp_have_fakeroot)) {
	err_msg += "Couldn't read have_fakeroot from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    have_fakeroot = temp_have_fakeroot;

    uint4 revision2;
    if (!unpack_uint(&start, end, &revision2)) {
	err_msg += "Couldn't read revision2 from base file " +
	name + "base" + ch + "\n";
	return false;
    }
    if (revision != revision2) {
	err_msg += "Revision number mismatch in " +
		name + "base" + ch + ": " +
		om_tostring(revision) + " vs " + om_tostring(revision2) + "\n";
	return false;
    }

    return true;
}

uint4
Btree_base::get_revision()
{
    return revision;
}

uint4
Btree_base::get_block_size()
{
    return block_size;
}

int4
Btree_base::get_root()
{
    return root;
}

int4
Btree_base::get_level()
{
    return level;
}

int4
Btree_base::get_bit_map_size()
{
    return bit_map_size;
}

int4
Btree_base::get_item_count()
{
    return item_count;
}

int4
Btree_base::get_last_block()
{
    return last_block;
}

bool
Btree_base::get_have_fakeroot()
{
    return have_fakeroot;
}

void
Btree_base::set_revision(uint4 revision_)
{
    revision = revision_;
}

void
Btree_base::set_block_size(uint4 block_size_)
{
    block_size = block_size_;
}

void
Btree_base::set_root(int4 root_)
{
    root = root_;
}

void
Btree_base::set_level(int4 level_)
{
    level = level_;
}

void
Btree_base::set_bit_map_size(int4 bit_map_size_)
{
    bit_map_size = bit_map_size_;
}

void
Btree_base::set_item_count(int4 item_count_)
{
    item_count = item_count_;
}

void
Btree_base::set_last_block(int4 last_block_)
{
    last_block = last_block_;
}

void
Btree_base::set_have_fakeroot(bool have_fakeroot_)
{
    have_fakeroot = have_fakeroot_;
}

void
Btree_base::write_to_file(const std::string &filename)
{
    std::string buf;
    buf += pack_uint(revision);
    buf += pack_uint(CURR_FORMAT);
    buf += pack_uint(block_size);
    buf += pack_uint(static_cast<uint4>(root));
    buf += pack_uint(static_cast<uint4>(level));
    buf += pack_uint(static_cast<uint4>(bit_map_size));
    buf += pack_uint(static_cast<uint4>(item_count));
    buf += pack_uint(static_cast<uint4>(last_block));
    buf += pack_uint(have_fakeroot);
    buf += pack_uint(revision);  // REVISION2

    int h = sys_open_to_write(filename);
    fdcloser closefd(h);

    sys_write_bytes(h, buf.length(),
		    reinterpret_cast<const byte *>(buf.data()));
    sys_flush(h);
}
