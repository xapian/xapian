/* btree_util.h: common macros/functions in the Btree implementation.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2004 Olly Betts
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

#ifndef OM_HGUARD_BTREE_UTIL_H
#define OM_HGUARD_BTREE_UTIL_H

#include "btree_types.h"
#include "omassert.h"

#include <string.h>  /* memset */

// FIXME: This named constant probably isn't used everywhere it should be...
#define BYTES_PER_BLOCK_NUMBER 4

/* The unit of access into the DB files is an unsigned char, which is defined
   as 'byte' with a typedef.

   Other integer values are built up from these bytes, either in pairs or fours.
   'int2' and 'int4' are signed types which will comfortably accommodate
   all 2 byte and 4 byte ranges:
*/

/*
   If a signed int cannot hold the full range of two bytes replace 'int' by
   'long' throughout the code.

   FIXME: surely if a signed int cannot hold the full range of two bytes,
   then the compiler violates ANSI?  Or am I misunderstanding...
*/

// FIXME: 65536 in Asserts below should really be block_size
inline int
GETINT1(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536);
    return p[c];
}

inline void
SETINT1(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536);
    p[c] = x;
}

inline int
GETINT2(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c + 1 < 65536);
    return p[c] << 8 | p[c + 1];
}

inline void
SETINT2(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c + 1 < 65536);
    p[c] = x >> 8;
    p[c + 1] = x;
}

inline int
get_int4(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c + 3 < 65536);
    return p[c] << 24 | p[c + 1] << 16 | p[c + 2] << 8 | p[c + 3];
}

inline void
set_int4(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c + 3 < 65536);
    p[c] = x >> 24;
    p[c + 1] = x >> 16;
    p[c + 2] = x >> 8;
    p[c + 3] = x;
}

/*  The B-tree blocks have a number of internal lengths and offsets held in 1, 2
    or 4 bytes. To make the coding a little clearer,
       we use  for
       ------  ---
       K1      the 1 byte length of key
       I2      the 2 byte length of an item (key-tag pair)
       I3      the two byte item length followed by the 1 byte key kength
       D2      the 2 byte offset to the item from the directory
       C2      the 2 byte counter that ends each key and begins each tag
*/

#define K1 1
#define I2 2
#define I3 3
#define D2 2
#define C2 2

/*  and when setting or getting K1, I2, D2, C2 we use SETK, GETK ... defined as: */

#define GETK(p, c)    GETINT1(p, c)
#define SETK(p, c, x) SETINT1(p, c, x)
//#define GETI(p, c)    GETINT2(p, c)
#define SETI(p, c, x) SETINT2(p, c, x)
//#define GETD(p, c)    GETINT2(p, c)
#define SETD(p, c, x) SETINT2(p, c, x)
//#define GETC(p, c)    GETINT2(p, c)
#define SETC(p, c, x) SETINT2(p, c, x)

/* A B-tree comprises (a) a base file, containing essential information (Block
   size, number of the B-tree root block etc), (b) a bitmap, the Nth bit of the
   bitmap being set if the Nth block of the B-tree file is in use, and (c) a
   file DB containing the B-tree proper. The DB file is divided into a sequence
   of equal sized blocks, numbered 0, 1, 2 ... some of which are free, some in
   use. Those in use are arranged in a tree.

   Each block, b, has a structure like this:

     R L M T D o1 o2 o3 ... oN <gap> [item] .. [item] .. [item] ...
     <---------- D ----------> <-M->

   And then,

   R = REVISION(b)  is the revision number the B-tree had when the block was
                    written into the DB file.
   L = GET_LEVEL(b) is the level of the block, which is the number of levels
                    towards the root of the B-tree structure. So leaf blocks
                    have level 0 and the one root block has the highest level
                    equal to the number of levels in the B-tree.
   M = MAX_FREE(b)  is the size of the gap between the end of the directory and
                    the first item of data. (It is not necessarily the maximum
                    size among the bits of space that are free, but I can't
                    think of a better name.)
   T = TOTAL_FREE(b)is the total amount of free space left in b.
   D = DIR_END(b)   gives the offset to the end of the directory.

   o1, o2 ... oN are a directory of offsets to the N items held in the block.
   The items are key-tag pairs, and as they occur in the directory are ordered
   by the keys.

   An item has this form:

           I K key x C tag
             <--K-->
           <------I------>

   A long tag presented through the API is split up into C tags small enough to
   be accommodated in the blocks of the B-tree. The key is extended to include
   a counter, x, which runs from 1 to C. The key is preceded by a length, K,
   and the whole item with a length, I, as depicted above.

   Here are the corresponding definitions:

*/

#define REVISION(b)      static_cast<unsigned int>(get_int4(b, 0))
#define GET_LEVEL(b)     GETINT1(b, 4)
#define MAX_FREE(b)      GETINT2(b, 5)
#define TOTAL_FREE(b)    GETINT2(b, 7)
#define DIR_END(b)       GETINT2(b, 9)
#define DIR_START        11

#define SET_REVISION(b, x)      set_int4(b, 0, x)
#define SET_LEVEL(b, x)         SETINT1(b, 4, x)
#define SET_MAX_FREE(b, x)      SETINT2(b, 5, x)
#define SET_TOTAL_FREE(b, x)    SETINT2(b, 7, x)
#define SET_DIR_END(b, x)       SETINT2(b, 9, x)

/** Flip to sequential addition block-splitting after this number of observed
 *  sequential additions (in negated form). */
#define SEQ_START_POINT (-10)

/** Even for items of at maximum size, it must be possible to get this number of
 *  items in a block */
#define BLOCK_CAPACITY 4



/* if you've been reading the comments from the top, the next four procedures
   will not cause any headaches.

   Recall that item has this form:

           i k
           | |
           I K key x C tag
             <--K-->
           <------I------>


   item_of(p, c) returns i, the address of the item at block address p,
   directory offset c,

   component_of(p, c) returns the number marked 'x' above,

   components_of(p, c) returns the number marked 'C' above,
*/

class Key {
    const byte *p;
public:
    explicit Key(const byte * p_) : p(p_) { }
    const byte * get_address() const { return p; }
    void read(string * key) const {
	key->assign(reinterpret_cast<const char *>(p + K1), length());
    }
    bool operator==(Key key2) const;
    bool operator!=(Key key2) const { return !(*this == key2); }
    bool operator<(Key key2) const;
    bool operator>=(Key key2) const { return !(*this < key2); }
    bool operator>(Key key2) const { return key2 < *this; }
    bool operator<=(Key key2) const { return !(key2 < *this); }
    int length() const {
	return GETK(p, 0) - C2 - K1;
    }
    char operator[](size_t i) const {
	return p[i + K1];
    }
};

// Item_wr wants to be "Item with non-const p and more methods" - we can't
// achieve that nicely with inheritance, so we use a template base class.
template <class T> class Item_base {
protected:
    T p;
public:
    /* Item from block address and offset to item pointer */
    Item_base(T p_, int c) : p(p_ + GETINT2(p_, c)) { }
    Item_base(T p_) : p(p_) { }
    T get_address() const { return p; }
    int size() const { return GETINT2(p, 0); } /* I in diagram above */
    int component_of() const {
	return GETINT2(p, GETK(p, I2) + I2 - C2);
    }
    int components_of() const {
	return GETINT2(p, GETK(p, I2) + I2);
    }
    Key key() const { return Key(p + I2); }
    void append_chunk(string * tag) const {
	/* number of bytes to extract from current component */
	int cd = GETK(p, I2) + I2 + C2;
	int l = size() - cd;
	tag->append(reinterpret_cast<const char *>(p + cd), l);
    }
    /** Get this item's tag as a block number (this block should not be at
     *  level 0).
     */
    uint4 block_given_by() const {
	return get_int4(p, size() - BYTES_PER_BLOCK_NUMBER);
    }
};

class Item : public Item_base<const byte *> {
public:
    /* Item from block address and offset to item pointer */
    Item(const byte * p_, int c) : Item_base<const byte *>(p_, c) { }
    Item(const byte * p_) : Item_base<const byte *>(p_) { }
};

class Item_wr : public Item_base<byte *> {
public:
    /* Item_wr from block address and offset to item pointer */
    Item_wr(byte * p_, int c) : Item_base<byte *>(p_, c) { }
    Item_wr(byte * p_) : Item_base<byte *>(p_) { }
    void set_component_of(int i) {
	SETINT2(p, GETK(p, I2) + I2 - C2, i);
    }
    void set_components_of(int m) {
	SETINT2(p, GETK(p, I2) + I2, m);
    }
    // Takes size as we may be truncating newkey.
    void set_key_and_block(Key newkey, int truncate_size, uint4 n) {
	int i = truncate_size;
	// Read the length now because we may be copying the key over itself.
	// FIXME that's stupid!  sort this out
	int newkey_len = newkey.length();
	int size = I2 + K1 + i + C2;
	// Item size (4 since tag contains block number)
	SETINT2(p, 0, size + 4);
	// Key size
	SETINT1(p, I2, size - I2);
	// Copy the main part of the key, possibly truncating.
	memmove(p + I2 + K1, newkey.get_address() + K1, i);
	// Copy the count part.
	memmove(p + I2 + K1 + i, newkey.get_address() + K1 + newkey_len, C2);
	// Set tag contents to block number
//	set_block_given_by(n);
	set_int4(p, size, n);
    }

    /** Set this item's tag to point to block n (this block should not be at
     *  level 0).
     */
    void set_block_given_by(uint4 n) {
	set_int4(p, size() - BYTES_PER_BLOCK_NUMBER, n);
    }
    /** Form an item with a null key and with block number n in the tag.
     */
    void form_null_key(uint4 n) {
	set_int4(p, I3, n);
	SETK(p, I2, K1);     /* null key */
	SETI(p, 0, I3 + 4);  /* total length */
    }
};

int sys_open_to_read(const string & name);
int sys_open_to_read_no_except(const string & name);
int sys_open_to_write(const string & name);
void sys_unlink_if_exists(const string &filename);
// Return true on success
inline bool sys_close(int h) {
    return close(h) == 0;
}

string sys_read_all_bytes(int h, size_t max);
void sys_write_string(int h, const string &s);
int sys_flush(int h);

inline byte *zeroed_new(size_t size)
{
    byte *temp = new byte[size];
    if (temp) memset(temp, 0, size);

    return temp;
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

#endif /* OM_HGUARD_BTREE_UTIL_H */
