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

	/** The current revision number
	 */
	quartz_revision_number_t revision;
    public:
	/** Open the table.
	 *
	 *  @param - whether to open the table for read only access.
	 */
	QuartzDbTable(bool readonly);

	/** Close the table.
	 */
	~QuartzDbTable();

	/** Get the revision number of this table.  The revision number
	 *  increases monotonically, incrementing by one each time a
	 *  modification is applied to the database.
	 *
	 *  The revision numbers of all the tables comprising a database
	 *  should remain in step, and can be used to ensure that a user is
	 *  accessing a consistent view of the database.
	 *
	 *  @return the current revision number.
	 */
	quartz_revision_number_t get_revision_number();

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
	bool read_entry(QuartzDbKey &key, QuartzDbTag & tag);

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
	bool read_entry_exact(const QuartzDbKey &key, QuartzDbTag & tag);

	/** Modify the entries in the table.
	 *
	 *  The key / tag pairs in entries
	 *
	 *  If an entry is specified with a null tag (ie, 0 length), then
	 *  the entry will be removed from the database.  Otherwise, the
	 *  key/tag pair will be added to the 
	 *
	 *  @param entries   The key / tag pairs to store in the table.
	 */
	set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries);
};

#endif /* OM_HGUARD_QUARTZ_DB_TABLE_H */
