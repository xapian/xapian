/* btree.cc: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#ifdef HAVE_GLIBC
#if !defined _XOPEN_SOURCE
// Need this to get pread and pwrite with GNU libc
#define _XOPEN_SOURCE 500
#endif
#endif

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>   /* for memmove */
#include <limits.h>   /* for CHAR_BIT */

#include <errno.h>
#include "autoptr.h"

#include "btree.h"
#include "btree_util.h"
#include "btree_base.h"

#include "omassert.h"
#include "omdebug.h"
#include <xapian/error.h>
#include "utils.h"

#include <algorithm>  // for std::min()
#include <string>

#ifdef __WIN32__
# include <io.h> // for _commit()
#endif

// Only useful for platforms like Windows which distinguish between text and
// binary files.
#ifndef __WIN32__
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif

using std::min;
using std::string;
 
const string::size_type Btree::max_key_len;

// FIXME: This named constant isn't used everywhere it should be below...
#define BYTES_PER_BLOCK_NUMBER 4

//#define BTREE_DEBUG_FULL 1
#undef BTREE_DEBUG_FULL

#ifdef BTREE_DEBUG_FULL
/*------debugging aids from here--------*/

static void print_key(const byte * p, int c, int j);
static void print_tag(const byte * p, int c, int j);

/*
static void report_cursor(int N, Btree * B, Cursor * C)
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

int sys_open_to_read_no_except(const string & name)
{
    int fd = open(name, O_RDONLY | O_BINARY);
    return fd;
}

int sys_open_to_read(const string & name)
{
    int fd = sys_open_to_read_no_except(name);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " to read: " + strerror(errno);
	throw Xapian::DatabaseOpeningError(message);
    }
    return fd;
}

static int sys_open_to_write_no_except(const string & name)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
    return fd;
}

int sys_open_to_write(const string & name)
{
    int fd = sys_open_to_write_no_except(name);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " to write: " + strerror(errno);
	throw Xapian::DatabaseOpeningError(message);
    }
    return fd;
}

static int sys_open_for_readwrite(const string & name)
{
    int fd = open(name, O_RDWR | O_BINARY);
    if (fd < 0) {
	string message = string("Couldn't open ")
		+ name + " read/write: " + strerror(errno);
	throw Xapian::DatabaseOpeningError(message);
    }
    return fd;
}

static void sys_write_bytes(int h, int n, const char * p)
{
    while (1) {
	ssize_t bytes_written = write(h, p, n);
	if (bytes_written == n) {
	    // normal case - write succeeded, so return.
	    return;
	} else if (bytes_written == -1) {
	    if (errno == EINTR) continue;
	    string message = "Error writing block: ";
	    message += strerror(errno);
	    throw Xapian::DatabaseError(message);
	} else if (bytes_written == 0) {
	    string message = "Error writing block: wrote no data";
	    throw Xapian::DatabaseError(message);
	} else if (bytes_written < n) {
	    /* Wrote part of the block, which is not an error.  We should
	     * continue writing the rest of the block.
	     */
	    n -= bytes_written;
	    p += bytes_written;
	}
    }
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
	    retval.append(buf, bytes_read);
	    bytes_to_read -= bytes_read;
	    if (bytes_to_read == 0) {
		break;
	    }
	} else if (bytes_read == 0) {
	    // end of file, we're finished
	    break;
	} else if (bytes_read == -1) {
	    if (errno == EINTR) continue;
	    string message = "Error reading all bytes: ";
	    message += strerror(errno);
	    throw Xapian::DatabaseError(message);
	}
    }
    return retval;
}

void
sys_write_string(int h, const string &s)
{
    sys_write_bytes(h, s.length(), s.data());
}

int sys_flush(int h) {
#if defined HAVE_FDATASYNC
    return (fdatasync(h) != -1);
#elif defined HAVE_FSYNC
    return (fsync(h) != -1);
#elif defined __WIN32__
    return (_commit(h) != -1);
#else
#error "Have neither fsync() nor fdatasync() nor _commit() - cannot sync."
#endif
}

static void sys_unlink(const string &filename)
{
    if (unlink(filename) == -1) {
	string message = "Failed to unlink ";
	message += filename;
	message += ": ";
	message += strerror(errno);
	throw Xapian::DatabaseCorruptError(message);
    }
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

/// read_block(n, p) reads block n of the DB file to address p.
void
Btree::read_block(uint4 n, byte * p)
{
    /* Use the base bit_map_size not the bitmap's size, because
     * the latter is uninitialised in readonly mode.
     */
    Assert(n / CHAR_BIT < base.get_bit_map_size());

#ifdef HAVE_PREAD
    off_t offset = (off_t)block_size * n;
    int m = block_size;
    while (1) {
	ssize_t bytes_read = pread(handle, (char *)p, m, offset);
	// normal case - read succeeded, so return.
	if (bytes_read == m) return;
	if (bytes_read == -1) {
	    if (errno == EINTR) continue;
	    string message = "Error reading block " + om_tostring(n) + ": ";
	    message += strerror(errno);
	    throw Xapian::DatabaseError(message);
	} else if (bytes_read == 0) {
	    string message = "Error reading block " + om_tostring(n) + ": got end of file";
	    throw Xapian::DatabaseError(message);
	} else if (bytes_read < m) {
	    /* Read part of the block, which is not an error.  We should
	     * continue reading the rest of the block.
	     */
	    m -= bytes_read;
	    p += bytes_read;
	    offset += bytes_read;
	}
    }
#else
    if (lseek(handle, (off_t)block_size * n, SEEK_SET) == -1) {
	string message = "Error seeking to block: ";
	message += strerror(errno);
	throw Xapian::DatabaseError(message);
    }

    int m = block_size;
    while (1) {
	ssize_t bytes_read = read(handle, (char *)p, m);
	// normal case - read succeeded, so return.
	if (bytes_read == m) return;
	if (bytes_read == -1) {
	    if (errno == EINTR) continue;
	    string message = "Error reading block " + om_tostring(n) + ": ";
	    message += strerror(errno);
	    throw Xapian::DatabaseError(message);
	} else if (bytes_read == 0) {
	    string message = "Error reading block " + om_tostring(n) + ": got end of file";
	    throw Xapian::DatabaseError(message);
	} else if (bytes_read < m) {
	    /* Read part of the block, which is not an error.  We should
	     * continue reading the rest of the block.
	     */
	    m -= bytes_read;
	    p += bytes_read;
	}
    }
#endif
}

/** write_block(n, p) writes block n in the DB file from address p.
 *  In writing we check to see if the DB file has as yet been
 *  modified. If not (so this is the first write) the old base is
 *  deleted. This prevents the possibility of it being openend
 *  subsequently as an invalid base.
 */
void
Btree::write_block(uint4 n, const byte * p)
{
    /* Check that n is in range. */
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
	}
    }

#ifdef HAVE_PWRITE
    off_t offset = (off_t)block_size * n;
    int m = block_size;
    while (true) {
	ssize_t bytes_written = pwrite(handle, p, m, offset);
	if (bytes_written == m) {
	    // normal case - write succeeded, so return.
	    return;
	} else if (bytes_written == -1) {
	    if (errno == EINTR) continue;
	    string message = "Error writing block: ";
	    message += strerror(errno);
	    throw Xapian::DatabaseError(message);
	} else if (bytes_written == 0) {
	    string message = "Error writing block: wrote no data";
	    throw Xapian::DatabaseError(message);
	} else if (bytes_written < m) {
	    /* Wrote part of the block, which is not an error.  We should
	     * continue writing the rest of the block.
	     */
	    m -= bytes_written;
	    p += bytes_written;
	    offset += bytes_written;
	}
    }
#else
    if (lseek(handle, (off_t)block_size * n, SEEK_SET) == -1) {
	string message = "Error seeking to block: ";
	message += strerror(errno);
	throw Xapian::DatabaseError(message);
    }

    sys_write_bytes(handle, block_size, (const char *)p);
#endif
}


/* A note on cursors:

   Each B-tree level has a corresponding array element C[j] in a
   cursor, C. C[0] is the leaf (or data) level, and C[B->level] is the
   root block level. Within a level j,

       C[j].p  addresses the block
       C[j].c  is the offset into the directory entry in the block
       C[j].n  is the number of the block at C[j].p

   A look up in the B-tree causes navigation of the blocks starting
   from the root. In each block, p,  we find an offset, c, to an item
   which gives the number, n, of the block for the next level. This
   leads to an array of values p,c,n which are held inside the cursor.

   Structure B has a built-in cursor, at B->C. But other cursors may
   be created. If BC is a created cursor, BC->C is the cursor in the
   sense given above, and BC->B is the handle for the B-tree again.
*/


void
Btree::set_overwritten()
{
    // initial debugging line
    DEBUGLINE(DB, "overwritten set to true");
    overwritten = true;
    throw Xapian::DatabaseModifiedError("Db block overwritten");
}

/* block_to_cursor(C, j, n) puts block n into position C[j] of cursor
   C, writing the block currently at C[j] back to disk if necessary.
   Note that

       C[j].rewrite

   is true iff C[j].n is different from block n in file DB. If it is
   false no rewriting is necessary.
*/

void
Btree::block_to_cursor(Cursor * C_, int j, uint4 n)
{
    if (n == C_[j].n) return;
    byte * p = C_[j].p;

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
	/* unsigned comparison */
	if (REVISION(p) > REVISION(C_[j + 1].p)) {
	    set_overwritten();
	    return;
	}
    }
    AssertEq(j, GET_LEVEL(p));
}

/** set_block_given_by(p, c, n) finds the item at block address p,
 *  directory offset c, and sets its tag value to n.  For blocks not at
 *  the data level, when GET_LEVEL(p) > 0, the tag of an item is just
 *  the block number of another block in the B-tree structure.
 */

static void set_block_given_by(byte * p, int c, uint4 n)
{
    c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - BYTES_PER_BLOCK_NUMBER;
			   /* c is an offset to a block number */
    set_int4(p, c, n);
}

/** block_given_by(p, c) finds the item at block address p, directory offset c,
 *  and returns its tag value as an integer.
 */
uint4
Btree::block_given_by(const byte * p, int c)
{
    c = GETD(p, c);        /* c is an offset to an item */
    c += GETI(p, c) - BYTES_PER_BLOCK_NUMBER;
			   /* c is an offset to a block number */
    return get_int4(p, c);
}

/** Btree::alter(); is called when the B-tree is to be altered.
 
   It causes new blocks to be forced for the current set of blocks in
   the cursor.

   The point is that if a block at level 0 is to be altered it may get
   a new number. Then the pointer to this block from level 1 will need
   changing. So the block at level 1 needs altering and may get a new
   block number. Then the pointer to this block from level 2 will need
   changing ... and so on back to the root.

   The clever bit here is spotting the cases when we can make an early
   exit from this process. If C[j].rewrite is true, C[j+k].rewrite
   will be true for k = 1,2 ... We have been through all this before,
   and there is no need to do it again. If C[j].n was free at the
   start of the transaction, we can copy it back to the same place
   without violating the integrity of the B-tree. We don't then need a
   new n and can return. The corresponding C[j].rewrite may be true or
   false in that case.
*/

void
Btree::alter()
{
    int j = 0;
    byte * p = C[j].p;
    while (true) {
	if (C[j].rewrite) return; /* all new, so return */
	C[j].rewrite = true;

	uint4 n = C[j].n;
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

/** compare_keys(k1, k2) compares two keys pointed to by k1 and k2.

   (Think of them as the key part of two items, with the pointers
   addressing the length indicator at the beginning of the keys.) The
   result is <0, 0, or >0 according as k1 precedes, is equal to, or
   follows k2. The comparison is for byte sequence collating order,
   taking lengths into account. So if the keys are made up of lower
   case ASCII letters we get alphabetical ordering.

   Now remember that items are added into the B-tree in fastest time
   when they are preordered by their keys. This is therefore the piece
   of code that needs to be followed to arrange for the preordering.

   This is complicated by the fact that keys have two parts - a value
   and then a count.  We first compare the values, and only if they
   are equal do we compare the counts.
*/

int Btree::compare_keys(const byte * key1, const byte * key2)
{
    int key1_len = GETK(key1, 0);
    int key2_len = GETK(key2, 0);
    if (key1_len == key2_len)
	return memcmp(key1 + K1, key2 + K1, key1_len - K1);

    int k_smaller = (key2_len < key1_len ? key2_len : key1_len) - C2;

    // Compare the first part of the keys
    int diff = memcmp(key1 + K1, key2 + K1, k_smaller - K1);
    if (diff != 0) return diff;

    diff = key1_len - key2_len;
    if (diff != 0) return diff;

    // Compare the count
    return memcmp(key1 + k_smaller, key2 + k_smaller, C2);
}

/** find_in_block(p, key, offset, c) searches for the key in the block at p.
 
   offset is D2 for a data block, and 0 for and index block, when the
   first key is dummy and never needs to be tested. What we get is the
   directory entry to the last key <= the key being searched for.

   The lookup is by binary chop, with i and j set to the left and
   right ends of the search area. In sequential addition, c will often
   be the answer, so we test the keys round c and move i and j towards
   c if possible.
*/

int Btree::find_in_block(const byte * p, const byte * key, int offset, int c)
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

/** find(C_) searches for the key of B->kt in the B-tree.
 
   Result is true if found, false otherwise.  When false, the B_tree
   cursor is positioned at the last key in the B-tree <= the search
   key.  Goes to first (null) item in B-tree when key length == 0.
*/

bool
Btree::find(Cursor * C_)
{
    // Note: the parameter is needed when we're called by BCursor
    const byte * p;
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
    return (compare_keys(kt + I2, key_of(p, c)) == 0);
}

/** compress(p) compresses the block at p by shuffling all the items up to the end.
   
   MAX_FREE(p) is then maximized, and is equal to TOTAL_FREE(p).
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

static void form_null_key(byte * b, uint4 n)
{
    set_int4(b, I3, n);
    SETK(b, I2, K1);     /* null key */
    SETI(b, 0, I3 + 4);  /* total length */
}


/** Btree needs to gain a new level to insert more items: so split root block
 *  and construct a new one.
 */
void
Btree::split_root(Cursor * C_, int j)
{
    /* gain a level */
    level ++;

    /* check level overflow */
    AssertNe(level, BTREE_CURSOR_LEVELS);

    byte * q = zeroed_new(block_size);
    if (q == 0) {
	throw std::bad_alloc();
    }
    C_[j].p = q;
    C_[j].split_p = zeroed_new(block_size);
    if (C_[j].split_p == 0) {
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

    uint4 old_root = C_[j - 1].split_n;
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
void Btree::make_index_item(byte * result, unsigned int result_len,
			    const byte * prevkey, const byte * newkey,
			    const uint4 blocknumber, bool truncate) const
{
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

    // FIXME: abort not good - better than buffer overrun though
    if (I2 + i + C2 + 4 > (int)result_len) abort();

    SETI(result, 0, I2 + i + C2 + 4); // Set item length
    SETK(result, I2, i + C2);    // Set key length
    memmove(result + I2 + K1, newkey + K1, i - K1); // Copy the main part of the key
    memmove(result + I2 + i, newkey + newkey_len, C2); // copy count part

    // Set tag contents to block number
    set_int4(result, I2 + i + C2, blocknumber);
}

/** enter_key(C, j, prevkey, newkey) is called after a block split.
  
   It enters in the block at level C[j] a separating key for the block
   at level C[j - 1]. The key itself is newkey. prevkey is the
   preceding key, and at level 1 newkey can be trimmed down to the
   first point of difference to prevkey for entry in C[j].

   This code looks longer than it really is. If j exceeds the number
   of B-tree levels the root block has split and we have to construct
   a new one, but this is a rare event.

   The key is constructed in b, with block number C[j - 1].n as tag,
   and this is added in with add_item. add_item may itself cause a
   block split, with a further call to enter_key. Hence the recursion.
*/
void
Btree::enter_key(Cursor * C_, int j, byte * prevkey, byte * newkey)
{
    Assert(compare_keys(prevkey, newkey) < 0);
    Assert(j >= 1);
    if (j > level) split_root(C_, j);

    /*  byte * p = C_[j - 1].p;  -- see below */

    uint4 blocknumber = C_[j - 1].n;

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
	   uint4 n = get_int4(newkey, newkey_len);
	   int new_total_free = TOTAL_FREE(p) + (newkey_len - K1);
	   form_null_key(newkey - I2, n);
	   SET_TOTAL_FREE(p, new_total_free);
       }
     */

    C_[j].c = find_in_block(C_[j].p, b + I2, 0, 0) + D2;
    C_[j].rewrite = true; /* a subtle point: this *is* required. */
    add_item(C_, b, j);
}

/* split_off(C, j, c, p, q) splits the block at p at directory offset c.

   In fact p is just C[j].p, and q is C[j].split_p, a block buffer
   provided at each cursor level to accommodate the split.

   The first half block goes into q, with block number in C[j].split_n
   copied from C[j].n, the second half into p with a new block number.
*/

void
Btree::split_off(Cursor * C_, int j, int c, byte * p, byte * q)
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

/** mid_point(p) finds the directory entry in c that determines the
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

/** add_item_to_block(p, kt_, c) adds item kt_ to the block at p.
 
   c is the offset in the directory that needs to be expanded to
   accommodate the new entry for the item. We know before this is
   called that there is enough room, so it's just a matter of byte
   shuffling.
*/

void
Btree::add_item_to_block(byte * p, byte * kt_, int c)
{
    int dir_end = DIR_END(p);
    int kt_len = GETI(kt_, 0);
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
    memmove(p + o, kt_, kt_len);

    SET_MAX_FREE(p, new_max);

    SET_TOTAL_FREE(p, new_total);
}

/** Btree::add_item(C_, kt_, j) adds item kt_ to the block at cursor level C_[j].
 *
 *  If there is not enough room the block splits and the item is then
 *  added to the appropriate half.
 */
void
Btree::add_item(Cursor * C_, byte * kt_, int j)
{
    byte * p = C_[j].p;
    int c = C_[j].c;
    uint4 n;

    int kt_len = GETI(kt_, 0);
    int needed = kt_len + D2;
    if (TOTAL_FREE(p) < needed) {
	int m;
	byte * q = C_[j].split_p;
	int add_to_upper_half;

        // Prepare to split p. After splitting, the block is in two halves, the
        // lower half is q, the upper half p again. add_to_upper_half becomes
        // true when the item gets added to p, false when it gets added to q.

        if (seq_count < 0) {
	    // If we're not in sequential mode, we split at the mid point
	    // of the node.
            m = mid_point(p);
            split_off(C_, j, m, p, q);
            add_to_upper_half = c >= m;
        } else {
	    // During sequential addition, split at the insert point
            m = c;
            split_off(C_, j, m, p, q);
	    // And add item to lower half if q has room, otherwise upper half
            add_to_upper_half = TOTAL_FREE(q) < needed;
	}

	if (add_to_upper_half) {
	    c -= (m - DIR_START);
	    Assert(seq_count < 0 || c <= DIR_START + D2);
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(p));
	    add_item_to_block(p, kt_, c);
	    n = C_[j].n;
	} else {
	    Assert(c >= DIR_START);
	    Assert(c <= DIR_END(q));
	    add_item_to_block(q, kt_, c);
	    n = C_[j].split_n;
	}
	write_block(C_[j].split_n, q);

	enter_key(C_, j + 1,                /* enters a separating key at level j + 1 */
		  key_of(q, DIR_END(q) - D2), /* - between the last key of block q, */
		  key_of(p, DIR_START));      /* - and the first key of block p */
    } else {
	add_item_to_block(p, kt_, c);
	n = C_[j].n;
    }
    if (j == 0) {
	changed_n = n;
	changed_c = c;
    }
}

/** Btree::delete_item(C_, j, repeatedly) is (almost) the converse of add_item.
 *
 * If repeatedly is true, the process repeats at the next level when a
 * block has been completely emptied, freeing the block and taking out
 * the pointer to it.  Emptied root blocks are also removed, which
 * reduces the number of levels in the B-tree.
 */
void
Btree::delete_item(Cursor * C_, int j, bool repeatedly)
{
    byte * p = C_[j].p;
    int c = C_[j].c;
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
	    base.free_block(C_[j].n);
	    C_[j].rewrite = false;
	    C_[j].n = BLK_UNUSED;
	    C_[j + 1].rewrite = true;  /* *is* necessary */
	    delete_item(C_, j + 1, true);
	}
    } else {
	/* j == B->level */
	while (dir_end == DIR_START + D2 && j > 0) {
	    /* single item in the root block, so lose a level */
	    uint4 new_root = block_given_by(p, DIR_START);
	    delete [] p;
	    C_[j].p = 0;
	    base.free_block(C_[j].n);
	    C_[j].rewrite = false;
	    C_[j].n = BLK_UNUSED;
	    delete [] C_[j].split_p;
	    C_[j].split_p = 0;
	    C_[j].split_n = BLK_UNUSED;
	    level--;

	    block_to_cursor(C_, level, new_root);
	    if (overwritten) return;

	    j--;
	    p = C_[j].p;
	    dir_end = DIR_END(p); /* prepare for the loop */
	}
    }
}

/* debugging aid:
static addcount = 0;
*/

/** add_kt(found) adds the item (key-tag pair) at B->kt into the
   B-tree, using cursor C.
   
   found == find() is handed over as a parameter from Btree::add.
   Btree::alter() prepares for the alteration to the B-tree. Then
   there are a number of cases to consider:

     If an item with the same key is in the B-tree (found is true),
     the new kt replaces it.

     If then kt is smaller, or the same size as, the item it replaces,
     kt is put in the same place as the item it replaces, and the
     TOTAL_FREE measure is reduced.

     If kt is larger than the item it replaces it is put in the
     MAX_FREE space if there is room, and the directory entry and
     space counts are adjusted accordingly.

     - But if there is not room we do it the long way: the old item is
     deleted with delete_item and kt is added in with add_item.

     If the key of kt is not in the B-tree (found is false), the new
     kt is added in with add_item.
*/

int
Btree::add_kt(int found)
{
    int components = 0;

    if (overwritten) return 0;

    /*
    {
	printf("%d) %s ", addcount++, (found ? "replacing" : "adding"));
	print_bytes(kt[I2] - K1 - C2, kt + I2 + K1); putchar('\n');
    }
    */
    alter();

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

/* delete_kt() corresponds to add_kt(found), but there are only
   two cases: if the key is not found nothing is done, and if it is
   found the corresponding item is deleted with delete_item.
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
	print_bytes(B->kt[I2] - K1 - C2, B->kt + I2 + K1); putchar('\n');
    }
    */
    if (found) {
	components = components_of(C[0].p, C[0].c);
	alter();
	delete_item(C, 0, true);
    }
    return components;
}

/* Btree::form_key(key) treats address kt as an item holder and fills in
the key part:

	   (I) K key c (C tag)

The bracketed parts are left blank. The key is filled in with key_len bytes and
K set accordingly. c is set to 1.
*/

void Btree::form_key(const string & key)
{
    Assert(key.length() <= max_key_len);

    // This just so it doesn't fall over horribly in non-debug builds.
    string::size_type key_len = min(key.length(), max_key_len);

    int c = I2;
    SETK(kt, c, key_len + K1 + C2);
    c += K1;
    memmove(kt + c, key.data(), key_len);
    c += key_len;
    SETC(kt, c, 1);
}

/* Btree::add(key, tag) adds the key/tag item to the
   B-tree, replacing any existing item with the same key. The result is true
   for an addition, false for a replacement.

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

bool
Btree::add(const string &key, const string &tag)
{
    Assert(!overwritten);

    form_key(key);

    int ck = GETK(kt, I2) + I2 - C2;  // offset to the counter in the key
    int ct = ck + C2;                 // offset to the tag counter
    int cd = ct + C2;                 // offset to the tag data
    int L = max_item_size - cd;	      // largest amount of tag data for any tag
    int first_L = L;                  // - amount for tag1
    int found = find(C);
    if (full_compaction && !found) {
	byte * p = C[0].p;
	int n = TOTAL_FREE(p) % (max_item_size + D2) - D2 - cd;
	if (n > 0) first_L = n;
    }

    // a null tag must be added in of course
    int m = tag.empty() ? 1 : (tag.length() - first_L + L - 1) / L + 1;
				      // there are m items to add
    /* FIXME: sort out this error higher up and turn this into
     * an assert.
     */
    if (m >= BYTE_PAIR_RANGE) return false;

    int n = 0; // initialise to shut off warning
				      // - and there will be n to delete
    int o = 0;                        // offset into the tag
    int residue = tag.length();       // bytes of the tag remaining to add in
    int replacement = false;          // has there been a replacement ?
    int i;
    for (i = 1; i <= m; i++) {
	int l = i == m ? residue :
		i == 1 ? first_L : L;
	Assert(cd + l <= block_size);
	Assert(string::size_type(o + l) <= tag.length());
	memmove(kt + cd, tag.data() + o, l);
	o += l;
	residue -= l;

	SETC(kt, ck, i);
	SETC(kt, ct, m);
	SETI(kt, 0, cd + l);
	if (i > 1) found = find(C);
	n = add_kt(found);
	if (n > 0) replacement = true;
    }
    /* o == tag.length() here, and n may be zero */
    for (i = m + 1; i <= n; i++) {
	SETC(kt, ck, i);
	delete_kt();

	if (overwritten) return false;
    }
    if (replacement) return false;
    item_count++;
    return true;
}

/* Btree::del(key) returns false if the key is not in the B-tree,
   otherwise deletes it and returns true.

   Again, this is parallel to Btree::add, but simpler in form.
*/

bool
Btree::del(const string &key)
{
    Assert(!overwritten);

    if (key.empty()) return false;
    form_key(key);

    int n = delete_kt();  /* there are n items to delete */
    if (n <= 0) return false;

    for (int i = 2; i <= n; i++) {
	int c = GETK(kt, I2) + I2 - C2;
	SETC(kt, c, i);

	delete_kt();

	if (overwritten) return false;
    }

    item_count--;
    return true;
}

bool
Btree::find_key(const string &key)
{
    Assert(!overwritten);

    form_key(key);
    return find(C);
}

bool
Btree::find_tag(const string &key, string * tag)
{
    Assert(!overwritten);

    form_key(key);
    if (!find(C)) return false;

    int n = components_of(C[0].p, C[0].c);
				    /* n components to join */
    int ck = GETK(kt, I2) + I2 - C2;/* offset to the key counter */
    int cd = ck + 2 * C2;           /* offset to the tag data */
    int i = 1;                      /* see below */

    byte * p = item_of(C[0].p, C[0].c); /* pointer to current component */
    
    tag->resize(0);
    if (n > 1) {
	string::size_type space_for_tag = (string::size_type) max_item_size * n;
	tag->reserve(space_for_tag);
    }	

    while (true) { // FIXME: code to do very similar thing in bcursor.cc...
	/* number of bytes to extract from current component */
	int l = GETI(p, 0) - cd;
	tag->append(reinterpret_cast<char *>(p + cd), l);

	if (i == n) break;
	i++;
	SETC(kt, ck, i);
	find(C);

	if (overwritten) return false;

	p = item_of(C[0].p, C[0].c);
    }
 
    return true;
}

void
Btree::set_full_compaction(bool parity)
{
    Assert(!overwritten);

    if (parity) seq_count = 0;
    full_compaction = parity;
}

/************ B-tree opening and closing ************/

bool
Btree::basic_open(const string & name_,
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

	for (size_t i = 0; i < basenames.size(); ++i) {
	    base_ok[i] = bases[i].read(name, basenames[i], err_msg);
	}

	// FIXME: assumption that there are only two bases
	if (base_ok[0] && base_ok[1]) both_bases = true;
	if (!base_ok[0] && !base_ok[1]) {
	    string message = "Error opening table `";
	    message += name_;
	    message += "':\n";
	    message += err_msg;
	    throw Xapian::DatabaseOpeningError(message);
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
	    DEBUGLINE(UNKNOWN, "Checking (ch == " << ch << ") against "
		      "basenames[" << i << "] == " << basenames[i]);
	    DEBUGLINE(UNKNOWN, "bases[" << i << "].get_revision() == " <<
		      bases[i].get_revision());
	    DEBUGLINE(UNKNOWN, "base_ok[" << i << "] == " << base_ok[i]);
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
	Assert(basep);

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

    /* k holds constructed items as well as keys */
    kt = zeroed_new(block_size);
    if (kt == 0) {
	throw std::bad_alloc();
    }

    max_item_size = (block_size - DIR_START - BLOCK_CAPACITY * D2) / BLOCK_CAPACITY;

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

	SETC(p, o, 1); o -= C2;        // number of components in tag
	SETC(p, o, 1); o -= K1;        // component one in key
	SETK(p, o, K1 + C2); o -= I2;  // null key length
	SETI(p, o, I3 + 2 * C2);       // length of the item
	SETD(p, DIR_START, o);         // its directory entry
	SET_DIR_END(p, DIR_START + D2);// the directory size

	o -= (DIR_START + D2);
	SET_MAX_FREE(p, o);
	SET_TOTAL_FREE(p, o);
	SET_LEVEL(p, 0);

	if (!writable) {
	    /* reading - revision number doesn't matter as long as
	     * it's not greater than the current one. */
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
Btree::do_open_to_write(const string & name_,
			bool revision_supplied,
			uint4 revision_)
{
    /* FIXME: do the exception safety the right way, by making all the
     * parts into sensible objects.
     */
    if (!basic_open(name_, revision_supplied, revision_)) {
	if (!revision_supplied) {
	    throw Xapian::DatabaseOpeningError("Failed to open for writing");
	}
	/* When the revision is supplied, it's not an exceptional
	 * case when open failed, so we just return false here.
	 */
	return false;
    }

    writable = true;

    handle = sys_open_for_readwrite(name + "DB");

    for (int j = 0; j <= level; j++) {
	C[j].n = BLK_UNUSED;
	C[j].split_n = BLK_UNUSED;
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

    // swap for writing
    other_base_letter = base_letter == 'A' ? 'B' : 'A';

    changed_n = 0;
    changed_c = DIR_START;
    seq_count = SEQ_START_POINT;

    return true;
}

void
Btree::open_to_write(const string & name_)
{
    // Any errors are thrown if revision_supplied is false
    (void)do_open_to_write(name_, false, 0);
}

bool
Btree::open_to_write(const string & name_, uint4 n)
{
    return do_open_to_write(name_, true, n);
}

Btree::Btree()
	: revision_number(0),
	  item_count(0),
	  block_size(0),
	  other_revision_number(0),
	  both_bases(false),
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
	  overwritten(false),
	  seq_count(0),
	  changed_n(0),
	  changed_c(0),
	  max_item_size(0),
	  Btree_modified(false),
	  full_compaction(false),
	  writable(false),
	  dont_close_handle(false)
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
	throw Xapian::DatabaseError("Can't delete file: `" + filename +
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
Btree::create(const string &name_, int block_size)
{
    if (block_size > BYTE_PAIR_RANGE) {
	/* block size too large (64K maximum) */
	throw Xapian::InvalidArgumentError("Btree block size too large");
    }

    if (block_size < 2048) {
	/* block size far too small */
	throw Xapian::InvalidArgumentError("Btree block size too small");
    }

    /* indeed it will need to be a good bit bigger */

    /* write initial values of to files */
    {
	/* create the base file */
	Btree_base base;
	base.set_block_size(block_size);
	base.set_have_fakeroot(true);
	base.set_sequential(true);
	base.write_to_file(name_ + "baseA");

	/* create the main file */
	{
	    int h = sys_open_to_write(name_ + "DB");      /* - null */
	    if (h == -1 || !sys_close(h)) {
		string message = "Error creating DB file: ";
		message += strerror(errno);
		throw Xapian::DatabaseOpeningError(message);
	    }
	}
    }
}

Btree::~Btree() {
    if (handle != -1) {
	// If an error occurs here, we just ignore it, since we're just
	// trying to free everything.
	if (!dont_close_handle) (void)sys_close(handle);
	handle = -1;
    }

    for (int j = level; j >= 0; j--) {
	delete [] C[j].p;
	delete [] C[j].split_p;
    }

    delete [] kt;
    delete [] buffer;
}

void
Btree::commit(uint4 revision)
{
    Assert(!overwritten);

    int j;
    if (revision < next_revision) {
	throw Xapian::DatabaseError("New revision too low");
    }

    for (j = level; j >= 0; j--) {
	if (C[j].rewrite) {
	    write_block(C[j].n, C[j].p);
	}
    }

    if (!sys_flush(handle)) {
	if (!dont_close_handle) (void)sys_close(handle);
	handle = -1;
	throw Xapian::DatabaseError("Can't commit new revision - failed to close DB");
    }

    if (Btree_modified) {
	faked_root_block = false;
    }

    if (faked_root_block) {
	/* We will use a dummy bitmap. */
	base.clear_bit_map();
    }

    base.set_revision(revision);
    base.set_root(C[level].n);
    base.set_level(level);
    base.set_item_count(item_count);
    base.set_have_fakeroot(faked_root_block);
    base.set_sequential(sequential);

    {
	int tmp = base_letter;
	base_letter = other_base_letter;
	other_base_letter = tmp;
    }
    both_bases = true;
    other_revision_number = revision_number;
    revision_number = revision;
    root = C[level].n;

    overwritten = false;
    Btree_modified = false;

    prev_ptr = &Btree::prev_default;
    next_ptr = &Btree::next_default;

    for (int i = 0; i < BTREE_CURSOR_LEVELS; ++i) {
	C[i].n = BLK_UNUSED;
	C[i].split_n = BLK_UNUSED;
	C[i].c = -1;
	C[i].rewrite = false;
    }
 
    base.write_to_file(name + "base" + (char)base_letter);
    base.commit();

    next_revision = revision_number + 1;

    read_root();

    changed_n = 0;
    changed_c = DIR_START;
    seq_count = SEQ_START_POINT;
}

/************ B-tree reading ************/

void
Btree::do_open_to_read(const string & name_,
		       bool revision_supplied,
		       uint4 revision_)
{
    if (!basic_open(name_, revision_supplied, revision_)) {
	throw Xapian::DatabaseOpeningError("Failed to open table for reading");
    }

    handle = sys_open_to_read(name + "DB");

    if (sequential) {
	prev_ptr = &Btree::prev_for_sequential;
	next_ptr = &Btree::next_for_sequential;
    } else {
	prev_ptr = &Btree::prev_default;
	next_ptr = &Btree::next_default;
    }

    C[level].n = BLK_UNUSED;
    C[level].p = new byte[block_size];
    if (C[level].p == 0) {
	throw std::bad_alloc();
    }

    read_root();
}

void
Btree::open_to_read(const string & name_)
{
    do_open_to_read(name_, false, 0);
}

void
Btree::open_to_read(const string & name_, uint4 n)
{
    do_open_to_read(name_, true, n);
}

void
Btree::open_to_read(const Btree &btree)
{
    name = btree.name;

    both_bases = btree.both_bases;
    {
	Btree_base tmp(btree.base);
	base.swap(tmp);
    }

    revision_number = btree.revision_number;
    block_size = btree.block_size;
    root = btree.root;
    level = btree.level;
    //bit_map_size = btree.bit_map_size;
    item_count = btree.item_count;
    faked_root_block = btree.faked_root_block;
    sequential = btree.sequential;
    other_revision_number = btree.other_revision_number;

    /* k holds constructed items as well as keys */
    kt = zeroed_new(block_size);
    if (kt == 0) {
	throw std::bad_alloc();
    }

    max_item_size = btree.max_item_size;

    base_letter = btree.base_letter;
    next_revision = revision_number + 1;

    dont_close_handle = true;
    handle = btree.handle;

    if (sequential) {
	prev_ptr = &Btree::prev_for_sequential;
	next_ptr = &Btree::next_for_sequential;
    } else {
	prev_ptr = &Btree::prev_default;
	next_ptr = &Btree::next_default;
    }

    C[level].n = BLK_UNUSED;
    C[level].p = new byte[block_size];
    if (C[level].p == 0) {
	throw std::bad_alloc();
    }

    C[level].n = btree.C[level].n;
    memcpy(C[level].p, btree.C[level].p, block_size);
}

void
Btree::reopen_to_read(const Btree &btree)
{
    Assert(!writable);
    Assert(name == btree.name);
    Assert(dont_close_handle);
    Assert(handle == btree.handle);

    both_bases = btree.both_bases;
    {
	Btree_base tmp(btree.base);
	base.swap(tmp);
    }

    revision_number = btree.revision_number;
    root = btree.root;
    if (level != btree.level) {
	C[btree.level].p = C[level].p;
	C[level].n = BLK_UNUSED;
	C[level].p = 0;
	level = btree.level;
    }
    item_count = btree.item_count;
    faked_root_block = btree.faked_root_block;
    sequential = btree.sequential;
    other_revision_number = btree.other_revision_number;

    base_letter = btree.base_letter;
    next_revision = revision_number + 1;

    if (sequential) {
	prev_ptr = &Btree::prev_for_sequential;
	next_ptr = &Btree::next_for_sequential;
    } else {
	prev_ptr = &Btree::prev_default;
	next_ptr = &Btree::next_default;
    }

    C[level].n = btree.C[level].n;
    memcpy(C[level].p, btree.C[level].p, block_size);
}

bool
Btree::prev_for_sequential(Cursor * C_, int /*dummy*/)
{
    byte * p = C_[0].p;
    int c = C_[0].c;
    if (c == DIR_START) {
	uint4 n = C_[0].n;
	while (true) {
	    if (n == 0) return false;
	    n--;
	    read_block(n, p);
	    if (overwritten) return false;
	    if (REVISION(p) > 1) {
		set_overwritten();
		return false;
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_END(p);
	C_[0].n = n;
    }
    c -= D2;
    C_[0].c = c;
    return true;
}

bool
Btree::next_for_sequential(Cursor * C_, int /*dummy*/)
{
    byte * p = C_[0].p;
    int c = C_[0].c;
    c += D2;
    if (c == DIR_END(p)) {
	uint4 n = C_[0].n;
	while (true) {
	    n++;
	    if (n > base.get_last_block()) return false;
	    read_block(n, p);
	    if (overwritten) return false;
	    if (REVISION(p) > 1) {
		set_overwritten();
		return false;
	    }
	    if (GET_LEVEL(p) == 0) break;
	}
	c = DIR_START;
	C_[0].n = n;
    }
    C_[0].c = c;
    return true;
}

bool
Btree::prev_default(Cursor * C_, int j)
{
    byte * p = C_[j].p;
    int c = C_[j].c;
    Assert(c >= 0);
    Assert(c < 65536);
    if (c == DIR_START) {
	if (j == level) return false;
	if (!prev_default(C_, j + 1)) return false;
	c = DIR_END(p);
    }
    c -= D2;
    C_[j].c = c;
    if (j > 0) {
	block_to_cursor(C_, j - 1, block_given_by(p, c));
	if (overwritten) return false;
    }
    return true;
}

bool
Btree::next_default(Cursor * C_, int j)
{
    byte * p = C_[j].p;
    int c = C_[j].c;
    c += D2;
    Assert(c >= 0);
    Assert(c < 65536);
    if (c == DIR_END(p)) {
	if (j == level) return false;
	if (!next_default(C_, j + 1)) return false;
	c = DIR_START;
    }
    C_[j].c = c;
    if (j > 0) {
	block_to_cursor(C_, j - 1, block_given_by(p, c));
	if (overwritten) return false;
#ifdef BTREE_DEBUG_FULL
	printf("Block in Btree:next_default");
	report_block_full(j - 1, C_[j - 1].n, C_[j - 1].p);
#endif /* BTREE_DEBUG_FULL */
    }
    return true;
}
