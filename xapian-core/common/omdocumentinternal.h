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

/// A term in a document.
struct OmDocumentTerm {
    /** Make a new term.
     *
     *  This creates a new term, and adds one posting at the specified
     *  position.
     *
     *  @param tname_ The name of the new term.
     *  @param tpos   Optional positional information.
     */
    OmDocumentTerm(const om_termname & tname_, om_termpos tpos = 0);

    /** The name of this term.
     */
    om_termname tname;

    /** Within document frequency of the term.
     *  This is the number of occurrences of the term in the document.
     */
    om_termcount wdf;

    typedef std::vector<om_termpos> term_positions;

    /** Positional information.
     *
     *  This is a list of positions at which the term occurs in the
     *  document. The list is in strictly increasing order of term
     *  position.
     *
     *  The positions start at 1.
     *
     *  Note that, even if positional information is present, the WDF might
     *  not be equal to the length of the position list, since a term might
     *  occur multiple times at a single position, but will only have one
     *  entry in the position list for each position.
     */
    term_positions positions;

    /** Term frequency information.
     *
     *  This is the number of documents indexed by the term.
     *
     *  If the information is not available, the value will be 0.
     */
    om_doccount termfreq;

    /** Add an entry to the posting list.
     *
     *  This method increments the wdf.  If positional information is
     *  supplied, this also adds an entry to the list of positions, unless
     *  there is already one for the specified position.
     *
     *  @param tpos The position within the document at which the term
     *              occurs.  If this information is not available, use
     *              the default value of 0.
     */
    void add_posting(om_termpos tpos = 0);

    /** Returns a string representing the OmDocumentTerm.
     *  Introspection method.
     */
    std::string get_description() const;
};

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

	Internal()
		: ptr(NULL), data_here(true), keys_here(true), terms_here(true)
	{}

	void read_termlist(OmTermListIterator t,
			   const OmTermListIterator & tend);

	/** Returns a string representing the object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif  // OM_HGUARD_OMDOCUMENTINTERNAL_H
