/* btree_types.h: Btree implementation common types
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_BTREE_TYPES_H
#define OM_HGUARD_BTREE_TYPES_H

#include <string>

typedef unsigned char byte;
typedef long int4;
typedef unsigned long uint4;

enum Btree_errors {
    BTREE_ERROR_NONE = 0,

    BTREE_ERROR_BLOCKSIZE = 3,
    BTREE_ERROR_SPACE,

    BTREE_ERROR_BASE_CREATE,
    BTREE_ERROR_BASE_DELETE,
    BTREE_ERROR_BASE_READ,
    BTREE_ERROR_BASE_WRITE,

    BTREE_ERROR_BITMAP_CREATE,
    BTREE_ERROR_BITMAP_READ,
    BTREE_ERROR_BITMAP_WRITE,

    BTREE_ERROR_DB_CREATE,
    BTREE_ERROR_DB_OPEN,
    BTREE_ERROR_DB_CLOSE,
    BTREE_ERROR_DB_READ,
    BTREE_ERROR_DB_WRITE,

    BTREE_ERROR_KEYSIZE,
    BTREE_ERROR_TAGSIZE,

    BTREE_ERROR_REVISION
};

struct Cursor {
    /** Constructor, to set important elements to 0.
     */
    Cursor() : p(0), c(-1), n(-1), rewrite(false), split_p(0), split_n(-1) {}

    byte * p;         /* pointer to a block */
    int c;            /* offset in the block's directory */
    int4 n;           /* block number */
    int rewrite;      /* true if the block is not the same as on disk, and so needs rewriting */
    byte * split_p;   /* pointer to a block split off from main block */
    int4 split_n;     /* - and its block number */

};

/* n is kept in tandem with p. The unassigned state is when member p == 0 and n == -1.
   Similarly split.p == 0 corresponds to split.n == -1. Settings to -1 are not strictly
   neccessary in the code below, so the lines

        C[j].n = -1;
        C[j].split_n = -1;

   might sometimes be omitted, but they help keep the intention clear.
*/

struct Btree_item {

    /* Constructor which zeroes all the fields */
    Btree_item()
	    : key_size(0), key_len(0), key(0),
    	      tag_size(0), tag_len(0), tag(0)
    {
    }

    int key_size;       /* capacity of item->key */
    int key_len;        /* length of retrieved key */
    byte * key;         /* pointer to the key */

    int tag_size;       /* capacity of item->tag */
    int tag_len;        /* length of retrieved tag */
    byte * tag;         /* pointer to the tag */

};

extern std::string Btree_strerror(Btree_errors err);

#endif /* OM_HGUARD_BTREE_TYPES_H */

