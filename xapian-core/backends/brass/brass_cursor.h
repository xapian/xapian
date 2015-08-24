/* brass_cursor.h: Interface to Btree cursors
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007,2008,2009,2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_BRASS_CURSOR_H
#define OM_HGUARD_BRASS_CURSOR_H

#include <xapian/visibility.h>

#include "brass_types.h"

#include <string>
using std::string;

#define BLK_UNUSED uint4(-1)

namespace Brass {

class Cursor {
    private:
        // Prevent copying
        Cursor(const Cursor &);
        Cursor & operator=(const Cursor &);

    public:
	/// Constructor, to initialise important elements.
	Cursor() : p(0), c(-1), n(BLK_UNUSED), rewrite(false)
	{}

	/// pointer to a block
	byte * p;
	/// offset in the block's directory
	int c;
	/** block number
	 *
	 * n is kept in tandem with p.  The unassigned state is when
	 * p == 0 and n == BLK_UNUSED.
	 * 
	 * Setting n to BLK_UNUSED is necessary in at least some cases.
	 */

	uint4 n;
	/// true if the block is not the same as on disk, and so needs rewriting
	bool rewrite;
};

}

class BrassTable;

/** A cursor pointing to a position in a Btree table, for reading several
 *  entries in order, or finding approximate matches.
 */
class XAPIAN_VISIBILITY_DEFAULT BrassCursor {
    private:
	/// Copying not allowed
        BrassCursor(const BrassCursor &);

	/// Assignment not allowed
        BrassCursor & operator=(const BrassCursor &);

	/** Rebuild the cursor.
	 *
	 *  Called when the table has changed.
	 */
	void rebuild();

    protected:
	/** Whether the cursor is positioned at a valid entry.
	 *
	 *  false initially, and after the cursor has dropped
	 *  off either end of the list of items
	 */
	bool is_positioned;

	/** Whether the cursor is off the end of the table.
	 */
	bool is_after_end;

    private:
	/** Status of the current_tag member. */
	enum { UNREAD, UNCOMPRESSED, COMPRESSED } tag_status;

    protected:
	/// The Btree table
	const BrassTable * B;

    private:
	/// Pointer to an array of Cursors
	Brass::Cursor * C;

	unsigned long version;

	/** The value of level in the Btree structure. */
	int level;

	/** Get the key.
	 *
	 *  The key of the item at the cursor is copied into key.
	 *
	 *  The cursor must be positioned before calling this method.
	 *
	 *  e.g.
	 *
	 *    BrassCursor BC(&btree);
	 *    string key;
	 *
	 *    // Now do something to each key in the Btree
	 *    BC.find_entry(string()); // must give result true
	 *
	 *    while (BC.next()) {
	 *        BC.get_key(&key);
	 *        do_something_to(key);
	 *    }
	 */
	void get_key(string * key) const;

    public:
	/** Create a cursor attached to a Btree.
	 *
	 *  Creates a cursor, which can be used to remember a position inside
	 *  the B-tree. The position is simply the item (key and tag) to which
	 *  the cursor points. A cursor is either positioned or unpositioned,
	 *  and is initially unpositioned.
	 *
	 *  NB: You must not try to use a BrassCursor after the Btree it is
	 *  attached to is destroyed.  It's safe to destroy the BrassCursor
	 *  after the Btree though, you just may not use the BrassCursor.
	 */
	BrassCursor(const BrassTable *B);

	/** Destroy the BrassCursor */
	~BrassCursor();

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
	 *
	 *  @param keep_compressed  Don't uncompress the tag - e.g. useful
	 *			    if it's just being opaquely copied
	 *			    (default: false).
	 *
	 *  @return	true if current_tag holds compressed data (always
	 *		false if keep_compressed was false).
	 */
	bool read_tag(bool keep_compressed = false);

	/** Advance to the next key.
	 *
	 *  If cursor is unpositioned, the result is simply false.
	 *
	 *  If cursor is positioned, and points to the very last item in the
	 *  Btree the cursor is made unpositioned, and the result is false.
	 *  Otherwise the cursor is moved to the next item in the B-tree,
	 *  and the result is true.
	 *  
	 *  Effectively, BrassCursor::next() loses the position of BC when it
	 *  drops off the end of the list of items. If this is awkward, one can
	 *  always arrange for a key to be present which has a rightmost
	 *  position in a set of keys,
	 */
	bool next();
 
	/** Move to the previous key.
	 *
	 *  This is like BrassCursor::next, but BC is taken to the previous
	 *  rather than next item.
	 */
	bool prev();

	/** Position the cursor on the highest entry with key <= @a key.
	 *
	 *  If the exact key is found in the table, the cursor will be
	 *  set to point to it, and the method will return true.
	 *
	 *  If the key is not found, the cursor will be set to point to
	 *  the key preceding that asked for, and the method will return
	 *  false.  If there is no key preceding that asked for, the cursor
	 *  will point to a null key.
	 *
	 *  Note:  Since the B-tree always contains a null key, which precedes
	 *  everything, a call to BrassCursor::find_entry always results in a
	 *  valid key being pointed to by the cursor.
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

	/// Position the cursor on the highest entry with key < @a key.
	void find_entry_lt(const string &key) {
	    if (find_entry(key)) prev();
	}

	/** Position the cursor on the lowest entry with key >= @a key.
	 *
	 *  @return true if the exact key was found in the table, false
	 *          otherwise.
	 */
	bool find_entry_ge(const string &key);

	/** Set the cursor to be off the end of the table.
	 */
	void to_end() { is_after_end = true; }

	/** Determine whether cursor is off the end of table.
	 *
	 *  @return true if the cursor has been moved off the end of the
	 *          table, past the last entry in it, and false otherwise.
	 */
	bool after_end() const { return is_after_end; }

	/// Return a pointer to the BrassTable we're a cursor for.
	const BrassTable * get_table() const { return B; }
};

class MutableBrassCursor : public BrassCursor {
  public:
    /** Create a mutable cursor attached to a Btree.
     *
     *  A mutable cursor allows the items to be deleted.
     *
     *  Creates a cursor, which can be used to remember a position inside
     *  the B-tree. The position is simply the item (key and tag) to which
     *  the cursor points. A cursor is either positioned or unpositioned,
     *  and is initially unpositioned.
     *
     *  NB: You must not try to use a MutableBrassCursor after the Btree it is
     *  attached to is destroyed.  It's safe to destroy the MutableBrassCursor
     *  after the Btree though, you just may not use the MutableBrassCursor.
     */
    MutableBrassCursor(BrassTable *B_) : BrassCursor(B_) { }

    /** Delete the current key/tag pair, leaving the cursor on the next
     *  entry.
     *
     *  @return false if the cursor ends up unpositioned.
     */
    bool del();
};

#endif /* OM_HGUARD_BRASS_CURSOR_H */
