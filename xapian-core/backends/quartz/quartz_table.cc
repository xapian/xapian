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

bool
QuartzDiskCursor::find_entry(const QuartzDbKey &key)
{
    Assert(!(key.value.empty()));

    is_after_end = false;

    int found = Bcursor_find_key(cursor, key.value.data(), key.value.size());
    // FIXME: check for errors

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    int err = Bcursor_get_key(cursor, item);
    // FIXME: check for errors

    is_positioned = Bcursor_get_tag(cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    Btree_item_lose(item);

    return found;
}

void
QuartzDiskCursor::next()
{
    Assert(!is_after_end);
    if (!is_positioned) {
	is_after_end = true;
	return;
    }

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    Bcursor_get_key(cursor, item);
    // FIXME: check for errors
    is_positioned = Bcursor_get_tag(cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    Btree_item_lose(item);
}

void
QuartzDiskCursor::prev()
{
    Assert(!is_after_end);
    Assert(current_key.value.size() != 0);

    if (!is_positioned) {
	// FIXME: want to go to last item in table - this method
	// should work, but isn't going to be of great efficiency.
	int found = Bcursor_find_key(cursor,
				     current_key.value.data(),
				     current_key.value.size());
	// FIXME: check for errors
	Assert(found);
    } else {
	Bcursor_prev(cursor);
	// FIXME: check for errors
    }

    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    Bcursor_get_key(cursor, item);
    // FIXME: check for errors

    if (item->key_len != 0) {
	Bcursor_prev(cursor);
	// FIXME: check for errors
	Bcursor_get_key(cursor, item);
	// FIXME: check for errors
    }

    is_positioned = Bcursor_get_tag(cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copies
    current_key.value =
	    string(reinterpret_cast<const char *>(item->key), item->key_len);
    current_tag.value =
	    string(reinterpret_cast<const char *>(item->tag), item->tag_len);

    Btree_item_lose(item);
}


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
	if (btree_for_reading == 0 || btree_for_reading->error) {
	    // FIXME: explain why
	    std::string errormsg = "Cannot open table `"+path+"' for reading: ";
	    if (btree_for_reading)
		errormsg += om_tostring(btree_for_reading->error) + ", ";
	    errormsg += strerror(errno);
	    close();
	    throw OmOpeningError(errormsg);
	}
	opened = true;
	return;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "DB")) {
	int err_num = Btree_create(path.c_str(), blocksize);
	if (err_num != 0) {
	    // FIXME: check for errors

	    throw OmOpeningError("Cannot create table `" + path + "': " +
				 om_tostring(err_num) + ", " + strerror(errno));
	    // FIXME: explain why
	}
    }

    btree_for_writing = Btree_open_to_write(path.c_str());
    // FIXME: check for errors

    if (btree_for_writing == 0 || btree_for_writing->error) {
	// FIXME: explain why
	std::string errormsg = "Cannot open table `"+path+"' for writing: ";
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
	std::string errormsg = "Cannot open table `" + path +
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
    close();

    if (readonly) {
	btree_for_reading = Btree_open_to_read_revision(path.c_str(), revision);
	// FIXME: check for errors
	if (btree_for_reading == 0 || btree_for_reading->error) {
	    // FIXME: throw an exception if it's not just this revision which
	    // is unopenable.
	    close();
	    return false;
	}
	opened = true;
	return true;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "DB")) {
	if (!Btree_create(path.c_str(), blocksize)) {
	// FIXME: check for errors
	    // FIXME: explain why
	    throw OmOpeningError("Cannot create table `" + path + "'.");
	}
    }

    btree_for_writing = Btree_open_to_write_revision(path.c_str(), revision);
    // FIXME: check for errors
    if (btree_for_writing == 0 || btree_for_writing->error) {
	// FIXME: throw an exception if it's not just this revision which
	// is unopenable.
	close();
	return false;
    }

    AssertEq(btree_for_writing->revision_number, revision);

    btree_for_reading = Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number);
    // FIXME: check for errors
    if (btree_for_reading == 0 || btree_for_reading->error) {
	// FIXME: throw an exception if it's not just this revision which
	// is unopenable.
	close();
	return false;
    }

    AssertEq(btree_for_reading->revision_number, revision);

    opened = true;
    return true;
}

QuartzDiskTable::~QuartzDiskTable()
{
    close();
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

bool
QuartzDiskTable::get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    Assert(opened);
    Assert(!(key.value.empty()));

    // FIXME: avoid having to create a cursor here.
    struct Bcursor * cursor = Bcursor_create(btree_for_reading);
    // FIXME: check for errors

    int found = Bcursor_find_key(cursor, key.value.data(), key.value.size());
    // FIXME: check for errors

    if (!found) {
	Bcursor_lose(cursor);
	return false;
    }
    
    Btree_item * item = Btree_item_create();
    if (item == 0) throw std::bad_alloc();

    Bcursor_get_tag(cursor, item);
    // FIXME: check for errors

    // FIXME: unwanted copy
    tag.value = string(reinterpret_cast<char *>(item->tag), item->tag_len);

    // FIXME: ensure that these loses get called whatever exit route happens.
    Btree_item_lose(item);
    Bcursor_lose(cursor);
    
    return true;
}

QuartzDiskCursor *
QuartzDiskTable::cursor_get() const
{
    Assert(opened);
    return new QuartzDiskCursor(btree_for_reading);
}

bool
QuartzDiskTable::set_entry(const QuartzDbKey & key, const QuartzDbTag * tag)
{
    Assert(key.value.size() != 0);
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
    // FIXME: check for errors
    // FIXME: want to indicate that the database closed successfully even
    // if we now can't open it.  Or is this a panic situation?

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
	entry = changed_entries.get_all_entries().begin();
	Assert(entry != changed_entries.get_all_entries().end());
	// Don't apply the null entry.
	for (entry++;
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

QuartzBufferedCursor *
QuartzBufferedTable::cursor_get() const
{
    return new QuartzBufferedCursor(disktable->cursor_get(),
				    &changed_entries);
}

bool
QuartzBufferedCursor::find_entry(const QuartzDbKey &key)
{
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
	    return true;
	}
    } else if (diskcursor->current_key.value == key.value) {
	// Have an exact match on disk, and it's not shadowed by the cache.
	current_key.value = diskcursor->current_key.value;
	current_tag.value = diskcursor->current_tag.value;
	return true;
    }

    // No exact match.  Move backwards until match isn't a deletion.
    while (*keyptr >= diskcursor->current_key && tagptr == 0) {
	if (keyptr->value.size() == 0) {
	    // diskcursor must also point to a null key - we're done.
	    current_key.value = "";
	    current_tag.value = "";
	    return false;
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

    return false;
}

bool
QuartzBufferedCursor::after_end()
{
    return (diskcursor->after_end() && changed_entries->after_end(iter));
}

void
QuartzBufferedCursor::next()
{
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
    throw OmUnimplementedError("QuartzBufferedCursor::prev() not yet implemented");
}

