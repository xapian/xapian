/* omindexer.h: Indexing API for Muscat
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

#ifndef OM_HGUARD_OMINDEXER_H
#define OM_HGUARD_OMINDEXER_H

#include "omindexdoc.h"

///////////////////////////////////////////////////////////////////
// OmWritableDatabase class
// ========================

/** This class provides writable access to a database.
 *
 *  NOTE: this class is still under heavy development, and the interface
 *  is liable to change in the near future.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError may be thrown if the database cannot
 *  be opened.
 */
class OmWritableDatabase {
    private:
	class Internal;
	Internal *internal;
    public:
	/** Open a database for writing.
	 *
	 *  Note that the database may not actually be opened at the time
	 *  that this method exits, and thus, failure to open the database
	 *  may not result in an OmOpeningError exception being thrown
	 *  until the database is used.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	OmWritableDatabase(const string & type, const vector<string> &
			   params);

	/** Close database.
	 */
	~OmWritableDatabase();

	/** Copy database.
	 */
	OmWritableDatabase(const OmWritableDatabase &other);

	/** Assign to an existing database.
	 */
	void operator=(const OmWritableDatabase &other);

	/** Add a new document to the database.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @return         The document ID of the newly added document.
	 */
	om_docid add_document(const OmDocumentContents & document);
};

#endif /* OM_HGUARD_OMINDEXER_H */
