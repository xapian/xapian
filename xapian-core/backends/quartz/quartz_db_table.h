/* quartz_db_table.h: A table in a quartz database
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

#ifndef OM_HGUARD_QUARTZ_DB_TABLE_H
#define OM_HGUARD_QUARTZ_DB_TABLE_H

#include "config.h"
#include "quartz_types.h"
#include "refcnt.h"
#include <string>
#include <map>

/** A tag in a quartz table.
 *
 *  A tag is a piece of data associated with a given key.
 */
struct QuartzDbTag {
    public:
	/** The contents of the tag.
	 *
	 *  Tags may be of arbitrary length.  Note though that they will be
	 *  loaded into memory in their entirety, so should not be permitted
	 *  to grow without bound in normal usage.
	 *
	 *  Tags which are null strings _are_ valid, and are different from a
	 *  tag simply not being in the table.
	 */
	string value;
};


/** The key used to access a block of data in a quartz table.
 */
struct QuartzDbKey {
    public:
	/** The contents of the key.
	 *
	 *  Keys may be of arbitrary length.  Note though that they will be
	 *  loaded into memory in their entirety, so should not be permitted
	 *  to grow without bound in normal usage.
	 *
	 *  Keys may not have null contents.
	 */
	string value;

	/** Comparison operator, so that keys may be used in standard
	 *  containers.
	 */
	bool operator < (const QuartzDbKey & a) const {return (value<a.value);}
};

class QuartzDbTable;

/** An object holding the revision number of a table.
 *
 *  A table's revision number increases monotonically, incrementing by
 *  one each time a modification is applied to the table.
 *
 *  The revision numbers of all the tables comprising a database
 *  should remain in step, and can be used to ensure that a user is
 *  accessing a consistent view of the database.
 *
 *  The absolute value of a revision number should be considered
 *  immaterial - all that matters is the difference between revision
 *  numbers.  It may be assumed that revision numbers will not cycle
 *  through all the available values during a database session, and
 *  hence that if two revision numbers are the same they correspond to
 *  the same revision.
 *
 *  Hence, the only operation which may be applied to revision numbers
 *  is comparison.
 */
class QuartzRevisionNumber {
    friend class QuartzDbTable;
    private:
	/// The actual value of the revision number.
	quartz_revision_number_t value;

	/// Private constructor, only QuartzDbTable calls this.
	QuartzRevisionNumber(quartz_revision_number_t value_)
		: value(value_) {}
    public:
	// use standard destructor
	// ~QuartzRevisionNumber();

	// use standard copy constructor
	// QuartzRevisionNumber(const QuartzRevisionNumber &);

	// use standard assignment operator
	// void operator = (const QuartzRevisionNumber &);

	/// Compare two revision numbers
	bool operator == (QuartzRevisionNumber other) const {
	    return (value == other.value);
	}

	/// Increment a revision number
	QuartzRevisionNumber increment() {
	    ++value;
	    return *this;
	}

	/** Introspection method.
	 *  Note: don't try and use this to get at the actual revision
	 *  number - that would be foolish.  (See the class
	 *  documentation for why.)
	 */
	string get_description() const;
};

inline ostream &
operator << (ostream &os, QuartzRevisionNumber obj) {
    return os << (obj.get_description());
}


/** Class managing a table in a Quartz database.
 *
 *  A table is a store holding a set of key/tag pairs.  See the
 *  documentation for QuartzDbKey and QuartzDbTag for details of what
 *  comprises a valid key or tag.
 */
class QuartzDbTable : public RefCntBase {
    private:
	/// Copying not allowed
	QuartzDbTable(const QuartzDbTable &);

	/// Assignment not allowed
	void operator=(const QuartzDbTable &);

	/** Store the data in memory for now.  FIXME: this is only to assist
	 *  the early development.
	 */
	std::map<QuartzDbKey, QuartzDbTag> data;

	/** The path at which the table is stored.
	 */
	string path;
	
	/** Whether the table is readonly.
	 */
	bool readonly;

	/** The current revision number.
	 */
	QuartzRevisionNumber(revision);

    public:
	/** Open the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 *  @param revision_      - revision number to open.
	 */
	QuartzDbTable(string path_,
		      bool readonly_,
		      QuartzRevisionNumber revision_);

	/** Open the latest revision of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 */
	QuartzDbTable(string path_,
		      bool readonly_);

	/** Close the table.
	 */
	~QuartzDbTable();

	/** Get an object holding the revision number at which this table
	 *  is currently open.
	 *
	 *  It is possible that there are other, more recent or older
	 *  revisions available.
	 *
	 *  See the documentation for the QuartzRevisionNumber class for
	 *  an explanation of why the actual revision number may not be
	 *  accessed.
	 *
	 *  @return the current revision number.
	 */
	QuartzRevisionNumber get_open_revision_number() const;

	/** Get the latest revision number stored in this table.
	 *
	 *  It is possible that there are other, older, revisions of this
	 *  table available, and indeed that the revision currently open
	 *  is one of these older revisions.
	 */
	QuartzRevisionNumber get_latest_revision_number() const;

	/** Return a count of the number of entries in the table.
	 *
	 *  @return THe number of entries in the table.
	 */
	quartz_tablesize_t get_entry_count() const;

	/** Read an entry from the table.
	 *
	 *  If the key is found in the table, the tag will be filled with
	 *  the data associated with the key.
	 *
	 *  If the key is not found, the key will be set to the value of
	 *  the key preceding that asked for, and the tag will be filled
	 *  with the data associated with that key.
	 *
	 *  If there is no key preceding that asked for, the key and tag
	 *  will be set to a null value.  Note that, if you are testing
	 *  for this condition, you should test whether the key has a null
	 *  value, since null tag values are allowed to be stored in
	 *  tables.
	 *
	 *  @param key  The key to look for in the table.
	 *  @param tag  A tag object to fill with the value found.
	 *
	 *  @return true if the exact key was found in the table, false
	 *          otherwise.
	 */
	bool get_nearest_entry(QuartzDbKey &key, QuartzDbTag & tag) const;

	/** Read an entry from the table, if and only if it is exactly that
	 *  being asked for.
	 *
	 *  If the key is found in the table, the tag will be filled with
	 *  the data associated with the key.  If the key is not found,
	 *  the tag will be unmodified.
	 *
	 *  @param key  The key to look for in the table
	 *  @param tag  A tag object to fill with the value if found.
	 *
	 *  @return true if key is found in table,
	 *          false if key is not found in table.
	 */
	bool get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const;

	/** Modify the entries in the table.
	 *
	 *  Each key / tag pair is added to the table.
	 *
	 *  If the key already exists in the table, the existing tag
	 *  is replaced by the supplied one.
	 *
	 *  If an entry is specified with a null pointer for the tag, then
	 *  the entry will be removed from the table, if it exists.  If
	 *  it does not exist, no action will be taken.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  The table will be left in an
	 *  unmodified state.
	 *
	 *  @param entries       The key / tag pairs to store in the table.
	 *  @param new_revision  The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or undefined behaviour will
	 *          result.  If not specified, the new revision number will be
	 *          the current one plus 1.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries,
			 QuartzRevisionNumber new_revision);
};

#endif /* OM_HGUARD_QUARTZ_DB_TABLE_H */
