/* omdatabaseinternal.h: Class definition for OmDatabase::Internal
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

#ifndef OM_HGUARD_OMDATABASEINTERNAL_H
#define OM_HGUARD_OMDATABASEINTERNAL_H

#include <vector>

#include <om/omenquire.h>
#include "omlocks.h"
#include "refcnt.h"
#include "database_builder.h"
#include "document.h"
#include "database.h"
#include "om/omdatabase.h"

/////////////////////////////
// Internals of OmDatabase //
/////////////////////////////

/** Reference counted internals for OmDatabase.
 */
class OmDatabase::Internal {
    private:
	/// Add a database, based on parameters.
	void add_database(const OmSettings &params, bool readonly);
    
    public:
	/// databases which this consists of.
	std::vector<RefCntPtr<Database> > databases;

	/** A lock to control concurrent access to this object.
	 *  This is not intended to control access to the Database objects.
	 */
	OmLock mutex;

	/// average document length - 0 means "not yet calculated"
	mutable om_doclength avlength;

	/** Make a new internal object, with the user supplied parameters.
	 *
	 *  This opens the database and stores it in the ref count pointer.
	 *
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @param readonly flag as to whether to open database read only
	 */
	Internal(const OmSettings &params, bool readonly);

	/** Make a copy of this object, copying the ref count pointer.
	 */
        Internal(const Internal &other)	: databases(other.databases), mutex(),
		avlength(other.avlength)
	{ }

	Internal() : avlength(0) { }

	/// Add a database, based on parameters.
	void add_database(const OmSettings &params);

	// Add an already opened database
	void add_database(RefCntPtr<Database> newdb);

	/// get average document length
	om_doclength get_avlength() const;

	LeafPostList *open_post_list(const om_termname & tname,
				     const OmDatabase &db) const;

	LeafTermList *open_term_list(om_docid did, const OmDatabase &db) const;

	LeafDocument *open_document(om_docid did) const;
};

#endif // OM_HGUARD_OMDATABASEINTERNAL_H
