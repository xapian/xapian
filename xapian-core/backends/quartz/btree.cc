/* btree.cc: Btree implementation
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

/** The next 4 lines are needed to make fdatasync appear.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>   /* for calloc */
#include <string.h>   /* for memmove */
#include <limits.h>   /* for CHAR_BIT */
#include <fcntl.h>    /* O_RDONLY etc */
#include <unistd.h>
#include <errno.h>
#include "om/autoptr.h"

#include "btree.h"

#include "omassert.h"
#include <string>

/* The unit of access into the DB files is an unsigned char, which is defined
   as 'byte' with a typedef.

   Other integer values are built up from these bytes, either in pairs or fours.
   'int2' and 'int4' are signed types which will comfortably accommodate
   all 2 byte and 4 byte ranges:
*/

/*
   If a signed int cannot hold the full range of two bytes replace 'int' by
   'long' throughout the code.
*/


#define GETINT1(p, c)    ((p)[(c)])
#define SETINT1(p, c, x) (p)[(c)] = (x);

#define GETINT2(p, c)    ((p)[(c)]<<8 | (p)[(c)+1])
#define SETINT2(p, c, x) (p)[(c)] = (x)>>8; (p)[(c)+1] = (x)

#define GETINT4(p, c)    ((p)[(c)]<<24 | (p)[(c)+1]<<16 | (p)[(c)+2]<<8 | (p)[(c)+3])
#define SETINT4(p, c, x) (p)[(c)] = (x)>>24; (p)[(c)+1] = (x)>>16; (p)[(c)+2] = (x)>>8; (p)[(c)+3] = (x)

/* or generating less code: */

static int get_int4(byte * p, int c) { return GETINT4(p, c); }
static unsigned int get_uint4(byte * p, int c) { return (unsigned int)GETINT4(p, c); }
static void set_int4(byte * p, int c, int x) { SETINT4(p, c, x); }

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
   L = LEVEL(b)     is the level of the block, which is the number of levels
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

#define REVISION(b)      get_uint4(b, 0)
#define LEVEL(b)         GETINT1(b, 4)
#define MAX_FREE(b)      GETINT2(b, 5)
#define TOTAL_FREE(b)    GETINT2(b, 7)
#define DIR_END(b)       GETINT2(b, 9)
#define DIR_START        11

#define SET_REVISION(b, x)      set_int4(b, 0, x)
#define SET_LEVEL(b, x)         SETINT1(b, 4, x)
#define SET_MAX_FREE(b, x)      SETINT2(b, 5, x)
#define SET_TOTAL_FREE(b, x)    SETINT2(b, 7, x)
#define SET_DIR_END(b, x)       SETINT2(b, 9, x)

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

#define BIT_MAP_INC 1000
    /* increase the bit map by this number of bytes if it overflows */

#define SEQ_START_POINT (-10)
    /* Flip to sequential addition block-splitting after this number of observed
       sequential additions */

#define BLOCK_CAPACITY 4
    /* Even for items of at maximum size, it must be possible to get this number of
       items in a block */

#define TAG_CAPACITY 10
    /* However small the block, and however large the key, it must be possible to
       construct items with this number of bytes in the tag */

#define KEY_CAPACITY 3
    /* A small block size forces down the maximum size of key, but not below this
       value */


/*------debugging aids from here--------

static void print_bytes(int n, byte * p);
static void report_block_full(struct Btree * B, int m, int n, byte * p);
static int print_key(byte * p, int c, int j);

static void report_cursor(int N, struct Btree * B, struct Cursor * C)
{
    int i;
    printf("%d)\n", N);
    for (i = 0; i <= B->level; i++)
        printf("p=%d, c=%d, n=[%d], rewrite=%d\n",
                C[i].p, C[i].c, C[i].n, C[i].rewrite);
}

------to here--------*/

/* Input/output is defined with calls to the basic Unix system interface: */

static int valid_handle(int h) { return h >= 0; }

static int sys_open_to_read_no_except(const std::string & name)
{
    int fd = open(name.c_str(), O_RDONLY, 0666);
    return fd;
}

static int sys_open_to_read(const std::string & name)
{
    int fd = sys_open_to_read_no_except(name);
    if (fd < 0) {
	std::string message = std::string("Couldn't open ")
		+ name + " to read: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static int sys_open_to_write_no_except(const std::string & name)
{
    int fd = open(name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    return fd;
}

static int sys_open_to_write(const std::string & name)
{
    int fd = sys_open_to_write_no_except(name);
    if (fd < 0) {
	std::string message = std::string("Couldn't open ")
		+ name + " to write: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static int sys_open_for_readwrite(const std::string & name)
{
    int fd = open(name.c_str(), O_RDWR, 0666);
    if (fd < 0) {
	std::string message = std::string("Couldn't open ")
		+ name + " read/write: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static void sys_read_block(int h, int m, int4 n, byte * p)
{
    if (lseek(h, (off_t)m * n, SEEK_SET) == -1) {
	std::string message = "Error seeking to block: ";
	message += strerror(errno);
	throw OmDatabaseError(message);
    }
    ssize_t bytes_read;
    while (1) {
	bytes_read = read(h, (char *)p, m);
	if (bytes_read == m) {
	    // normal case - read succeeded, so return.
	    break;
	} else if (bytes_read == -1) {
	    std::string message = "Error reading block: ";
	    message += strerror(errno);
	    throw OmDatabaseError(message);
	} else if (bytes_read == 0) {
	    std::string message = "Error reading block: got end of file";
	    throw OmDatabaseError(message);
	} else if (bytes_read < m) {
	    /* Read part of the block, which is not an error.  We should
	     * continue reading the rest of the block.
	     */
	    m -= bytes_read;
	    p += bytes_read;
	}
    }
}

static void sys_write_block(int h, int m, int4 n, byte * p)
{
    if (lseek(h, (off_t)m * n, SEEK_SET) == -1) {
	std::string message = "Error seeking to block: ";
	message += strerror(errno);
	throw OmDatabaseError(message);
    }
    ssize_t bytes_written;
    while (1) {
	bytes_written = write(h, (char *)p, m);
	if (bytes_written == m) {
	    // normal case - read succeeded, so return.
	    break;
	} else if (bytes_written == -1) {
	    std::string message = "Error writing block: ";
	    message += strerror(errno);
	    throw OmDatabaseError(message);
	} else if (bytes_written == 0) {
	    std::string message = "Error writing block: wrote no data";
	    throw OmDatabaseError(message);
	} else if (bytes_written < m) {
	    /* Wrote part of the block, which is not an error.  We should
	     * continue writing the rest of the block.
	     */
	    m -= bytes_written;
	    p += bytes_written;
	}
    }
}

/* FIXME: implement sys_{read,write}_block in terms of
 * sys_{read,write}_bytes and put the same error logic into them.
 */
static int sys_read_bytes(int h, int n, byte * p)
{   return read(h, (char *)p, n) == n;
}

static int sys_write_bytes(int h, int n, byte * p)
{   return write(h, (char *)p, n) == n;
}

static int sys_flush(int h) {
#ifdef _POSIX_SYNCHRONIZED_IO
    fdatasync(h);
#else
    fsync(h);
#endif
    return true;
}

static int sys_close(int h) {
    return close(h) == 0;
}  /* 0 if success */

static void sys_unlink(const std::string &filename)
{
    int err = unlink(filename.c_str());
    if (err != 0) {
	std::string message = "Failed to unlink ";
	message += filename;
	message += ": ";
	message += strerror(errno);
	throw OmDatabaseCorruptError(message);
    }
}

/** Btree_strerror() converts Btree_errors values to more meaningful strings.
 */
std::string
Btree_strerror(Btree_errors err)
{
    switch (err) {
	case BTREE_ERROR_NONE:
	    return "no error";
	case BTREE_ERROR_BLOCKSIZE:
	    return "Bad block size";
	case BTREE_ERROR_SPACE:
	    return "Out of memory";
	case BTREE_ERROR_BASE_CREATE:
	    return "Error creating the base file";
	case BTREE_ERROR_BASE_DELETE:
	    return "Failed to delete the base file";
	case BTREE_ERROR_BASE_READ:
	    return "Failed to read the base file";
	case BTREE_ERROR_BASE_WRITE:
	    return "Failed to write to the base file";

	case BTREE_ERROR_BITMAP_CREATE:
	    return "Failed to create bitmap";
	case BTREE_ERROR_BITMAP_READ:
	    return "Failed to read bitmap";
	case BTREE_ERROR_BITMAP_WRITE:
	    return "Failed to write to bitmap";

	case BTREE_ERROR_DB_CREATE:
	    return "Failed to create database";
	case BTREE_ERROR_DB_OPEN:
	    return "Failed to open database";
	case BTREE_ERROR_DB_CLOSE:
	    return "Failed to close database";
	case BTREE_ERROR_DB_READ:
	    return "Failed to read database";
	case BTREE_ERROR_DB_WRITE:
	    return "Failed to write to database";

	case BTREE_ERROR_KEYSIZE:
	    return "Bad key size";
	case BTREE_ERROR_TAGSIZE:
	    return "Bad tag size";

	case BTREE_ERROR_REVISION:
	    return "Bad revision number";
    }
    return "Unknown error";
}

/* There are two bit maps in bit_map0 and bit_map. The nth bit of bitmap is 0
   if the nth block is free, otherwise 1. bit_map0 is the initial state of
   bitmap at the start of the current transaction.

   Note use of the limits.h values:
   UCHAR_MAX = 255, an unsigned with all bits set, and
   CHAR_BIT = 8, the number of bits per byte

   BYTE_PAIR_RANGE below is the smallest +ve number that can't be held in two
   bytes -- 64K effectively.
*/

#define BYTE_PAIR_RANGE (1 << 2 * CHAR_BIT)

/* Nearly all the defined procedures have 'struct Btree * B' as first argument.
   This is defined in Btree.h, and is the handle for the whole B-tree structure.

   block_free_at_start(B, n) is true iff (if and only if) block n was free at
   the start of the transaction on the B-tree.
*/

static int block_free_at_start(struct Btree * B, int4 n)
{   int i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    return (B->bit_map0[i] & bit) == 0;
}

/* free_block(B, n) causes block n to be marked free in the bit map.
   B->bit_map_low is the lowest byte in the bit map known to have a free bit
   set. Searching starts from there when looking for a free block.
*/

static void free_block(struct Btree * B, int4 n)
{   int i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    B->bit_map[i] &= ~ bit;

    if (B->bit_map_low > i &&
       (B->bit_map0[i] & bit) == 0) /* free at start */
        B->bit_map_low = i;
}

/* extend_bit_map(B) increases the size of the two bit maps in an obvious way.
   The bitmap file grows and shrinks as the DB file grows and shrinks in
   internal usage. But the DB file itself does not reduce in size, no matter
   how many blocks are freed.
*/

static void extend_bit_map(struct Btree * B)
{   int n = B->bit_map_size + BIT_MAP_INC;
    B->bit_map0 = (byte *)realloc(B->bit_map0, n);   /*?*/
    B->bit_map = (byte *)realloc(B->bit_map, n);     /*?*/
    {   int i;
        for (i = B->bit_map_size; i < n; i++)
        {   B->bit_map0[i] = 0;
            B->bit_map[i] = 0;
        }
    }
    B->bit_map_size = n;
}

/* next_free_block(B) returns the number of the next available free block in
   the bitmap, marking it as 'in use' before returning. More precisely, we get
   a block that is both free now (in bit_map) and was free at the beginning of
   the transaction on the B-tree (in bit_map0).

   Starting at bit_map_low we go up byte at a time until we find a byte with a
   free (zero) bit, and then go up that byte bit at a time. If the bit map has
   no free bits it is extended so that it will have.
*/

static int next_free_block(struct Btree * B)
{   int4 i;
    int x;
    for (i = B->bit_map_low;; i++)
    {   if (i >= B->bit_map_size) extend_bit_map(B);
        x = B->bit_map0[i] | B->bit_map[i];
        if (x != UCHAR_MAX) break;
    }
    {   int4 n = i * CHAR_BIT;
        int d = 0x1;
        while ((x & d) != 0) { d <<= 1; n++; }
        B->bit_map[i] |= d;   /* set as 'in use' */
        B->bit_map_low = i;
        return n;
    }
}

/** read_block(B, n, p) reads block n to address p in the DB file.
 */
static void read_block(struct Btree * B, int4 n, byte * p)
{
    /* Check that n is in range. */
    Assert(n >= 0);
    Assert(n / CHAR_BIT < B->bit_map_size);

    sys_read_block(B->handle, B->block_size, n, p);
    /** Previously, this would set B->error to BTREE_ERROR_DB_READ
     *  when sys_read_block() failed.  However, it now throws an
     *  exception, so we never get here.
    {  / * failure to read a block * /
       B->error = BTREE_ERROR_DB_READ;
    }
     */
}

/** write_block(B, n, p) writes block n to address p in the DB file.
 *  In writing we check to see if the DB file has as yet been modified. If not
 *  (so this is the first write) the old base is deleted. This prevents the
 *  possibility of it being openend subsequently as an invalid base.
 */
static void write_block(struct Btree * B, int4 n, byte * p)
{
    /* Check that n is in range. */
    Assert(n >= 0);
    Assert(n / CHAR_BIT < B->bit_map_size);

    /* don't write to non-free */;
    AssertParanoid(block_free_at_start(B, n));

    /* write revision is okay */
    AssertParanoid(REVISION(p) == B->next_revision);

    if (! B->Btree_modified) {
	// Things to do when we start a modification session.

        B->Btree_modified = true;

        if (B->both_bases) {
	    /* delete the old base */
            sys_unlink(B->name + "base" + B->other_base_letter);
	    /* This used to set B->error as below, but will now throw
	     * an exception if it fails.
		B->error = BTREE_ERROR_BASE_DELETE;
		// FIXME: must exit, otherwise we could cause a corrupt
		// database to be read.
	     */
        }
    }

    sys_write_block(B->handle, B->block_size, n, p);
    /* This used to set B->error as below, but will now throw
     * an exception if it fails.
        B->error = BTREE_ERROR_DB_WRITE;
     */
}


/* A note on cursors:

   Each B-tree level has a correponding array element C[j] in a cursor, C. C[0]
   is the leaf (or data) level, and C[B->level] is the root block level. Within
   a level j,

       C[j].p  addresses the block
       C[j].c  is the offset into the directory entry in the block
       C[j].n  is the number of the block at C[j].p

   A look up in the B-tree causes navigation of the blocks starting from the
   root. In each block, p,  we find an offset, c, to an item which gives the
   number, n, of the block for the next level. This leads to an array of values
   p,c,n which are held inside the cursor.

   Structure B has a built-in cursor, at B->C. But other cursors may be
   created. If BC is a created cursor, BC->C is the cursor in the sense given
   above, and BC->B is the handle for the B-tree again.
*/


static void set_overwritten(struct Btree * B)
{
    printf("overwritten set to true\n");  /* initial debbugging line */
    B->overwritten = true;
}

/* block_to_cursor(B, C, j, n) puts block n into position C[j] of cursor C,
   writing the block currently at C[j] back to disk if necessary. Note that

       C[j].rewrite

   is true iff C[j].n is different from block n in file DB. If it is false no
   rewriting is necessary.
*/

static void block_to_cursor(struct Btree * B, struct Cursor * C, int j, int4 n)
{   byte * p = C[j].p;
    if (n == C[j].n) return;
    if (C[j].rewrite) {
	write_block(B, C[j].n, p);
        C[j].rewrite = false;
    }
    read_block(B, n, p);
    if (B->overwritten == true) return;

    C[j].n = n;
    B->C[j].n = n; /* not necessarily the same (in B-tree read mode) */
    if (j < B->level) {
	if (REVISION(p) > REVISION(C[j + 1].p)) { /* unsigned comparison */
            set_overwritten(B);
            return;
        }
    }
    AssertEq(j, LEVEL(p));
}

/* set_block_given_by(p, c, n) finds the item at block address p, directory
   offset c, and sets its tag value to n. For blocks not at the data level,
   when LEVEL(p) > 0, the tag of an item is just the block number of another
   block in the B-tree structure.

   (The built in '4' below is the number of bytes per block number.)
*/

static void set_block_given_by(byte * p, int c, int4 n)
{   c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - 4;   /* c is an offset to a block number */
    SETINT4(p, c, n);
}

/* block_given_by(p, c) finds the item at block address p, directory offset c,
   and returns its tag value to as an integer.
*/

static int block_given_by(byte * p, int c)
{   c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - 4;   /* c is an offset to a block number */
    return GETINT4(p, c);
}

/* Btree_alter(B, C); is called when the B-tree is to be altered. It causes new
   blocks to be forced for the current set of blocks in the cursor.

   The point is that if a block at level 0 is to be altered it may get a new
   number. Then the pointer to this block from level 1 will need changing. So
   the block at level 1 needs altering and may get a new block number. Then the
   pointer to this block from level 2 will need changing ... and so on back to
   the root.

   The clever bit here is spotting the cases when we can make an early exit
   from this process. If C[j].rewrite is true, C[j+k].rewrite will be true for
   k = 1,2 ... We have been through all this before, and there is no need to do
   it again. If C[j].n was free at the start of the transaction, we can copy it
   back to the same place without violating the integrity of the B-tree. We
   don't then need a new n and can return. The corresponding C[j].rewrite may
   be true or false in that case.
*/

static void Btree_alter(struct Btree * B, struct Cursor * C)
{
    int j = 0;
    byte * p = C[j].p;
    while(true) {
	if (C[j].rewrite) return; /* all new, so return */
	C[j].rewrite = true;

	int4 n = C[j].n;
	if (block_free_at_start(B, n)) return;
	free_block(B, n);
	n = next_free_block(B);
	C[j].n = n;
	SET_REVISION(p, B->next_revision);

	if (j == B->level) return;
	j++;
	p = C[j].p;
	set_block_given_by(p, C[j].c, n);
    }
}

/* compare_keys(k1, k2) compares two keys pointed to by k1 and k2. (Think of
   them as the key part of two items, with the pointers addressing the length
   indicator at the beginning of the keys.) The result is <0, 0, or >0
   according as k1 precedes, is equal to, or follows k2. The comparison is for
   byte sequence collating order, taking lengths into account. So if the keys
   are made up of lower case ASCII letters we get alphabetical ordering.

   Now remember that items are added into the B-tree in fastest time when they
   are preordered by their keys. This is therefore the piece of code that needs
   to be followed to arrange for the preordering.
*/

static int compare_keys(byte * key, byte * k)
{   int key_len = GETK(key, 0);
    int k_len = GETK(k, 0);
    int k_smaller = k_len < key_len ? k_len : key_len;
    int i; for (i = K1; i < k_smaller; i++)
    {   int diff = (int) key[i] - k[i];
        if (diff != 0) return diff;
    }
    return key_len - k_len;
}

/* find_in_block(p, key, offset, c) searches for the key in the block at p.
   offset is D2 for a data block, and 0 for and index block, when the first key
   is dummy and never needs to be tested. What we get is the directory entry to
   the last key <= the key being searched for.

   The lookup is by binary chop, with i and j set to the left and right ends
   of the search area. In sequential addition, c will often be the answer, so
   we test the keys round c and move i and j towards c if possible.

*/

static int find_in_block(byte * p, byte * key, int offset, int c)
{   int i = DIR_START - offset;
    int j = DIR_END(p);

    if (c < j && i < c && compare_keys(key, p + GETD(p, c) + I2) >= 0) i = c;
    c += D2;
    if (c < j && i < c && compare_keys(key, p + GETD(p, c) + I2) < 0) j = c;

    while (j - i > D2)
    {   int k = i + (j - i)/D4*D2; /* mid way */
        int t = compare_keys(key, p + GETD(p, k) + I2);
        if (t < 0) j = k; else i = k;
    }
    return i;
}

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

static byte * item_of(byte * p, int c)
{   c = GETD(p, c);
    return p + c;
}

static int component_of(byte * p, int c)
{   p += GETD(p, c);
    p += GETK(p, I2) + I2 - C2;
    return GETC(p, 0);
}

static int components_of(byte * p, int c)
{   p += GETD(p, c);
    p += GETK(p, I2) + I2;
    return GETC(p, 0);
}

static byte * key_of(byte * p, int c)
{   c = GETD(p, c);
    return p + c + I2;
}

/* find(B, C) searches for the key of B->kt in the B-tree. Result is 1 if
   found, 0 otherwise. When 0, the B_tree cursor is positioned at the last key
   in the B-tree <= the search key. Goes to first (null) item in B-tree when
   key length == 0.

   (In this case, example debugging lines are shown commented. Debugging is easy
   with the help of the B-tree writing code included further down.)
*/

static int find(struct Btree * B, struct Cursor * C)
{   byte * p; int c;
    byte * k = B->kt + I2;
    int j;
    for (j = B->level; j > 0; j--)
    {   p = C[j].p;
        c = find_in_block(p, k, 0, C[j].c);
            /*  debug with e.g. report_block_full(B, j, C[j].n, p);  */
        C[j].c = c;
        block_to_cursor(B, C, j - 1, block_given_by(p, c));
        if (B->overwritten) return false;
    }
    p = C[0].p;
    c = find_in_block(p, k, D2, C[j].c);
            /*  and again report_block_full(B, j, C[j].n, p);  */
    C[0].c = c;
    if (c < DIR_START) return false;
    return compare_keys(B->kt + I2, key_of(p, c)) == 0;
}

/* compress(B, p) compresses the block at p by shuffling all the items up to
   the end. MAX_FREE(p) is then maximized, and is equal to TOTAL_FREE(p).
*/

static void compress(struct Btree * B, byte * p)
{   int e = B->block_size;
    byte * b = B->buffer;
    int dir_end = DIR_END(p);
    int c; for (c = DIR_START; c < dir_end; c += D2)
    {   int o = GETD(p, c);
        int l = GETI(p, o);
        e -= l;
        memmove(b + e, p + o, l); SETD(p, c, e);  /* reform in b */
    }
    memmove(p + e, b + e, B->block_size - e);  /* copy back */
    e -= dir_end;
    SET_TOTAL_FREE(p, e);
    SET_MAX_FREE(p, e);
}

/* form_null_key(b, n) forms in b a null key with block number n in the tag.
 */

static void form_null_key(byte * b, int4 n)
{    SETINT4(b, I3, n);
     SETK(b, I2, K1);     /* null key */
     SETI(b, 0, I3 + 4);  /* total length */
}


/* add_item needs declaring here, because it is used rcursively.
*/

static void add_item(struct Btree * B, struct Cursor * C, byte * kt, int j);

/* enter_key(B, C, j, kq, kp) is called after a block split. It enters in the
   block at level C[j] a separating key for the block at level C[j - 1]. The
   key itself is kp. kq is the preceding key, and at level 1 kq can be trimmed
   down to the first point of difference to kp for entry in C[j].

   This code looks longer than it really is. If j exceeds the number of B-tree
   levels the root block has split and we have to construct a new one, but this
   is a rare event.

   The key is constructed in b, with block number C[j - 1].n as tag, and this
   is added in with add_item. add_item may itself cause a block split, with a
   further call to enter_key. Hence the recursion.
*/

static void enter_key(struct Btree * B, struct Cursor * C, int j, byte * kq, byte * kp)
{
    if (j > B->level) {
	/* gain a level */
	B->level ++;

	/* check level overflow */
	AssertNe(B->level, BTREE_CURSOR_LEVELS);

	byte * q = (byte *)calloc(B->block_size, 1);      /*?*/
	if (q == 0) {
	    B->error = BTREE_ERROR_SPACE;
	    throw std::bad_alloc();
	}
	C[j].p = q;
	C[j].split_p = (byte *)calloc(B->block_size, 1);  /*?*/
	if (C[j].split_p == 0) {
	    B->error = BTREE_ERROR_SPACE;
	    throw std::bad_alloc();
	}
	C[j].c = DIR_START;
	C[j].n = next_free_block(B);
	C[j].rewrite = true;
	SET_REVISION(q, B->next_revision);
	SET_LEVEL(q, j);
	SET_DIR_END(q, DIR_START);
	compress(B, q);   /* to reset TOTAL_FREE, MAX_FREE */

	/* form a null key in b with a pointer to the old root */

	int4 old_root = C[j - 1].split_n;
	byte b[10]; /* 7 is exact */
	form_null_key(b, old_root);
	add_item(B, C, b, j);
    }
    {   /*  byte * p = C[j - 1].p;  -- see below */

        int4 n = C[j - 1].n;

        int kq_len = GETK(kq, 0);
        int i;
        byte b[UCHAR_MAX + 1];

        if (j > 1) i = GETK(kp, 0); else
        {   i = K1;
            while (i < kq_len && kq[i] == kp[i]) i++;
            i++;
        }
        memmove(b + I2, kp, i);
        SETK(b, I2, i);
        set_int4(b, i + I2, n);
        SETI(b, 0, i + I2 + 4);

        /* when j > 1 we can make the first key of block p null, but is it worth it?
           Other redundant keys still creep in. The code to do it is commented out
           here:
        */
        /*
        if (j > 1)
        {   int kp_len = GETK(kp, 0);
            int n = get_int4(kp, kp_len);
            int new_total_free = TOTAL_FREE(p) + (kp_len - K1);
            form_null_key(kp - I2, n);
            SET_TOTAL_FREE(p, new_total_free);
        }
        */

        C[j].c = find_in_block(C[j].p, b + I2, 0, 0) + D2;
        C[j].rewrite = true; /* a subtle point: this *is* required. */
        add_item(B, C, b, j);
    }
}

/* split_off(B, C, j, c, p, q) splits the block at p at directory offset c.
   In fact p is just C[j].p, and q is C[j].split_p, a block buffer provided at
   each cursor level to accommodate the split.

   The first half block goes into q, with block number in C[j].split_n copied
   from C[j].n, the second half into p with a new block number.
*/

static void split_off(struct Btree * B, struct Cursor * C, int j, int c, byte * p, byte * q)
{
    /* p is C[j].p, q is C[j].split_p */

    C[j].split_n = C[j].n;
    C[j].n = next_free_block(B);

    memmove(q, p, B->block_size);  /* replicate the whole block in q */
    SET_DIR_END(q, c);
    compress(B, q);      /* to reset TOTAL_FREE, MAX_FREE */

    {   int residue = DIR_END(p) - c;
        int new_dir_end = DIR_START + residue;
        memmove(p + DIR_START, p + c, residue);
        SET_DIR_END(p, new_dir_end);
    }
    compress(B, p);      /* to reset TOTAL_FREE, MAX_FREE */
}

/* mid_point(B, p) finds the directory entry in c that determines the
   approximate mid point of the data in the block at p.
 */

static int mid_point(struct Btree * B, byte * p)
{   int n = 0;
    int dir_end = DIR_END(p);
    int size = B->block_size - TOTAL_FREE(p) - dir_end;
    int c; for (c = DIR_START; c < dir_end; c += D2)
    {   int o = GETD(p, c);
        int l = GETI(p, o);
        n += 2 * l;
        if (n >= size)
        {   if (l < n-size) return c;
            return c + D2;
        }
    }

    /* falling out of mid_point */
    Assert(false);
    return 0; /* Stop compiler complaining about end of method. */
}

/* add_item_to_block(B, p, kt, c) adds item kt to the block at p. c is the
   offset in the directory that needs to be expanded to accommodate the new
   entry for the item. We know before this is called that there is enough room,
   so it's just a matter of byte shuffling.
*/

static void add_item_to_block(struct Btree * B, byte * p, byte * kt, int c)
{
    int dir_end = DIR_END(p);
    int kt_len = GETI(kt, 0);
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    int new_max = MAX_FREE(p) - needed;

    Assert(new_total >= 0);

    if (new_max < 0)
    {   compress(B, p);
        new_max = MAX_FREE(p) - needed;
	Assert(new_max >= 0);
    }
    Assert(dir_end >= c);

    memmove(p + c + D2, p + c, dir_end - c);
    dir_end += D2; SET_DIR_END(p, dir_end);
    {   int o = dir_end + new_max;
        SETD(p, c, o);
        memmove(p + o, kt, kt_len);
    }

    SET_MAX_FREE(p, new_max);

    SET_TOTAL_FREE(p, new_total);
}

/* add_item(B, C, kt, j) adds item kt to the block at cursor level C[j]. If
   there is not enough room the block splits and the item is then added to the
   appropriate half.
*/

static void add_item(struct Btree * B, struct Cursor * C, byte * kt, int j)
{
    byte * p = C[j].p;
    int c = C[j].c;
    int4 changed_n;

    int kt_len = GETI(kt, 0);
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    if (new_total < 0)
    {   int m;
        byte * q = C[j].split_p;
        if (B->seq_count < 0 /*|| j > 0*/ ) m = mid_point(B, p); else
        {   if (c < DIR_END(p))
            {   m = c - D2;
                /* splits at dirend-2 */
            }
            else
            {   m = c;
                /* splits at dirend. (This has all been cautiously tested) */
            }
        }
        split_off(B, C, j, m, p, q);
        if (c >= m)
        {   c -= (m - DIR_START);
            add_item_to_block(B, p, kt, c);
            changed_n = C[j].n;
        }
        else
        {   add_item_to_block(B, q, kt, c);
            changed_n = C[j].split_n;
        }
        write_block(B, C[j].split_n, q);

        enter_key(B, C, j + 1,                /* enters a separating key at level j + 1 */
                  key_of(q, DIR_END(q) - D2), /* - between the last key of block q, */
                  key_of(p, DIR_START));      /* - and the first key of block p */
    }
    else
    {   add_item_to_block(B, p, kt, c);
        changed_n = C[j].n;
    }
    if (j == 0) { B->changed_n = changed_n; B->changed_c = c; }
}

/* delete_item(B, C, j, repeatedly) is (almost) the converse of add_item. If
   repeatedly is true, the process repeats at the next level when a block has
   been completely emptied, freeing the block and taking out the pointer to it.
   Emptied root blocks are also removed, which reduces the number of levels in
   the B-tree.
*/

static void delete_item(struct Btree * B, struct Cursor * C, int j, int repeatedly)

{
    byte * p = C[j].p;
    int c = C[j].c;
    int o = GETD(p, c);              /* offset of item to be deleted */
    int kt_len = GETI(p, o);         /* - and its length */
    int dir_end = DIR_END(p) - D2;   /* directory length will go down by 2 bytes */

    memmove(p + c, p + c + D2, dir_end - c);
    SET_DIR_END(p, dir_end);
    {   int max_free = MAX_FREE(p) + D2;
        SET_MAX_FREE(p, max_free);
    }
    {   int total_free = TOTAL_FREE(p) + kt_len + D2;
        SET_TOTAL_FREE(p, total_free);
    }
    if (!repeatedly) return;
    if (j < B->level)
    {   if (dir_end == DIR_START)
        {
            free_block(B, C[j].n);
            C[j].rewrite = false;
            C[j].n = -1;
            C[j + 1].rewrite = true;  /* *is* necessary */
            delete_item(B, C, j + 1, true);
        }
    }
    else /* j == B->level */
    {   while (dir_end == DIR_START + D2 && j > 0)
        {   /* single item in the root block, so lose a level */
            int new_root = block_given_by(p, DIR_START);
            free(p); C[j].p = 0;
            free_block(B, C[j].n);
            C[j].rewrite = false;
            C[j].n = -1;
            free(C[j].split_p); C[j].split_p = 0;
            C[j].split_n = -1;
            B->level--;

            block_to_cursor(B, C, B->level, new_root);
	    if (B->overwritten) return;

            j--; p = C[j].p; dir_end = DIR_END(p); /* prepare for the loop */
        }
    }
}

/* debugging aid:
static addcount = 0;
*/

/* add_kt(found, B, C) adds the item (key-tag pair) at B->kt into the B-tree
   given by B, using cursor C. found == find(B, C) is handed over as a
   parameter from Btree_add. Btree_alter(B, C) prepares for the alteration to
   the B-tree. Then there are a number of cases to consider:

      If an item with the same key is in the B-tree (found is true), the new kt
      replaces it.

         If then kt is smaller, or the same size as, the item it replaces, kt
         is put in the same place as the item it replaces, and the TOTAL_FREE
         measure is reduced.

         If kt is larger than the item it replaces it is put in the MAX_FREE
         space if there is room, and the directory entry and space counts are
         adjusted accordingly.

            - But if there is not room we do it the long way: the old item is
            deleted with delete_item and kt is added in with add_item.

      If the key of kt is not in the B-tree (found is false), the new kt is
      added in with add_item.

*/

static int add_kt(int found, struct Btree * B, struct Cursor * C)
{
    int components = 0;
    byte * kt = B->kt;

    if (B->overwritten) return 0;

    /*
    {
        printf("%d) %s ", addcount++, (found ? "replacing " : "adding "));
        print_bytes(kt[I2] - K1 - C2, kt + I2 + K1); printf("\n");
    }
    */
    Btree_alter(B, C);

    if (found)  /* replacement */
    {   B->seq_count = SEQ_START_POINT;
        {   byte * p = C[0].p;
            int c = C[0].c;
            int o = GETD(p, c);
            int kt_size = GETI(kt, 0);
            int needed = kt_size - GETI(p, o);

            components = components_of(p, c);

            if (needed <= 0)   /* simple replacement */
            {   memmove(p + o, kt, kt_size);
                {   int new_total = TOTAL_FREE(p) - needed;
                    SET_TOTAL_FREE(p, new_total);
                }
            }
            else   /* new item into the block's freespace */
            {   int new_max = MAX_FREE(p) - kt_size;
                if (new_max >= 0)
                {   o = DIR_END(p) + new_max;
                    memmove(p + o, kt, kt_size);
                    SETD(p, c, o);
                    SET_MAX_FREE(p, new_max);
                    {   int new_total = TOTAL_FREE(p) - needed;
                        SET_TOTAL_FREE(p, new_total);
                    }
                }
                else   /* do it the long way */
                {   delete_item(B, C, 0, false);
                    add_item(B, C, B->kt, 0);
                }
            }
        }
    }
    else  /* addition */
    {
        if (B->changed_n == C[0].n && B->changed_c == C[0].c)
            { if (B->seq_count < 0) B->seq_count++; }
        else
            B->seq_count = SEQ_START_POINT;
        C[0].c += D2;
        add_item(B, C, B->kt, 0);
    }
    return components;
}

/* delete_kt(B, C) corresponds to add_kt(B, C), but there are only two cases: if
   the key is not found nothing is done, and if it is found the corresponding
   item is deleted with delete_item.
*/

static int delete_kt(struct Btree * B, struct Cursor * C)
{
    int found = find(B, C);
    if (B->overwritten) return 0;

    int components = 0;
    B->seq_count = SEQ_START_POINT;

    /*
    {
        printf("%d) %s ", addcount++, (found ? "deleting " : "ignoring "));
        print_bytes(B->kt[I2] - K1 - C2, B->kt + I2 + K1); printf("\n");
    }
    */
    if (found)
    {
        components = components_of(C[0].p, C[0].c);
        Btree_alter(B, C);
        delete_item(B, C, 0, true);
    }
    return components;
}

/* form_key(B, p, key, key_len) treats address p as an item holder and fills in
the key part:

           (I) K key c (C tag)

The bracketed parts are left blank. The key is filled in with key_len bytes and
K set accordingly. c is set to 1.
*/

static void form_key(struct Btree * B, byte * p, byte * key, int key_len)
{
    if (key_len > B->max_key_len)
    {
        key_len = B->max_key_len;  /* just to keep things working */
	/* FIXME: handle this higher up in the code. */
        B->error = BTREE_ERROR_KEYSIZE;
    }
    {   int c = I2;
        SETK(p, c, key_len + K1 + C2); c += K1;
        memmove(p + c, key, key_len); c += key_len;
        SETC(p, c, 1);
    }
}

/* Btree_add(B, key, key_len, tag, tag_len) adds the key/tag item to the
   B-tree, replacing any existing item with the same key. The result is 1 for
   an addition, 0 for a replacement.

   For a long tag, we end end up having to add m components, of the form

       key 1 m tag1
       key 2 m tag2
       ...
       key m m tagm

   and tag1+tag2+...+tagm are equal to tag. These is their turn may be replacing
   n components of the form

       key 1 n TAG1
       key 2 n TAG2
       ...
       key n n TAGn

   and n may be greater than, equal to, or less than m. These cases are dealt
   with in the code below. If m < n for example, we end up with a series of
   deletions.
*/

extern int Btree_add(struct Btree * B, byte * key, int key_len,
                                       byte * tag, int tag_len)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = B->C;
    byte * kt = B->kt;

    form_key(B, kt, key, key_len);

    {
        int ck = GETK(kt, I2) + I2 - C2;  /* offset to the counter in the key */
        int ct = ck + C2;                 /* offset to the tag counter */
        int cd = ct + C2;                 /* offset to the tag data */
        int L = B->max_item_size - cd;    /* largest amount of tag data for any tagi */

        int first_L = L;                  /* - amount for tag1 */
        int found = find(B, C);
        if (B->full_compaction && !found)
        {   byte * p = C[0].p;
            int n = TOTAL_FREE(p) % (B->max_item_size + D2) - D2 - cd;
            if (n > 0) first_L = n;
        }
        {
            int m = tag_len == 0 ? 1 :        /* a null tag must be added in of course */
                    (tag_len - first_L + L - 1) / L + 1;
                                              /* there are m items to add */
            int n;                            /* - and there will be n to delete */
            int o = 0;                        /* offset into the tag */
            int residue = tag_len;            /* bytes of the tag remaining to add in */
            int replacement = false;          /* has there been a replacement ? */
            int i;
	    /* FIXME: sort out this error higher up and turn this into
	     * an assert.
	     */
            if (m >= BYTE_PAIR_RANGE) { B->error = BTREE_ERROR_TAGSIZE; return 0; }
            for (i = 1; i <= m; i++)
            {
                int l = i == m ? residue :
                        i == 1 ? first_L : L;
                memmove(kt + cd, tag + o, l); o += l; residue -= l;
                SETC(kt, ck, i);
                SETC(kt, ct, m);
                SETI(kt, 0, cd + l);
                if (i > 1) found = find(B, C);
                n = add_kt(found, B, C); if (n > 0) replacement = true;
            }
            /* o == tag_len here, and n may be zero */
            for (i = m + 1; i <= n; i++)
            {   SETC(kt, ck, i);
                delete_kt(B, C);

                if (B->overwritten) return 0;

            }
            if (replacement) return 0;
            B->item_count++; return 1;
        }
    }
}

/* Btree_delete(B, key, key_len) returns 0 if the key is not in the B-tree,
   otherwise deletes it and returns 1.

   Again, this is parallel to Btree-add, but simpler in form.
*/

extern int Btree_delete(struct Btree * B, byte * key, int key_len)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = B->C;
    byte * kt = B->kt;

    if (key_len == 0) return 0;
    form_key(B, kt, key, key_len);

    {   int n = delete_kt(B, C);  /* there are n items to delete */
        int i; for (i = 2; i <= n; i++)
        {
            int c = GETK(kt, I2) + I2 - C2;
            SETC(kt, c, i);

            delete_kt(B, C);

            if (B->overwritten) return 0;

        }
        if (n > 0) { B->item_count--; return 1; }
        return 0;
    }

}

extern int Btree_find_key(struct Btree * B, byte * key, int key_len)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = B->C;
    form_key(B, B->kt, key, key_len);
    return find(B, C);
}

extern struct Btree_item * Btree_item_create(void)
{
    struct Btree_item * item = (struct Btree_item *) calloc(1, sizeof(struct Btree_item));
    if (item == 0) return 0;
    item->key_size = -1;
    item->tag_size = -1;
    return item;
}

extern int Btree_find_tag(struct Btree * B, byte * key, int key_len, struct Btree_item * item)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = B->C;
    byte * kt = B->kt;

    form_key(B, kt, key, key_len);
    if (!find(B, C)) return 0;
    {   int n = components_of(C[0].p, C[0].c);
                                        /* n components to join */
        int ck = GETK(kt, I2) + I2 - C2;/* offset to the key counter */
        int cd = ck + 2 * C2;           /* offset to the tag data */
        int o = 0;                      /* cursor into item->tag */
        int i = 1;                      /* see below */
        byte * p;                       /* pointer to current component */
        int l;                          /* number of bytes to extract from current component */

        p = item_of(C[0].p, C[0].c);
        l = GETI(p, 0) - cd;
        {   int4 space_for_tag = (int4) B->max_item_size * n;
            if (item->tag_size < space_for_tag)
            {   free(item->tag);
                item->tag = (byte *) calloc(space_for_tag + 5, 1);
                if (item->tag == 0) {
		    B->error = BTREE_ERROR_SPACE;
		    throw std::bad_alloc();
		}
                item->tag_size = space_for_tag + 5;
            }
        }
        while(true)
        {
	    Assert(o + l <= item->tag_size);

            memmove(item->tag + o, p + cd, l); o += l;
            if (i == n) break;
            i++; SETC(kt, ck, i);
            find(B, C);

            if (B->overwritten) return 0;

            p = item_of(C[0].p, C[0].c);
            l = GETI(p, 0) - cd;
        }
        item->tag_len = o;
    }
    return 1;
}

extern void Btree_item_lose(struct Btree_item * item)
{
    free(item->key); free(item->tag);
    free(item);
}

extern void Btree_full_compaction(struct Btree * B, int parity)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (parity) B->seq_count = 0;
    B->full_compaction = parity;
}

/************ B-tree opening and closing ************/

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

static byte * read_base(const std::string & name, char ch,
			std::string &err_msg)
{
    int h = sys_open_to_read_no_except(name + "base" + ch);
    byte w[2];
    byte * p;
    int size;
    if ( ! valid_handle(h)) {
	err_msg += "Couldn't open " + name + "base" +
		ch + ": " + strerror(errno) + "\n";
	return 0;
    }
    if (! sys_read_bytes(h, 2, w)) {
	err_msg += "Couldn't read from " + name + "base" + ch +
		": " + strerror(errno) + "\n";
	sys_close(h);
	return 0;
    }
    size = GETINT2(w, 0);
    p = (byte *)malloc(size);
    if (p != 0) {
	SETINT2(p, 0, size);
	if (sys_read_bytes(h, size - 2, p + 2) &&
	    get_int4(p, B_REVISION) == get_int4(p, B_REVISION2)) {
	    sys_close(h);
	    return p;
	} else {
	    err_msg += "Couldn't read revision number from " +
		    name + "base" + ch + ": " + strerror(errno) + "\n";
	}
    } else {
	sys_close(h);
	throw std::bad_alloc();
    }

    free(p);
    sys_close(h);
    return 0;
}

static byte * read_bit_map(const std::string & name, char ch, int size)
{
    byte * p = (byte *)malloc(size);
    if (p != 0) {
	int h = sys_open_to_read_no_except(name + "bitmap" + ch);
	if (valid_handle(h) &&
	    sys_read_bytes(h, size, p) &&
	    sys_close(h)) {
	    return p;
	}
    }
    free(p); return 0;
}

static int write_bit_map(struct Btree * B)
{
    int h = sys_open_to_write(B->name + "bitmap" + B->other_base_letter);
    return valid_handle(h) &&
	    sys_write_bytes(h, B->bit_map_size, B->bit_map) &&
	    sys_flush(h) &&
	    sys_close(h);
}

static int write_base(struct Btree * B)
{
    int h = sys_open_to_write(B->name + "base" + B->other_base_letter);
    return valid_handle(h) &&
	    sys_write_bytes(h, GETINT2(B->base, 0), B->base) &&
	    sys_flush(h) &&
	    sys_close(h);
}

static struct Btree * basic_open(const char * name_,
				 int revision_supplied,
				 uint4 revision)
{
    struct Btree * B = new struct Btree;
    Assert(B != 0);

    int ch = 'X'; /* will be 'A' or 'B' */

    B->name = name_;

    {
	std::string err_msg;
	byte * baseA = read_base(B->name, 'A', err_msg);
        byte * baseB = read_base(B->name, 'B', err_msg);
        byte * base;
        byte * other_base;

        if (baseA != 0 && baseB != 0) B->both_bases = true;
        if (baseA == 0 && baseB == 0) {
	    std::string message = "Error opening table `"; 
	    message += name_;
	    message += "': ";
	    message += err_msg;
	    /*  FIXME: use a smart pointer, or turn this into a
	     *  constructor. */
	    delete B;
	    throw OmOpeningError(message);
	}

        if (revision_supplied) {
	    if (baseA != 0 && get_uint4(baseA, B_REVISION) == revision) {
		ch = 'A';
	    } else if (baseB != 0 &&
		       get_uint4(baseB, B_REVISION) == revision) {
		ch = 'B';
	    } else {
		/* Couldn't open the revision that was asked for.
		 * This shouldn't throw an exception, but should just return
		 * 0 to upper levels.
		 */
		free(baseA);
		free(baseB);
		delete B;
		return 0;
	    }
        } else {
	    if (baseA == 0) ch = 'B'; else
            if (baseB == 0) ch = 'A'; else
            ch = get_int4(baseA, B_REVISION) > get_int4(baseB, B_REVISION) ? 'A' : 'B';
                                  /* unsigned comparison */
        }

        if (ch == 'A') {
	    base = baseA;
	    other_base = baseB;
	} else if (ch == 'B') {
	    base = baseB;
	    other_base = baseA;
	} else {
	    Assert(false);
	    B->error = BTREE_ERROR_BASE_READ;
	    free (baseA);
	    free (baseB);
	    delete B;
	}

        /* base now points to the most recent base block */

        B->base = base;

        B->revision_number = get_int4(base, B_REVISION);
        B->block_size =      get_int4(base, B_BLOCK_SIZE);
        B->root =            get_int4(base, B_ROOT);
        B->level =           get_int4(base, B_LEVEL);
        B->bit_map_size =    get_int4(base, B_BIT_MAP_SIZE);
        B->item_count =      get_int4(base, B_ITEM_COUNT);
        B->last_block =      get_int4(base, B_LAST_BLOCK);
        B->faked_root_block = (GETINT1(base, B_HAVE_FAKEROOT) != 0);

        if (other_base != 0) {
	    B->other_revision_number = get_int4(other_base, B_REVISION);
            free(other_base);
        }
    }

    /* k holds contructed items as well as keys */
    B->kt = (byte *)calloc(1, B->block_size);
    if (B->kt == 0) goto no_space;

    B->max_item_size = (B->block_size - DIR_START - BLOCK_CAPACITY * D2) / BLOCK_CAPACITY;

    /* This upper limit corresponds to K1 == 1 */
    B->max_key_len = UCHAR_MAX - K1 - C2;

    {
	int max = B->max_item_size - I3 - C2 - C2 - TAG_CAPACITY;

        /* This limit would come into effect with large keys in a B-tree with a
	   small block size.
        */

        if (B->max_key_len > max) B->max_key_len = max;
    }

    /* ready to open the main file */

    B->base_letter = ch;
    B->next_revision = B->revision_number + 1;

    return B;
no_space:
    Btree_quit(B); return 0;
}

static void read_root(struct Btree * B, struct Cursor * C)
{
    if (B->faked_root_block) {
	/* root block for an unmodified database. */
        int o = B->block_size - C2;
        byte * p = C[0].p;

	/* clear block - shouldn't be neccessary, but is a bit nicer,
	 * and means that the same operations should always produce
	 * the same database. */
	memset(p, 0, B->block_size);
	
        SETC(p, o, 1); o -= C2;        /* number of components in tag */
        SETC(p, o, 1); o -= K1;        /* component one in key */
        SETK(p, o, K1 + C2); o -= I2;  /* null key length */
        SETI(p, o, I3 + 2 * C2);       /* length of the item */
        SETD(p, DIR_START, o);         /* its directory entry */
        SET_DIR_END(p, DIR_START + D2);/* the directory size */

        o -= (DIR_START + D2);
        SET_MAX_FREE(p, o);
        SET_TOTAL_FREE(p, o);
	SET_LEVEL(p, 0);

        if (B->bit_map0 == 0) {
	    /* reading - revision number doesn't matter as long as it's 
	     * not greater than the current one.*/
            SET_REVISION(p, 0);
            C[0].n = 0;
        } else {
	    /* writing - */
            SET_REVISION(p, B->next_revision);
            C[0].n = next_free_block(B);
        }
    } else {
	/* using a root block stored on disk */
	block_to_cursor(B, C, B->level, B->root);
	if (B->overwritten) return;

	if (REVISION(C[B->level].p) >= B->next_revision) set_overwritten(B);
	/* although this is unlikely */
    }
}

static struct Btree * open_to_write(const char * name, int revision_supplied, uint4 revision)
{
    /* FIXME: do the exception safety the right way, by making all the
     * parts into sensible objects.
     */
    AutoPtr<Btree> B(basic_open(name, revision_supplied, revision));
    if (B.get() == 0 || B->error) {
	if (!revision_supplied) {
	    std::string message = "Failed to open for writing";
	    if (B.get()) {
		message += std::string(": ");
		message += Btree_strerror(B->error);
	    }
	    throw OmOpeningError(message);
	} else {
	    /* When the revision is supplied, it's not an exceptional
	     * case when open failed.  We should just return 0 here
	     * instead.
	     */
	    return 0;
	}
    }

    B->handle = sys_open_for_readwrite(B->name + "DB");

    B->bit_map0 = read_bit_map(B->name, B->base_letter, B->bit_map_size);
    if (B->bit_map0 == 0) {
	std::string message = "Failed to read bit map from table ";
	message += B->name;

	B->error = BTREE_ERROR_BITMAP_READ;
	throw OmOpeningError(message);
    }

    B->bit_map = (byte *) malloc(B->bit_map_size);
    if (B->bit_map == 0) {
	throw std::bad_alloc();
    }
    memmove(B->bit_map, B->bit_map0, B->bit_map_size);

    {   struct Cursor * C = B->C;
        int j; for (j = 0; j <= B->level; j++)
        {   C[j].n = -1;
            C[j].split_n = -1;
            C[j].p = (byte *)malloc(B->block_size);
            if (C[j].p == 0) {
		throw std::bad_alloc();
	    }
            C[j].split_p = (byte *)malloc(B->block_size);
            if (C[j].split_p == 0) {
		throw std::bad_alloc();
	    }
        }
        read_root(B.get(), C);
    }

    B->buffer = (byte *)calloc(1, B->block_size);
    if (B->buffer == 0) {
	throw std::bad_alloc();
    }

    B->other_base_letter = B->base_letter == 'A' ? 'B' : 'A'; /* swap for writing */

    B->changed_n = -1;
    B->seq_count = SEQ_START_POINT;

    return B.release();
}

extern struct Btree * Btree_open_to_write(const char * name)
{
    return open_to_write(name, false, 0);
}

extern struct Btree * Btree_open_to_write_revision(const char * name, uint4 n)
{
    return open_to_write(name, true, n);
}

extern void Btree_quit(struct Btree * B)
{
    delete B;
}

Btree::~Btree() {
    if (valid_handle(handle)) {
	// If an error occurs here, we just ignore it, since we're just
	// trying to free everything.
	sys_close(handle);
	handle = -1;
    }

    for (int j = level; j >= 0; j--) {
        free(C[j].p);
        free(C[j].split_p);
    }

    free(kt); free(buffer);
    free(bit_map); free(bit_map0); free(base);
}

extern int Btree_close(struct Btree * B_, uint4 revision)
{
    /* Automatically destroy the Btree when exiting by any
     * means.
     */
    AutoPtr<Btree> B(B_);

    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = B->C;
    int j;
    Btree_errors error = BTREE_ERROR_REVISION;
    if (revision < B->next_revision) {
	/* FIXME: should Btree_close() throw exceptions (so as it's
	 * likely to be called from destructors they'll need to catch
	 * the exception), or return an error value?  (So we need to
	 * trap errors here and convert them)
	 * I think it should throw an exception, but need to discuss.
	 *  -CME  2000-11-16
	 */
	return error; /* revision too low */
    }

    for (j = B->level; j >= 0; j--)
    {
        if (C[j].rewrite)
        {   write_block(B.get(), C[j].n, C[j].p);
            if (B->error) {
		return error;
	    }
        }
    }

    error = BTREE_ERROR_DB_CLOSE;
    if ( ! (sys_flush(B->handle) &&
            sys_close(B->handle))) {
	B->handle = -1;
	return error;
    }

    error = BTREE_ERROR_BITMAP_WRITE;
    {   int i = B->bit_map_size - 1;
        while (B->bit_map[i] == 0 && i > 0) i--;
        B->bit_map_size = i + 1;

        {   int x = B->bit_map[i];
            int4 n = (i + 1) * CHAR_BIT - 1;
            int d = 0x1 << (CHAR_BIT - 1);
            while ((x & d) == 0) { d >>= 1; n--; }
            B->last_block = n;
        }
    }

    B->faked_root_block &= ! B->Btree_modified; /* still faked? */
    if (B->faked_root_block) B->bit_map[0] = 0; /* if so, dummy bit map */

    if (! write_bit_map(B.get())) {
	return error;
    }

    error = BTREE_ERROR_BASE_WRITE;
    set_int4(B->base, B_REVISION, revision);
    set_int4(B->base, B_REVISION2, revision);
    set_int4(B->base, B_ROOT, C[B->level].n);
    set_int4(B->base, B_LEVEL, B->level);
    set_int4(B->base, B_BIT_MAP_SIZE, B->bit_map_size);
    set_int4(B->base, B_ITEM_COUNT, B->item_count);
    set_int4(B->base, B_LAST_BLOCK, B->last_block);
    SETINT1(B->base, B_HAVE_FAKEROOT, B->faked_root_block ? 1 : 0);

    if (! write_base(B.get())) {
	return error;
    }

    error = BTREE_ERROR_NONE;

    return error;
    /* B destroyed here */
}

/************ B-tree reading ************/

struct Bcursor {

    int positioned;    /* false initially, and after the cursor has dropped
                          of either end of the list of items */
    struct Btree * B;
    struct Cursor * C;

    int shared_level; /* The value of shared_level in the Btree structure. */
};

static int prev(struct Btree * B, struct Cursor * C, int j);
static int next(struct Btree * B, struct Cursor * C, int j);

static int prev_for_revision_1(struct Btree * B, struct Cursor * C, int dummy);
static int next_for_revision_1(struct Btree * B, struct Cursor * C, int dummy);

static struct Btree * open_to_read(const char * name, int revision_supplied, uint4 revision)
{
    AutoPtr<Btree> B(basic_open(name, revision_supplied, revision));
    if (B.get() == 0 || B->error) {
	std::string message = "Failed to open table to read: ";
	if (B.get()) {
	    message += Btree_strerror(B->error);
	} else {
	    message += "unknown error";
	}
	throw OmOpeningError(message);
    }

    B->handle = sys_open_to_read(B->name + "DB");

    {
        int common_levels = B->revision_number <= 1 ? 1 : 2;
        B->shared_level = B->level > common_levels ? common_levels : B->level;
    }

    B->prev = B->revision_number <= 1 ? prev_for_revision_1 : prev;
    B->next = B->revision_number <= 1 ? next_for_revision_1 : next;

    {   struct Cursor * C = B->C;
        for (int j = B->shared_level; j <= B->level; j++) {
	    C[j].n = -1;
            C[j].p = (byte *)malloc(B->block_size);
            if (C[j].p == 0) {
		throw std::bad_alloc();
	    }
        }
        read_root(B.get(), C);
    }
    return B.release();
}

extern struct Btree * Btree_open_to_read(const char * name)
{
    return open_to_read(name, false, 0);
}

extern struct Btree * Btree_open_to_read_revision(const char * name, uint4 n)
{
    return open_to_read(name, true, n);
}

// FIXME: continue working through errors after this point
// when I'm more awake. JJJ
extern struct Bcursor * Bcursor_create(struct Btree * B)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Bcursor * BC = (struct Bcursor *) calloc(1, sizeof(struct Bcursor));
    // FIXME: throw std::bad_alloc() here?
    if (BC == 0) return 0;
    BC->B = B;
    BC->C = (struct Cursor *) calloc(B->level + 1, sizeof(struct Cursor));
    if (BC->C == 0) goto no_space;
    {
	BC->shared_level = B->shared_level;

	struct Cursor * C = BC->C;
        struct Cursor * C_of_B = B->C;
        int j; for (j = 0; j < B->shared_level; j++)
        {   C[j].n = -1;
            C[j].p = (byte *)malloc(B->block_size);
            if (C[j].p == 0) goto no_space;
        }
        for (j = B->shared_level; j <= B->level; j++)
        {   C[j].n = C_of_B[j].n;
            C[j].p = C_of_B[j].p;
        }
    }
    return BC;
no_space:
    B->error = BTREE_ERROR_SPACE; return BC;
}

extern void Bcursor_lose(struct Bcursor * BC)
{
    struct Cursor * C = BC->C;
    // Use the value of shared_level stored in the cursor rather than the
    // Btree, since the Btree might have been deleted already.
    int j; for (j = 0; j < BC->shared_level; j++) free(C[j].p);
    free(C);
    free(BC);
}

static void force_block_to_cursor(struct Btree * B, struct Cursor * C, int j)
{
    int n = C[j].n;
    if (n != B->C[j].n) {
        C[j].n = -1;
        block_to_cursor(B, C, j, n);
	if (B->overwritten) return;
    }
}

static int prev_for_revision_1(struct Btree * B, struct Cursor * C, int dummy)
{   byte * p = C[0].p;
    int c = C[0].c;
    if (c == DIR_START)
    {
        int n = C[0].n;
        while(true)
        {   n--;
            if (n < 0) return false;
            read_block(B, n, p);
            if (B->overwritten == true) return false;
            if (REVISION(p) > 1) { B->overwritten = true; return false; }
            if (LEVEL(p) == 0) break;
        }
        c = DIR_END(p);
        C[0].n = n;
    }
    c -= D2;
    C[0].c = c;
    return true;
}

static int next_for_revision_1(struct Btree * B, struct Cursor * C, int dummy)
{   byte * p = C[0].p;
    int c = C[0].c;
    c += D2;
    if (c == DIR_END(p))
    {
        int n = C[0].n;
        while(true)
        {   n++;
            if (n > B->last_block) return false;
            read_block(B, n, p);
            if (B->overwritten == true) return false;
            if (REVISION(p) > 1) { B->overwritten = true; return false; }
            if (LEVEL(p) == 0) break;
        }
        c = DIR_START;
        C[0].n = n;
    }
    C[0].c = c;
    return true;
}

static int prev(struct Btree * B, struct Cursor * C, int j)
{   byte * p = C[j].p;
    int c = C[j].c;
    if (c == DIR_START)
    {   if (j == B->level) return false;

        if (j + 1 >= B->shared_level)
        {   force_block_to_cursor(B, C, j + 1);
            if (B->overwritten) return false;
        }
        if (! prev(B, C, j + 1)) return false;

        c = DIR_END(p);
    }
    c -= D2;
    C[j].c = c;
    if (j > 0) {
	block_to_cursor(B, C, j - 1, block_given_by(p, c));
        if (B->overwritten) return false;
    }
    return true;
}

static int next(struct Btree * B, struct Cursor * C, int j)
{   byte * p = C[j].p;
    int c = C[j].c;
    c += D2;
    if (c == DIR_END(p))
    {   if (j == B->level) return false;

        if (j + 1 >= B->shared_level)
        {   force_block_to_cursor(B, C, j + 1);
            if (B->overwritten) return false;
        }
        if (! next(B, C, j + 1)) return false;

        c = DIR_START;
    }
    C[j].c = c;
    if (j > 0) {
	block_to_cursor(B, C, j - 1, block_given_by(p, c));
        if (B->overwritten) return false;
    }
    return true;
}

extern int Bcursor_prev(struct Bcursor * BC)
{
    struct Btree * B = BC->B;

    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (! BC->positioned) return false;

    {   struct Cursor * C = BC->C;
        while(true)
        {   if (! B->prev(B, C, 0)) { BC->positioned = false; return false; }
            if (component_of(C[0].p, C[0].c) == 1) return true;
        }
    }
}

extern int Bcursor_next(struct Bcursor * BC)
{
    struct Btree * B = BC->B;

    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (! BC->positioned) return false;

    {   struct Cursor * C = BC->C;
        while(true)
        {   if (! B->next(B, C, 0)) { BC->positioned = false; return false; }
            if (component_of(C[0].p, C[0].c) == 1) return true;
        }
    }
}

extern int Bcursor_find_key(struct Bcursor * BC, byte * key, int key_len)
{
    struct Btree * B = BC->B;

    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    struct Cursor * C = BC->C;
    form_key(B, B->kt, key, key_len);
    {   int found = find(B, C);

        if (B->overwritten) return false;

	if (! found) {
	    if (C[0].c < DIR_START) {
		C[0].c = DIR_START;
		if (! B->prev(B, C, 0)) return false;
	    }
	    while (component_of(C[0].p, C[0].c) != 1) {
		if (! B->prev(B, C, 0)) return false;
	    }
	}
        BC->positioned = true;
        return found;
    }
}

extern int Bcursor_get_key(struct Bcursor * BC, struct Btree_item * item)
{
    AssertEq(BC->B->error, 0);
    Assert(!BC->B->overwritten);

    if (! BC->positioned) return false;

    {   struct Cursor * C = BC->C;
        byte * p = key_of(C[0].p, C[0].c);
        int l = GETK(p, 0) - K1 - C2;       /* number of bytes to extract */
        if (item->key_size < l)
        {   free(item->key);
            item->key = (byte *) calloc(l + 1, 1); /* 1 extra in case l == 0 */
            if (item->key == 0) { BC->B->error = BTREE_ERROR_SPACE; return false; }
            item->key_size = l + 1;
        }
        memmove(item->key, p + K1, l);
        item->key_len = l;
        return true;
    }
}

extern int Bcursor_get_tag(struct Bcursor * BC, struct Btree_item * item)
{
    AssertEq(BC->B->error, 0);
    Assert(!BC->B->overwritten);

    struct Btree * B = BC->B;
    if (! BC->positioned) return false;

    {   struct Cursor * C = BC->C;
        byte * p = item_of(C[0].p, C[0].c); /* pointer to current component */
        int ct = GETK(p, I2) + I2;          /* offset to the tag */

        int n = GETC(p, ct);                /* n components to join */

        int cd = ct + C2;                   /* offset to the tag data */

        int o = 0;                          /* cursor into item->tag */
        int i;                              /* see below */
        int l = GETI(p, 0) - cd;            /* number of bytes to extract from current
                                               component */

        {   int4 space_for_tag = (int4) B->max_item_size * n;
            if (item->tag_size < space_for_tag)
            {   free(item->tag);
                item->tag = (byte *) calloc(space_for_tag + 5, 1);
                if (item->tag == 0) { B->error = BTREE_ERROR_SPACE; return false; }
                item->tag_size = space_for_tag + 5;
            }
        }
        for (i = 1; i <= n; i++)
        {
	    Assert(o + l <= item->tag_size);

            memmove(item->tag + o, p + cd, l); o += l;
            BC->positioned = B->next(B, C, 0);

           if (B->overwritten) return false;

            p = item_of(C[0].p, C[0].c);
            l = GETI(p, 0) - cd;
        }
        item->tag_len = o;
        return BC->positioned;
    }
}

/*********** B-tree creating ************/

extern int Btree_create(const char * name_, int block_size)
{
    std::string name(name_);
    byte * b = 0;
    Btree_errors error;

    error = BTREE_ERROR_BLOCKSIZE;
    if (block_size > BYTE_PAIR_RANGE) goto end; /* block size too large (64K maximum) */

    if (block_size < DIR_START + BLOCK_CAPACITY * (D2 + I3 + KEY_CAPACITY + 2 * C2 + TAG_CAPACITY))
        goto end;  /* block size far too small */

    /* indeed it will need to be a good bit bigger */

    /* write initial values of to files */
    {
	/* create the bitmap file */
	error = BTREE_ERROR_BITMAP_CREATE;
        b = (byte *)calloc(1, 1);
        if (b == 0) goto end;
        b[0] = 0;  /* set the root block as 'in use' */
        {
	    int h = sys_open_to_write(name + "bitmapA");
            if ( ! (valid_handle(h) &&
                    sys_write_bytes(h, 1, b) &&
                    sys_close(h))) goto end;
        }
        free(b);

	/* create the base file */
        error = BTREE_ERROR_BASE_CREATE;
        b = (byte *)calloc(B_SIZE, 1);
        if (b == 0) goto end;
        SETINT2(b, 0, B_SIZE);
        set_int4(b, B_BLOCK_SIZE, block_size);
        set_int4(b, B_BIT_MAP_SIZE, 1);
	SETINT1(b, B_HAVE_FAKEROOT, 1);
        {
	    int h = sys_open_to_write(name + "baseA");
            if ( ! (valid_handle(h) &&
                    sys_write_bytes(h, B_SIZE, b) &&
                    sys_close(h))) goto end;
        }

	/* create the main file */
        error = BTREE_ERROR_DB_CREATE;
        {
	    int h = sys_open_to_write(name + "DB");      /* - null */
            if ( ! (valid_handle(h) &&
                    sys_close(h))) goto end;
        }
    }
    error = BTREE_ERROR_NONE;
end:
    free(b); return error;
}

/*********** B-tree checking ************/

static int block_free_now(struct Btree * B, int4 n)
{   int4 i = n / CHAR_BIT;
    int bit = 0x1 << n % CHAR_BIT;
    return (B->bit_map[i] & bit) == 0;
}

static void print_bytes(int n, byte * p)
{   int i;
    for (i = 0; i < n; i++) printf("%c", p[i]);
}

static void print_key(byte * p, int c, int j)
{   byte * k = key_of(p, c);
    int l = GETK(k, 0);

    if (j == 0)
    {
        print_bytes(l - K1 - C2, k + K1);
        printf("/%d", GETC(k, l - C2));
    }
    else
    {   int i;
        for (i = K1; i < l; i++) /*printf("%c", k[i] < 32 ? '.' : k[i]);*/
        {   int ch = k[i];
            if (ch < 32) printf("/%d",ch); else
            printf("%c",ch);
        }
    }
}

static void print_tag(byte * p, int c, int j)
{   int o = GETD(p, c);
    int o_tag = o + I2 + GETK(p, o + I2);
    int l = o + GETI(p, o) - o_tag;

    if (j == 0)
    {   printf("/%d", GETC(p, o_tag));
        print_bytes(l - C2, p + o_tag + C2);
    }
    else
        printf("--> [%d]", get_int4(p, o_tag));
}

static void print_spaces(int n)
{   print_bytes(n, (byte *) "                              ");
}

/* report_block(B, m, n, p) prints the block at p, block number n, indented by
   m spaces.
*/

static int block_usage(struct Btree * B, byte * p)
{   int space = B->block_size - DIR_END(p);
    int free = TOTAL_FREE(p);
    return (space - free) * 100 / space;  /* a percentage */
}

static void report_block(struct Btree * B, int m, int n, byte * p)
{   int j = LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    print_spaces(m);
    printf("[%d] *%d (%d) %d%% ",
           n, REVISION(p), (dir_end - DIR_START)/D2, block_usage(B, p));

    for (c = DIR_START; c < dir_end; c += D2)
    {
        if (c == DIR_START + 6) printf ("... ");
        if (c >= DIR_START + 6 && c < dir_end - 6) continue;

        print_key(p, c, j); printf(" ");
    }
    printf("\n");
}

static void report_block_full(struct Btree * B, int m, int n, byte * p)
{   int j = LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    printf("\n");
    print_spaces(m);
    printf("Block [%d] revision *%d items (%d) usage %d%%:\n",
            n, REVISION(p), (dir_end - DIR_START)/D2, block_usage(B, p));

    for (c = DIR_START; c < dir_end; c += D2)
    {
        print_spaces(m);
        {   print_key(p, c, j);
            printf(" ");
            print_tag(p, c, j);

        }
        printf("\n");
    }
}

static void failure(int n)
{   fprintf(stderr, "B-tree error %d\n", n);
    exit(1);
}

static void block_check(struct Btree * B, struct Cursor * C, int j, int opts)
{   byte * p = C[j].p;
    int4 n = C[j].n;
    int c;
    int significant_c = j == 0 ? DIR_START : DIR_START + D2;
        /* the first key in an index block is dummy, remember */

    int max_free = MAX_FREE(p);
    int dir_end = DIR_END(p);
    int total_free = B->block_size - dir_end;

    if (block_free_at_start(B, n)) failure(0);
    if (block_free_now(B, n)) failure(1);
    free_block(B, n);

    if (j != LEVEL(p)) failure(10);
    if (dir_end <= DIR_START || dir_end > B->block_size) failure(20);

    if (opts & 1) report_block(B, 3*(B->level - j), n, p);

    if (opts & 2) report_block_full(B, 3*(B->level - j), n, p);

    for (c = DIR_START; c < dir_end; c += D2)
    {   int o = GETD(p, c);
        if (o > B->block_size) failure(21);
        if (o - dir_end < max_free) failure(30);
        {   int kt_len = GETI(p, o);
            if (o + kt_len > B->block_size) failure(40);
            total_free -= kt_len;
        }
        if (c > significant_c && compare_keys(key_of(p, c - D2), key_of(p,c)) >= 0)
            failure(50);
    }
    if (total_free != TOTAL_FREE(p)) failure(60);

    if (j == 0) return;
    for (c = DIR_START; c < dir_end; c += D2) {
	C[j].c = c;
        block_to_cursor(B, C, j - 1, block_given_by(p, c));
        if (B->overwritten) return;

        block_check(B, C, j - 1, opts);

        {   byte * q = C[j - 1].p;
            /* if j == 1, and c > DIR_START, the first key of level j - 1 must be >= the
               key of p, c: */

            if (j == 1 && c > DIR_START)
                if (compare_keys(key_of(q, DIR_START), key_of(p, c)) < 0) failure(70);

            /* if j > 1, the second key of level j - 1 must be >= the key of p, c: */

            if (j > 1 && DIR_END(q) > DIR_START + D2)
                if (compare_keys(key_of(q, DIR_START + D2), key_of(p, c)) < 0) failure(80);

            /* the last key of level j - 1 must be < the key of p, c + D2, if c + D2 < dir_end: */

            if (c + D2 < dir_end &&
                compare_keys(key_of(q, DIR_END(q) - D2), key_of(p, c + D2)) >= 0) failure(90);

            if (REVISION(q) > REVISION(p)) failure(91);
        }
    }
}

extern void Btree_check(const char * name, const char * opt_string)
{
    struct Btree * B = Btree_open_to_write(name);
    struct Cursor * C = B->C;

    int opts = 0;

    if (B->error) { printf("error %d (%s) in opening %s\n",
			   B->error,
			   Btree_strerror(B->error).c_str(),
			   name); exit(1); }

    for (size_t i = 0; i < strlen(opt_string); i++) {
	switch (opt_string[i]) {
            case 't': opts |= 1; break; /* short tree printing */
            case 'f': opts |= 2; break; /* full tree printing */
            case 'b': opts |= 4; break;
            case 'v': opts |= 8; break;
            case '+': opts |= 1+4+8; break;
            case '?': printf("use t,f,b,v or + in the option string\n"); exit(0);
            default: printf("option %s unknown\n", opt_string); exit(1);
        }
    }
    if (opts & 8)
    printf("base%c  Revision *%ld  levels %d  root [%ld]%s  blocksize %d  items %ld  lastblock %ld\n",
            B->base_letter,
            B->revision_number,
            B->level,
            C[B->level].n,
	    B->faked_root_block ? "(faked)" : "",
            B->block_size,
            B->item_count,
            B->last_block);

    {   int i;
        int limit = B->bit_map_size - 1;

        limit = limit * CHAR_BIT + CHAR_BIT - 1;

        if (opts & 4)
        {   for (i = 0; i <= limit; i++)
            {   printf("%c", block_free_at_start(B, i) ? '.' : '*');
                if (i > 0) {
                    if ((i + 1) % 100 == 0) {
                        printf("\n");
                    } else if ((i + 1) % 10 == 0) {
                        printf(" ");
                    }
                }
            }
            printf("\n\n");
        }
    }

    if (B->faked_root_block) printf("void "); else

    {   block_check(B, C, B->level, opts);

        /* the bit map should now be entirely clear: */

        {   int i; for (i = 0; i < B->bit_map_size; i++)
                if (B->bit_map[i] != 0) failure(100);
        }
    }
    Btree_quit(B);
    printf("B-tree checked okay\n");
}

