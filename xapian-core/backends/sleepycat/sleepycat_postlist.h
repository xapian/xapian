/* sleepycat_postlist.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPYCAT_POSTLIST_H
#define OM_HGUARD_SLEEPYCAT_POSTLIST_H

#include "config.h"
#ifdef MUS_BUILD_BACKEND_SLEEPYCAT

#include "leafpostlist.h"
#include <stdlib.h>

#include "sleepycat_database.h"
class SleepycatDatabaseInternals;
#include "sleepycat_list.h"
#include "inmemory_positionlist.h"

/** A postlist in a sleepycat database.
 */
class SleepycatPostList : public LeafPostList {
    friend class SleepycatDatabase;
    private:
        /** The termname for this postlist: this is used for
	 *  introspection methods.
	 */
	om_termname tname;

	/** List object which deals with the low-level list accessing
	 *  and unpacking.
	 */
	SleepycatList mylist;

	/** List of positions of the current term.
	 *  This list is populated when get_position_list() is called.
	 */
	InMemoryPositionList mypositions;

	RefCntPtr<const SleepycatDatabase> this_db;

	/** Create a SleepycatPostList from the specified internals, and
	 *  using the specified termid.
	 *
	 *  @param tid_        The termid to use to open the postlist.
	 *  @param internals_  The database internals to use.
	 *  @param tname_      The termname for this postlist: this is
	 *                     used for introspection methods.
	 */
	SleepycatPostList(om_termid tid_,
			  SleepycatDatabaseInternals * internals_,
			  const om_termname & tname_,
			  RefCntPtr<const SleepycatDatabase> this_db_);
    public:
	~SleepycatPostList();

	om_doccount   get_termfreq() const;// Number of docs indexed by this term

	om_docid     get_docid() const;     // Current docid
	om_weight    get_weight() const;    // Current weight
	om_doclength get_doclength() const; // Length of current document
        om_termcount get_wdf() const;	    // Within Document Frequency
	PositionList *get_position_list(); // Get positions
        PostList * next(om_weight w_min);  // Move to next docid
        PostList * skip_to(om_docid did, om_weight w_min);  // Skip to next docid >= docid
	bool       at_end() const;      // True if we're off the end of the list

	std::string intro_term_description() const;
};

#endif /* MUS_BUILD_BACKEND_SLEEPYCAT */

#endif /* OM_HGUARD_SLEEPYCAT_POSTLIST_H */
