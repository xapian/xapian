/* multi_database.h: C++ class definition for multiple database access
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

#ifndef OM_HGUARD_MULTI_DATABASE_H
#define OM_HGUARD_MULTI_DATABASE_H

#include "omassert.h"
#include "database.h"
#include "leafpostlist.h"
#include <om/omenquire.h>
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

/** A database which contains other databases.
 */
class MultiDatabase : public IRDatabase {
    /** DatabaseBuilder is a friend of this class, so that it can call
     *  the constructor.
     */
    friend class DatabaseBuilder;

    /** OmDatabaseGroup::Internal is a friend of this class,
     *  so that it can call the constructor, passing in IRDatabase pointers.
     */
    friend class OmDatabaseGroup::Internal;

    /** MultiMatch is a friend of this class so that it can access
     *  `databases'.  FIXME: this isn't very clean.
     */
    friend class MultiMatch;

    private:
	mutable set<om_termname> terms;

	// List of subdatabases
	vector<OmRefCntPtr<IRDatabase> > databases;

	mutable bool length_initialised;
	mutable om_doclength avlength;

	/** Create and open a multi database from database parameters.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @exception OmInvalidArgumentError if invalid arguments are
	 *             supplied.
	 *  
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.  The meanings
	 *                of these parameters are dependent on the database
	 *                type.
	 *                Note that if params.readonly is set, all
	 *                sub-databases will be opened readonly.
	 */
	MultiDatabase(const DatabaseBuilderParams & params);

	/** Create a multi database from a set of existing databases.
	 *
	 *  @param databases_  A vector containing the existing databases.
	 *
	 *  @exception OmInvalidArgumentError if no databases are specified
	 *             in the vector.
	 */
	MultiDatabase(vector<OmRefCntPtr<IRDatabase> > databases_);
	
    public:
	~MultiDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * open_post_list(const om_termname & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;

	/** MultiDatabase is a readonly database type, and thus this method is
	 *  not supported: if called an exception will be thrown.
	 */
	om_docid add_document(const struct OmDocumentContents & document) {
	    throw OmUnimplementedError("MultiDatabase::add_document() not implemented");
	}
};

#endif /* OM_HGUARD_MULTI_DATABASE_H */
