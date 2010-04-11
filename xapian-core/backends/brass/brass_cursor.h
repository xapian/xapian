/** @file brass_cursor.h
 * @brief Brass B-tree cursor.
 */
/* Copyright (C) 2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_BRASS_CURSOR_H
#define XAPIAN_INCLUDED_BRASS_CURSOR_H

#include "brass_table.h"

#include <string>

/** A "movable finger" into a BrassTable.
 *
 *  BrassTable allows looking up a single entry by its exact key.
 *  BrassCursor allows you to go forwards and backwards through entries in
 *  key order, and to find the first entry before or after a specified key.
 */
class BrassCursor : public BrassCBlock {
    /// Copying is not allowed.
    BrassCursor(const BrassCursor &);

    /// Assignment is not allowed.
    void operator=(const BrassCursor &);

    void set_current_key() {
	if (data && item >= (is_leaf() ? 0 : -1))
	    read_key(current_key);
    }

    bool find(const std::string & key, int mode) {
	bool result = BrassCBlock::find(key, mode);
	set_current_key();
	return result;
    }

  public:
    std::string current_key;

    std::string current_tag;

    BrassCursor(const BrassTable & table_) : BrassCBlock(table_) {
	brass_block_t root = table.get_root();
	if (root != brass_block_t(-1))
	    read(root);
    }

    bool after_end() const { return item == -2; }

    void to_end() { item = -2; }

    bool next() {
	bool result = BrassCBlock::next();
	if (result)
	    set_current_key();
	return result;
    }

    bool prev() {
	bool result = BrassCBlock::prev();
	if (result)
	    set_current_key();
	return result;
    }

    bool find_entry_le(const std::string & key) { return find(key, LE); }

    void find_entry_lt(const std::string & key) { (void)find(key, LT); }

    bool find_entry_ge(const std::string & key) { return find(key, GE); }

    bool read_tag(bool keep_compressed = false) {
	(void)keep_compressed; // FIXME handle this
	(void)BrassCBlock::read_tag(current_tag);
	return false; // FIXME return true if it was compressed
    }

    const BrassTable * get_table() const { return &table; }
};

class MutableBrassCursor : public BrassCursor {
  public:
    /** Create a mutable cursor over a BrassTable.
     *
     *  A standard BrassCursor is read-only.  MutableBrassCursor adds the
     *  ability to delete the entry currently pointed to.
     *
     *  @param table_ The BrassTable to operate on.  The caller has
     *		      resposibility for ensuring table_ remains valid while the
     *		      constructed BrassMutableCursor is still in use (but the
     *		      BrassMutableCursor can be safely destroyed without table_
     *		      being valid).
     */
    MutableBrassCursor(BrassTable & table_) : BrassCursor(table_) { }

    /** Delete the currently pointed to entry.
     *
     *  The cursor's position is left on the entry after the current one.
     *
     *  @return false if the cursor ends up unpositioned.
     */
    bool del() {
	Assert(!after_end());

	// MutableBrassCursor is only constructable from a non-const
	// BrassTable* but we store that in the const BrassTable* "table"
	// member of the BrassCursor class to avoid duplicating storage.  So we
	// know it is safe to cast away that const again here.
	(const_cast<BrassTable&>(table)).del(current_key);

	// If we're iterating an older revision of the tree, then the deletion
	// happens in a new (uncommitted) revision and the cursor still sees
	// the deleted key.  But if we're iterating the new uncommitted
	// revision then the deleted key is no longer visible.  We need to
	// handle both cases - either find_entry_ge() finds the deleted key or
	// not.
	if (!find_entry_ge(current_key))
	    return !after_end();
	return next();
    }
};

#endif // XAPIAN_INCLUDED_BRASS_CURSOR_H
