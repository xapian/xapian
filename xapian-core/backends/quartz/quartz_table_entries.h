/* quartz_table_entries.h: Storage of a set of table entries from a quartz db
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

#ifndef OM_HGUARD_QUARTZ_TABLE_ENTRIES_H
#define OM_HGUARD_QUARTZ_TABLE_ENTRIES_H

#include "autoptr.h"
#include "quartz_types.h"

#include <map>
#include <string>
using std::map;
using std::string;

/** This class stores a set of entries from a quartz database table.
 *
 *  It may be used to store a set of proposed modifications to the table,
 *  or to cache entries read from the table.
 */
class QuartzTableEntries {
    public:
	typedef map<string, string *> items;
    private:
	/// Copying not allowed
	QuartzTableEntries(const QuartzTableEntries &);

	/// Assignment not allowed
	void operator=(const QuartzTableEntries &);

	/// The entries stored in this object
	items entries;

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
	string * get_tag(const string &key);
	const string * get_tag(const string &key) const;
	//@}

	/** Return whether the given entry is stored in this object.
	 *
	 *  @return true if the entry is stored in this object (even if it
	 *          is stored as being marked for deletion), false if it is
	 *          not stored.
	 */
	bool have_entry(const string &key) const;

	/** Get an iterator to the item specified if it exists.
	 *  If it doesn't exist, the iterator points to the first item
	 *  which does exist which is before that specified.
	 *  If there are no items before that specified, it points to the
	 *  first item, which has a null key.
	 *
	 *  @param key  The key to look for an item at or before.
	 *
	 *  @return  The iterator.
	 */
	items::const_iterator get_iterator(const string & key) const;

	/** Get the item pointed to by the iterator.
	 *
	 *  The iterator must point to an item when this is called.
	 *
	 *  @param iterator  The iterator.
	 *  @param keyptr    A pointer to a pointer to a key, which is set
	 *                   to point to the item's key.  The key pointed
	 *                   to is owned by the QuartzTableEntries object,
	 *                   and must not be deleted.
	 *  @param tagptr    A pointer to a pointer to a tag, which is set
	 *                   to point to the item's tag.  The tag pointed
	 *                   to is owned by the QuartzTableEntries object,
	 *                   and must not be deleted.
	 */
	void get_item(items::const_iterator iter,
		      const string ** keyptr,
		      const string ** tagptr) const;

	/** Decrease the iterator.
	 */
	void prev(items::const_iterator & iter) const;

	/** Advance the iterator.
	 */
	void next(items::const_iterator & iter) const;

	/** Check whether the iterator is at the end.
	 *
	 *  @return true if iterator is pointing to the last item,
	 *          false otherwise.
	 */
	bool after_end(items::const_iterator & iter) const;

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
	void set_tag(const string &key, AutoPtr<string> tag);

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
	items & get_all_entries();
};

#endif /* OM_HGUARD_QUARTZ_TABLE_ENTRIES_H */
