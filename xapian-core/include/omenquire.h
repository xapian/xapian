/* omenquire.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _omenquire_h_
#define _omenquire_h_

// Include these for now, but later they will be hidden.
#include "match.h"
#include "expand.h"
#include "stemmer.h"
#include "rset.h"

// Internal state
class EnquireState;

// This class provides an interface to the information retrieval
// system for the purpose of searching.

class Enquire {
    private:
	EnquireState *state;
    public:
        Enquire();
        ~Enquire();

	// Set the database to use.
	//
	// First parameter is a string describing the database:
	// the start of the string is a case sensitive name followed by a :,
	// specifying the type of the database.  The subsequent string is a
	// set of / separated entries, whose meaning is dependent on the type
	// of the database.
	//
	// Second parameter is a flag: if set, the database will be opened
	// read-only.
	void set_database(string, bool = true);
};

#endif /* _omenquire_h_ */
