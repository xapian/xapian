/* omdocument.h: representation of a document
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

#ifndef OM_HGUARD_OMDOCUMENT_H
#define OM_HGUARD_OMDOCUMENT_H

#include <vector>

#include <om/omtypes.h>
#include <om/omtermlistiterator.h>
#include <om/omkeylistiterator.h>

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
	std::string value;

	/// Construct from a string.
	OmData(const std::string &data) : value(data) {}

	/// Default constructor.
	OmData() {}

	/** Returns a string representing the OmData.
	 *  Introspection method.
	 */
	std::string get_description() const { return "OmData(" + value + ")"; }
};

/// A key in a document.
class OmKey {
    public:
	/// The value of a key.
	std::string value;

	/// Ordering for keys, so they can be stored in STL containers.
	bool operator < (const OmKey &k) const { return(value < k.value); }

	/// Construct from a string.
	OmKey(const std::string &data) : value(data) {}

	/// Default constructor.
	OmKey() {}

	/// Default destructor.
	~OmKey() {}

	/** Returns a string representing the OmKey.
	 *  Introspection method.
	 */
	std::string get_description() const { return "OmKey(" + value + ")"; }
};

/// A document in the database - holds keys and records
class OmDocument {
    private:
	class Internal;
	Internal *internal;

    public:
	/** Constructor is only used by internal classes.
	 *
	 *  @param params int internal opaque class
	 */
	explicit OmDocument(OmDocument::Internal *internal_);

	/** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmDocument(const OmDocument &other);

	/** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const OmDocument &other);

	/// Make a new empty OmDocument
	OmDocument();

	/// Destructor.
	~OmDocument();

	/// Get key by number (>= 0)
	OmKey get_key(om_keyno key) const;

	void add_key(om_keyno keyno, const OmKey &key);

	void remove_key(om_keyno keyno);

	void clear_keys();

	/** Get data stored in document.
	 *  This can be expensive, and shouldn't normally be used
	 *  in a match decider functor.
	 */
	OmData get_data() const;

	/// Set data stored in a document.
	void set_data(const OmData &data);

	/// Set data stored in a document, from a string.
	void set_data(const std::string &data);

	/** Add an occurrence of a term to the document.
	 *
	 *  Multiple occurrences of the term at the same position are
	 *  represented only once in the positional information, but do
	 *  increase the wdf.
	 *
	 *  @param tname  The name of the term.
	 *  @param tpos   The position of the term.
	 */
	void add_posting(const om_termname & tname, om_termpos tpos);

	/** Remove an occurrence of a term to the document.
	 *
	 *  @param tname  The name of the term.
	 *  @param tpos   The position of the term.
	 */
	void remove_posting(const om_termname & tname, om_termpos tpos);

	/** Remove a term and all postings associated with it.
	 *
	 *  @param tname  The name of the term.
	 */
	void remove_term(const om_termname & tname);

	/// Remove all terms and postings.
	void clear_terms();

	OmTermListIterator termlist_begin() const;
	OmTermListIterator termlist_end() const;

	OmKeyListIterator keylist_begin() const;
	OmKeyListIterator keylist_end() const;

	/** Returns a string representing the OmDocument.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif  // OM_HGUARD_OMDOCUMENT_H
