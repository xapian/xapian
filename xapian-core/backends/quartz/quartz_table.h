/* quartz_table.h: A table in a quartz database
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

#ifndef OM_HGUARD_QUARTZ_TABLE_H
#define OM_HGUARD_QUARTZ_TABLE_H

#include "config.h"
#include "quartz_types.h"
#include "quartz_table_entries.h"
#include "om/autoptr.h"
#include <string>
#include <map>

#include "btree.h"

class QuartzDiskTable;
class QuartzBufferedTable;

/** A cursor pointing to a position in a quartz table, for reading several
 *  entries in order, or finding approximate matches.
 */
class QuartzCursor {
    private:
	/// Copying not allowed
	QuartzCursor(const QuartzCursor &);

	/// Assignment not allowed
	void operator=(const QuartzCursor &);

    public:
	/// Initialise the cursor 
	QuartzCursor() {}

	/// Destroy the cursor
	virtual ~QuartzCursor() {}

	/** Current key pointed to by cursor.
	 */
	QuartzDbKey current_key;

	/** Current tag pointed to by cursor.
	 */
	QuartzDbTag current_tag;

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
	 *  @param key    The key to look for in the table.
	 *
	 *  @return true if the exact key was found in the table, false
	 *          otherwise.
	 */
	virtual bool find_entry(const QuartzDbKey &key) = 0;

	/** Move the cursor forward in the table.
	 *
	 *  Unless there are no more entries in the table, this method moves
	 *  the cursor forward one position.
	 */
	virtual void next() = 0;

	/** Move the cursor back in the table.
	 *
	 *  If there are no previous entries in the table, the cursor
	 *  will point to a null key.  Otherwise, this method moves the
	 *  cursor back one position.
	 */
	virtual void prev() = 0;

	/** Determine whether cursor is off the end of table.
	 *
	 *  @return true if the cursor has been moved off the end of the
	 *          table, past the last entry in it, and false otherwise.
	 */
	virtual bool after_end() = 0;
};

/** A cursor in a disktable.
 */
class QuartzDiskCursor : public QuartzCursor {
    friend QuartzDiskTable;
    private:
	/** Whether the cursor is positioned at a valid entry.
	 */
	bool is_positioned;

	/** Whether the cursor is off the end of the table.
	 */
	bool is_after_end;

	/** The btree cursor.  This points to the next item, not the current
	 *  item.
	 */
	struct Bcursor * cursor;

    public:
	/// Create the cursor
	QuartzDiskCursor(struct Btree * btree)
		: is_positioned(false),
		  cursor(Bcursor_create(btree)) {}
	
	/// Destroy the cursor
	~QuartzDiskCursor() { Bcursor_lose(cursor); }

	/** Virtual methods of QuartzCursor.
	 */
	//@{
	bool find_entry(const QuartzDbKey &key);
	void next();
	void prev();
	bool after_end() { return is_after_end; }
	//@}
};

/** A cursor in a bufftable.
 */
class QuartzBufferedCursor : public QuartzCursor {
    friend QuartzBufferedTable;
    private:
	/** The cursor on disk.
	 */
	QuartzDiskCursor * diskcursor;

	/** Pointer to changes stored in the buffer.
	 */
	const QuartzTableEntries * changed_entries;

	/** Iterator in changed entries.
	 */
	QuartzTableEntries::items::const_iterator iter;
    public:
	/// Standard constructor
	QuartzBufferedCursor(QuartzDiskCursor * diskcursor_,
			     const QuartzTableEntries * changed_entries_)
		: diskcursor(diskcursor_),
		  changed_entries(changed_entries_)
		{}

	/// Destroy the cursor
	~QuartzBufferedCursor() { delete(diskcursor);}

	/** Virtual methods of QuartzCursor.
	 */
	//@{
	bool find_entry(const QuartzDbKey &key);
	void next();
	void prev();
	bool after_end();
	//@}
};


/** Base class for quartz tables.
 *
 *  This specifies the interface that is shared between objects which
 *  represent actual tables on disk, and objects which represent a modified
 *  table, yet to be written to disk.
 */
class QuartzTable {
    private:
	/// Copying not allowed
	QuartzTable(const QuartzTable &);

	/// Assignment not allowed
	void operator=(const QuartzTable &);
    public:
	/** Create new table.
	 */
	QuartzTable() {}

	/** Close the table.
	 */
	virtual ~QuartzTable() {}

	/** Return a count of the number of entries in the table.
	 *
	 *  @return The number of entries in the table.
	 */
	virtual quartz_tablesize_t get_entry_count() const = 0;

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
	virtual bool get_exact_entry(const QuartzDbKey & key,
				     QuartzDbTag & tag) const = 0;

	/** Get a cursor for reading from the table.
	 */
	virtual QuartzCursor * cursor_get() const = 0;
};

/** Class managing a table in a Quartz database.
 *
 *  A table is a store holding a set of key/tag pairs.  See the
 *  documentation for QuartzDbKey and QuartzDbTag for details of what
 *  comprises a valid key or tag.
 */
class QuartzDiskTable : public QuartzTable {
    private:
	/// Copying not allowed
	QuartzDiskTable(const QuartzDiskTable &);

	/// Assignment not allowed
	void operator=(const QuartzDiskTable &);

	/** The path at which the table is stored.
	 */
	std::string path;

	/** The blocksize to create the database with, if it needs creating.
	 */
	unsigned int blocksize;

	/** Whether the table has been opened.
	 */
	bool opened;
	
	/** Whether the table is readonly.
	 */
	bool readonly;

	/** The btree structure for reading.
	 */
	struct Btree * btree_for_reading;

	/** The btree structure for writing.
	 */
	struct Btree * btree_for_writing;

	/** Close the table.  This closes and frees any of the btree
	 *  structures which have been created and opened.
	 */
	void close();
    public:
	/** Create a new table.  This does not open the table - the open()
	 *  method must be called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 *  @param blocksize_     - Size of blocks to use.  This parameter is
	 *                          only used when creating the table.
	 */
	QuartzDiskTable(std::string path_,
			bool readonly_,
			unsigned int blocksize_);

	/** Close the table.
	 */
	~QuartzDiskTable();

	/** Open the table at the specified revision.
	 *
	 *  @param revision_      - revision number to open.
	 *
	 *  @return true if table is successfully opened at desired revision,
	 *          false if table cannot be opened at desired revision (but
	 *          table is otherwise consistent)
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the table is
	 *             in a corrupt state.
	 *  @exception OmOpeningError will be thrown if the table cannot be
	 *             opened (but is not corrupt - eg, permission problems,
	 *             not present, etc).
	 */
	bool open(quartz_revision_number_t revision_);

	/** Open the latest revision of the table.
	 *
	 *  @exception OmDatabaseCorruptError will be thrown if the table is
	 *             in a corrupt state.
	 *  @exception OmOpeningError will be thrown if the table cannot be
	 *             opened (but is not corrupt - eg, permission problems,
	 *             not present, etc).
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
	 *  If a null pointer is supplied for the tag, then
	 *  the entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  All modifications since the
	 *  previous apply() will be lost.
	 *
	 *  Note that other methods (such as get_nearest_entry())
	 *  must not be performed after this call until apply() has been
	 *  called.
	 *
	 *  @param key   The key to store in the table.
	 *  @param tag   A pointer to the tag to store in the table, or 0
	 *               to delete the entry in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 *
	 */
	bool set_entry(const QuartzDbKey & key, const QuartzDbTag * tag);

	/** Apply changes to the table.
	 *
	 *  Changes made to the table by calling set_entry() are applied to
	 *  disk.  In the event of failure, the changes may be discarded.
	 *
	 *  @param new_revision  The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or undefined behaviour will
	 *          result.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool apply(quartz_revision_number_t new_revision);

	/** Virtual methods of QuartzTable.
	 */
	//@{
	quartz_tablesize_t get_entry_count() const;
	bool get_exact_entry(const QuartzDbKey & key,
			     QuartzDbTag & tag) const;
	QuartzDiskCursor * cursor_get() const;
	//@}
};

/** A buffered table in a Quartz database.
 *
 *  This class provides buffered access to a quartz database.  This allows
 *  a set of modifications to be built up in memory, and flushed to disk
 *  only when it grows sufficiently large.
 */
class QuartzBufferedTable : public QuartzTable {
    private:
	/// Copying not allowed
	QuartzBufferedTable(const QuartzBufferedTable &);

	/// Assignment not allowed
	void operator=(const QuartzBufferedTable &);

	/// The underlying table
	QuartzDiskTable * disktable;

	/** Entries which have been changed.
	 */
	QuartzTableEntries changed_entries;

	/** Number of entries in the table, including the changed entries.
	 */
	quartz_tablesize_t entry_count;
    public:
	/** Create a new table.  This does not open the table - the open()
	 *  method must be called before use is made of the table.
	 *
	 */
	QuartzBufferedTable(QuartzDiskTable * disktable_);

	/** Close the table.
	 *
	 *  Any outstanding changes (ie, changes made without apply() having
	 *  subsequently been called) will be lost.
	 */
	~QuartzBufferedTable();

	/** Apply any outstanding changes.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  The table on disk will be left in an
	 *  unmodified state, and the changes made to it will be lost.
	 *
	 *  @param new_revision  The new revision number to store the
	 *                       modifications under.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool apply(quartz_revision_number_t new_revision);

	/** Cancel any outstanding changes.
	 *
	 *  This causes any modifications held in memory to be forgotten.
	 */
	void cancel();

	/** Determine whether the object contains modifications.
	 *
	 *  @return true if the changed_entries object contains
	 *          modifications to the database, false if no changes have
	 *          been made.
	 */
	bool is_modified();

	/** Get a pointer to the tag for a given key.
	 *
	 *  If the tag is not present in the database, or is currently
	 *  marked for deletion, this will return a null pointer.
	 *
	 *  The pointer is owned by the QuartzBufferedTable (actually, by
	 *  its changed_entries object) - it may be modified, but must not
	 *  be deleted.
	 */
	QuartzDbTag * get_tag(const QuartzDbKey &key);

	/** Get a pointer to the tag for a given key, creating a new tag if
	 *  not present.
	 *
	 *  This will never return a null pointer.
	 *
	 *  The pointer is owned by the QuartzBufferedTable (actually, by
	 *  its changed_entries object) - it may be modified, but must not
	 *  be deleted.
	 */
	QuartzDbTag * get_or_make_tag(const QuartzDbKey &key);

	/** Remove the tag for a given key.
	 *
	 *  This removes the tag for a given key.  If the tag doesn't exist,
	 *  no action is taken.
	 */
	void delete_tag(const QuartzDbKey &key);

	/** Virtual methods of QuartzTable.
	 */
	//@{
	quartz_tablesize_t get_entry_count() const;
	bool get_exact_entry(const QuartzDbKey & key,
			     QuartzDbTag & tag) const;
	QuartzBufferedCursor * cursor_get() const;
	//@}
};

#endif /* OM_HGUARD_QUARTZ_TABLE_H */
