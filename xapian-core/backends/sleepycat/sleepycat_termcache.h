/* sleepy_termcache.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_TERMCACHE_H
#define OM_HGUARD_SLEEPY_TERMCACHE_H

class SleepyDatabaseInternals;

/** Termname to termID mappings.
 *
 *  This class maintains a cache of termname to termID mappings, looking
 *  up in the database only when required.
 *
 *  Entries are not deleted from the cache until the object is destroyed.
 */
class SleepyDatabaseTermCache {
    private:
        /** Pointer to the database internals.  These are owned by
	 *  someone else (typically a SleepyDatabase), so we don't have to
	 *  worry about deleting them.
	 */
	SleepyDatabaseInternals * internals;

	/// Copying is not allowed.
	SleepyDatabaseTermCache(const SleepyDatabaseTermCache &);

	/// Assignment is not allowed.
	void operator=(const SleepyDatabaseTermCache &);
    public:
	/** Create a term cache, for the database refered to by the
	 *  supplied internals.
	 *
	 *  @param internals_  A pointer to the internals to use.  This is
	 *                     owned by the caller (so SleepyDatabaseTermCache
	 *                     won't delete it).
	 */
	SleepyDatabaseTermCache(SleepyDatabaseInternals *internals_)
		: internals(internals_) {}

	/** Convert a term ID into a termname.
	 *  This is expected to be called with known term ID's, since it
	 *  will throw an exception if the term ID is not found.
	 *
	 *  @param tid  The term ID to lookup.
	 *
	 *  @return     The termname string corresponding to term ID tid.
	 *
	 *  @exception OmRangeError is thrown if the term id supplied is
	 *                          not found in the database.
	 *
	 *  @exception OmDatabaseError is thrown if a error occurs when
	 *                             accessing the database.
	 */
	om_termname term_id_to_name(om_termid tid) const;

	/** Convert a termname into a term ID.
	 *
	 *  @param tname  The termname to lookup in the database.
	 *
	 *  @return The term ID which corresponds to the specified termname.
	 *          If the termname is not found in the database, the special
	 *          value of 0 is returned.
	 *
	 *  @exception OmDatabaseError is thrown if a error occurs when
	 *                             accessing the database.
	 */
	om_termid term_name_to_id(const om_termname & tname) const;

	/** Make a new term ID, and store it in the database.
	 *
	 *  If the term already exists, the existing term ID is returned,
	 *  and nothing is changed.
	 * 
	 *  @param tname   The termname to make an ID for.
	 *
	 *  @return A term ID which now corresponds to the specified termname.
	 *
	 *  @exception OmDatabaseError is thrown if a error occurs when
	 *                             accessing the database.
	 */
	om_termid assign_new_termid(const om_termname & tname) const;
};

#endif /* OM_HGUARD_SLEEPY_TERMCACHE_H */
