/* quartz_table.cc: A table in a quartz database
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

#include "config.h"
#include "omdebug.h"

#include "quartz_table.h"
#include "om/omerror.h"
#include "utils.h"
#include <string.h>
#include <errno.h>

// FIXME: just temporary
#include <stdio.h>

QuartzDiskTable::QuartzDiskTable(std::string path_,
				 bool readonly_,
				 unsigned int blocksize_)
	: path(path_),
	  blocksize(blocksize_),
	  opened(false),
          readonly(readonly_),
	  btree_for_reading(0),
	  btree_for_writing(0)
{
}

void
QuartzDiskTable::close()
{
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

void
QuartzDiskTable::open()
{
    close();

    if (readonly) {
	btree_for_reading = Btree_open_to_read(path.c_str());
	if (btree_for_reading == 0) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot open table `"+path+"' for reading.");
	}
	opened = true;
	return;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "DB")) {
#if 1
	Btree_create(path.c_str(), blocksize);
#else
	if (!Btree_create(path.c_str(), blocksize)) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot create table `" + path + "'.");
	}
#endif
    }

    btree_for_writing = Btree_open_to_write(path.c_str());
    if (btree_for_writing == 0) {
	// FIXME: explain why
	throw OmOpeningError("Cannot open table `"+path+"' for writing.");
    }

    btree_for_reading = Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number);
    if (btree_for_reading == 0) {
	close();
	// FIXME: explain why
	throw OmOpeningError("Cannot open table `" + path +
			     "' for reading and writing.");
    }

    opened = true;
}

bool
QuartzDiskTable::open(quartz_revision_number_t revision)
{
    close();

    if (readonly) {
	btree_for_reading = Btree_open_to_read_revision(path.c_str(), revision);
	if (btree_for_reading == 0) {
	    // FIXME: throw an exception if it's not just this revision which
	    // unopenable.
	    return false;
	}
	opened = true;
	return true;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "/DB")) {
#if 1
	Btree_create(path.c_str(), blocksize);
#else
	if (!Btree_create(path.c_str(), blocksize)) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot create table `" + path + "'.");
	}
#endif
    }

    btree_for_writing = Btree_open_to_write_revision(path.c_str(), revision);
    if (btree_for_writing == 0) {
	// FIXME: throw an exception if it's not just this revision which
	// unopenable.
	return false;
    }

    AssertEq(btree_for_writing->revision_number, revision);

    btree_for_reading = Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number);
    if (btree_for_reading == 0) {
	close();
	// FIXME: throw an exception if it's not just this revision which
	// unopenable.
	return false;
    }

    AssertEq(btree_for_reading->revision_number, revision);

    opened = true;
    return true;
}

QuartzDiskTable::~QuartzDiskTable()
{
}

quartz_revision_number_t
QuartzDiskTable::get_open_revision_number() const
{
    Assert(opened);
    return btree_for_reading->revision_number;
}

quartz_revision_number_t
QuartzDiskTable::get_latest_revision_number() const
{
    // FIXME: implement with a call to martin's code
    if (btree_for_reading->both_bases &&
	btree_for_reading->other_revision_number > btree_for_reading->revision_number) {
	return btree_for_reading->other_revision_number;
    }
    return btree_for_reading->revision_number;
}

quartz_tablesize_t
QuartzDiskTable::get_entry_count() const
{
    Assert(opened);
    return btree_for_reading->item_count;
}

AutoPtr<QuartzCursor>
QuartzDiskTable::make_cursor()
{
    Assert(opened);
    return AutoPtr<QuartzCursor>(new QuartzCursor(btree_for_reading));
}

bool
QuartzDiskTable::get_nearest_entry(QuartzDbKey &key,
				   QuartzDbTag &tag,
				   QuartzCursor &cursor) const
{
    Assert(opened);
    Assert(!(key.value.empty()));

    int found = Bcursor_find_key(cursor.cursor, key.value.data(), key.value.size());
    // FIXME: check for errors

    Btree_item * item = Btree_item_create();
    // FIXME: check for errors

    int err = Bcursor_get_key(cursor.cursor, item);
    Assert(err != 0); // Must be positioned
    // FIXME: check for errors

    cursor.is_positioned = Bcursor_get_tag(cursor.cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    key.value = string(reinterpret_cast<char *>(item->key), item->key_len);
    tag.value = string(reinterpret_cast<char *>(item->tag), item->tag_len);

    Btree_item_lose(item);

    return found;
}

bool
QuartzDiskTable::get_next_entry(QuartzDbKey &key,
				QuartzDbTag &tag,
				QuartzCursor &cursor) const
{
    Assert(opened);

    if (!cursor.is_positioned) {
	return false;
    }

    Btree_item * item = Btree_item_create();
    // FIXME: check for errors
    Bcursor_get_key(cursor.cursor, item);
    // FIXME: check for errors
    cursor.is_positioned = Bcursor_get_tag(cursor.cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    tag.value = string(reinterpret_cast<char *>(item->tag), item->tag_len);
    key.value = string(reinterpret_cast<char *>(item->key), item->key_len);

    return true;
}

bool
QuartzDiskTable::get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    Assert(opened);
    Assert(!(key.value.empty()));

    // FIXME: avoid having to create a cursor here.
    struct Bcursor * cursor = Bcursor_create(btree_for_reading);
    int found = Bcursor_find_key(cursor, key.value.data(), key.value.size());

    if (!found) {
	Bcursor_lose(cursor);
	return false;
    }
    
    Btree_item * item = Btree_item_create();
    Bcursor_get_tag(cursor, item);
    // FIXME: unwanted copy
    tag.value = string(reinterpret_cast<char *>(item->tag), item->tag_len);

    // FIXME: ensure that these loses get called whatever exit route happens.
    Btree_item_lose(item);
    Bcursor_lose(cursor);
    
    return true;
}

bool
QuartzDiskTable::set_entry(const QuartzDbKey & key, const QuartzDbTag * tag)
{
    Assert(opened);
    if(readonly) throw OmInvalidOperationError("Attempt to modify a readonly table.");

    if (tag == 0) {
	// delete entry
	int result = Btree_delete(btree_for_writing,
				  key.value.data(),
				  key.value.size());
	// FIXME: Check result
    } else {
	// add entry
	int result = Btree_add(btree_for_writing,
			       key.value.data(),
			       key.value.size(),
			       tag->value.data(),
			       tag->value.size());
	// FIXME: Check result
    }

    return true;
}

bool
QuartzDiskTable::apply(quartz_revision_number_t new_revision)
{
    Assert(opened);
    if(readonly) throw OmInvalidOperationError("Attempt to modify a readonly table.");

    // Close reading table
    Assert(btree_for_reading != 0);
    Btree_quit(btree_for_reading);
    btree_for_reading = 0;

    // Close writing table and write changes
    Assert(btree_for_writing != 0);
    Btree_close(btree_for_writing, new_revision);
    // FIXME: check for errors
    btree_for_writing = 0;

    // Reopen table
    open();

    return true;
}




QuartzBufferedTable::QuartzBufferedTable(QuartzDiskTable * disktable_)
	: disktable(disktable_),
	  entry_count(disktable->get_entry_count())
{
}

QuartzBufferedTable::~QuartzBufferedTable()
{
}

bool
QuartzBufferedTable::apply(quartz_revision_number_t new_revision)
{
    bool result;
    try {
	std::map<QuartzDbKey, QuartzDbTag *>::const_iterator entry;
	for (entry = changed_entries.get_all_entries().begin();
	     entry != changed_entries.get_all_entries().end();
	     entry++) {
	    result = disktable->set_entry(entry->first, entry->second);
	}
	disktable->apply(new_revision);
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();
    AssertEq(entry_count, disktable->get_entry_count());
    return result;
}

void
QuartzBufferedTable::cancel()
{
    changed_entries.clear();
    entry_count = disktable->get_entry_count();
}

bool
QuartzBufferedTable::is_modified()
{
    return !changed_entries.empty();
}

QuartzDbTag *
QuartzBufferedTable::get_tag(const QuartzDbKey &key)
{
    if (changed_entries.have_entry(key)) {
	return changed_entries.get_tag(key);
    } else {
	AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	QuartzDbTag * tagptr = tag.get();

	bool found = disktable->get_exact_entry(key, *tagptr);

	if (found) {
	    changed_entries.set_tag(key, tag);
	    AssertEq(changed_entries.get_tag(key), tagptr);
	} else {
	    tagptr = 0;
	}

	return tagptr;
    }
}

QuartzDbTag *
QuartzBufferedTable::get_or_make_tag(const QuartzDbKey &key)
{
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

	    return tagptr;
	} else {
	    return tagptr;
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

    return tagptr;
}

void
QuartzBufferedTable::delete_tag(const QuartzDbKey &key)
{
    // This reads the tag to check if it currently exists, so we can keep
    // track of the number of entries in the table.
    if (get_tag(key) != 0) {
	entry_count -= 1;
    }
    changed_entries.set_tag(key, AutoPtr<QuartzDbTag>(0));
}

quartz_tablesize_t
QuartzBufferedTable::get_entry_count() const
{
    return entry_count;
}

AutoPtr<QuartzCursor>
QuartzBufferedTable::make_cursor()
{
    return disktable->make_cursor();
}

bool
QuartzBufferedTable::get_nearest_entry(QuartzDbKey &key,
				       QuartzDbTag &tag,
				       QuartzCursor &cursor) const
{
    Assert(key.value != "");

    // Whether we have an exact match.
    bool have_exact;

    // Set disktable part of cursor.
    cursor.current_key = key;
    have_exact = disktable->get_nearest_entry(cursor.current_key,
					      cursor.current_tag,
					      cursor);
    Assert(have_exact || key.value != cursor.current_key.value);

    // Set and read changed_entries part of cursor.
    cursor.iter = changed_entries.get_iterator(key);

    const QuartzDbKey * keyptr;
    const QuartzDbTag * tagptr;

    // We should have an item, even if it's just the initial null item
    Assert(changed_entries.get_item_and_advance(cursor.iter, &keyptr, &tagptr));

    if (keyptr->value == key.value) {
	// Have an exact match in changed entries => this is the one to use
	have_exact = true;
	key = *keyptr;
	tag = *tagptr;
    } else {
	Assert(*keyptr < key);
	// We use whichever match is greater
	if (*keyptr > cursor.current_key) {
	    Assert(!have_exact);
	    key = *keyptr;
	    tag = *tagptr;
	    // Advance the disktable part, not caring whether it now points
	    // to an entry, but if it does it must be an entry > key asked
	    // for and therefore > key found.
	    disktable->get_next_entry(cursor.current_key,
				      cursor.current_tag,
				      cursor);

	} else {
	    Assert(have_exact || key.value != cursor.current_key.value);
	    key = cursor.current_key;
	    tag = cursor.current_tag;
	    // Advance iterator, not caring whether it now points
	    // to an entry, but if it does it must be an entry > key asked
	    // for and therefore > key found.
	    changed_entries.get_item_and_advance(cursor.iter, &keyptr, &tagptr);
	}
    }

    return have_exact;
}

bool
QuartzBufferedTable::get_next_entry(QuartzDbKey &key,
				    QuartzDbTag &tag,
				    QuartzCursor &cursor) const
{
    bool have_entry;

    have_entry = disktable->get_next_entry(cursor.current_key,
					   cursor.current_tag,
					   cursor);

    if (have_entry) {
	key = cursor.current_key;
	tag = cursor.current_tag;
    }

    return have_entry;
}

bool
QuartzBufferedTable::get_exact_entry(const QuartzDbKey &key,
				     QuartzDbTag & tag) const
{
    if (changed_entries.have_entry(key)) {
	tag = *(changed_entries.get_tag(key));
	return true;
    }

    return disktable->get_exact_entry(key, tag);
}

