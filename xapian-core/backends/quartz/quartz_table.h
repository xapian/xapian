/* quartz_table.h: A table in a quartz database
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

#ifndef OM_HGUARD_QUARTZ_TABLE_H
#define OM_HGUARD_QUARTZ_TABLE_H

#include "quartz_types.h"
#include "bcursor.h"
#include <string>
using std::string;

class QuartzTable;

/** A cursor pointing to a position in a quartz table, for reading several
 *  entries in order, or finding approximate matches.
 */
class QuartzCursor {
    friend class QuartzTable;
    private:
	/// Copying not allowed
	QuartzCursor(const QuartzCursor &);

	/// Assignment not allowed
	void operator=(const QuartzCursor &);

	/** Whether the cursor is positioned at a valid entry.
	 */
	bool is_positioned;

	/** Whether the cursor is off the end of the table.
	 */
	bool is_after_end;

	/** The btree cursor.  This points to the next item, not the current
	 *  item.
	 */
	Bcursor cursor;

    public:
	/// Initialise the cursor 
	QuartzCursor(Btree * btree);

	/// Destroy the cursor
	~QuartzCursor() {}

	/** Current key pointed to by cursor.
	 */
	string current_key;

	/** Current tag pointed to by cursor.
	 */
	string current_tag;

	/** Find an entry, or a near match, in the table.
	 *
	 *  If the exact key is found in the table, the cursor will be
	 *  set to point to it, and the method will return true.
	 *
	 *  If the key is not found, the cursor will be set to point to
	 *  the key preceding that asked for, and the method will return
	 *  false.
	 *
	 *  If there is no key preceding that asked for, the cursor will
	 *  point to a null key.
	 *
	 *  Note: Calling this method with a null key, then calling next()
	 *  will leave the cursor pointing to the first key.
	 *
	 *  @param key    The key to look for in the table.
	 *
	 *  @return true if the exact key was found in the table, false
	 *          otherwise.
	 */
	bool find_entry(const string &key);

	/** Move the cursor forward in the table.
	 *
	 *  Unless there are no more entries in the table, this method moves
	 *  the cursor forward one position.
	 */
	void next();

	/** Move the cursor back in the table.
	 *
	 *  If there are no previous entries in the table, the cursor
	 *  will point to a null key.  Otherwise, this method moves the
	 *  cursor back one position.
	 */
	void prev();

	/** Determine whether cursor is off the end of table.
	 *
	 *  @return true if the cursor has been moved off the end of the
	 *          table, past the last entry in it, and false otherwise.
	 */
	bool after_end() { return is_after_end; }
};


/** Class managing a table in a Quartz database.
 *
 *  A table is a store holding a set of key/tag pairs.
 *
 *  A key is used to access a block of data in a quartz table.
 * 
 *  Keys are of limited length.
 *
 *  Keys may not have null contents.
 *
 *  A tag is a piece of data associated with a given key.  The contents
 *  of the tag are opaque to QuartzTable.
 *
 *  Tags may be of arbitrary length.  Note though that they will be
 *  loaded into memory in their entirety, so should not be permitted
 *  to grow without bound in normal usage.
 *
 *  Tags which are null strings _are_ valid, and are different from a
 *  tag simply not being in the table.
 */
class QuartzTable {
    private:
	/// Copying not allowed
	QuartzTable(const QuartzTable &);

	/// Assignment not allowed
	void operator=(const QuartzTable &);

	/** The path at which the table is stored.
	 */
	string path;

	/** The blocksize to create the database with, if it needs creating.
	 */
	unsigned int blocksize;

	/** Whether the table is readonly.
	 */
	bool readonly;

	/** Has this table been modified?
	 */
	bool is_modified_flag;

	/** The btree object.
	 */
	Btree * btree;

	/** Close the table.  This closes and frees any of the btree
	 *  structures which have been created and opened.
	 */
	void close();

    public:
	/** Create a new table object.
	 *
	 *  This does not create the table on disk - the create() method must
	 *  be called before the table is created on disk
	 *
	 *  This also does not open the table - the open() method must be
	 *  called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 *  @param blocksize_     - Size of blocks to use.  This parameter is
	 *                          only used when creating the table.
	 */
	QuartzTable(string path_, bool readonly_, unsigned int blocksize_);

	/** Close the table.
	 *
	 *  Any outstanding changes (ie, changes made without apply() having
	 *  subsequently been called) will be lost.
	 */
	~QuartzTable();

	/** Determine whether the table exists on disk.
	 */
	bool exists();

	/** Create the table on disk.
	 *
	 *  @exception Xapian::DatabaseCreateError if the table can't be created.
	 */
	void create();

	/** Open the table at the specified revision.
	 *
	 *  @param revision_      - revision number to open.
	 *
	 *  @return true if table is successfully opened at desired revision,
	 *          false if table cannot be opened at desired revision (but
	 *          table is otherwise consistent)
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the table
	 *	is in a corrupt state.
	 *  @exception Xapian::DatabaseOpeningError will be thrown if the table
	 *	cannot be opened (but is not corrupt - eg, permission problems,
	 *	not present, etc).
	 */
	bool open(quartz_revision_number_t revision_);

	/** Open the latest revision of the table.
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the table
	 *	is in a corrupt state.
	 *  @exception Xapian::DatabaseOpeningError will be thrown if the table
	 *	cannot be opened (but is not corrupt - eg, permission problems,
	 *	not present, etc).
	 */
	void open();

	/** Get the revision number at which this table
	 *  is currently open.
	 *
	 *  It is possible that there are other, more recent or older
	 *  revisions available.
	 *
	 *  @return the current revision number.
	 */
	quartz_revision_number_t get_open_revision_number() const;

	/** Get the latest revision number stored in this table.
	 *
	 *  It is possible that there are other, older, revisions of this
	 *  table available, and indeed that the revision currently open
	 *  is one of these older revisions.
	 */
	quartz_revision_number_t get_latest_revision_number() const;

	/** Set an entry in the table.
	 *
	 *  If the key already exists in the table, the existing tag
	 *  is replaced by the supplied one.
	 *
	 *  If the tag parameter is omitted, then
	 *  the entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  [FIXME: erm, it returns void...]
	 *  All modifications since the
	 *  previous apply() will be lost.
	 *
	 *  @param key   The key to store in the table.
	 *  @param tag   The tag to store in the table, or omit
	 *               to delete the entry in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise. [FIXME: erm, it returns void...]
	 *
	 */
	void set_entry(const string & key, const string & tag);
	void set_entry(const string & key);

	/** Apply any outstanding changes to the table.
	 *
	 *  Changes made to the table by calling set_entry() are committed
	 *  to the Btree.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by an exception.  Changes made will not be committed to the Btree
	 *  - they will be discarded.
	 *
	 *  @param new_revision  The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or an exception will be
	 *          thrown.
	 */
	void apply(quartz_revision_number_t new_revision);

	/** Cancel any outstanding changes.
	 *
	 *  This will discard any modifications which haven't been committed
	 *  by calling apply().
	 */
	void cancel();

	/** Determine whether the object contains uncommitted modifications.
	 *
	 *  @return true if there have been modifications since the last
	 *          the last call to apply().
	 */
	bool is_modified() const { return is_modified_flag; }

	/** Return a count of the number of entries in the table.
	 *
	 *  @return The number of entries in the table.
	 */
	quartz_tablesize_t get_entry_count() const;

	/** Read an entry from the table, if and only if it is exactly that
	 *  being asked for.
	 *
	 *  If the key is found in the table, the tag will be filled with
	 *  the data associated with the key.  If the key is not found,
	 *  the tag will be unmodified.
	 *
	 *  @param key  The key to look for in the table.
	 *  @param tag  A tag object to fill with the value if found.
	 *
	 *  @return true if key is found in table,
	 *          false if key is not found in table.
	 */
	bool get_exact_entry(const string & key, string & tag) const;

	/** Get a cursor for reading from the table.
	 *  The cursor is owned by the caller - it is the caller's
	 *  responsibility to ensure that it is deleted.
	 */
	QuartzCursor * cursor_get() const;
};

#endif /* OM_HGUARD_QUARTZ_TABLE_H */
