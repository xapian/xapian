/* btree_bitmap.cc: Btree bitmap implementation
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

#include "btree_bitmap.h"
#include "btree.h"
#include "om/omerror.h"
#include <errno.h>

Btree_bitmap::Btree_bitmap(Btree *B_)
	: size(0), low(0), bit_map0(0), bit_map(0), B(B_)
{
}

Btree_bitmap::~Btree_bitmap()
{
    delete [] bit_map;
    bit_map = 0;
    delete [] bit_map0;
    bit_map0 = 0;
}

void
Btree_bitmap::read_from_file(const std::string &name, char ch, int size)
{
    bit_map0 = new byte[size];
    if (bit_map0) {
	int h = sys_open_to_read_no_except(name + "bitmap" + ch);
	fdcloser fdclose(h);

	if (valid_handle(h) &&
	    sys_read_bytes(h, size, bit_map0)) {
	    return;
	}
	std::string message = "Failed to read bit map from table ";
	message += name;

	B->error = BTREE_ERROR_BITMAP_READ;
	throw OmOpeningError(message);
    } else {
	throw std::bad_alloc();
    }
}

void
Btree_bitmap::read(const std::string &name, char ch, int size_)
{
    size = size_;
    read_from_file(name, ch, size);
    bit_map = new byte[size];

    memmove(bit_map, bit_map0, size);
}

/*
   block_free_at_start(B, n) is true iff (if and only if) block n was free at
   the start of the transaction on the B-tree.
*/

bool
Btree_bitmap::block_free_at_start(int4 n) const
{
    int i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    return (bit_map0[i] & bit) == 0;
}

/* free_block(B, n) causes block n to be marked free in the bit map.
   B->bit_map_low is the lowest byte in the bit map known to have a free bit
   set. Searching starts from there when looking for a free block.
*/

void
Btree_bitmap::free_block(int4 n)
{
    int i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    bit_map[i] &= ~ bit;

    if (low > i &&
       (bit_map0[i] & bit) == 0) /* free at start */
        low = i;
}

/* extend(B) increases the size of the two bit maps in an obvious way.
   The bitmap file grows and shrinks as the DB file grows and shrinks in
   internal usage. But the DB file itself does not reduce in size, no matter
   how many blocks are freed.
*/

#define BIT_MAP_INC 1000
    /* increase the bit map by this number of bytes if it overflows */

void
Btree_bitmap::extend()
{
    int n = size + BIT_MAP_INC;
    byte *new_bit_map0 = 0;
    byte *new_bit_map = 0;

    try {
	new_bit_map0 = new byte[n];
	new_bit_map = new byte[n];

	memmove(new_bit_map0, bit_map0, size);
	memmove(new_bit_map, bit_map, size);

        for (int i = size; i < n; i++)
        {
	    new_bit_map0[i] = 0;
            new_bit_map[i] = 0;
        }
    } catch (...) {
	delete [] new_bit_map0;
	delete [] new_bit_map;
	throw;
    }
    delete [] bit_map0;
    bit_map0 = new_bit_map0;
    delete [] bit_map;
    bit_map = new_bit_map;
    size = n;
}

/* next_free_block(B) returns the number of the next available free block in
   the bitmap, marking it as 'in use' before returning. More precisely, we get
   a block that is both free now (in bit_map) and was free at the beginning of
   the transaction on the B-tree (in bit_map0).

   Starting at bit_map_low we go up byte at a time until we find a byte with a
   free (zero) bit, and then go up that byte bit at a time. If the bit map has
   no free bits it is extended so that it will have.
*/

int
Btree_bitmap::next_free_block()
{
    int4 i;
    int x;
    for (i = low;; i++)
    {  
	if (i >= size) {
	    extend();
	}
        x = bit_map0[i] | bit_map[i];
        if (x != UCHAR_MAX) break;
    }
    int4 n = i * CHAR_BIT;
    int d = 0x1;
    while ((x & d) != 0) { d <<= 1; n++; }
    bit_map[i] |= d;   /* set as 'in use' */
    low = i;
    return n;
}

int
Btree_bitmap::get_size() const
{
    return size;
}

bool
Btree_bitmap::block_free_now(int4 n)
{
    int4 i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    return (bit_map[i] & bit) == 0;
}

int
Btree_bitmap::write_to_file(const std::string &name)
{
    int h = sys_open_to_write(name);
    return valid_handle(h) &&
	    sys_write_bytes(h, size, bit_map) &&
	    sys_flush(h) &&
	    sys_close(h);
}

int4
Btree_bitmap::get_last_block()
{
    int i = size - 1;
    while (bit_map[i] == 0 && i > 0) {
	i--;
    }
    size = i + 1;

    int x = bit_map[i];
    int4 n = (i + 1) * CHAR_BIT - 1;
    int d = 0x1 << (CHAR_BIT - 1);
    while ((x & d) == 0) { d >>= 1; n--; }

    return n;
}

bool
Btree_bitmap::is_empty() const
{
    for (int i = 0; i < size; i++) {
	if (bit_map[i] != 0) {
	    return false;
	}
    }
    return true;
}

void
Btree_bitmap::clear()
{
    for (int i=0; i<size; ++i) {
	bit_map[i] = 0;
    }
}
