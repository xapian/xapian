/* sleepy_database.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_POSTLIST_H
#define OM_HGUARD_SLEEPY_POSTLIST_H

#include "leafpostlist.h"
#include <stdlib.h>

class SleepyDatabase;
class SleepyDatabaseInternals;
#include "sleepy_list.h"

/** A poslist in a sleepycat database.
 */
class SleepyPostList : public LeafPostList {
    friend class SleepyDatabase;
    private:
        /** The termname for this postlist: this is used for
	 *  introspection methods.
	 */
	om_termname tname;

	/** List object which deals with the low-level list accessing
	 *  and unpacking.
	 */
	SleepyList mylist;

	/** Create a SleepyPostList from the specified internals, and
	 *  using the specified termid.
	 *
	 *  @param tid_        The termid to use to open the postlist.
	 *  @param internals_  The database internals to use.
	 *  @param tname_      The termname for this postlist: this is
	 *                     used for introspection methods.
	 */
	SleepyPostList(om_termid tid_,
		       SleepyDatabaseInternals * internals_,
		       const om_termname & tname_);
    public:
	~SleepyPostList();

	om_doccount   get_termfreq() const;// Number of docs indexed by this term

	om_docid      get_docid() const;   // Current docid
	om_weight     get_weight() const;  // Current weight
        PostList * next(om_weight w_min);  // Move to next docid
        PostList * skip_to(om_docid did, om_weight w_min);  // Skip to next docid >= docid
	bool       at_end() const;      // True if we're off the end of the list

	string intro_term_description() const;
};

#endif /* OM_HGUARD_SLEEPY_POSTLIST_H */
