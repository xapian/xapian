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
#include "om/omerror.h"
#include <errno.h>

/************ Base file parameters ************/

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

Btree_base::Btree_base()
	: data(0),
	  revision(0),
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
	: data(0),
	  revision(0),
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
	: data(0),
	  revision(other.revision),
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
    if (data) {
	delete [] data;
	data = 0;
    }
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
    if (data) {
	delete [] data;
	data = 0;
    }
}

/** A tiny class used to close a filehandle safely in the presence
 *  of exceptions.
 */
class fdcloser {
    public:
	fdcloser(int fd_) : fd(fd_) {}
	~fdcloser() {
	    if (fd >= 0) {
		sys_close(fd);
	    }
	}
    private:
	int fd;
};

bool
Btree_base::read(const std::string & name, char ch, std::string &err_msg)
{
    int h = sys_open_to_read_no_except(name + "base" + ch);
    fdcloser closefd(h);
    byte w[2];
    int size;
    if ( ! valid_handle(h)) {
	err_msg += "Couldn't open " + name + "base" +
		ch + ": " + strerror(errno) + "\n";
	return false;
    }
    if (! sys_read_bytes(h, 2, w)) {
	err_msg += "Couldn't read from " + name + "base" + ch +
		": " + strerror(errno) + "\n";
	return false;
    }
    size = GETINT2(w, 0);

    if (data) {
	delete [] data;
	data = 0;
    }
    data = new byte[size];
    if (data != 0) {
	SETINT2(data, 0, size);
	if (sys_read_bytes(h, size - 2, data + 2) &&
	    get_int4(data, B_REVISION) == get_int4(data, B_REVISION2)) {

	    revision = get_uint4(data, B_REVISION);
	    block_size = get_int4(data, B_BLOCK_SIZE);
	    root = get_int4(data, B_ROOT);
	    level = get_int4(data, B_LEVEL);
	    bit_map_size = get_int4(data, B_BIT_MAP_SIZE);
	    item_count = get_int4(data, B_ITEM_COUNT);
	    last_block = get_int4(data, B_LAST_BLOCK);
	    have_fakeroot = GETINT1(data, B_HAVE_FAKEROOT);
	    return true;
	} else {
	    err_msg += "Couldn't read revision number from " +
		    name + "base" + ch + ": " + strerror(errno) + "\n";
	    return false;
	}
    } else {
	throw std::bad_alloc();
    }

    return false;
}

#if 0
bool 
Btree_base::write(struct Btree * B)
{
    if (data) {
	delete [] data;
	data = 0;
    }
    data = new byte[B_SIZE];

    int h = sys_open_to_write(B->name + "base" + B->other_base_letter);
    fdcloser closefd(h);

    memset(data, 0, B_SIZE);

    Assert(false);
    /* FIXME: things need a bit of reorganisation here. */

    return valid_handle(h) &&
	    sys_write_bytes(h, GETINT2(B->base, 0), B->base) &&
	    sys_flush(h) &&
	    sys_close(h);
}
#endif

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
    if (data) {
	delete [] data;
	data = 0;
    }
    data = new byte[B_SIZE];
    memset(data, 0, B_SIZE);

    SETINT2(data, 0, B_SIZE);
    set_int4(data, B_REVISION, revision);
    set_int4(data, B_REVISION2, revision);
    set_int4(data, B_BLOCK_SIZE, block_size);
    set_int4(data, B_ROOT, root);
    set_int4(data, B_LEVEL, level);
    set_int4(data, B_BIT_MAP_SIZE, bit_map_size);
    set_int4(data, B_ITEM_COUNT, item_count);
    set_int4(data, B_LAST_BLOCK, last_block);
    SETINT1(data, B_HAVE_FAKEROOT, have_fakeroot);

    int h = sys_open_to_write(filename);
    fdcloser closefd(h);

    sys_write_bytes(h, B_SIZE, data);
    sys_flush(h);
}
