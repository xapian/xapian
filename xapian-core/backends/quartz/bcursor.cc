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
#include "omdebug.h"

#ifdef MUS_DEBUG_VERBOSE
static string
hex_encode(const string & input)
{
    const char * table = "0123456789abcdef";
    string result;
    for (string::const_iterator i = input.begin(); i != input.end(); ++i) {
	unsigned char val = *i;
	result += "\\x";
	result += table[val/16];
	result += table[val%16];
    }

    return result;
}
#endif

Bcursor::Bcursor(Btree *B_)
	: is_positioned(false),
	  is_after_end(false),
	  have_read_tag(false),
	  B(B_),
	  level(B_->level)
{
    C = new Cursor[level + 1];

    for (int j = 0; j < level; j++) {
        C[j].n = BLK_UNUSED;
	C[j].p = new byte[B->block_size];
    }
    C[level].n = B->C[level].n;
    C[level].p = B->C[level].p;
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
    DEBUGCALL(DB, bool, "Bcursor::prev", "");
    Assert(B->level <= level);
    Assert(!is_after_end);

    if (!is_positioned) {
	// We've read the last key and tag, and we're now not positioned.
	// Simple fix - seek to the current key, and then it's as if we
	// read the key but not the tag.
	(void)find_entry(current_key);
	have_read_tag = false;
    }

    if (have_read_tag) {
	while (true) {
	    if (! B->prev(C, 0)) {
		is_positioned = false;
		return false;
	    }
	    if (Item(C[0].p, C[0].c).component_of() == 1) {
		break;
	    }
	}
    }

    while (true) {
	if (! B->prev(C, 0)) {
	    is_positioned = false;
	    return false;
	}
	if (Item(C[0].p, C[0].c).component_of() == 1) {
	    break;
	}
    }
    get_key(&current_key);
    // FIXME: check for errors
    have_read_tag = false;

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) << "'");
    return true;
}

bool
Bcursor::next()
{
    DEBUGCALL(DB, bool, "Bcursor::next", "");
    Assert(B->level <= level);
    Assert(!is_after_end);
    if (!have_read_tag) {
	while (true) {
	    if (! B->next(C, 0)) {
		is_positioned = false;
		break;
	    }
	    if (Item(C[0].p, C[0].c).component_of() == 1) {
		is_positioned = true;
		break;
	    }
	}
    }

    if (!is_positioned) {
	is_after_end = true;
	return false;
    }

    get_key(&current_key);
    // FIXME: check for errors
    have_read_tag = false;

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) << "'");
    return true;
}

bool
Bcursor::find_entry(const string &key)
{
    DEBUGCALL(DB, bool, "Bcursor::find_entry", key);
    Assert(B->level <= level);

    is_after_end = false;

    bool found;

    if (key.size() > Btree::max_key_len) {
	is_positioned = true;
	// Can't find key - too long to possibly be present, so find the
	// truncated form but ignore "found".
	B->form_key(key.substr(0, Btree::max_key_len));
	(void)(B->find(C));
	found = false;
    } else {
	is_positioned = true;
	B->form_key(key);
	found = B->find(C);
    }

    if (!found) {
	if (C[0].c < DIR_START) {
	    C[0].c = DIR_START;
	    if (! B->prev(C, 0)) goto done;
	}
	while (Item(C[0].p, C[0].c).component_of() != 1) {
	    if (! B->prev(C, 0)) {
		is_positioned = false;
		break;
	    }
	}
    }
done:

    bool err = get_key(&current_key);
    (void)err; // FIXME: check for errors
    have_read_tag = false;

    DEBUGLINE(DB, "Found entry: key=`" << hex_encode(current_key) << "'");

    RETURN(found);
}

bool
Bcursor::get_key(string * key) const
{
    Assert(B->level <= level);

    if (!is_positioned) return false;

    (void)Item(C[0].p, C[0].c).key().read(key);
    return true;
}

bool
Bcursor::get_tag(string * tag)
{
    Assert(B->level <= level);

    if (!is_positioned) return false;

    Item item(C[0].p, C[0].c);

    /* n components to join */
    int n = item.components_of();

    tag->resize(0);
    if (n > 1) tag->reserve(B->max_item_size * n);

    item.append_chunk(tag);
    is_positioned = B->next(C, 0);

    // FIXME: code to do very similar thing in btree.cc...
    for (int i = 2; i <= n; i++) {
	(void)Item(C[0].p, C[0].c).append_chunk(tag);
	// We need to call B->next(...) on the last pass so that the
	// cursor ends up on the next key.
	is_positioned = B->next(C, 0);
    }
    return is_positioned;
}


void
Bcursor::read_tag()
{
    DEBUGCALL(DB, void, "Bcursor::read_tag", "");

    if (have_read_tag) return;

    is_positioned = get_tag(&current_tag);
    // FIXME: check for errors

    have_read_tag = true;

    DEBUGLINE(DB, "tag=`" << hex_encode(current_tag) << "'");
}

void
Bcursor::del()
{
    Assert(!is_after_end);

    // FIXME: this isn't the most efficient approach, but I struggled to
    // make the obvious approaches work.
    B->del(current_key);
    find_entry(current_key);
    next();

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) << "'");
}
