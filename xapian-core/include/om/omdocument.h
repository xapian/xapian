/* omdocument.h: class for performing a match
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

#ifndef OM_HGUARD_OMDOCUMENT_H
#define OM_HGUARD_OMDOCUMENT_H

#include "om/omtypes.h"

///////////////////////////////////////////////////////////////////
// OmData class
// ============
// Representing the document data

/** The data in a document.
 *  This contains the arbitrary chunk of data which is associated
 *  with each document in the database: it is up to the user to define
 *  the format of this data, and to set it at indexing time.
 */
class OmData {
    public:
	/// The data.
	string value;
};

/// A key in a document.
class OmKey {
    public:
	/// The value of a key.
	string value;

	/// Ordering for keys, so they can be stored in STL containers.
	bool operator < (const OmKey &k) const { return(value < k.value); }
};

/// Opaque class containing parameters used to construct an OmDocument.
class OmDocumentParams;

/// A document in the database - holds keys and records
class OmDocument {
    private:
	class Internal;
	Internal *internal;

    public:
	/** Constructor is only used by internal classes.
	 *  
	 *  @param params Parameters used to construct the document.
	 */
	explicit OmDocument(const OmDocumentParams & params);

	/** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmDocument(const OmDocument &other);

	/** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const OmDocument &other);

	/** Destructor. */
	~OmDocument();

	/** Get key by number (>= 0)
	 */
	OmKey get_key(om_keyno key) const;
	
	/** Get data stored in document.
	 *  This can be expensive, and shouldn't normally be used
	 *  in a match decider functor.
	 */
	OmData get_data() const;     
};

#endif  // OM_HGUARD_OMDOCUMENT_H
