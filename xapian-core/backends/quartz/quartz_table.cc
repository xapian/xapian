/* quartz_table.cc: A table in a quartz database
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

QuartzCursor::QuartzCursor(Btree * btree)
	: is_positioned(false),
	  cursor(new Bcursor(btree)) {}

bool
QuartzCursor::find_entry(const string &key)
{
    DEBUGCALL(DB, bool, "QuartzCursor::find_entry", key);

    is_after_end = false;

    int found = false;

    if (key.size() > Btree::max_key_len) {
	// Can't find key - too long to possibly be present.
	(void) cursor->find_key(key.substr(0, Btree::max_key_len));
	// FIXME: check for errors
    } else {
	found = cursor->find_key(key);
	// FIXME: check for errors
    }

    bool err = cursor->get_key(&current_key);
    (void)err; // FIXME: check for errors

    is_positioned = cursor->get_tag(&current_tag);
    // FIXME: check for errors

    DEBUGLINE(DB, "Found entry: key=`" << hex_encode(current_key) <<
	      "', tag=`" << hex_encode(current_tag) << "'");

    RETURN(found);
}

void
QuartzCursor::next()
{
    DEBUGCALL(DB, bool, "QuartzCursor::next", "");
    Assert(!is_after_end);
    if (!is_positioned) {
	is_after_end = true;
	return;
    }

    cursor->get_key(&current_key);
    // FIXME: check for errors
    is_positioned = cursor->get_tag(&current_tag);
    // FIXME: check for errors

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) <<
	      "', tag=`" << hex_encode(current_tag) << "'");
}

void
QuartzCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzCursor::prev", "");
    Assert(!is_after_end);
    Assert(!current_key.empty());

    if (!is_positioned) {
	// FIXME: want to go to last item in table - this method
	// should work, but isn't going to be of great efficiency.
	int found = cursor->find_key(current_key);
	(void)found; // FIXME: check for errors
	Assert(found);
    } else {
	cursor->prev();
	// FIXME: check for errors
    }

    cursor->get_key(&current_key);
    // FIXME: check for errors

    if (!current_key.empty()) {
	cursor->prev();
	// FIXME: check for errors
	cursor->get_key(&current_key);
	// FIXME: check for errors
    }

    is_positioned = cursor->get_tag(&current_tag);
    // FIXME: check for errors

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key) <<
	      "', tag=`" << hex_encode(current_tag) << "'");
}


QuartzTable::QuartzTable(string path_, bool readonly_, unsigned int blocksize_)
	: path(path_),
	  blocksize(blocksize_),
          readonly(readonly_),
	  is_modified_flag(false),
	  btree(0)
{
    DEBUGCALL(DB, void, "QuartzTable::QuartzTable",
	      path_ << ", " << readonly_ << ","  << blocksize_);
}

void
QuartzTable::close()
{
    DEBUGCALL(DB, void, "QuartzTable::close", "");
    if (btree) {
	delete btree;
	btree = 0;
    }
}

bool
QuartzTable::exists() {
    DEBUGCALL(DB, bool, "QuartzTable::exists", "");
    // FIXME: use btree library to check if table exists yet.
    return (file_exists(path + "DB") &&
	    (file_exists(path + "baseA") || file_exists(path + "baseB")));
}

void
QuartzTable::create()
{
    DEBUGCALL(DB, void, "QuartzTable::create", "");
    Assert(!readonly);

    close();

    Btree::create(path, blocksize);
}

void
QuartzTable::erase()
{
    DEBUGCALL(DB, void, "QuartzTable::erase", "");
    Assert(!readonly);

    close();

    Btree::erase(path);
}

void
QuartzTable::open()
{
    DEBUGCALL(DB, void, "QuartzTable::open", "");
    DEBUGLINE(DB, "opening at path " << path);
    close();

    try {
	btree = new Btree();

	if (readonly) {
	    btree->open_to_read(path);
	    return;
	}

	btree->open_to_write(path);
    } catch (...) {
	delete btree;
	btree = 0;
	throw;
    }
}

bool
QuartzTable::open(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, bool, "QuartzTable::open", revision);
    DEBUGLINE(DB, "opening for particular revision at path " << path);
    close();

    try {
	btree = new Btree();

	if (readonly) {
	    btree->open_to_read(path, revision);
	    RETURN(true);
	}

	if (!btree->open_to_write(path, revision)) {
	    // Can't open at the requested revision.
	    close();
	    RETURN(false);
	}

	AssertEq(btree->revision_number, revision);
    } catch (...) {
	delete btree;
	btree = 0;
	throw;
    }
    RETURN(true);
}

QuartzTable::~QuartzTable()
{
    DEBUGCALL(DB, void, "QuartzTable::~QuartzTable", "");
    close();
}

quartz_revision_number_t
QuartzTable::get_open_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t,
	      "QuartzTable::get_open_revision_number", "");
    Assert(btree);
    RETURN(btree->revision_number);
}

quartz_revision_number_t
QuartzTable::get_latest_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t,
	      "QuartzTable::get_latest_revision_number", "");
    Assert(btree);
    RETURN(btree->get_latest_revision_number());
}

quartz_tablesize_t
QuartzTable::get_entry_count() const
{
    DEBUGCALL(DB, quartz_tablesize_t, "QuartzTable::get_entry_count", "");
    if (!btree) RETURN(0);
    RETURN(btree->item_count);
}

bool
QuartzTable::get_exact_entry(const string &key, string & tag) const
{
    DEBUGCALL(DB, bool, "QuartzTable::get_exact_entry", key << ", " << tag);
    Assert(btree);
    Assert(!(key.empty()));

    if (key.size() > Btree::max_key_len) RETURN(false);

    if (!readonly) RETURN(btree->find_tag(key, &tag));

    // FIXME: avoid having to create a cursor here.
    Bcursor cursor(btree);
    // FIXME: check for errors

    if (!cursor.find_key(key)) RETURN(false);
    
    cursor.get_tag(&tag);
    // FIXME: check for errors

    RETURN(true);
}

QuartzCursor *
QuartzTable::cursor_get() const
{
    DEBUGCALL(DB, QuartzCursor *, "QuartzTable::cursor_get", "");
    Assert(btree);
    RETURN(new QuartzCursor(btree));
}

void
QuartzTable::set_entry(const string & key, const string & tag)
{
    DEBUGCALL(DB, void, "QuartzTable::set_entry", key << ", " << tag);

    Assert(!key.empty());
    Assert(btree);
    Assert(!readonly);

    if (key.size() > Btree::max_key_len) {
	throw Xapian::InvalidArgumentError(
		"Key too long: length was " +
		om_tostring(key.size()) +
		" bytes, maximum length of a key is " + 
		om_tostring(Btree::max_key_len) +
		" bytes");
    }

    // add entry
    is_modified_flag = true;
    bool result = btree->add(key, tag);
    DEBUGLINE(DB, "Result of add: " << result);
    (void)result; // FIXME: Check result
}

void
QuartzTable::set_entry(const string & key)
{
    DEBUGCALL(DB, void, "QuartzTable::set_entry", key);

    Assert(!key.empty());
    Assert(btree);
    Assert(!readonly);

    if (key.size() > Btree::max_key_len) {
	throw Xapian::InvalidArgumentError(
		"Key too long: length was " +
		om_tostring(key.size()) +
		" bytes, maximum length of a key is " + 
		om_tostring(Btree::max_key_len) +
		" bytes");
    }

    // delete entry
    bool result = btree->del(key);
    DEBUGLINE(DB, "Result of delete: " << result);
    (void)result; // FIXME: Check result
}

void
QuartzTable::apply(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzTable::apply", new_revision);

    Assert(btree);
    Assert(!readonly);

    // FIXME: this doesn't work (probably because the table revisions get
    // out of step) but it's wasteful to keep applying changes to value
    // and position if they're never used...
    //
    // if (!is_modified_flag) return;

    // Commit changes to the writing table.
    btree->commit(new_revision);
    is_modified_flag = false;
}

void
QuartzTable::cancel()
{
    DEBUGCALL(DB, void, "QuartzTable::cancel", "");
    Assert(!readonly);

    if (is_modified_flag) {
	// FIXME : this could be done without closing and reopening the Btrees
	// filedescriptors, etc.
	open();
	is_modified_flag = false;
    }
}
