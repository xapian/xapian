/* omdatabase.h
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

#ifndef OM_HGUARD_OMDATABASE_H
#define OM_HGUARD_OMDATABASE_H

#include <vector>
#include "omindexdoc.h"

///////////////////////////////////////////////////////////////////
// OmDatabaseGroup class
// =====================

/** This class encapsulates a set of databases.
 *
 *  This class is used in conjunction with an OmEnquire object.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError may be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class OmDatabaseGroup {
    private:
	class Internal;
	Internal *internal;

	/** InternalInterface is a class used externally to interact
	 *  with OmDatabaseGroup objects.
	 */
	class InternalInterface;
	friend class InternalInterface;

    public:
	OmDatabaseGroup();
	~OmDatabaseGroup();
	OmDatabaseGroup(const OmDatabaseGroup &other);
	void operator=(const OmDatabaseGroup &other);

	/** Add a new database to use.
	 *
	 *  The database may not be opened by this call: the system may wait
	 *  until a get_mset (or similar call).
	 *  Thus, failure to open the database may not result in an
	 *  OmOpeningError exception being thrown until the database is used.
	 *
	 *  The database will always be opened read-only.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *  database: meaning and number required depends on database type.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void add_database(const string & type,
			  const vector<string> & params);
};

///////////////////////////////////////////////////////////////////
// OmWritableDatabase class
// ========================

/** This class provides writable access to a database.
 *
 *  NOTE: this class is still under heavy development, and the interface
 *  is liable to change in the near future.
 */
class OmWritableDatabase {
    private:
	class Internal;

	/** Reference counted internals. */
	Internal *internal;
    public:
	/** Open a database for writing.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if an invalid
	 *             argument is supplied, for example, an unknown database
	 *             type.
	 *  @exception OmOpeningError may be thrown if the database cannot
	 *             be opened.
	 */
	OmWritableDatabase(const string & type,
			   const vector<string> & params);

	/** Destroy this handle on the database.
	 *  If there are no copies of this object remaining, the database
	 *  will be closed.
	 *
	 *  Calling this method will ensure that all changes made are
	 *  flushed to the database.
	 */
	~OmWritableDatabase();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmWritableDatabase(const OmWritableDatabase &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const OmWritableDatabase &other);

	/** Add a new document to the database.
	 *
	 *  This method atomically adds the document: the result is either
	 *  that the document is added, or that the document fails to be
	 *  added and an exception is thrown.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @return         The document ID of the newly added document.
	 *
	 *  @exception OmDatabaseError will be thrown if a problem occurs
	 *             while writing to the database.
	 */
	om_docid add_document(const OmDocumentContents & document);
};

#endif /* OM_HGUARD_OMDATABASE_H */

