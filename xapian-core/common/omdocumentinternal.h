/* omdocumentinternal.h: internal class representing a document
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

#ifndef OM_HGUARD_OMDOCUMENTINTERNAL_H
#define OM_HGUARD_OMDOCUMENTINTERNAL_H

#include "om/omtypes.h"
#include "refcnt.h"
#include "document.h"

// A document - holds keys, terms, and document data
// Data be in a database (accessed via a Document) or held by this class
// (or some combination if a document from a database is being amended).
class OmDocument::Internal {
    public:
	/// The reference counted pointer to a Document instance
	RefCntPtr<Document> ptr;

	OmDatabase database;

	om_docid did;

	bool data_here, keys_here, terms_here;

	/// The (user defined) data associated with this document.
	OmData data;

	/// Type to store keys in.
	typedef std::map<om_keyno, OmKey> document_keys;

	/// The keys associated with this document.
	document_keys keys;

	/// Type to store terms in.
	typedef std::map<om_termname, OmDocumentTerm> document_terms;

	/// The terms (and their frequencies and positions) in this document.
	document_terms terms;

	explicit Internal(Document *ld, const OmDatabase &database_,
			  om_docid did_)
		: ptr(ld), database(database_), did(did_), data_here(false),
		  keys_here(false), terms_here(false) {}

	explicit Internal(RefCntPtr<Document> ptr_, const OmDatabase &database_,
			  om_docid did_)
	        : ptr(ptr_), database(database_), did(did_), data_here(false),
		  keys_here(false), terms_here(false) {}

	Internal(const Internal &other)
		: ptr(other.ptr), data_here(other.data_here),
		  keys_here(other.keys_here), terms_here(other.terms_here) {}

	/** Add an occurrence of a term to the document.
	 *
	 *  Multiple occurrences of the term at the same position are represented
	 *  only once in the positional information, but do increase the wdf.
	 *
	 *  @param tname  The name of the term.
	 *  @param tpos   The position of the term.
	 */
	void add_posting(const om_termname & tname, om_termpos tpos = 0);

	/** Returns a string representing the object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif  // OM_HGUARD_OMDOCUMENTINTERNAL_H
