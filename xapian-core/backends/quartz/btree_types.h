/* btree_types.h: Btree implementation common types
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

#ifndef OM_HGUARD_BTREE_TYPES_H
#define OM_HGUARD_BTREE_TYPES_H

#include <string>
using std::string;

typedef unsigned char byte;
typedef long int4;
typedef unsigned long uint4;

#define BLK_UNUSED uint4(-1)

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

class Cursor {
    private:
        // Prevent copying
        Cursor(const Cursor &);
        Cursor & operator=(const Cursor &);

    public:
	/// Constructor, to initiaise important elements.
	Cursor() : p(0), c(-1), n(BLK_UNUSED), rewrite(false), split_p(0), split_n(BLK_UNUSED)
	{}

	/// pointer to a block
	byte * p;
	/// offset in the block's directory
	int c;
	/// block number
	uint4 n;
	/// true if the block is not the same as on disk, and so needs rewriting
	int rewrite;
	/// pointer to a block split off from main block
	byte * split_p;
	/// block number of a block split off from main block
	uint4 split_n;
};

/* n is kept in tandem with p.
   
   The unassigned state is when member p == 0 and n == BLK_UNUSED.

   Similarly split.p == 0 corresponds to split.n == BLK_UNUSED.

   Setting n/split_n to BLK_UNUSED is necessary in at least some cases.
*/

extern string Btree_strerror(Btree_errors err);

#endif /* OM_HGUARD_BTREE_TYPES_H */
