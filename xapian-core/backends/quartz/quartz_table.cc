/* quartz_table.cc: A cursor in a Btree table.
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
#include "omdebug.h"

#include "quartz_table.h"
#include "btree.h"
#include <xapian/error.h>
#include "utils.h"
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

bool
QuartzCursor::find_entry(const string &key)
{
    DEBUGCALL(DB, bool, "QuartzCursor::find_entry", key);

    is_after_end = false;

    int found = false;

    if (key.size() > Btree::max_key_len) {
	// Can't find key - too long to possibly be present.
	(void) cursor.find_key(key.substr(0, Btree::max_key_len));
	// FIXME: check for errors
    } else {
	found = cursor.find_key(key);
	// FIXME: check for errors
    }

    bool err = cursor.get_key(&current_key);
    (void)err; // FIXME: check for errors
    have_read_tag = false;
    is_positioned = true;

    read_tag(); // FIXME : shouldn't need to call this here

    DEBUGLINE(DB, "Found entry: key=`" << hex_encode(current_key) << "'");

    RETURN(found);
}

void
QuartzCursor::read_tag()
{
    DEBUGCALL(DB, void, "QuartzCursor::read_tag", "");

    if (have_read_tag) return;

    is_positioned = cursor.get_tag(&current_tag);
    // FIXME: check for errors

    have_read_tag = true;

    DEBUGLINE(DB, "tag=`" << hex_encode(current_tag) << "'");
}

void
QuartzCursor::next()
{
    DEBUGCALL(DB, bool, "QuartzCursor::next", "");
    Assert(!is_after_end);
    if (!have_read_tag) {
	if (!cursor.next()) is_positioned = false;
    }

    if (!is_positioned) {
	is_after_end = true;
	return;
    }

    cursor.get_key(&current_key);
    // FIXME: check for errors
    have_read_tag = false;

    read_tag(); // FIXME : shouldn't need to call this here

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) << "'");
}

#if 0 // Unused and untested in its current form...
void
QuartzCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzCursor::prev", "");
    Assert(!is_after_end);
    Assert(!current_key.empty());

    if (!is_positioned) {
	// FIXME: want to go to last item in table - this method
	// should work, but isn't going to be of great efficiency.
	int found = cursor.find_key(current_key);
	(void)found; // FIXME: check for errors
	Assert(found);
    } else if (have_read_tag) {
	cursor.prev();
	// FIXME: check for errors
    }

    cursor.prev();
    // FIXME: check for errors
    cursor.get_key(&current_key);
    // FIXME: check for errors
    have_read_tag = false;

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) << "'");
}
#endif
