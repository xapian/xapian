/* btree.cc: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>   /* for calloc */
#include <string.h>   /* for memmove */
#include <limits.h>   /* for CHAR_BIT */

#include <errno.h>
#include "autoptr.h"

#include "btree.h"
#include "btree_util.h"
#include "btree_base.h"

#include "omassert.h"
#include "omdebug.h"
#include "om/omerror.h"
#include "utils.h"

#include <algorithm>  // for std::min()
#include <string>

using std::min;
using std::string;

//#define BTREE_DEBUG_FULL 1
#undef BTREE_DEBUG_FULL

#ifdef BTREE_DEBUG_FULL
/*------debugging aids from here--------*/

static void print_bytes(int n, byte * p);
static void print_key(byte * p, int c, int j);
static void print_tag(byte * p, int c, int j);

/*
static void report_cursor(int N, struct Btree * B, struct Cursor * C)
{
    int i;
    printf("%d)\n", N);
    for (i = 0; i <= B->level; i++)
	printf("p=%d, c=%d, n=[%d], rewrite=%d\n",
		C[i].p, C[i].c, C[i].n, C[i].rewrite);
}
*/

/*------to here--------*/
#endif /* BTREE_DEBUG_FULL */

/* Input/output is defined with calls to the basic Unix system interface: */

bool valid_handle(int h) { return h >= 0; }

int sys_open_to_read_no_except(const string & name)
{
    int fd = open(name, O_RDONLY, 0666);
    return fd;
}

int sys_open_to_read(const string & name)
{
    int fd = sys_open_to_read_no_except(name);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " to read: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static int sys_open_to_write_no_except(const string & name)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    return fd;
}

int sys_open_to_write(const string & name)
{
    int fd = sys_open_to_write_no_except(name);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " to write: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static int sys_open_for_readwrite(const string & name)
{
    int fd = open(name, O_RDWR, 0666);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " read/write: " + strerror(errno);
	throw OmOpeningError(message);
    }
    return fd;
}

static void sys_lseek(int h, off_t offset)
{
    if (lseek(h, offset, SEEK_SET) == -1) {
	string message = "Error seeking to block: ";
	message += strerror(errno);
	throw OmDatabaseError(message);
    }
}

static void sys_read_block(int h, int m, int4 n, byte * p)
{
    sys_lseek(h, (off_t)m * n);
    sys_read_bytes(h, m, p);
}

static void sys_write_block(int h, int m, int4 n, const byte * p)
{
    sys_lseek(h, (off_t)m * n);
    sys_write_bytes(h, m, p);
}

int sys_read_bytes(int h, int n, byte * p)
{
    ssize_t bytes_read;
    while (1) {
	bytes_read = read(h, (char *)p, n);
	if (bytes_read == n) {
	    // normal case - read succeeded, so return.
	    break;
	} else if (bytes_read == -1) {
	    string message = "Error reading block: ";
	    message += strerror(errno);
	    throw OmDatabaseError(message);
	} else if (bytes_read == 0) {
	    string message = "Error reading block: got end of file";
	    throw OmDatabaseError(message);
	} else if (bytes_read < n) {
	    /* Read part of the block, which is not an error.  We should
	     * continue reading the rest of the block.
	     */
	    n -= bytes_read;
	    p += bytes_read;
	}
    }
    return true;
}

string sys_read_all_bytes(int h, size_t bytes_to_read)
{
    ssize_t bytes_read;
    string retval;
    while (1) {
	char buf[1024];
	bytes_read = read(h, buf, min(sizeof(buf), bytes_to_read));
	if (bytes_read > 0) {
	    // add byte to string, continue unless we reached max
	    retval.append(buf, buf+bytes_read);
	    bytes_to_read -= bytes_read;
	    if (bytes_to_read == 0) {
		break;
	    }
	} else if (bytes_read == 0) {
	    // end of file, we're finished
	    break;
	} else if (bytes_read == -1) {
	    string message = "Error reading block: ";
	    message += strerror(errno);
	    throw OmDatabaseError(message);
	}
    }
    return retval;
}

int sys_write_bytes(int h, int n, const byte * p)
{
    ssize_t bytes_written;
    while (1) {
	bytes_written = write(h, (char *)p, n);
	if (bytes_written == n) {
	    // normal case - read succeeded, so return.
	    break;
	} else if (bytes_written == -1) {
	    string message = "Error writing block: ";
	    message += strerror(errno);
	    throw OmDatabaseError(message);
	} else if (bytes_written == 0) {
	    string message = "Error writing block: wrote no data";
	    throw OmDatabaseError(message);
	} else if (bytes_written < n) {
	    /* Wrote part of the block, which is not an error.  We should
	     * continue writing the rest of the block.
	     */
	    n -= bytes_written;
	    p += bytes_written;
	}
    }
    return true;
}

int sys_flush(int h) {
#ifdef HAVE_FDATASYNC
    fdatasync(h);
#else // HAVE_FDATASYNC
#ifdef HAVE_FSYNC
    fsync(h);
#else // HAVE_FSYNC
#error "Have neither fsync() nor fdatasync() - can't sync."
#endif // HAVE_FSYNC
#endif // HAVE_FDATASYNC
    return true;
}

int sys_close(int h) {
    return close(h) == 0;
}  /* 0 if success */

static void sys_unlink(const string &filename)
{
    if (unlink(filename) == -1) {
	string message = "Failed to unlink ";
	message += filename;
	message += ": ";
	message += strerror(errno);
	throw OmDatabaseCorruptError(message);
    }
}

/** Btree_strerror() converts Btree_errors values to more meaningful strings.
 */
string
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
   */

/** read_block(B, n, p) reads block n to address p in the DB file.
 */
void
Btree::read_block(int4 n, byte * p)
{
    /* Check that n is in range. */
    Assert(n >= 0);
    /* Use the base bit_map_size not the bitmap's size, because
     * the latter is uninitialised in readonly mode.
     */
    Assert(n / CHAR_BIT < base.get_bit_map_size());

    sys_read_block(handle, block_size, n, p);
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
void
Btree::write_block(int4 n, const byte * p)
{
    /* Check that n is in range. */
    Assert(n >= 0);
    Assert(n / CHAR_BIT < base.get_bit_map_size());

    /* don't write to non-free */;
    AssertParanoid(base.block_free_at_start(n));

    /* write revision is okay */
    AssertParanoid(REVISION(p) == next_revision);

    if (! Btree_modified) {
	// Things to do when we start a modification session.

	Btree_modified = true;

	if (both_bases) {
	    /* delete the old base */
	    sys_unlink(name + "base" + other_base_letter);
	    /* This used to set B->error as below, but will now throw
	     * an exception if it fails.
		B->error = BTREE_ERROR_BASE_DELETE;
		// FIXME: must exit, otherwise we could cause a corrupt
		// database to be read.
	     */
	}
    }

    sys_write_block(handle, block_size, n, p);
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


void
Btree::set_overwritten()
{
    DEBUGLINE(DB, "overwritten set to true\n");  /* initial debbugging line */
    overwritten = true;
    throw OmDatabaseModifiedError("Db block overwritten");
}

/* block_to_cursor(B, C, j, n) puts block n into position C[j] of cursor C,
   writing the block currently at C[j] back to disk if necessary. Note that

       C[j].rewrite

   is true iff C[j].n is different from block n in file DB. If it is false no
   rewriting is necessary.
*/

void
Btree::block_to_cursor(struct Cursor * C_, int j, int4 n)
{
    byte * p = C_[j].p;
    if (n == C_[j].n) return;

    // FIXME: only needs to be done in write mode
    if (C_[j].rewrite) {
	Assert(C == C_);
	write_block(C_[j].n, p);
	C_[j].rewrite = false;
    }
    read_block(n, p);
    if (overwritten) return;

    C_[j].n = n;
    C[j].n = n; /* not necessarily the same (in B-tree read mode) */
    if (j < level) {
	if (REVISION(p) > REVISION(C_[j + 1].p)) { /* unsigned comparison */
	    set_overwritten();
	    return;
	}
    }
    AssertEq(j, GET_LEVEL(p));
}

/* set_block_given_by(p, c, n) finds the item at block address p, directory
   offset c, and sets its tag value to n. For blocks not at the data level,
   when GET_LEVEL(p) > 0, the tag of an item is just the block number of
   another block in the B-tree structure.

   (The built in '4' below is the number of bytes per block number.)
*/

static void set_block_given_by(byte * p, int c, int4 n)
{
    c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - 4;   /* c is an offset to a block number */
    set_int4(p, c, n);
}

/* block_given_by(p, c) finds the item at block address p, directory offset c,
   and returns its tag value as an integer.
*/

static int block_given_by(byte * p, int c)
{
    c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - 4;   /* c is an offset to a block number */
    return get_int4(p, c);
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

void
Btree::alter(Cursor * C)
{
    int j = 0;
    byte * p = C[j].p;
    while (true) {
	if (C[j].rewrite) return; /* all new, so return */
	C[j].rewrite = true;

	int4 n = C[j].n;
	if (base.block_free_at_start(n)) return;
	base.free_block(n);
	n = base.next_free_block();
	C[j].n = n;
	SET_REVISION(p, next_revision);

	if (j == level) return;
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

   This is complicated by the fact that keys have two parts - a value and
   then a count.  We first compare the values, and only if they are equal
   do we compare the counts.
*/

static int compare_keys(const byte * key1, const byte * key2)
{
    int key1_len = GETK(key1, 0);
    int key2_len = GETK(key2, 0);
    int k_smaller = (key2_len < key1_len ? key2_len : key1_len) - C2;
    int i;

    // Compare the first part of the keys
    for (i = K1; i < k_smaller; i++) {
	int diff = (int) key1[i] - key2[i];
	if (diff != 0) return diff;
    }

    {
	int diff = key1_len - key2_len;
	if (diff != 0) return diff;
    }

    // Compare the count
    for (; i < k_smaller + C2; i++) {
	int diff = (int) key1[i] - key2[i];
	if (diff != 0) return diff;
    }
    return 0;
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
{
    int i = DIR_START - offset;
    int j = DIR_END(p);

    if (c != -1) {
	if (c < j && i < c && compare_keys(key, p + GETD(p, c) + I2) >= 0)
	    i = c;
	c += D2;
	if (c < j && i < c && compare_keys(key, p + GETD(p, c) + I2) < 0)
	    j = c;
    }

    while (j - i > D2) {
	int k = i + ((j - i)/D4)*D2; /* mid way */
	int t = compare_keys(key, p + GETD(p, k) + I2);
	if (t < 0) j = k; else i = k;
    }
    return i;
}

/* find(B, C_) searches for the key of B->kt in the B-tree. Result is 1 if
   found, 0 otherwise. When 0, the B_tree cursor is positioned at the last key
   in the B-tree <= the search key. Goes to first (null) item in B-tree when
   key length == 0.

   (In this case, example debugging lines are shown commented. Debugging is easy
   with the help of the B-tree writing code included further down.)
*/

bool
Btree::find(struct Cursor * C_)
{
    /* FIXME: is the parameter necessary? */
    byte * p;
    int c;
    byte * k = kt + I2;
    int j;
    for (j = level; j > 0; j--) {
	p = C_[j].p;
	c = find_in_block(p, k, 0, C_[j].c);
#ifdef BTREE_DEBUG_FULL
	printf("Block in Btree:find - code position 1");
	report_block_full(j, C_[j].n, p);
#endif /* BTREE_DEBUG_FULL */
	C_[j].c = c;
	block_to_cursor(C_, j - 1, block_given_by(p, c));
	if (overwritten) return false;
    }
    p = C_[0].p;
    c = find_in_block(p, k, D2, C_[j].c);
#ifdef BTREE_DEBUG_FULL
    printf("Block in Btree:find - code position 2");
    report_block_full(j, C_[j].n, p);
#endif /* BTREE_DEBUG_FULL */
    C_[0].c = c;
    if (c < DIR_START) return false;
    return compare_keys(kt + I2, key_of(p, c)) == 0;
}

/* compress(B, p) compresses the block at p by shuffling all the items up to
   the end. MAX_FREE(p) is then maximized, and is equal to TOTAL_FREE(p).
*/

void
Btree::compress(byte * p)
{
    int e = block_size;
    byte * b = buffer;
    int dir_end = DIR_END(p);
    for (int c = DIR_START; c < dir_end; c += D2) {
	int o = GETD(p, c);
	int l = GETI(p, o);
	e -= l;
	memmove(b + e, p + o, l);
	SETD(p, c, e);  /* reform in b */
    }
    memmove(p + e, b + e, block_size - e);  /* copy back */
    e -= dir_end;
    SET_TOTAL_FREE(p, e);
    SET_MAX_FREE(p, e);
}

/* form_null_key(b, n) forms in b a null key with block number n in the tag.
 */

static void form_null_key(byte * b, int4 n)
{
    set_int4(b, I3, n);
    SETK(b, I2, K1);     /* null key */
    SETI(b, 0, I3 + 4);  /* total length */
}


/** Btree needs to gain a new level to insert more items: so split root block
 *  and construct a new one.
 */
void
Btree::split_root(struct Cursor * C_, int j)
{
    /* gain a level */
    level ++;

    /* check level overflow */
    AssertNe(level, BTREE_CURSOR_LEVELS);

    byte * q = zeroed_new(block_size);
    if (q == 0) {
	error = BTREE_ERROR_SPACE;
	throw std::bad_alloc();
    }
    C_[j].p = q;
    C_[j].split_p = zeroed_new(block_size);
    if (C_[j].split_p == 0) {
	error = BTREE_ERROR_SPACE;
	throw std::bad_alloc();
    }
    C_[j].c = DIR_START;
    C_[j].n = base.next_free_block();
    C_[j].rewrite = true;
    SET_REVISION(q, next_revision);
    SET_LEVEL(q, j);
    SET_DIR_END(q, DIR_START);
    compress(q);   /* to reset TOTAL_FREE, MAX_FREE */

    /* form a null key in b with a pointer to the old root */

    int4 old_root = C_[j - 1].split_n;
    byte b[10]; /* 7 is exact */
    form_null_key(b, old_root);
    add_item(C_, b, j);
}

/** Make an item with key newkey.  Key is optionally truncated to minimal key
 *  that differs from prevkey, the preceding key in a block, and tag containing
 *  a block number.  Must preserve counts at end of the keys, however.
 *
 *  Store result in buffer "result".
 */
void Btree::make_index_item(byte * result, int result_len,
			    const byte * prevkey, const byte * newkey,
			    const int4 blocknumber, bool truncate) const
{
    // FIXME: check we don't overrun result_len
    Assert(compare_keys(prevkey, newkey) < 0);

    int prevkey_len = GETK(prevkey, 0) - C2;
    int newkey_len = GETK(newkey, 0) - C2;
    int i;

    if (truncate) {
	i = K1;
	while (i < prevkey_len && prevkey[i] == newkey[i]) {
	    i++;
	}

	// Want one byte of difference.
	if (i < newkey_len) i++;
    } else {
	i = newkey_len;
    }
    SETI(result, 0, I2 + i + C2 + sizeof(blocknumber)); // Set item length
    SETK(result, I2, i + C2);    // Set key length
    memmove(result + I2 + K1, newkey + K1, i - K1); // Copy the main part of the key
    memmove(result + I2 + i, newkey + newkey_len, C2); // copy count part

    // Set tag contents to block number
    set_int4(result, I2 + i + C2, blocknumber);
}

/* enter_key(B, C, j, prevkey, newkey) is called after a block split. It enters
   in the
   block at level C[j] a separating key for the block at level C[j - 1]. The
   key itself is newkey. prevkey is the preceding key, and at level 1 newkey
   can be trimmed down to the first point of difference to prevkey for entry in
   C[j].

   This code looks longer than it really is. If j exceeds the number of B-tree
   levels the root block has split and we have to construct a new one, but this
   is a rare event.

   The key is constructed in b, with block number C[j - 1].n as tag, and this
   is added in with add_item. add_item may itself cause a block split, with a
   further call to enter_key. Hence the recursion.
*/
void
Btree::enter_key(struct Cursor * C_, int j, byte * prevkey, byte * newkey)
{
    Assert(compare_keys(prevkey, newkey) < 0);
    Assert(j >= 1);
    if (j > level) split_root(C_, j);

    /*  byte * p = C_[j - 1].p;  -- see below */

    int4 blocknumber = C_[j - 1].n;

    // Keys are truncated here: but don't truncate the count at the end away.
    // FIXME: check that b is big enough.  Dynamically allocate.
    byte b[UCHAR_MAX + 1];
    make_index_item(b, UCHAR_MAX + 1, prevkey, newkey, blocknumber, j == 1);

    /* when j > 1 we can make the first key of block p null, but is it worth it?
       Other redundant keys still creep in. The code to do it is commented out
here:
     */
    /*
       if (j > 1) {
	   int newkey_len = GETK(newkey, 0);
	   int n = get_int4(newkey, newkey_len);
	   int new_total_free = TOTAL_FREE(p) + (newkey_len - K1);
	   form_null_key(newkey - I2, n);
	   SET_TOTAL_FREE(p, new_total_free);
       }
     */

    C_[j].c = find_in_block(C_[j].p, b + I2, 0, 0) + D2;
    C_[j].rewrite = true; /* a subtle point: this *is* required. */
    add_item(C_, b, j);
}

/* split_off(B, C, j, c, p, q) splits the block at p at directory offset c.
   In fact p is just C[j].p, and q is C[j].split_p, a block buffer provided at
   each cursor level to accommodate the split.

   The first half block goes into q, with block number in C[j].split_n copied
   from C[j].n, the second half into p with a new block number.
*/

void
Btree::split_off(struct Cursor * C_, int j, int c, byte * p, byte * q)
{
    /* p is C[j].p, q is C[j].split_p */

    C_[j].split_n = C_[j].n;
    C_[j].n = base.next_free_block();

    memmove(q, p, block_size);  /* replicate the whole block in q */
    SET_DIR_END(q, c);
    compress(q);      /* to reset TOTAL_FREE, MAX_FREE */

    int residue = DIR_END(p) - c;
    int new_dir_end = DIR_START + residue;
    memmove(p + DIR_START, p + c, residue);
    SET_DIR_END(p, new_dir_end);

    compress(p);      /* to reset TOTAL_FREE, MAX_FREE */
}

/* mid_point(B, p) finds the directory entry in c that determines the
   approximate mid point of the data in the block at p.
 */

int
Btree::mid_point(byte * p)
{
    int n = 0;
    int dir_end = DIR_END(p);
    int size = block_size - TOTAL_FREE(p) - dir_end;
    for (int c = DIR_START; c < dir_end; c += D2) {
	int o = GETD(p, c);
	int l = GETI(p, o);
	n += 2 * l;
	if (n >= size) {
	    if (l < n - size) return c;
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

void
Btree::add_item_to_block(byte * p, byte * kt, int c)
{
    int dir_end = DIR_END(p);
    int kt_len = GETI(kt, 0);
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    int new_max = MAX_FREE(p) - needed;

    Assert(new_total >= 0);

    if (new_max < 0) {
	compress(p);
	new_max = MAX_FREE(p) - needed;
	Assert(new_max >= 0);
    }
    Assert(dir_end >= c);

    memmove(p + c + D2, p + c, dir_end - c);
    dir_end += D2;
    SET_DIR_END(p, dir_end);

    int o = dir_end + new_max;
    SETD(p, c, o);
    memmove(p + o, kt, kt_len);

    SET_MAX_FREE(p, new_max);

    SET_TOTAL_FREE(p, new_total);
}

/* add_item(B, C, kt, j) adds item kt to the block at cursor level C[j]. If
   there is not enough room the block splits and the item is then added to the
   appropriate half.
*/

void
Btree::add_item(struct Cursor * C, byte * kt, int j)
{
    byte * p = C[j].p;
    int c = C[j].c;
    int4 n;

    int kt_len = GETI(kt, 0);
    int needed = kt_len + D2;
    int new_total = TOTAL_FREE(p) - needed;
    if (new_total < 0) {
	int m;
	byte * q = C[j].split_p;
	if (seq_count < 0 /*|| j > 0*/ )
	    m = mid_point(p);
	else {
	    if (c < DIR_END(p)) {
		m = c - D2;
		/* splits at dirend-2 */
	    } else {
		m = c;
		/* splits at dirend. (This has all been cautiously tested) */
	    }
	}
	split_off(C, j, m, p, q);
	if (c >= m) {
	    c -= (m - DIR_START);
	    add_item_to_block(p, kt, c);
	    n = C[j].n;
	} else {   add_item_to_block(q, kt, c);
	    n = C[j].split_n;
	}
	write_block(C[j].split_n, q);

	enter_key(C, j + 1,                /* enters a separating key at level j + 1 */
		  key_of(q, DIR_END(q) - D2), /* - between the last key of block q, */
		  key_of(p, DIR_START));      /* - and the first key of block p */
    } else {
	add_item_to_block(p, kt, c);
	n = C[j].n;
    }
    if (j == 0) {
	changed_n = n;
	changed_c = c;
    }
}

/* delete_item(B, C, j, repeatedly) is (almost) the converse of add_item. If
   repeatedly is true, the process repeats at the next level when a block has
   been completely emptied, freeing the block and taking out the pointer to it.
   Emptied root blocks are also removed, which reduces the number of levels in
   the B-tree.
*/

void
Btree::delete_item(struct Cursor * C, int j, int repeatedly)
{
    byte * p = C[j].p;
    int c = C[j].c;
    int o = GETD(p, c);              /* offset of item to be deleted */
    int kt_len = GETI(p, o);         /* - and its length */
    int dir_end = DIR_END(p) - D2;   /* directory length will go down by 2 bytes */

    memmove(p + c, p + c + D2, dir_end - c);
    SET_DIR_END(p, dir_end);
    SET_MAX_FREE(p, MAX_FREE(p) + D2);
    SET_TOTAL_FREE(p, TOTAL_FREE(p) + kt_len + D2);

    if (!repeatedly) return;
    if (j < level) {
	if (dir_end == DIR_START) {
	    base.free_block(C[j].n);
	    C[j].rewrite = false;
	    C[j].n = -1;
	    C[j + 1].rewrite = true;  /* *is* necessary */
	    delete_item(C, j + 1, true);
	}
    } else {
	/* j == B->level */
	while (dir_end == DIR_START + D2 && j > 0) {
	    /* single item in the root block, so lose a level */
	    int new_root = block_given_by(p, DIR_START);
	    delete [] p;
	    C[j].p = 0;
	    base.free_block(C[j].n);
	    C[j].rewrite = false;
	    C[j].n = -1;
	    delete [] C[j].split_p;
	    C[j].split_p = 0;
	    C[j].split_n = -1;
	    level--;

	    block_to_cursor(C, level, new_root);
	    if (overwritten) return;

	    j--;
	    p = C[j].p;
	    dir_end = DIR_END(p); /* prepare for the loop */
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

int
Btree::add_kt(int found, struct Cursor * C)
{
    int components = 0;

    if (overwritten) return 0;

    /*
    {
	printf("%d) %s ", addcount++, (found ? "replacing " : "adding "));
	print_bytes(kt[I2] - K1 - C2, kt + I2 + K1); printf("\n");
    }
    */
    alter(C);

    if (found) { /* replacement */
	seq_count = SEQ_START_POINT;
	sequential = false;

	byte * p = C[0].p;
	int c = C[0].c;
	int o = GETD(p, c);
	int kt_size = GETI(kt, 0);
	int needed = kt_size - GETI(p, o);

	components = components_of(p, c);

	if (needed <= 0) {
	    /* simple replacement */
	    memmove(p + o, kt, kt_size);
	    SET_TOTAL_FREE(p, TOTAL_FREE(p) - needed);
	} else {
	    /* new item into the block's freespace */
	    int new_max = MAX_FREE(p) - kt_size;
	    if (new_max >= 0) {
		o = DIR_END(p) + new_max;
		memmove(p + o, kt, kt_size);
		SETD(p, c, o);
		SET_MAX_FREE(p, new_max);
		SET_TOTAL_FREE(p, TOTAL_FREE(p) - needed);
	    } else {
		/* do it the long way */
		delete_item(C, 0, false);
		add_item(C, kt, 0);
	    }
	}
    } else {
	/* addition */
	if (changed_n == C[0].n && changed_c == C[0].c) {
	    if (seq_count < 0) seq_count++;
	} else {
	    seq_count = SEQ_START_POINT;
	    sequential = false;
	}
	C[0].c += D2;
	add_item(C, kt, 0);
    }
    return components;
}

/* delete_kt(B, C) corresponds to add_kt(B, C), but there are only two cases: if
   the key is not found nothing is done, and if it is found the corresponding
   item is deleted with delete_item.
*/

int
Btree::delete_kt()
{
    int found = find(C);
    if (overwritten) return 0;

    int components = 0;
    seq_count = SEQ_START_POINT;
    sequential = false;

    /*
    {
	printf("%d) %s ", addcount++, (found ? "deleting " : "ignoring "));
	print_bytes(B->kt[I2] - K1 - C2, B->kt + I2 + K1); printf("\n");
    }
    */
    if (found)
    {
	components = components_of(C[0].p, C[0].c);
	alter(C);
	delete_item(C, 0, true);
    }
    return components;
}

/* form_key(B, p, key, key_len) treats address p as an item holder and fills in
the key part:

	   (I) K key c (C tag)

The bracketed parts are left blank. The key is filled in with key_len bytes and
K set accordingly. c is set to 1.
*/

void form_key(struct Btree * B, byte * p, const byte * key, int key_len)
{
    Assert(key_len <= B->max_key_len);

    // This just so it doesn't fall over horribly in non-debug builds.
    if (key_len > B->max_key_len) key_len = B->max_key_len;

    int c = I2;
    SETK(p, c, key_len + K1 + C2); c += K1;
    memmove(p + c, key, key_len); c += key_len;
    SETC(p, c, 1);
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
    return B->add(key, key_len, tag, tag_len);
}

int
Btree::add(byte *key, int key_len,
	   byte *tag, int tag_len)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    form_key(this, kt, key, key_len);

    {
	int ck = GETK(kt, I2) + I2 - C2;  /* offset to the counter in the key */
	int ct = ck + C2;                 /* offset to the tag counter */
	int cd = ct + C2;                 /* offset to the tag data */
	int L = max_item_size - cd;    /* largest amount of tag data for any tagi */

	int first_L = L;                  /* - amount for tag1 */
	int found = find(C);
	if (full_compaction && !found) {
	    byte * p = C[0].p;
	    int n = TOTAL_FREE(p) % (max_item_size + D2) - D2 - cd;
	    if (n > 0) first_L = n;
	}
	{
	    int m = tag_len == 0 ? 1 :        /* a null tag must be added in of course */
		    (tag_len - first_L + L - 1) / L + 1;
				              /* there are m items to add */
	    int n = 0; /* initialise to shut off warning */
					      /* - and there will be n to delete */
	    int o = 0;                        /* offset into the tag */
	    int residue = tag_len;            /* bytes of the tag remaining to add in */
	    int replacement = false;          /* has there been a replacement ? */
	    int i;
	    /* FIXME: sort out this error higher up and turn this into
	     * an assert.
	     */
	    if (m >= BYTE_PAIR_RANGE) { error = BTREE_ERROR_TAGSIZE; return 0; }
	    for (i = 1; i <= m; i++) {
		int l = i == m ? residue :
			i == 1 ? first_L : L;
		memmove(kt + cd, tag + o, l);
		o += l;
		residue -= l;

		SETC(kt, ck, i);
		SETC(kt, ct, m);
		SETI(kt, 0, cd + l);
		if (i > 1) found = find(C);
		n = add_kt(found, C);
		if (n > 0) replacement = true;
	    }
	    /* o == tag_len here, and n may be zero */
	    for (i = m + 1; i <= n; i++) {
		SETC(kt, ck, i);
		delete_kt();

		if (overwritten) return 0;
	    }
	    if (replacement) return 0;
	    item_count++;
	    return 1;
	}
    }
}

/* Btree_delete(B, key, key_len) returns 0 if the key is not in the B-tree,
   otherwise deletes it and returns 1.

   Again, this is parallel to Btree-add, but simpler in form.
*/

extern int Btree_delete(struct Btree * B, byte * key, int key_len)
{
    return B->delete_(key, key_len);
}

int
Btree::delete_(byte * key, int key_len)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    if (key_len == 0) return 0;
    form_key(this, kt, key, key_len);

    int n = delete_kt();  /* there are n items to delete */
    for (int i = 2; i <= n; i++) {
	int c = GETK(kt, I2) + I2 - C2;
	SETC(kt, c, i);

	delete_kt();

	if (overwritten) return 0;
    }
    if (n > 0) {
	item_count--;
	return 1;
    }
    return 0;
}

bool
Btree::find_key(byte * key, int key_len)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    form_key(this, kt, key, key_len);
    return find(C);
}

extern int Btree_find_key(struct Btree * B, byte * key, int key_len)
{
    return B->find_key(key, key_len);
}

extern struct Btree_item * Btree_item_create(void)
{
    struct Btree_item * item = new Btree_item;
    if (item == 0) return 0;
    item->key_size = -1;
    item->tag_size = -1;
    return item;
}

extern int Btree_find_tag(struct Btree * B, byte * key, int key_len, struct Btree_item * item)
{
    return B->find_tag(key, key_len, item);
}

int
Btree::find_tag(byte * key, int key_len, struct Btree_item * item)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    form_key(this, kt, key, key_len);
    if (!find(C)) return 0;
    {
	int n = components_of(C[0].p, C[0].c);
				        /* n components to join */
	int ck = GETK(kt, I2) + I2 - C2;/* offset to the key counter */
	int cd = ck + 2 * C2;           /* offset to the tag data */
	int o = 0;                      /* cursor into item->tag */
	int i = 1;                      /* see below */
	byte * p;                       /* pointer to current component */
	int l;                          /* number of bytes to extract from current component */

	p = item_of(C[0].p, C[0].c);
	l = GETI(p, 0) - cd;
	{
	    int4 space_for_tag = (int4) max_item_size * n;
	    if (item->tag_size < space_for_tag) {
		delete [] item->tag;
		item->tag = zeroed_new(space_for_tag + 5);
		if (item->tag == 0) {
		    error = BTREE_ERROR_SPACE;
		    throw std::bad_alloc();
		}
		item->tag_size = space_for_tag + 5;
	    }
	}
	while (true) {
	    Assert(o + l <= item->tag_size);

	    memmove(item->tag + o, p + cd, l);
	    o += l;

	    if (i == n) break;
	    i++;
	    SETC(kt, ck, i);
	    find(C);

	    if (overwritten) return 0;

	    p = item_of(C[0].p, C[0].c);
	    l = GETI(p, 0) - cd;
	}
	item->tag_len = o;
    }
    return 1;
}

extern void Btree_item_lose(struct Btree_item * item)
{
    delete [] item->key;
    delete [] item->tag;
    delete item;
}

extern void Btree_full_compaction(struct Btree * B, int parity)
{
    B->set_full_compaction(parity);
}

void
Btree::set_full_compaction(int parity)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    if (parity) seq_count = 0;
    full_compaction = parity;
}

/************ B-tree opening and closing ************/

int
Btree::write_base()
{
    base.write_to_file(name + "base" + other_base_letter);
    return true;
}

bool
Btree::basic_open(const char * name_,
		  bool revision_supplied,
		  uint4 revision_)
{
    int ch = 'X'; /* will be 'A' or 'B' */

    /* FIXME: move this into constructor initialiser name(name_) */
    name = name_;

    {
	string err_msg;
	vector<char> basenames;
	basenames.push_back('A');
	basenames.push_back('B');

	vector<Btree_base> bases(basenames.size());
	vector<bool> base_ok(basenames.size());

	for (size_t i=0; i<basenames.size(); ++i) {
	    base_ok[i] = bases[i].read(name, basenames[i], err_msg);
	}

	// FIXME: assumption that there are only two bases
	if (base_ok[0] && base_ok[1]) both_bases = true;
	if (!base_ok[0] && !base_ok[1]) {
	    string message = "Error opening table `";
	    message += name_;
	    message += "':\n";
	    message += err_msg;
	    throw OmOpeningError(message);
	}

	if (revision_supplied) {
	    bool found_revision = false;
	    for (size_t i = 0; i < basenames.size(); ++i) {
		if (base_ok[i] && bases[i].get_revision() == revision_) {
		    ch = basenames[i];
		    found_revision = true;
		    break;
		}
	    }
	    if (!found_revision) {
		/* Couldn't open the revision that was asked for.
		 * This shouldn't throw an exception, but should just return
		 * 0 to upper levels.
		 */
		return false;
	    }
	} else {
	    uint4 highest_revision = 0;
	    for (size_t i = 0; i < basenames.size(); ++i) {
		if (base_ok[i] && bases[i].get_revision() >= highest_revision) {
		    ch = basenames[i];
		    highest_revision = bases[i].get_revision();
		}
	    }
	}

	Btree_base *basep = 0;
	Btree_base *other_base = 0;

	for (size_t i = 0; i < basenames.size(); ++i) {
	    /*
	    cerr << "Checking (ch == " << ch << ") against "
		    "basenames[" << i << "] == " << basenames[i] << endl;
	    cerr << "bases[i].get_revision() == " << bases[i].get_revision()
		    << endl;
	    cerr << "base_ok[i] == " << base_ok[i] << endl;
	    */
	    if (ch == basenames[i]) {
		basep = &bases[i];

		// FIXME: assuming only two bases for other_base
		size_t otherbase_num = 1-i;
		if (base_ok[otherbase_num]) {
		    other_base = &bases[otherbase_num];
		}
		break;
	    }
	}
	if (!basep) {
	    Assert(false);
	    error = BTREE_ERROR_BASE_READ;
	}

	/* basep now points to the most recent base block */

	/* Avoid copying the bitmap etc. - swap contents with the base
	 * object in the vector, since it'll be destroyed anyway soon.
	 */
	base.swap(*basep);

	revision_number =  base.get_revision();
	block_size =       base.get_block_size();
	root =             base.get_root();
	level =            base.get_level();
	//bit_map_size =     basep->get_bit_map_size();
	item_count =       base.get_item_count();
	faked_root_block = base.get_have_fakeroot();
	sequential =       base.get_sequential();

	if (other_base != 0) {
	    other_revision_number = other_base->get_revision();
	}
    }

    /* k holds contructed items as well as keys */
    kt = zeroed_new(block_size);
    if (kt == 0) {
	throw std::bad_alloc();
    }

    max_item_size = (block_size - DIR_START - BLOCK_CAPACITY * D2) / BLOCK_CAPACITY;

    /* This upper limit corresponds to K1 == 1 */
    max_key_len = UCHAR_MAX - K1 - C2;

    {
	int max = max_item_size - I3 - C2 - C2 - TAG_CAPACITY;

	/* This limit would come into effect with large keys in a B-tree with a
	   small block size.
	*/

	if (max_key_len > max) max_key_len = max;
    }

    /* ready to open the main file */

    base_letter = ch;
    next_revision = revision_number + 1;

    return true;
}

void
Btree::read_root()
{
    if (faked_root_block) {
	/* root block for an unmodified database. */
	int o = block_size - C2;
	byte * p = C[0].p;

	/* clear block - shouldn't be neccessary, but is a bit nicer,
	 * and means that the same operations should always produce
	 * the same database. */
	memset(p, 0, block_size);

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

	if (!writable) {
	    /* reading - revision number doesn't matter as long as it's
	     * not greater than the current one.*/
	    SET_REVISION(p, 0);
	    C[0].n = 0;
	} else {
	    /* writing - */
	    SET_REVISION(p, next_revision);
	    C[0].n = base.next_free_block();
	}
    } else {
	/* using a root block stored on disk */
	block_to_cursor(C, level, root);
	if (overwritten) return;

	if (REVISION(C[level].p) >= next_revision) set_overwritten();
	/* although this is unlikely */
    }
}

bool
Btree::do_open_to_write(const char * name_,
			bool revision_supplied,
			uint4 revision_)
{
    /* FIXME: do the exception safety the right way, by making all the
     * parts into sensible objects.
     */
    if (!basic_open(name_, revision_supplied, revision_)) {
	if (!revision_supplied) {
	    string message = "Failed to open for writing";
	    if (error != BTREE_ERROR_NONE) {
		message += string(": ");
		message += Btree_strerror(error);
	    }
	    throw OmOpeningError(message);
	} else {
	    /* When the revision is supplied, it's not an exceptional
	     * case when open failed.  We should just return 0 here
	     * instead.
	     */
	    return false;
	}
    }

    writable = true;

    handle = sys_open_for_readwrite(name + "DB");

    for (int j = 0; j <= level; j++) {
	C[j].n = -1;
	C[j].split_n = -1;
	C[j].p = new byte[block_size];
	if (C[j].p == 0) {
	    throw std::bad_alloc();
	}
	C[j].split_p = new byte[block_size];
	if (C[j].split_p == 0) {
	    throw std::bad_alloc();
	}
    }
    read_root();

    buffer = zeroed_new(block_size);
    if (buffer == 0) {
	throw std::bad_alloc();
    }

    other_base_letter = base_letter == 'A' ? 'B' : 'A'; /* swap for writing */

    changed_n = 0;
    changed_c = DIR_START;
    seq_count = SEQ_START_POINT;

    return true;
}

bool
Btree::open_to_write(const char * name)
{
    return do_open_to_write(name, false, 0);
}

bool
Btree::open_to_write(const char * name, uint4 n)
{
    return do_open_to_write(name, true, n);
}

struct Btree * Btree_open_to_write(const char * name)
{
    AutoPtr<Btree> B(new Btree());

    if (B->open_to_write(name)) {
	return B.release();
    } else {
	return 0;
    }
}

struct Btree * Btree_open_to_write_revision(const char * name,
					    unsigned long revision)
{
    AutoPtr<Btree> B(new Btree());

    if (B->open_to_write(name, revision)) {
	return B.release();
    } else {
	return 0;
    }
}

extern void Btree_quit(struct Btree * B)
{
    delete B;
}

Btree::Btree()
	: error(BTREE_ERROR_NONE),
	  overwritten(false),
	  revision_number(0),
	  other_revision_number(0),
	  both_bases(false),
	  item_count(0),
	  max_key_len(0),
	  block_size(0),
	  base_letter('A'),
	  faked_root_block(true),
	  sequential(true),
	  handle(-1),
	  level(0),
	  root(0),
	  kt(0),
	  buffer(0),
	  next_revision(0),
	  base(),
	  other_base_letter(0),
	  seq_count(0),
	  changed_n(0),
	  changed_c(0),
	  max_item_size(0),
	  shared_level(0),
	  Btree_modified(false),
	  full_compaction(false),
	  writable(false)
{
}

/** Delete file, throwing an error if can't delete it (but not if it
 *  doesn't exist)
 */
void
sys_unlink_if_exists(const string & filename)
{
    if (unlink(filename) == -1) {
	if (errno == ENOENT) return;
	throw OmDatabaseError("Can't delete file: `" + filename +
			      "': " + strerror(errno));
    }
}

void
Btree::erase(const string & tablename)
{
    sys_unlink_if_exists(tablename + "DB");
    sys_unlink_if_exists(tablename + "baseA");
    sys_unlink_if_exists(tablename + "baseB");
}

void
Btree::create(const char *name_, int block_size)
{
    string name(name_);

    if (block_size > BYTE_PAIR_RANGE) {
	/* block size too large (64K maximum) */
	throw OmInvalidArgumentError("Btree block size too large");
    }

    if (block_size < DIR_START + BLOCK_CAPACITY * (D2 + I3 + KEY_CAPACITY + 2 * C2 + TAG_CAPACITY)) {
	/* block size far too small */
	throw OmInvalidArgumentError("Btree block size too small");
    }

    /* indeed it will need to be a good bit bigger */

    /* write initial values of to files */
    {
	/* create the base file */
	/* error = BTREE_ERROR_BASE_CREATE; */
	Btree_base base;
	base.set_block_size(block_size);
	base.set_have_fakeroot(true);
	base.set_sequential(true);
	base.write_to_file(name + "baseA");

	/* create the main file */
	/* error = BTREE_ERROR_DB_CREATE; */
	{
	    int h = sys_open_to_write(name + "DB");      /* - null */
	    if ( ! (valid_handle(h) &&
		    sys_close(h))) {
		string message = "Error creating DB file: ";
		message += strerror(errno);
		throw OmOpeningError(message);
	    }
	}
    }
}

Btree::~Btree() {
    if (valid_handle(handle)) {
	// If an error occurs here, we just ignore it, since we're just
	// trying to free everything.
	sys_close(handle);
	handle = -1;
    }

    for (int j = level; j >= 0; j--) {
	delete [] C[j].p;
	delete [] C[j].split_p;
    }

    delete [] kt;
    delete [] buffer;
}

extern int Btree_close(struct Btree * B_, uint4 revision)
{
    int retval = B_->commit(revision);
    delete B_;
    return retval;
}

Btree_errors
Btree::commit(uint4 revision)
{
    AssertEq(error, 0);
    Assert(!overwritten);

    int j;
    Btree_errors errorval = BTREE_ERROR_REVISION;
    if (revision < next_revision) {
	/* FIXME: should Btree_close() throw exceptions, as it's
	 * likely to be called from destructors they'll need to catch
	 * the exception), or return an error value?  (So we need to
	 * trap errors here and convert them)
	 * I think it should throw an exception, but need to discuss.
	 *  -CME  2000-11-16
	 */
	return errorval; /* revision too low */
    }

    for (j = level; j >= 0; j--) {
	if (C[j].rewrite) {
	    write_block(C[j].n, C[j].p);
	    if (error) {
		return errorval;
	    }
	}
    }

    errorval = BTREE_ERROR_DB_CLOSE;
    if ( ! (sys_flush(handle) && sys_close(handle))) {
	handle = -1;
	return errorval;
    }

    if (Btree_modified) {
	faked_root_block = false;
    }

    if (faked_root_block) {
	/* We will use a dummy bitmap. */
	base.clear_bit_map();
    }

    errorval = BTREE_ERROR_BASE_WRITE;
    base.set_revision(revision);
    base.set_root(C[level].n);
    base.set_level(level);
    base.set_item_count(item_count);
    base.set_have_fakeroot(faked_root_block);
    base.set_sequential(sequential);

    if (! write_base()) {
	return errorval;
    }

    errorval = BTREE_ERROR_NONE;

    return errorval;
}

/************ B-tree reading ************/

bool
Btree::do_open_to_read(const char * name_,
		       bool revision_supplied,
		       uint4 revision_)
{
    if (!basic_open(name_, revision_supplied, revision_)) {
	string message = "Failed to open table to read: ";
	if (error != BTREE_ERROR_NONE) {
	    message += Btree_strerror(error);
	} else {
	    message += "unknown error";
	}
	throw OmOpeningError(message);
    }

    handle = sys_open_to_read(name + "DB");

    {
	int common_levels = revision_number <= 1 ? 1 : 2;
	shared_level = level > common_levels ? common_levels : level;
    }

    prev = sequential ? prev_for_sequential : prev_default;
    next = sequential ? next_for_sequential : next_default;

    for (int j = shared_level; j <= level; j++) {
	C[j].n = -1;
	C[j].p = new byte[block_size];
	if (C[j].p == 0) {
	    throw std::bad_alloc();
	}
    }
    read_root();

    return true;
}

bool
Btree::open_to_read(const char * name)
{
    return do_open_to_read(name, false, 0);
}

bool
Btree::open_to_read(const char * name, uint4 n)
{
    return do_open_to_read(name, true, n);
}

Btree * Btree_open_to_read(const char * name)
{
    AutoPtr<Btree> B(new Btree());

    if (B->open_to_read(name)) {
	return B.release();
    } else {
	return 0;
    }
}

Btree * Btree_open_to_read_revision(const char * name,
				    unsigned long revision)
{
    AutoPtr<Btree> B(new Btree());

    if (B->open_to_read(name, revision)) {
	return B.release();
    } else {
	return 0;
    }
}

AutoPtr<Bcursor> Btree::Bcursor_create()
{
    return AutoPtr<Bcursor>(new Bcursor(this));
}

#if 0
extern void Bcursor_lose(struct Bcursor * BC)
{
    delete BC;
}
#endif

void
Btree::force_block_to_cursor(struct Cursor * C_, int j)
{
    int n = C_[j].n;
    if (n != C[j].n) {
	C_[j].n = -1;
	block_to_cursor(C_, j, n);
	if (overwritten) return;
    }
}

int
Btree::prev_for_sequential(struct Btree * B, struct Cursor * C, int dummy)
{
    byte * p = C[0].p;
    int c = C[0].c;
    if (c == DIR_START) {
	int n = C[0].n;
	while (true) {
	    n--;
	    if (n < 0) return false;
	    B->read_block(n, p);
	    if (B->overwritten) return false;
	    if (REVISION(p) > 1) {
		B->set_overwritten();
		return false;
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_END(p);
	C[0].n = n;
    }
    c -= D2;
    C[0].c = c;
    return true;
}

int
Btree::next_for_sequential(struct Btree * B, struct Cursor * C, int dummy)
{
    byte * p = C[0].p;
    int c = C[0].c;
    c += D2;
    if (c == DIR_END(p)) {
	int n = C[0].n;
	while (true) {
	    n++;
	    if (n > B->base.get_last_block()) return false;
	    B->read_block(n, p);
	    if (B->overwritten) return false;
	    if (REVISION(p) > 1) {
		B->set_overwritten();
		return false;
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_START;
	C[0].n = n;
    }
    C[0].c = c;
    return true;
}

int
Btree::prev_default(struct Btree * B, struct Cursor * C, int j)
{
    byte * p = C[j].p;
    int c = C[j].c;
    if (c == DIR_START) {
	if (j == B->level) return false;

	if (j + 1 >= B->shared_level) {
	    B->force_block_to_cursor(C, j + 1);
	    if (B->overwritten) return false;
	}
	if (! prev_default(B, C, j + 1)) return false;

	c = DIR_END(p);
    }
    c -= D2;
    C[j].c = c;
    if (j > 0) {
	B->block_to_cursor(C, j - 1, block_given_by(p, c));
	if (B->overwritten) return false;
    }
    return true;
}

int
Btree::next_default(struct Btree * B, struct Cursor * C, int j)
{
    byte * p = C[j].p;
    int c = C[j].c;
    c += D2;
    if (c == DIR_END(p)) {
	if (j == B->level) return false;

	if (j + 1 >= B->shared_level) {
	    B->force_block_to_cursor(C, j + 1);
	    if (B->overwritten) return false;
	}
	if (! next_default(B, C, j + 1)) return false;

	c = DIR_START;
    }
    C[j].c = c;
    if (j > 0) {
	B->block_to_cursor(C, j - 1, block_given_by(p, c));
	if (B->overwritten) return false;
#ifdef BTREE_DEBUG_FULL
	printf("Block in Btree:next_default");
	B->report_block_full(j - 1, C[j - 1].n, C[j - 1].p);
#endif /* BTREE_DEBUG_FULL */
    }
    return true;
}

extern int Bcursor_prev(struct Bcursor * BC)
{
    return BC->prev();
}

extern int Bcursor_next(struct Bcursor * BC)
{
    return BC->next();
}

extern int Bcursor_find_key(struct Bcursor * BC, byte * key, int key_len)
{
    return BC->find_key(key, key_len);
}

extern int Bcursor_get_key(struct Bcursor * BC, struct Btree_item * item)
{
    return BC->get_key(item);
}

extern int Bcursor_get_tag(struct Bcursor * BC, struct Btree_item * item)
{
    return BC->get_tag(item);
}

/*********** B-tree creating ************/

extern void Btree_create(const char * name_, int block_size)
{
    Btree::create(name_, block_size);
}

/*********** B-tree checking ************/

static void print_bytes(int n, const byte * p)
{
    fwrite(p, n, 1, stdout);
}

static void print_key(byte * p, int c, int j)
{
    byte * k = key_of(p, c);
    int l = GETK(k, 0);

    if (j == 0) {
	print_bytes(l - K1 - C2, k + K1);
	printf("/%d", GETC(k, l - C2));
    } else {
	for (int i = K1; i < l; i++) {
	    /*putchar(k[i] < 32 ? '.' : k[i]);*/
	    int ch = k[i];
	    if (ch < 32) printf("/%d", ch); else putchar(ch);
	}
    }
}

static void print_tag(byte * p, int c, int j)
{
    int o = GETD(p, c);
    int o_tag = o + I2 + GETK(p, o + I2);
    int l = o + GETI(p, o) - o_tag;

    if (j == 0) {
	printf("/%d", GETC(p, o_tag));
	print_bytes(l - C2, p + o_tag + C2);
    } else
	printf("--> [%d]", get_int4(p, o_tag));
}

static void print_spaces(int n)
{
    print_bytes(n, (byte *) "                              ");
}

/* report_block(B, m, n, p) prints the block at p, block number n, indented by
   m spaces.
*/

static int block_usage(struct Btree * B, byte * p)
{
    int space = B->block_size - DIR_END(p);
    int free = TOTAL_FREE(p);
    return (space - free) * 100 / space;  /* a percentage */
}

static void report_block(struct Btree * B, int m, int n, byte * p)
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    print_spaces(m);
    printf("[%d] *%d (%d) %d%% ",
	   n, REVISION(p), (dir_end - DIR_START)/D2, block_usage(B, p));

    for (c = DIR_START; c < dir_end; c += D2) {
	if (c == DIR_START + 6) printf ("... ");
	if (c >= DIR_START + 6 && c < dir_end - 6) continue;

	print_key(p, c, j);
	printf(" ");
    }
    printf("\n");
}

void Btree::report_block_full(int m, int n, byte * p)
{
    int j = GET_LEVEL(p);
    int dir_end = DIR_END(p);
    int c;
    printf("\n");
    print_spaces(m);
    printf("Block [%d] level %d, revision *%d items (%d) usage %d%%:\n",
	   n, j, REVISION(p), (dir_end - DIR_START)/D2, block_usage(this, p));

    for (c = DIR_START; c < dir_end; c += D2) {
	print_spaces(m);
	print_key(p, c, j);
	printf(" ");
	print_tag(p, c, j);
	printf("\n");
    }
}

static void failure(int n)
{
    fprintf(stderr, "B-tree error %d\n", n);
    exit(1);
}

void
Btree::block_check(struct Cursor * C_, int j, int opts)
{
    byte * p = C_[j].p;
    int4 n = C_[j].n;
    int c;
    int significant_c = j == 0 ? DIR_START : DIR_START + D2;
	/* the first key in an index block is dummy, remember */

    int max_free = MAX_FREE(p);
    int dir_end = DIR_END(p);
    int total_free = block_size - dir_end;

    if (base.block_free_at_start(n)) failure(0);
    if (base.block_free_now(n)) failure(1);
    base.free_block(n);

    if (j != GET_LEVEL(p)) failure(10);
    if (dir_end <= DIR_START || dir_end > block_size) failure(20);

    if (opts & 1) report_block(this, 3*(level - j), n, p);

    if (opts & 2) report_block_full(3*(level - j), n, p);

    for (c = DIR_START; c < dir_end; c += D2) {
	int o = GETD(p, c);
	if (o > block_size) failure(21);
	if (o - dir_end < max_free) failure(30);

	int kt_len = GETI(p, o);
	if (o + kt_len > block_size) failure(40);
	total_free -= kt_len;

	if (c > significant_c &&
	    compare_keys(key_of(p, c - D2), key_of(p,c)) >= 0)
	    failure(50);
    }
    if (total_free != TOTAL_FREE(p)) failure(60);

    if (j == 0) return;
    for (c = DIR_START; c < dir_end; c += D2) {
	C_[j].c = c;
	block_to_cursor(C_, j - 1, block_given_by(p, c));
	if (overwritten) return;

	block_check(C_, j - 1, opts);

	byte * q = C_[j - 1].p;
	/* if j == 1, and c > DIR_START, the first key of level j - 1 must be >= the
	   key of p, c: */

	if (j == 1 && c > DIR_START)
	    if (compare_keys(key_of(q, DIR_START), key_of(p, c)) < 0)
		failure(70);

	/* if j > 1, and c > DIR_START, the second key of level j - 1 must be >= the key of p, c: */

	if (j > 1 && c > DIR_START && DIR_END(q) > DIR_START + D2 &&
	    compare_keys(key_of(q, DIR_START + D2), key_of(p, c)) < 0)
	    failure(80);

	/* the last key of level j - 1 must be < the key of p, c + D2, if c + D2 < dir_end: */

	if (c + D2 < dir_end &&
	    (j == 1 || DIR_END(q) - D2 > DIR_START) &&
	    compare_keys(key_of(q, DIR_END(q) - D2), key_of(p, c + D2)) >= 0)
	    failure(90);

	if (REVISION(q) > REVISION(p)) failure(91);
    }
}

void
Btree::check(const char * name, const char * opt_string)
{
    struct Btree * B = Btree_open_to_write(name);
    struct Cursor * C = B->C;

    int opts = 0;

    if (B->error) {
	printf("error %d (%s) in opening %s\n",
	       B->error, Btree_strerror(B->error).c_str(), name);
	exit(1);
    }

    for (size_t i = 0; opt_string[i]; i++) {
	switch (opt_string[i]) {
	    case 't': opts |= 1; break; /* short tree printing */
	    case 'f': opts |= 2; break; /* full tree printing */
	    case 'b': opts |= 4; break;
	    case 'v': opts |= 8; break;
	    case '+': opts |= 1 | 4 | 8; break;
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
		B->base.get_last_block());

    int limit = B->base.get_bit_map_size() - 1;

    limit = limit * CHAR_BIT + CHAR_BIT - 1;

    if (opts & 4) {
	for (int j = 0; j <= limit; j++) {
	    putchar(B->base.block_free_at_start(j) ? '.' : '*');
	    if (j > 0) {
		if ((j + 1) % 100 == 0) {
		    putchar('\n');
		} else if ((j + 1) % 10 == 0) {
		    putchar(' ');
		}
	    }
	}
	printf("\n\n");
    }

    if (B->faked_root_block)
	printf("void ");
    else {
	B->block_check(C, B->level, opts);

	/* the bit map should now be entirely clear: */

	if (!B->base.is_empty()) {
	    failure(100);
	}
    }
    Btree_quit(B);
    printf("B-tree checked okay\n");
}

void
Btree_check(const char * name, const char * opt_string)
{
    Btree::check(name, opt_string);
}
