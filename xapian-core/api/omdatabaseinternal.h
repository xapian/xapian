/* omdatabaseinternal.h: Class definition for OmDatabase::Internal
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <xapian/enquire.h>
#include "refcnt.h"
#include "document.h"
#include "database.h"
#include "om/omdatabase.h"

#include <vector>

class AllTermsList;

/////////////////////////////
// Internals of OmDatabase //
/////////////////////////////

/** Reference counted internals for OmDatabase.
 */
class OmDatabase::Internal {
    public:
	/// databases which this consists of.
	std::vector<RefCntPtr<Database> > databases;

	/** Wrap the given internal database object, in another class which
	 *  holds it as a refcount pointer.
	 *
	 *  @param db pointer to Database object
	 */
	Internal(Database *db);

	/** Make a copy of this object, copying the ref count pointer.
	 */
        Internal(const Internal &other)	: databases(other.databases)
	{ }

	Internal() { }

	/// Add a database, based on parameters.
	void add_database(Database *db);

	// Add an already opened database
	void add_database(RefCntPtr<Database> newdb);

	/// get average document length
	om_doclength get_avlength() const;

	LeafPostList * open_post_list(const string & tname,
				      const OmDatabase &db) const;

	LeafTermList * open_term_list(om_docid did, const OmDatabase &db) const;

	TermList * open_allterms() const;

	Document * open_document(om_docid did) const;

	PositionList * open_position_list(om_docid did,
					 const string &tname) const;
};

#endif // OM_HGUARD_OMDATABASEINTERNAL_H
