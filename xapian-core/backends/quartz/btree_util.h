/* btree_util.h: common macros/functions in the Btree implementation.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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
   then the compiler violate ANSI?  Or am I misunderstanding...
*/

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
       D4      = 2 * D2 (used once!)
       C2      the 2 byte counter that ends each key and begins each tag
*/

#define K1 1
#define I2 2
#define I3 3
#define D2 2
#define D4 4
#define C2 2

/*  and when setting or getting K1, I2, D2, C2 we use SETK, GETK ... defined as: */

#define GETK(p, c)    GETINT1(p, c)
#define SETK(p, c, x) SETINT1(p, c, x)
#define GETI(p, c)    GETINT2(p, c)
#define SETI(p, c, x) SETINT2(p, c, x)
#define GETD(p, c)    GETINT2(p, c)
#define SETD(p, c, x) SETINT2(p, c, x)
#define GETC(p, c)    GETINT2(p, c)
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

           I K key c C tag
             <--K-->
           <------I------>

   A long tag presented through the API is split up into C tags small enough to
   be accommodated in the blocks of the B-tree. The key is extended to include
   a counter, c, which runs from 1 to C. The key is preceded by a length, K,
   and the whole item with a length, I, as depicted above.

   Here are the corresponding definitions:

*/

#define REVISION(b)      (unsigned int)get_int4(b, 0)
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

   key_of(p, c) returns address k, marked above.
*/

inline byte * item_of(byte * p, int c)
{
    c = GETD(p, c);
    return p + c;
}

inline int component_of(const byte * p, int c)
{
    p += GETD(p, c);
    p += GETK(p, I2) + I2 - C2;
    return GETC(p, 0);
}

inline int components_of(const byte * p, int c)
{
    p += GETD(p, c);
    p += GETK(p, I2) + I2;
    return GETC(p, 0);
}

inline byte * key_of(byte * p, int c)
{
    c = GETD(p, c);
    return p + c + I2;
}

inline const byte * key_of(const byte * p, int c)
{
    c = GETD(p, c);
    return p + c + I2;
}

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
