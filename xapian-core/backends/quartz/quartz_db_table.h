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
#include <string>
#include <map>

/** A tag in a quartz table.
 *
 *  A tag is a piece of data associated with a given key.
 */
struct QuartzDbTag {
    public:
	/// The contents of the tag.
	string value;
};


/** The key used to access a block of data in a quartz database.
 */
struct QuartzDbKey {
    public:
	/// The contents of the key.
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

	/// Compare two revision numbers
	bool operator == (QuartzRevisionNumber other) const {
	    return (value == other.value);
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
 *  A table is a store holding a set of key/tag pairs.  Each key or tag may
 *  be of arbitrary length
 */
class QuartzDbTable {
    private:
	/// Copying not allowed
	QuartzDbTable(const QuartzDbTable &);

	/// Assignment not allowed
	void operator=(const QuartzDbTable &);

	/** Store the data in memory for now.  FIXME: this is only to assist
	 *  the early development.
	 */
	std::map<QuartzDbKey, QuartzDbTag> data;

	/** The current revision number.
	 */
	quartz_revision_number_t revision;

	/** Whether the database is readonly.
	 */
	bool readonly;
    public:
	/** Open the table.
	 *
	 *  @param - whether to open the table for read only access.
	 */
	QuartzDbTable(bool readonly_);

	/** Close the table.
	 */
	~QuartzDbTable();

	/** Get an object holding the revision number of this table.
	 *
	 *  See the documentation for the QuartzRevisionNumber class for
	 *  an explanation of why the actual revision number may not be
	 *  accessed.
	 *
	 *  @return the current revision number.
	 */
	QuartzRevisionNumber get_revision_number() const;

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
	 *  will be set to a null value.
	 *
	 *  @param key  The key to look for in the table.
	 *  @param tag  A tag object to fill with the value found.
	 *
	 *  @return true if the exact key was found in the table, false
	 *          otherwise.
	 */
	bool read_entry(QuartzDbKey &key, QuartzDbTag & tag) const;

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
	bool read_entry_exact(const QuartzDbKey &key, QuartzDbTag & tag) const;

	/** Modify the entries in the table.
	 *
	 *  Each key / tag pair is added to the database.
	 *
	 *  If the key already exists in the database, the existing tag
	 *  is replaced by the supplied one.
	 *
	 *  If an entry is specified with a null pointer for the tag, then
	 *  the entry will be removed from the database.
	 *
	 *  @param entries   The key / tag pairs to store in the table.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries);
};

#endif /* OM_HGUARD_QUARTZ_DB_TABLE_H */
