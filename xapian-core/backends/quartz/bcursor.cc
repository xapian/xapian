/* bcursor.cc: Btree cursor implementation
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

#include "bcursor.h"
#include "btree.h"
#include "btree_util.h"
#include "omassert.h"

Bcursor::Bcursor(Btree *B_)
	: positioned(false),
	  B(B_),
	  shared_level(B_->shared_level)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    C = new Cursor[B->level + 1];
    Cursor * C_of_B = B->C;

    int j;
    for (j = 0; j < B->shared_level; j++) {
        C[j].n = -1;
	C[j].p = new byte[B->block_size];
    }
    for (j = B->shared_level; j <= B->level; j++) {
        C[j].n = C_of_B[j].n;
	C[j].p = C_of_B[j].p;
    }
}

Bcursor::~Bcursor()
{
    // Use the value of shared_level stored in the cursor rather than the
    // Btree, since the Btree might have been deleted already.
    for (int j = 0; j < shared_level; j++) {
	delete [] C[j].p;
    }
    delete [] C;
}

int
Bcursor::prev()
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (!positioned) return false;

    while(true) {
        if (! B->prev(B, C, 0)) {
	    positioned = false;
	    return false;
	}
	if (component_of(C[0].p, C[0].c) == 1) {
	    return true;
	}
    }
    /* FIXME: return something here? */
}

int
Bcursor::next()
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (! positioned) return false;

    while(true) {
	if (! B->next(B, C, 0)) {
	    positioned = false;
	    return false;
	}
	if (component_of(C[0].p, C[0].c) == 1) {
	    return true;
	}
    }
    /* FIXME: return something here? */
}

int
Bcursor::find_key(byte * key, int key_len)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    form_key(B, B->kt, key, key_len);
    int found = B->find(C);

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
    positioned = true;
    return found;
}

int
Bcursor::get_key(struct Btree_item * item)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (! positioned) return false;

    byte * p = key_of(C[0].p, C[0].c);
    int l = GETK(p, 0) - K1 - C2;       /* number of bytes to extract */
    if (item->key_size < l) {
	free(item->key);
	item->key = (byte *) calloc(l + 1, 1); /* 1 extra in case l == 0 */
	if (item->key == 0) {
	    B->error = BTREE_ERROR_SPACE;
	    throw std::bad_alloc();
	}
	item->key_size = l + 1;
    }
    memmove(item->key, p + K1, l);
    item->key_len = l;
    return true;
}

int
Bcursor::get_tag(struct Btree_item * item)
{
    AssertEq(B->error, 0);
    Assert(!B->overwritten);

    if (!positioned) return false;

    byte * p = item_of(C[0].p, C[0].c); /* pointer to current component */
    int ct = GETK(p, I2) + I2;          /* offset to the tag */

    int n = GETC(p, ct);                /* n components to join */

    int cd = ct + C2;                   /* offset to the tag data */

    int o = 0;                          /* cursor into item->tag */
    int i;                              /* see below */
    int l = GETI(p, 0) - cd;            /* number of bytes to extract from current
					   component */

    {
	int4 space_for_tag = (int4) B->max_item_size * n;
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
    for (i = 1; i <= n; i++)
    {
	Assert(o + l <= item->tag_size);

	memmove(item->tag + o, p + cd, l); o += l;
	positioned = B->next(B, C, 0);

	if (B->overwritten) return false;

	p = item_of(C[0].p, C[0].c);
	l = GETI(p, 0) - cd;
    }
    item->tag_len = o;
    return positioned;
}

