/* bcursor.h: Interface to Btree cursors
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_BCURSOR_H
#define OM_HGUARD_BCURSOR_H

#include "btree_types.h"

#include <string>
using std::string;

class Btree;

/************ B-tree reading ************/

class Bcursor {
    private:
        // Prevent copying
        Bcursor(const Bcursor &);
        Bcursor & operator=(const Bcursor &);

    public:
	/** Create a bcursor attached to a Btree.
	 *
	 *  Creates a cursor, which can be used to remember a position inside
	 *  the B-tree. The position is simply the item (key and tag) to which
	 *  the cursor points. A cursor is either positioned or unpositioned,
	 *  and is initially unpositioned.
	 *
	 *  NB: You must not try to use a Bcursor after the Btree it is
	 *  attached to is destroyed.  It's safe to destroy the Bcursor
	 *  after the Btree though.
	 */
	Bcursor(Btree *B);

	/** Destroy the Bcursor */
	~Bcursor();

	/** Advance to the next key.
	 *
	 *  If cursor BC is unpositioned, the result is simply false.
	 *  
	 *  If cursor BC is positioned, and points to the very last item in the
	 *  Btree the cursor is made unpositioned, and the result is false.
	 *  Otherwise the cursor BC is moved to the next item in the B-tree,
	 *  and the result is true.
	 *  
	 *  Effectively, Bcursor::next() loses the position of BC when it drops
	 *  off the end of the list of items. If this is awkward, one can
	 *  always arrange for a key to be present which has a rightmost
	 *  position in a set of keys,
	 */
	bool next();
 
	/** Move to the previous key.
	 *
	 *  This is like Bcursor::next, but BC is taken to the previous rather
	 *  than next item.
	 */
	bool prev();

	/** Position the cursor at a key in the Btree.
	 * 
	 *  The result is true iff the specified key is found in the Btree.
	 *
	 *  If found, the cursor is made to point to the item with the given
	 *  key, and if not found, it is made to point to the last item in the
	 *  B-tree whose key is >= the key being searched for, The cursor is
	 *  then set as 'positioned'.  Since the B-tree always contains a null
	 *  key, which precedes everything, a call to Bcursor::find_key always
	 *  results in a valid key being pointed to by the cursor.
	 */
	bool find_key(const string &key);

	/** Get the key.
	 *
	 *  If cursor BC is unpositioned, the result is false.
	 *
	 *  If BC is positioned, the key of the item at cursor BC is copied
	 *  into key and the result is then true.
	 *
	 *  e.g.
	 *
	 *    Bcursor BC(&btree);
	 *    string key;
	 *
	 *    // Now do something to each key in the Btree
         *    BC.find_key(""); // must give result true
	 *
	 *    while (BC.next()) {
	 *        BC.get_key(&key)) {
	 *        do_something_to(key);
	 *    }
	 */
	bool get_key(string * key) const;

	/** Get the tag.
	 *
	 *  If cursor BC is unpositioned, the result is false.
	 *
	 *  If BC is positioned, the tag of the item at cursor BC is copied
	 *  into tag.  BC is then moved to the next item as if Bcursor::next()
	 *  had been called - this may leave BC unpositioned.  The result is
	 *  true iff BC is left positioned.
	 *
	 *  e.g.
	 *
	 *    Bcursor BC(&btree);
	 *    string key, tag;
	 *
	 *    // Now do something to each key-tag pair in the Btree
         *    BC.find_key(""); // must give result true
	 *
	 *    while (BC.get_key(&key)) {
	 *        BC.get_tag(&tag);
	 *        do_something_to(key, tag);
	 *    }
	 *
	 *  If BC is unpositioned by Bcursor::get_tag, Bcursor::get_key
	 *  gives result false the next time it called.
	 */
	bool get_tag(string * tag);

    private:
	/** false initially, and after the cursor has dropped
	 *  off either end of the list of items */
	bool positioned;
		       
	Btree * B;
	Cursor * C;

	/** The value of level in the Btree structure. */
	int level;
};

#endif /* OM_HGUARD_BCURSOR_H */
