/* bcursor.cc: Btree cursor implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
#include <errno.h>

#include "bcursor.h"
#include "btree.h"
#include "btree_util.h"
#include "omassert.h"

Bcursor::Bcursor(Btree *B_)
	: positioned(false),
	  B(B_),
	  level(B_->level)
{
    C = new Cursor[level + 1];
    Cursor * C_of_B = B->C;

    for (int j = 0; j < level; j++) {
        C[j].n = BLK_UNUSED;
	C[j].p = new byte[B->block_size];
    }
    C[level].n = C_of_B[level].n;
    C[level].p = C_of_B[level].p;
}

Bcursor::~Bcursor()
{
    // Use the value of level stored in the cursor rather than the
    // Btree, since the Btree might have been deleted already.
    for (int j = 0; j < level; j++) {
	delete [] C[j].p;
    }
    delete [] C;
}

bool
Bcursor::prev()
{
    Assert(level == B->level);

    if (!positioned) return false;

    while (true) {
        if (! B->prev(C, 0)) {
	    positioned = false;
	    return false;
	}
	if (component_of(C[0].p, C[0].c) == 1) {
	    return true;
	}
    }
}

bool
Bcursor::next()
{
    Assert(level == B->level);

    if (!positioned) return false;

    while (true) {
	if (! B->next(C, 0)) {
	    positioned = false;
	    return false;
	}
	if (component_of(C[0].p, C[0].c) == 1) {
	    return true;
	}
    }
}

bool
Bcursor::find_key(const string &key)
{
    Assert(level == B->level);

    B->form_key(key);
    if (B->find(C)) {
	positioned = true;
	return true;
    }

    if (C[0].c < DIR_START) {
	C[0].c = DIR_START;
	if (! B->prev(C, 0)) return false;
    }
    while (component_of(C[0].p, C[0].c) != 1) {
	if (! B->prev(C, 0)) return false;
    }

    positioned = true;
    return false;
}

bool
Bcursor::get_key(string * key) const
{
    Assert(level == B->level);

    if (! positioned) return false;

    const byte * p = key_of(C[0].p, C[0].c);
    int l = GETK(p, 0) - K1 - C2;       /* number of bytes to extract */
    key->assign(reinterpret_cast<const char *>(p + K1), l);
    return true;
}

bool
Bcursor::get_tag(string * tag)
{
    Assert(level == B->level);

    if (!positioned) return false;

    const byte * p = item_of(C[0].p, C[0].c); /* pointer to current component */
    int ct = GETK(p, I2) + I2;          /* offset to the tag */

    int n = GETC(p, ct);                /* n components to join */

    int cd = ct + C2;                   /* offset to the tag data */

    tag->resize(0);
    if (n > 1) {
	tag->reserve(B->max_item_size * n);
    }

     // FIXME: code to do very similar thing in btree.cc...
     for (int i = 1; i <= n; i++) {
	/* number of bytes to extract from current component */
	int l = GETI(p, 0) - cd;
	tag->append(reinterpret_cast<const char *>(p + cd), l);
	// We need to call B->next(...) on the last pass so that the
	// cursor ends up on the next key.
	positioned = B->next(C, 0);

	p = item_of(C[0].p, C[0].c);
    }
    return positioned;
}
