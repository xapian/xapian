/* quartz_table.h: A cursor in a Btree table.
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

/** A cursor pointing to a position in a quartz table, for reading several
 *  entries in order, or finding approximate matches.
 */
class QuartzCursor {
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

	/** Have we read the tag for the current key?
	 */
	bool have_read_tag;

	/** The btree cursor.  This points to the next item, not the current
	 *  item.
	 */
	Bcursor cursor;

    public:
	/// Initialise the cursor 
	QuartzCursor(Btree * btree)
	    : is_positioned(false),
	      is_after_end(false),
	      have_read_tag(false),
	      cursor(btree) {}

	/// Destroy the cursor
	~QuartzCursor() {}

	/** Current key pointed to by cursor.
	 */
	string current_key;

	/** Current tag pointed to by cursor.  You must call read_tag to
	 *  make this value available.
	 */
	string current_tag;

	/** Read the tag from the table and store it in current_tag.
	 *
	 *  Some cursor users don't need the tag, so for efficiency we
	 *  only read it when asked to.
	 */
	void read_tag();

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

#if 0 // Unused and untested in its current form...
	/** Move the cursor back in the table.
	 *
	 *  If there are no previous entries in the table, the cursor
	 *  will point to a null key.  Otherwise, this method moves the
	 *  cursor back one position.
	 */
	void prev();
#endif

	/** Determine whether cursor is off the end of table.
	 *
	 *  @return true if the cursor has been moved off the end of the
	 *          table, past the last entry in it, and false otherwise.
	 */
	bool after_end() { return is_after_end; }
};

#endif /* OM_HGUARD_QUARTZ_TABLE_H */
