/* sleepy_termlist.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_TERMLIST_H
#define OM_HGUARD_SLEEPY_TERMLIST_H

#include "termlist.h"
#include "om/omtypes.h"

class SleepyDatabase;
class SleepyDatabaseInternals;
class SleepyDatabaseTermCache;
#include "sleepy_list.h"

/** A termlist in a sleepycat database.
 */
class SleepyTermList : public LeafTermList {
    friend class SleepyDatabase;
    private:
	/** List object which deals with the low-level list accessing
	 *  and unpacking.
	 */
	SleepyList mylist;

	/** Database internals object.
	 */
	const SleepyDatabase * database;

	/** Object to do name to ID mapping.
	 */
	const SleepyDatabaseTermCache *termcache;

	/** Number of documents in the database.
	 */
	om_doccount db_size;

	/** (Unnormalised) Length of this document.
	 */
	om_doclength doc_len;

	/** Create a SleepyPostList from the specified internals and
	 *  term cache, using the specified document ID.
	 *
	 *  @param did_        The document ID whose posting list we open.
	 *  @param database_   The database to use.
	 *  @param internals_  The database internals to use.
	 *  @param termcache_  The term name to ID translator.
	 */
	SleepyTermList(om_docid did_,
		       const SleepyDatabase * database_,
		       const SleepyDatabaseInternals * internals_,
		       const SleepyDatabaseTermCache *termcache_);
    public:
	~SleepyTermList();

	om_termcount get_approx_size() const;

	OmExpandBits      get_weighting() const;  // Gets weight of current term
	const om_termname get_termname() const;  // Current term
	om_termcount      get_wdf() const;  // Occurences of current term in doc
	om_doccount       get_termfreq() const;  // Docs indexed by current term
	TermList *        next();
	bool              at_end() const;

	om_doclength      get_doclength() const; // Get length of document.
};

#endif /* OM_HGUARD_SLEEPY_TERMLIST_H */
