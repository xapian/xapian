/* quartz_table.cc: A table in a quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>
#include "omdebug.h"

#include "quartz_table.h"
#include "om/omerror.h"
#include "utils.h"
#include <string.h> // for strerror
#include <errno.h>
#include "omdebug.h"

#ifdef MUS_DEBUG_VERBOSE
static string
hex_encode(const string & input)
{
    const char * table = "0123456789abcdef";
    string result;
    for (string::const_iterator i = input.begin(); i != input.end(); ++i) {
	unsigned char val = *i;
	Assert(val < 256);
	result += "\\x";
	result += table[val/16];
	result += table[val%16];
    }

    return result;
}
#endif

bool
QuartzDiskCursor::find_entry(const QuartzDbKey &key)
{
    DEBUGCALL(DB, bool, "QuartzDiskCursor::find_entry",
	      "QuartzDbKey(" << key.value << ")");

    is_after_end = false;

    string::size_type key_len = key.value.size();

    int found = false;

    if (key_len > max_key_len) {
	// Can't find key - too long to possibly be present.
	(void) cursor->find_key(reinterpret_cast<const byte *>(
							key.value.data()),
				max_key_len);
	// FIXME: check for errors
    } else {
	found = cursor->find_key(reinterpret_cast<const byte *>(
							key.value.data()),
				 key_len);
	// FIXME: check for errors
    }

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    int err = cursor->get_key(item);
    (void)err; // FIXME: check for errors

    is_positioned = cursor->get_tag(item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    DEBUGLINE(DB, "Found entry: key=`" << hex_encode(current_key.value) <<
	      "', tag=`" << hex_encode(current_tag.value) << "'");

    Btree_item_lose(item);

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

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    cursor->get_key(item);
    // FIXME: check for errors
    is_positioned = cursor->get_tag(item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    Btree_item_lose(item);

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key.value) <<
	      "', tag=`" << hex_encode(current_tag.value) << "'");
}

void
QuartzDiskCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzDiskCursor::prev", "");
    Assert(!is_after_end);
    Assert(current_key.value.size() != 0);

    if (!is_positioned) {
	// FIXME: want to go to last item in table - this method
	// should work, but isn't going to be of great efficiency.
	int found = cursor->find_key(reinterpret_cast<const byte *>(
				     current_key.value.data()),
				     current_key.value.size());
	(void)found; // FIXME: check for errors
	Assert(found);
    } else {
	cursor->prev();
	// FIXME: check for errors
    }

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    cursor->get_key(item);
    // FIXME: check for errors

    if (item->key_len != 0) {
	cursor->prev();
	// FIXME: check for errors
	cursor->get_key(item);
	// FIXME: check for errors
    }

    is_positioned = cursor->get_tag(item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    Btree_item_lose(item);

    DEBUGLINE(DB, "Moved to entry: key=`" << hex_encode(current_key.value) <<
	      "', tag=`" << hex_encode(current_tag.value) << "'");
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
	Btree_quit(btree_for_reading);
	btree_for_reading = 0;
    }
    if (btree_for_writing != 0) {
	Btree_quit(btree_for_writing);
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

    Btree_create(path.c_str(), blocksize);
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
    close();

    if (readonly) {
	btree_for_reading = Btree_open_to_read(path.c_str());
	if (btree_for_reading == 0 || btree_for_reading->error) {
	    // FIXME: explain why
	    string errormsg = "Cannot open table `"+path+"' for reading: ";
	    if (btree_for_reading)
		errormsg += om_tostring(btree_for_reading->error) + ", ";
	    errormsg += strerror(errno);
	    close();
	    throw OmOpeningError(errormsg);
	}
	opened = true;
	return;
    }

    btree_for_writing = Btree_open_to_write(path.c_str());
    // FIXME: check for errors

    if (btree_for_writing == 0 || btree_for_writing->error) {
	// FIXME: explain why
	string errormsg = "Cannot open table `"+path+"' for writing: ";
	if (btree_for_writing)
	    errormsg += om_tostring(btree_for_writing->error) + ", ";
	errormsg += strerror(errno);
	close();
	throw OmOpeningError(errormsg);
    }

    btree_for_reading = Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number);
    // FIXME: check for errors
    if (btree_for_reading == 0 || btree_for_reading->error) {
	// FIXME: explain why
	string errormsg = "Cannot open table `" + path +
		"' for reading and writing: ";
	if (btree_for_reading)
	    errormsg += om_tostring(btree_for_reading->error) + ", ";
	errormsg += strerror(errno);
	close();
	throw OmOpeningError(errormsg);
    }

    opened = true;
}

bool
QuartzDiskTable::open(quartz_revision_number_t revision)
{
    DEBUGCALL(DB, bool, "QuartzDiskTable::open", revision);
    close();

    if (readonly) {
	btree_for_reading = Btree_open_to_read_revision(path.c_str(), revision);
	// FIXME: check for errors
	if (btree_for_reading == 0 || btree_for_reading->error) {
	    // FIXME: throw an exception if it's not just this revision which
	    // is unopenable.
	    close();
	    RETURN(false);
	}
	opened = true;
	RETURN(true);
    }

    btree_for_writing = Btree_open_to_write_revision(path.c_str(), revision);
    // FIXME: check for errors
    if (btree_for_writing == 0 || btree_for_writing->error) {
	// FIXME: throw an exception if it's not just this revision which
	// is unopenable.
	close();
	RETURN(false);
    }

    AssertEq(btree_for_writing->revision_number, revision);

    btree_for_reading = Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number);
    // FIXME: check for errors
    if (btree_for_reading == 0 || btree_for_reading->error) {
	// FIXME: throw an exception if it's not just this revision which
	// is unopenable.
	close();
	RETURN(false);
    }

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
    // FIXME: implement with a call to martin's code
    if (btree_for_reading->both_bases &&
	btree_for_reading->other_revision_number > btree_for_reading->revision_number) {
	RETURN(btree_for_reading->other_revision_number);
    }
    RETURN(btree_for_reading->revision_number);
}

quartz_tablesize_t
QuartzDiskTable::get_entry_count() const
{
    DEBUGCALL(DB, quartz_revision_number_t,
	      "QuartzDiskTable::get_entry_count", "");
    if (opened) 
	RETURN(btree_for_reading->item_count);
    else
	RETURN(0);
}

bool
QuartzDiskTable::get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    DEBUGCALL(DB, bool, "QuartzDiskTable::get_exact_entry",
	      "QuartzDbKey(" << key.value << "), "
	      "QuartzDbTag(" << tag.value << ")");
    Assert(opened);
    Assert(!(key.value.empty()));

    if (int(key.value.size()) > btree_for_reading->max_key_len) RETURN(false);

    // FIXME: avoid having to create a cursor here.
    AutoPtr<Bcursor> cursor = btree_for_reading->Bcursor_create();
    // FIXME: check for errors

    int found = cursor->find_key(reinterpret_cast<const byte *>(key.value.data()),
				 key.value.size());
    // FIXME: check for errors

    if (!found) {
	RETURN(false);
    }
    
    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    cursor->get_tag(item);
    // FIXME: check for errors

    // FIXME: unwanted copy
    tag.value = string(reinterpret_cast<char *>(item->tag), item->tag_len);

    // FIXME: ensure that these loses get called whatever exit route happens.
    Btree_item_lose(item);
    
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
QuartzDiskTable::set_entry(const QuartzDbKey & key, const QuartzDbTag * tag)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::set_entry",
	      "QuartzDbKey(" << key.value << "), "
	      "QuartzDbTag*(" << (tag == 0 ? "<NULL>" : tag->value) << ")");

    Assert(!key.value.empty());
    Assert(opened);
    if (readonly) throw OmInvalidOperationError("Attempt to modify a readonly table.");

    if (int(key.value.size()) > btree_for_writing->max_key_len) {
	throw OmInvalidArgumentError(
		"Key too long: length was " +
		om_tostring(key.value.size()) +
		" bytes, maximum length of a key is " + 
		om_tostring(btree_for_writing->max_key_len) +
		" bytes.");
    }

    if (tag == 0) {
	// delete entry
	int result;
	result = btree_for_writing->del((byte *)(key.value.data()),
					key.value.size());
	(void)result; // FIXME: Check result
    } else {
	// add entry
	int result;
	result = btree_for_writing->add((byte *)(key.value.data()),
					key.value.size(),
					(byte *)(tag->value.data()),
					tag->value.size());
	(void)result; // FIXME: Check result
    }
}

void
QuartzDiskTable::apply(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzDiskTable::apply", new_revision);

    Assert(opened);
    if (readonly) throw OmInvalidOperationError("Attempt to modify a readonly table.");

    // Close reading table
    Assert(btree_for_reading != 0);
    Btree_quit(btree_for_reading);
    btree_for_reading = 0;

    // Close writing table and write changes
    Assert(btree_for_writing != 0);
    int errorval = Btree_close(btree_for_writing, new_revision);
    if (errorval) {
	throw OmDatabaseError("Can't commit new revision: error code " +
			      om_tostring(errorval));
    }
    btree_for_writing = 0;

    // Reopen table
    if (!open(new_revision)) {
	throw OmDatabaseError("Can't open the revision we've just written (" +
			      om_tostring(new_revision) + ") for table at `" +
			      path + "'");
    }
    // FIXME: check for errors
    // FIXME: want to indicate that the database closed successfully even
    // if we now can't open it.  Or is this a panic situation?
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
QuartzBufferedTable::write_internal()
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::write_internal", "");
    try {
	QuartzTableEntries::items & entries = changed_entries.get_all_entries();
	map<QuartzDbKey, QuartzDbTag *>::iterator entry;
	entry = entries.begin();
	Assert(entry != entries.end());
	// Don't set the null entry.
	for (entry++;
	     entry != changed_entries.get_all_entries().end();
	     entry++) {
	    DEBUGLINE(DB, "QuartzBufferedTable::write_internal(): setting key " << hex_encode(entry->first.value) << " to " << ((entry->second)? (hex_encode(entry->second->value)) : string("<NULL>")));
	    disktable->set_entry(entry->first, entry->second);
	    delete entry->second;
	    entry->second = 0;
	}
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();
}

void
QuartzBufferedTable::write()
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::write", "");
    // FIXME: implement
    // write_internal();
}

void
QuartzBufferedTable::apply(quartz_revision_number_t new_revision)
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::apply", new_revision);
    write_internal();
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
QuartzBufferedTable::have_tag(const QuartzDbKey &key)
{
    DEBUGCALL(DB, bool, "QuartzBufferedTable::have_tag",
	      "QuartzDbKey(" << key.value << ")");
    if (changed_entries.have_entry(key)) {
	RETURN((changed_entries.get_tag(key) != 0));
    } else {
	// FIXME: don't want to read tag here - just want to check if there
	// is a tag.
	QuartzDbTag tag;
	RETURN(disktable->get_exact_entry(key, tag));
    }
}

QuartzDbTag *
QuartzBufferedTable::get_or_make_tag(const QuartzDbKey &key)
{
    DEBUGCALL(DB, QuartzDbTag *, "QuartzBufferedTable::get_or_make_tag",
	      "QuartzDbKey(" << key.value << ")");
    QuartzDbTag * tagptr;

    // Check cache first
    if (changed_entries.have_entry(key)) {
	tagptr = changed_entries.get_tag(key);
	if (tagptr == 0) {
	    // make new empty tag
	    AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	    tagptr = tag.get();
	    changed_entries.set_tag(key, tag);
	    entry_count += 1;

	    AssertEq(changed_entries.get_tag(key), tagptr);
	    Assert(tag.get() == 0);

	    RETURN(tagptr);
	} else {
	    RETURN(tagptr);
	}
    }

    AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
    tagptr = tag.get();

    bool found = disktable->get_exact_entry(key, *tag);

    changed_entries.set_tag(key, tag);
    if (found && tagptr == 0) {
	Assert(entry_count != 0);
	entry_count -= 1;
    } else if (!found && tagptr != 0) {
	entry_count += 1;
    }
    Assert(changed_entries.have_entry(key));
    AssertEq(changed_entries.get_tag(key), tagptr);
    Assert(tag.get() == 0);

    RETURN(tagptr);
}

void
QuartzBufferedTable::delete_tag(const QuartzDbKey &key)
{
    DEBUGCALL(DB, void, "QuartzBufferedTable::delete_tag",
	      "QuartzDbKey(" << key.value << ")");
    // This reads the tag to check if it currently exists, so we can keep
    // track of the number of entries in the table.
    if (have_tag(key)) entry_count -= 1;
    changed_entries.set_tag(key, AutoPtr<QuartzDbTag>(0));
}

quartz_tablesize_t
QuartzBufferedTable::get_entry_count() const
{
    DEBUGCALL(DB, quartz_tablesize_t, "QuartzBufferedTable::get_entry_count", "");
    RETURN(entry_count);
}

bool
QuartzBufferedTable::get_exact_entry(const QuartzDbKey &key,
				     QuartzDbTag & tag) const
{
    DEBUGCALL(DB, bool, "QuartzBufferedTable::get_exact_entry",
	      "QuartzDbKey(" << key.value << "), " <<
	      "QuartzDbTag(" << tag.value << ")");
    if (changed_entries.have_entry(key)) {
	const QuartzDbTag * tagptr = changed_entries.get_tag(key);
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
    return new QuartzBufferedCursor(disktable->cursor_get(),
				    &changed_entries);
}

bool
QuartzBufferedCursor::find_entry(const QuartzDbKey &key)
{
    DEBUGCALL(DB, bool, "QuartzBufferedCursor::find_entry",
	      "QuartzDbKey(" << key.value << ")");
    Assert(key.value != "");

    // Whether we have an exact match.
    bool have_exact;

    // Set diskcursor.
    have_exact = diskcursor->find_entry(key);
    Assert(have_exact || key.value != diskcursor->current_key.value);

    // Set changed_entries part of cursor.
    iter = changed_entries->get_iterator(key);

    const QuartzDbKey * keyptr;
    const QuartzDbTag * tagptr;
    changed_entries->get_item(iter, &keyptr, &tagptr);

    // Check for exact matches
    if (keyptr->value == key.value) {
	if (tagptr != 0) {
	    // Have an exact match in the cache, and it's not a deletion.
	    current_key.value = keyptr->value;
	    current_tag.value = tagptr->value;
	    RETURN(true);
	}
    } else if (diskcursor->current_key.value == key.value) {
	// Have an exact match on disk, and it's not shadowed by the cache.
	current_key.value = diskcursor->current_key.value;
	current_tag.value = diskcursor->current_tag.value;
	RETURN(true);
    }

    // No exact match.  Move backwards until match isn't a deletion.
    while (*keyptr >= diskcursor->current_key && tagptr == 0) {
	if (keyptr->value.size() == 0) {
	    // diskcursor must also point to a null key - we're done.
	    current_key.value = "";
	    current_tag.value = "";
	    RETURN(false);
	}
	if (keyptr->value == diskcursor->current_key.value) {
	    diskcursor->prev();
	}
	changed_entries->prev(iter);
	changed_entries->get_item(iter, &keyptr, &tagptr);
    }

    // Have found a match which isn't a deletion.
    if (*keyptr >= diskcursor->current_key) {
	// Match is in cache
	current_key.value = keyptr->value;
	current_tag.value = tagptr->value;
    } else {
	// Match is on disk
	current_key.value = diskcursor->current_key.value;
	current_tag.value = diskcursor->current_tag.value;
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
    const QuartzDbKey * keyptr;
    const QuartzDbTag * tagptr;

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
		keyptr->value == diskcursor->current_key.value) {
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
	    current_key.value = diskcursor->current_key.value;
	    current_tag.value = diskcursor->current_tag.value;
	}
    } else {
	if (diskcursor->after_end() || *keyptr < diskcursor->current_key) {
	    Assert(tagptr != 0);
	    current_key.value = keyptr->value;
	    current_tag.value = tagptr->value;
	} else {
	    current_key.value = diskcursor->current_key.value;
	    current_tag.value = diskcursor->current_tag.value;
	}
    }
}

void
QuartzBufferedCursor::prev()
{
    DEBUGCALL(DB, void, "QuartzBufferedCursor::prev", "");
    throw OmUnimplementedError("QuartzBufferedCursor::prev() not yet implemented");
}
