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

	// virtual methods of IndexerDestination
	void make_term(const om_termname &);
	om_docid make_doc(const om_docname &);
	void make_posting(const om_termname &, unsigned int, unsigned int);
};

inline void
SleepyDatabase::make_term(const om_termname &) {
    throw OmUnimplementedError("DADatabase::make_term() not implemented");
}

inline om_docid 
SleepyDatabase::make_doc(const om_docname &) {
    throw OmUnimplementedError("DADatabase::make_doc() not implemented");
}

inline void 
SleepyDatabase::make_posting(const om_termname &, unsigned int, unsigned int) {
    throw OmUnimplementedError("DADatabase::make_posting() not implemented");
}

#endif /* OM_HGUARD_SLEEPY_DATABASE_H */
