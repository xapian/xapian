/* dbdefs.h: Definitions for DB reading code
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

/* offsets in each data block of a DB file */

#define HEADER  32      /* size of the header in bytes */

#define CHECKPOINT  -8  /* unique number at start and end of block */
#define BLOCKNUMBER  -7 /* block number, relative to blockoffset */
#define BLOCK_VERSION  -6     /* DB version number at creation */
#define LEVEL  -5       /* 0 for a leaf block, to level for root block */
#define TYPE  -4        /* data or index block (used for consistency checking only) */
#define DLEN  -3        /* directory length */
#define MAXFREE  -2     /* max. free block left */
#define TOTALFREE  -1   /* total free space in block */

/* offsets in each base block of a DB file */

#define DB_VERSION     0 /* version number */
#define DB_BASE        1 /* dbinfo block number (0 or 1) */
#define DB_BASE2       2 /* the other dbinfo block number (1 or 0) */
#define DB_BITMAP      3 /* bitmap's first block number */
#define DB_BITMAP2     4 /* the other bitmap's first block number */
#define DB_BITMAPSIZE  5 /* total number of bitmap blocks */
#define DB_DISCBLOCKS  6 /* total number of blocks in the database */
#define DB_BLOCK_SIZE  7 /* block size in bytes */
#define DB_ROOT        8 /* root block of the B-tree */
#define DB_LEVELS      9 /* total number of levels in the B-tree (counting from 0) */
#define DB_BLOCK_OFFSET 10
                         /* first data block in B-tree */
#define DB_KEYPART    11 /* 0 or 1 */
                         /* 1 is for HD Muscat, when the keypart is 4 bytes */
                         /* 0 is for flimsy Muscat when the keypart is 1 byte */
#define DB_IMAGES     12 /* F, the number of possible images */
#define DB_STATE      13
#define DB_MAXBLOCK   14 /* last block used for expandable DB file, otherwise 0 */
#define DB_RELEASE    15 /* used to be DB_BHEAD (add 4 to get the value of b_head) */
                         /* is now 4 or 5 for Muscat 3.4 or Muscat 3.5 */
#define DB_COUNTER    16 /* total count on block writes */
#define DB_FLAGS      17 /* - bottom bit tested by DBundo */
#define DB_SPARE      18 /* 18-19 are spare (and zero) */
                         /* for future devoplments, Muscat 3.5 halts if the db_spare */
                         /* slot is non-zero */
#define DB_VERSION2   20 /* version number again */
#define DB_FVEC       21 /* begins a vector: */
                         /* v1 r1 .. vF rF (vi=version number; ri=root number) */
                         /* followed by: */
                         /* version number (again) */



