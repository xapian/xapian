/* postlist.h
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

#ifndef _postlist_h_
#define _postlist_h_

#include "omassert.h"

#include "omtypes.h"
#include "omerror.h"

class PostList
{
    private:
    public:
        virtual ~PostList() { return; }

	///////////////////////////////////////////////////////////////////
	// Information about the postlist
	//
	// These may be called at any point

	// (Upper bound on) Number of docs indexed by term
	virtual doccount get_termfreq() const = 0;
	// Gets max weight
        virtual weight get_maxweight() const = 0;

	///////////////////////////////////////////////////////////////////
	// Information about the current item
	//
	// These may only be called after a next() or a skip_to(),
	// and before at_end() returns true (or would do were it to be called)

	virtual docid  get_docid() const = 0;
	virtual weight get_weight() const = 0;

	// recalculate weights (used when tree has been autopruned)
        virtual weight recalc_maxweight() = 0;


	///////////////////////////////////////////////////////////////////
	// Movement around the postlist
	//
	// w_min in next() and skip_to() is simply a hint -
	// documents with a weight less than w_min will be ignored.
	// However, it may be best to return them anyway, if the weight
	// calculation is expensive, since many documents will be thrown
	// away anyway without calculating the weight.

	// Move to next docid
	virtual PostList *next(weight w_min) = 0;

	// Moves to next docid >= specified docid
	virtual PostList *skip_to(docid, weight w_min) = 0;

	// Returns true if we're off the end of the list
	virtual bool   at_end() const = 0;

	///////////////////////////////////////////////////////////////////
	// Introspection methods
	//
	// These are mainly useful for debugging.

	// Returns a description of the term or terms from which the postlist
	// derives.
	virtual string intro_term_description() const = 0;
};

#endif /* _postlist_h_ */
