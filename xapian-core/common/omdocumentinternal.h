/* omdocumentinternal.h: internal class representing a document
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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

#include <string>
#include <xapian/types.h>
#include "refcnt.h"
#include "document.h"

using namespace std;

/// A term in a document.
struct OmDocumentTerm {
    /** Make a new term.
     *
     *  @param tname_ The name of the new term.
     */
    OmDocumentTerm(const string & tname_);

    /** The name of this term.
     */
    string tname;

    /** Within document frequency of the term.
     *  This is the number of occurrences of the term in the document.
     */
    om_termcount wdf;

    typedef vector<om_termpos> term_positions;

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

    /** Add a position to the posting list.
     *
     *  This adds an entry to the list of positions, unless
     *  there is already one for the specified position.
     *
     *  This does not change the value of the wdf.
     *
     *  @param tpos The position within the document at which the term
     *              occurs.
     */
    void add_position(om_termpos tpos);

    /** Remove an entry from the posting list.
     *
     *  This removes an entry from the list of positions.
     *
     *  This does not change the value of the wdf.
     *
     *  @exception Xapian::InvalidArgumentError is thrown if the 
     */
    void remove_position(om_termpos tpos);

    /// Set the wdf
    void set_wdf(om_termcount wdf_) { wdf = wdf_; }

    /// Get the wdf
    om_termcount get_wdf() { return wdf; }

    /** Returns a string representing the OmDocumentTerm.
     *  Introspection method.
     */
    string get_description() const;
};

// A document - holds values, terms, and document data
// Data can be in a database (accessed via a Document) or held by this class
// (or some combination if a document from a database is being amended).
class OmDocument::Internal {
    public:
	/// The reference counted pointer to a Document instance
	RefCntPtr<Document> ptr;

	Xapian::Database database;

	om_docid did;

	bool data_here, values_here, terms_here;

	/// The (user defined) data associated with this document.
	string data;

	/// Type to store values in.
	typedef map<om_valueno, string> document_values;

	/// The values associated with this document.
	document_values values;

	/// Type to store terms in.
	typedef map<string, OmDocumentTerm> document_terms;

	/// The terms (and their frequencies and positions) in this document.
	document_terms terms;

	explicit Internal(Document *ld, const Xapian::Database &database_,
			  om_docid did_)
		: ptr(ld), database(database_), did(did_), data_here(false),
		  values_here(false), terms_here(false) {}

	explicit Internal(RefCntPtr<Document> ptr_, const Xapian::Database &database_,
			  om_docid did_)
	        : ptr(ptr_), database(database_), did(did_), data_here(false),
		  values_here(false), terms_here(false) {}

	Internal(const Internal &other)
		: ptr(other.ptr),
		  database(other.database),
		  did(other.did),
		  data_here(other.data_here),
		  values_here(other.values_here),
		  terms_here(other.terms_here),
		  data(other.data),
		  values(other.values),
		  terms(other.terms) {}

	Internal()
		: ptr(NULL), data_here(true), values_here(true), terms_here(true)
	{}

	void read_termlist(Xapian::TermIterator t, const Xapian::TermIterator & tend);

	/** Returns a string representing the object.
	 *  Introspection method.
	 */
	string get_description() const;

        /* calls ptr->get_all_values(); or whatever, as so many
         * methods seem to be doing this independantly.
         */
        void need_values();
};

#endif  // OM_HGUARD_OMDOCUMENTINTERNAL_H
