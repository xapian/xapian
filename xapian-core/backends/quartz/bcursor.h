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
	/** Destroy a Bcursor */
	~Bcursor();

	bool prev();
	bool next();
	bool find_key(const string &key);
	bool get_key(string * key) const;
	bool get_tag(string * tag);

	/** Create a bcursor attached to a Btree. */
	Bcursor(Btree *B_);

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
