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

QuartzDiskCursor::QuartzDiskCursor(Btree * btree)
	: is_positioned(false),
	  cursor(new Bcursor(btree)) {}

bool
QuartzDiskCursor::find_entry(const string &key)
{
    DEBUGCALL(DB, bool, "QuartzDiskCursor::find_entry", key);

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
QuartzDiskCursor::next()
{
    DEBUGCALL(DB, bool, "QuartzDiskCursor::next", "");
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
QuartzDiskCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzDiskCursor::prev", "");
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


QuartzDiskTable::QuartzDiskTable(string path_,
				 bool readonly_,
				 unsigned int blocksize_)
	: path(path_),
	  blocksize(blocksize_),
	  opened(false),
          readonly(readonly_),
	  btree_for_reading(0),
	  btree_for_writing(0)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::QuartzDiskTable", "");
}

void
QuartzDiskTable::close()
{
    DEBUGCALL(DB, void, "QuartzDiskTable::close", "");
    if (btree_for_reading != 0) {
	delete btree_for_reading;
	btree_for_reading = 0;
    }
    if (btree_for_writing != 0) {
	delete btree_for_writing;
	btree_for_writing = 0;
    }
    opened = false;
}

bool
QuartzDiskTable::exists() {
    DEBUGCALL(DB, bool, "QuartzDiskTable::exists", "");
    // FIXME: use btree library to check if table exists yet.
    return (file_exists(path + "DB") &&
	    (file_exists(path + "baseA") || file_exists(path + "baseB")));
}

void
QuartzDiskTable::create()
{
    DEBUGCALL(DB, void, "QuartzDiskTable::create", "");
    Assert(!readonly);

    close();

    Btree::create(path, blocksize);
}

void
QuartzDiskTable::erase()
{
    DEBUGCALL(DB, void, "QuartzDiskTable::erase", "");
    // FIXME: implement
    Assert(!readonly);

    close();

    Btree::erase(path);
}

void
QuartzDiskTable::open()
{
    DEBUGCALL(DB, void, "QuartzDiskTable::open", "");
    DEBUGLINE(DB, "opening at path " << path);
    close();

    if (readonly) {
	btree_for_reading = new Btree();
	btree_for_reading->open_to_read(path);
	opened = true;
	return;
    }

    btree_for_writing = new Btree();
    btree_for_writing->open_to_write(path);

    btree_for_reading = new Btree();
    btree_for_reading->open_to_read(*btree_for_writing);

    opened = true;
}

bool
QuartzDiskTable::open(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, bool, "QuartzDiskTable::open", revision);
    DEBUGLINE(DB, "opening for particular revision at path " << path);
    close();

    if (readonly) {
	btree_for_reading = new Btree();
	btree_for_reading->open_to_read(path, revision);
	opened = true;
	RETURN(true);
    }

    btree_for_writing = new Btree();
    if (!btree_for_writing->open_to_write(path, revision)) {
	// Can't open at the requested revision.
	close();
	RETURN(false);
    }

    AssertEq(btree_for_writing->revision_number, revision);

    btree_for_reading = new Btree();
    btree_for_reading->open_to_read(*btree_for_writing);

    AssertEq(btree_for_reading->revision_number, revision);

    opened = true;
    RETURN(true);
}

QuartzDiskTable::~QuartzDiskTable()
{
    DEBUGCALL(DB, void, "QuartzDiskTable::~QuartzDiskTable", "");
    close();
}

quartz_revision_number_t
QuartzDiskTable::get_open_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t,
	      "QuartzDiskTable::get_open_revision_number", "");
    Assert(opened);
    RETURN(btree_for_reading->revision_number);
}

quartz_revision_number_t
QuartzDiskTable::get_latest_revision_number() const
{
    DEBUGCALL(DB, quartz_revision_number_t,
	      "QuartzDiskTable::get_latest_revision_number", "");
    RETURN(btree_for_reading->get_latest_revision_number());
}

quartz_tablesize_t
QuartzDiskTable::get_entry_count() const
{
    DEBUGCALL(DB, quartz_tablesize_t, "QuartzDiskTable::get_entry_count", "");
    if (!opened) RETURN(0);
    RETURN(btree_for_reading->item_count);
}

bool
QuartzDiskTable::get_exact_entry(const string &key, string & tag) const
{
    DEBUGCALL(DB, bool, "QuartzDiskTable::get_exact_entry", key << ", " << tag);
    Assert(opened);
    Assert(!(key.empty()));

    if (key.size() > Btree::max_key_len) RETURN(false);

    // FIXME: avoid having to create a cursor here.
    Bcursor cursor(btree_for_reading);
    // FIXME: check for errors

    if (!cursor.find_key(key)) RETURN(false);
    
    cursor.get_tag(&tag);
    // FIXME: check for errors

    RETURN(true);
}

QuartzDiskCursor *
QuartzDiskTable::cursor_get() const
{
    DEBUGCALL(DB, QuartzDiskCursor *, "QuartzDiskTable::cursor_get", "");
    Assert(opened);
    RETURN(new QuartzDiskCursor(btree_for_reading));
}

void
QuartzDiskTable::set_entry(const string & key, const string & tag)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::set_entry", key << ", " << tag);

    Assert(!key.empty());
    Assert(opened);
    if (readonly)
	throw Xapian::InvalidOperationError("Attempt to modify a readonly table");

    if (key.size() > Btree::max_key_len) {
	throw Xapian::InvalidArgumentError(
		"Key too long: length was " +
		om_tostring(key.size()) +
		" bytes, maximum length of a key is " + 
		om_tostring(Btree::max_key_len) +
		" bytes");
    }

    // add entry
    bool result = btree_for_writing->add(key, tag);
    DEBUGLINE(DB, "Result of add: " << result);
    (void)result; // FIXME: Check result
}

void
QuartzDiskTable::set_entry(const string & key)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::set_entry", key);

    Assert(!key.empty());
    Assert(opened);
    if (readonly)
	throw Xapian::InvalidOperationError("Attempt to modify a readonly table");

    if (key.size() > Btree::max_key_len) {
	throw Xapian::InvalidArgumentError(
		"Key too long: length was " +
		om_tostring(key.size()) +
		" bytes, maximum length of a key is " + 
		om_tostring(Btree::max_key_len) +
		" bytes");
    }

    // delete entry
    bool result = btree_for_writing->del(key);
    DEBUGLINE(DB, "Result of delete: " << result);
    (void)result; // FIXME: Check result
}

void
QuartzDiskTable::apply(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::apply", new_revision);

    Assert(opened);
    if (readonly) throw Xapian::InvalidOperationError("Attempt to modify a readonly table");

    opened = false;

    // Commit changes to the writing table.
    Assert(btree_for_writing != 0);
    btree_for_writing->commit(new_revision);

    // Update the reading table to the new revision.
    Assert(btree_for_reading != 0);
    btree_for_reading->reopen_to_read(*btree_for_writing);

    opened = true;
}

QuartzBufferedTable::QuartzBufferedTable(QuartzDiskTable * disktable_)
	: disktable(disktable_),
	  entry_count(disktable->get_entry_count())
{
    DEBUGCALL(DB, void, "QuartzBufferedTable", disktable_);
}

QuartzBufferedTable::~QuartzBufferedTable()
{
    DEBUGCALL(DB, void, "~QuartzBufferedTable", "");
}

void
QuartzBufferedTable::apply(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::apply", new_revision);
    try {
	// One QuartzEntry to rule them all, One QuartzEntry to find them,
	// One QuartzEntry to bring them all, and in the backend bind them...
	QuartzTableEntries::items & entries = changed_entries.get_all_entries();
	map<string, string *>::iterator entry;
	entry = entries.begin();
	Assert(entry != entries.end());
	// Don't set the null entry.
	for (++entry; entry != entries.end(); ++entry) {
	    DEBUGLINE(DB, "QuartzBufferedTable::apply(): setting key " << hex_encode(entry->first) << " to " << ((entry->second)? (hex_encode(*(entry->second))) : string("<NULL>")));
	    if (entry->second) {
		disktable->set_entry(entry->first, *(entry->second));
		delete entry->second;
		entry->second = 0;
	    } else {
		disktable->set_entry(entry->first);
	    }
	}
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();

    disktable->apply(new_revision);

    AssertEq(entry_count, disktable->get_entry_count());
}

void
QuartzBufferedTable::cancel()
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::cancel", "");
    // FIXME: when write is implemented, ensure that this undoes any
    // changes which have been written (but not yet applied).
    changed_entries.clear();
    entry_count = disktable->get_entry_count();
}

bool
QuartzBufferedTable::is_modified()
{
    DEBUGCALL(DB, bool, "QuartzBufferedTable::is_modified", "");
    RETURN(!changed_entries.empty());
}

bool
QuartzBufferedTable::have_tag(const string &key)
{
    DEBUGCALL(DB, bool, "QuartzBufferedTable::have_tag", key);
    if (changed_entries.have_entry(key)) {
	RETURN((changed_entries.get_tag(key) != 0));
    }
    // FIXME: don't want to read tag here - just want to check if there
    // is a tag.
    string tag;
    RETURN(disktable->get_exact_entry(key, tag));
}

string *
QuartzBufferedTable::get_or_make_tag(const string &key)
{
    DEBUGCALL(DB, string *, "QuartzBufferedTable::get_or_make_tag", key);
    string * tagptr;

    // Check cache first
    if (changed_entries.have_entry(key)) {
	tagptr = changed_entries.get_tag(key);
	if (tagptr == 0) {
	    // make new empty tag
	    AutoPtr<string> tag(new string);
	    tagptr = tag.get();
	    changed_entries.set_tag(key, tag);
	    ++entry_count;

	    AssertEq(changed_entries.get_tag(key), tagptr);
	    Assert(tag.get() == 0);
	}
	RETURN(tagptr);
    }

    AutoPtr<string> tag(new string);
    tagptr = tag.get();

    bool found = disktable->get_exact_entry(key, *tag);

    changed_entries.set_tag(key, tag);
    if (!found) {
	DEBUGLINE(DB, "QuartzBufferedTable::get_or_make_tag - increasing entry_count - '" << key << "' added");
	++entry_count;
    }
    Assert(changed_entries.have_entry(key));
    AssertEq(changed_entries.get_tag(key), tagptr);
    Assert(tag.get() == 0);

    RETURN(tagptr);
}

void
QuartzBufferedTable::delete_tag(const string &key)
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::delete_tag", key);
    // This reads the tag to check if it currently exists, so we can keep
    // track of the number of entries in the table.
    if (have_tag(key)) {
	DEBUGLINE(DB, "decrementing entry_count - '" << key << "' deleted");
	--entry_count;
	changed_entries.set_tag(key, AutoPtr<string>(0));
    }
}

quartz_tablesize_t
QuartzBufferedTable::get_entry_count() const
{
    DEBUGCALL(DB, quartz_tablesize_t, "QuartzBufferedTable::get_entry_count", "");
    RETURN(entry_count);
}

bool
QuartzBufferedTable::get_exact_entry(const string &key, string & tag) const
{
    DEBUGCALL(DB, bool, "QuartzBufferedTable::get_exact_entry",
	      key << ", " << tag);
    if (changed_entries.have_entry(key)) {
	const string * tagptr = changed_entries.get_tag(key);
	if (tagptr == 0) RETURN(false);
	tag = *tagptr;
	RETURN(true);
    }

    RETURN(disktable->get_exact_entry(key, tag));
}

QuartzBufferedCursor *
QuartzBufferedTable::cursor_get() const
{
    DEBUGCALL(DB, QuartzBufferedCursor *, "QuartzBufferedTable::cursor_get", "");
    return new QuartzBufferedCursor(disktable->cursor_get(), &changed_entries);
}

bool
QuartzBufferedCursor::find_entry(const string &key)
{
    DEBUGCALL(DB, bool, "QuartzBufferedCursor::find_entry", key);
    // FIXME: think we now allow empty keys in find (to mean find first entry)
    // Assert(!key.empty());

    // Whether we have an exact match.
    bool have_exact;

    // Set diskcursor.
    have_exact = diskcursor->find_entry(key);
    Assert(have_exact || key != diskcursor->current_key);

    // Set changed_entries part of cursor.
    iter = changed_entries->get_iterator(key);

    const string * keyptr;
    const string * tagptr;
    changed_entries->get_item(iter, &keyptr, &tagptr);

    // Check for exact matches
    if (*keyptr == key) {
	if (tagptr != 0) {
	    // Have an exact match in the cache, and it's not a deletion.
	    current_key = *keyptr;
	    current_tag = *tagptr;
	    RETURN(true);
	}
    } else if (diskcursor->current_key == key) {
	// Have an exact match on disk, and it's not shadowed by the cache.
	current_key = diskcursor->current_key;
	current_tag = diskcursor->current_tag;
	RETURN(true);
    }

    // No exact match.  Move backwards until match isn't a deletion.
    while (*keyptr >= diskcursor->current_key && tagptr == 0) {
	if (keyptr->empty()) {
	    // diskcursor must also point to a null key - we're done.
	    current_key = "";
	    current_tag = "";
	    RETURN(false);
	}
	if (*keyptr == diskcursor->current_key) {
	    diskcursor->prev();
	}
	changed_entries->prev(iter);
	changed_entries->get_item(iter, &keyptr, &tagptr);
    }

    // Have found a match which isn't a deletion.
    if (*keyptr >= diskcursor->current_key) {
	// Match is in cache
	current_key = *keyptr;
	current_tag = *tagptr;
    } else {
	// Match is on disk
	current_key = diskcursor->current_key;
	current_tag = diskcursor->current_tag;
    }

    RETURN(false);
}

bool
QuartzBufferedCursor::after_end()
{
    DEBUGCALL(DB, bool, "QuartzBufferedCursor::after_end", "");
    RETURN(diskcursor->after_end() && changed_entries->after_end(iter));
}

void
QuartzBufferedCursor::next()
{
    DEBUGCALL(DB, void, "QuartzBufferedCursor::next", "");
    const string * keyptr;
    const string * tagptr;

    // Ensure that both cursors are after current position, or ended.
    while (!changed_entries->after_end(iter)) {
	changed_entries->get_item(iter, &keyptr, &tagptr);
	if (*keyptr > current_key) break;
	changed_entries->next(iter);
    }
    while (!diskcursor->after_end()) {
	if (diskcursor->current_key > current_key) break;
	diskcursor->next();
    }

    // While iter is the current item, and is pointing to a deleted item,
    // move forward.
    if (!changed_entries->after_end(iter)) {
	while (tagptr == 0 &&
	       (diskcursor->after_end() || *keyptr <= diskcursor->current_key)) {

	    if (!diskcursor->after_end() &&
		*keyptr == diskcursor->current_key) {
		diskcursor->next();
	    }

	    changed_entries->next(iter);
	    if (changed_entries->after_end(iter)) break;

	    changed_entries->get_item(iter, &keyptr, &tagptr);
	}
    }

    // Now we just have to pick the lower cursor, and store its key and tag.

    if (changed_entries->after_end(iter)) {
	if (diskcursor->after_end()) {
	    // Have reached end.
	} else {
	    // Only have diskcursor left.
	    current_key = diskcursor->current_key;
	    current_tag = diskcursor->current_tag;
	}
    } else {
	if (diskcursor->after_end() || *keyptr < diskcursor->current_key) {
	    Assert(tagptr != 0);
	    current_key = *keyptr;
	    current_tag = *tagptr;
	} else {
	    current_key = diskcursor->current_key;
	    current_tag = diskcursor->current_tag;
	}
    }
}

void
QuartzBufferedCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzBufferedCursor::prev", "");
    throw Xapian::UnimplementedError("QuartzBufferedCursor::prev() not yet implemented");
}
