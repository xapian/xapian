/* sleepy_document.h: A document in a sleepycat database
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

#ifndef OM_HGUARD_SLEEPY_DOCUMENT_H
#define OM_HGUARD_SLEEPY_DOCUMENT_H

#include "document.h"
#include <db_cxx.h>
#include <map>

/** A document in a sleepycat Database.
 */
class SleepyDocument : public LeafDocument {
    friend class SleepyDatabase;
    private:
	/** The database which the document is held in.
	 */
	Db *db;

	/** The document ID of the document.
	 */
	om_docid did;

	/** Constructor: called by SleepyDatabase to read a document
	 *
	 *  @param db_ The database to look for the document in.
	 *  @param did_ The document ID to look for.
	 *
	 *  @exception OmDatabaseError is thrown if the document can't be
	 *             accessed.
	 */
	SleepyDocument(Db * db_, om_docid did_);

	/** Constructor: called by SleepyDatabase to create a new document
	 *
	 *  @param db_   The database to store the document in.
	 *  @param data_ The data to store in the document.
	 *  @param keys_ The keys to store in the document.
	 *
	 *  @exception OmDatabaseError is thrown if the document can't be
	 *             accessed.
	 */
	SleepyDocument(Db * db_,
		       const OmData & data_,
		       const map<om_keyno, OmKey> & keys_ );

	/// Copying is not permitted
	SleepyDocument(const SleepyDocument &);
	/// Assignment is not permitted
	SleepyDocument & operator = (const SleepyDocument &);
    public:
	/** Return the document ID of this document.
	 */
	om_docid get_docid() const;

	// Virtual methods of Leaf Document
	OmKey do_get_key(om_keyno keyid) const;
	OmData do_get_data() const;
};

#endif /* OM_HGUARD_SLEEPY_DOCUMENT_H */
