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

#ifndef OM_HGUARD_SLEEPY_DATABASE_H
#define OM_HGUARD_SLEEPY_DATABASE_H

#include <stdlib.h>
#include "database.h"
#include "om/omerror.h"

class SleepyDatabaseTermCache;
class SleepyDatabaseInternals;

/** A database using the sleepycat database library.
 *  This currently uses the C++ interface, version 2.2.6, but may work
 *  with other versions.
 *
 *  Sleepycat is available from http://www.sleepycat.com/
 */
class SleepyDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    private:
	SleepyDatabaseInternals * internals;

	SleepyDatabaseTermCache * termcache;

	/** Create and open a sleepycat database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *  
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.  The meanings
	 *                of these parameters are dependent on the database
	 *                type.
	 */ 
	SleepyDatabase(const DatabaseBuilderParams & params);

	/** Make a new entry in a postlist.
	 *
	 *  This opens the specified postlist, and adds the information
	 *  specified.  If the document ID supplied is already in the
	 *  postlist, its entry is overwritten.
	 *
	 *  @param tid        The term ID of the postlist to be modified.
	 *  @param did        The document ID for the entry.
	 *  @param wdf        The WDF for the entry (number of occurrences
	 *                    in the given document).
	 *  @param positions  A list of positions at which the term occurs.
	 *                    This list must be strictly increasing (ie, no
	 *                    duplicates).
	 */
	void make_entry_in_postlist(om_termid tid,
				    om_docid did,
				    om_termcount wdf,
				    const vector<om_termpos> & positions);
    public:
	~SleepyDatabase();

	// Virtual methods of IRDatabase
	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * open_post_list(const om_termname& tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;

	// virtual method of IndexerDestination
	om_docid add_document(const struct OmDocumentContents & document);
};

#endif /* OM_HGUARD_SLEEPY_DATABASE_H */
