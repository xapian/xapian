/* sleepycat_document.h: A document in a sleepycat database
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

#ifndef OM_HGUARD_SLEEPYCAT_DOCUMENT_H
#define OM_HGUARD_SLEEPYCAT_DOCUMENT_H

#include "config.h"
#ifdef MUS_BUILD_BACKEND_SLEEPYCAT

#include "document.h"
#include <db_cxx.h>
#include <map>
#include <om/omdocument.h>
#include <om/omindexdoc.h>

/** A document in a sleepycat Database.
 */
class SleepycatDocument : public LeafDocument {
    friend class SleepycatDatabase;
    private:
	/** The database in which the document data is held.
	 */
	Db *document_db;

	/** The database in which the document keys are held.
	 */
	Db *key_db;

	/** The document ID of the document.
	 */
	om_docid did;

	/** The data for this document, if it has been read from the database.
	 */
	mutable OmData data;

	/** Whether we have read the data from the database (if so, it will
	 *  be stored in data, otherwise the contents of data will be
	 *  undefined).
	 */
	mutable bool have_data;

	/** The keys for this document.
	 *
	 *  If a key ID is not in this map, then the database has not been
	 *  checked for it: if the key is not in the database, it will simply
	 *  have an empty value.
	 */
	mutable std::map<om_keyno, OmKey> keys;

	/** Constructor: called by SleepycatDatabase to read a document
	 *
	 *  @param document_db_  The database to find the document data in.
	 *  @param key_db_       The database to find the document keys in.
	 *  @param did_          The document ID to look for.
	 *
	 *  @exception OmDatabaseError is thrown if the document can't be
	 *             accessed.
	 */
	SleepycatDocument(Db * document_db_,
		       Db * key_db_,
		       om_docid did_);

	/** Constructor: called by SleepycatDatabase to create a new document
	 *
	 *  @param document_db_  The database to store the document data in.
	 *  @param key_db_       The database to store the document keys in.
	 *  @param document_     The document to store in the database.
	 *
	 *  @exception OmDatabaseError is thrown if the document can't be
	 *             accessed.
	 */
	SleepycatDocument(Db * document_db_,
		       Db * key_db_,
		       const OmDocumentContents & document_);

	/// Copying is not permitted
	SleepycatDocument(const SleepycatDocument &);

	/// Assignment is not permitted
	SleepycatDocument & operator = (const SleepycatDocument &);
    public:
	/** Return the document ID of this document.
	 */
	om_docid get_docid() const;

	// Virtual methods of Leaf Document
	OmKey do_get_key(om_keyno keyid) const;
	std::map<om_keyno, OmKey> do_get_all_keys() const;
	OmData do_get_data() const;
};

#endif /* MUS_BUILD_BACKEND_SLEEPYCAT */

#endif /* OM_HGUARD_SLEEPYCAT_DOCUMENT_H */
