/* bcursor.h: Interface to Btree cursors
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

#ifndef OM_HGUARD_BCURSOR_H
#define OM_HGUARD_BCURSOR_H

#include "btree_types.h"

class Btree;

/************ B-tree reading ************/

class Bcursor {
    // FIXME: in future probably a Btree member function.
    friend Bcursor *Bcursor_create(Btree *);
    public:
	/** Destroy a Bcursor */
	~Bcursor();

	int prev();
	int next();
	int find_key(byte *key, int key_len);
	int get_key(Btree_item *item);
	int get_tag(Btree_item *item);

    private:
	/** Create a bcursor attached to a Btree. */
	Bcursor(Btree *B_);

	int positioned;    /* false initially, and after the cursor has dropped
			      of either end of the list of items */
	struct Btree * B;
	struct Cursor * C;

	int shared_level; /* The value of shared_level in the Btree structure. */
};

#endif /* OM_HGUARD_BCURSOR_H */
