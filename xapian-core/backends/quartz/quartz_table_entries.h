/* quartz_table_entries.h: Storage of a set of table entries from a quartz db
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

#ifndef OM_HGUARD_QUARTZ_TABLE_ENTRIES_H
#define OM_HGUARD_QUARTZ_TABLE_ENTRIES_H

#include "config.h"
#include <map>
#include "om/autoptr.h"
#include "quartz_types.h"
#include <string>

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

/** This class stores a set of entries from a quartz database table.
 *
 *  It may be used to store a set of proposed modificiations to the table,
 *  or to cache entries read from the table.
 */
class QuartzTableEntries {
    private:
	/// Copying not allowed
	QuartzTableEntries(const QuartzTableEntries &);

	/// Assignment not allowed
	void operator=(const QuartzTableEntries &);

	/// The entries stored in this object
	std::map<QuartzDbKey, QuartzDbTag *> entries;

    public:

	/** Initialise the cache of entries.
	 */
	QuartzTableEntries();

	/** Delete the entries.
	 */
	~QuartzTableEntries();

	/** Get a pointer to the tag for a given key.
	 *
	 *  This must not be called for a key which is not stored in the
	 *  object - have_entry() should be used first to check whether
	 *  a tag is stored or not.
	 *
	 *  @param key  The key that the tag is stored under.
	 *
	 *  @return A pointer to the tag.  The tag pointed to is still
	 *          owned by the QuartzTableEntries object - it may be modifed
	 *          if desired, but the user should not try to free the
	 *          pointer.  A null pointer will be returned if the tag
	 *          is marked for deletion.
	 */
	//@{
	QuartzDbTag * get_tag(const QuartzDbKey &key);
	const QuartzDbTag * get_tag(const QuartzDbKey &key) const;
	//@}

	/** Return whether the given entry is stored in this object.
	 *
	 *  @return true if the entry is stored in this object (even if it
	 *          is stored as being marked for deletion), false if it is
	 *          not stored.
	 */
	bool have_entry(const QuartzDbKey &key) const;

	/** Return whether there are any entries stored in this object.
	 *
	 *  @return true if the object stores no entries, false otherwise.
	 */
	bool empty() const;

	/** Set the tag associated with a given key.
	 *  If a tag is already associated with the key, it is freed and
	 *  replaced.  Any pointers to the old tag previously returned by
	 *  get_tag will become invalid.
	 *
	 *  To mark an entry for deletion, the tag pointer should be null.
	 *  This does not remove the key/tag from the object; to do this
	 *  use remove_tag().
	 *
	 *  @param key   The key to store the block under.
	 *
	 *  @param block The block to store.
	 */
	void set_tag(const QuartzDbKey &key, AutoPtr<QuartzDbTag> tag);

	/** Removes all the entries from the QuartzTableEntries object.
	 *
	 *  This simply removes the entries from the object - it does
	 *  not correspond to deleting them from the underlying table.
	 */
	void clear();

	/** Get all the entries in this object.
	 *
	 *  This returns a suitable object to be passed to
	 *  QuartzDiskTable::set_entries()
	 */
	std::map<QuartzDbKey, QuartzDbTag *> & get_all_entries();
};

#endif /* OM_HGUARD_QUARTZ_TABLE_ENTRIES_H */
