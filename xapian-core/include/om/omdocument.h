/* omdocument.h: representation of a document
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
#include "om/omtermlistiterator.h"
#include "om/omkeylistiterator.h"

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
    public:
	class Internal;
	/// @internal
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
	 *  This is a potentially expensive operation, and shouldn't normally
	 *  be used in a match decider functor.  Put data for use by match
	 *  deciders in a key instead.
	 */
	std::string get_data() const;

	/// Set data stored in a document.
	void set_data(const std::string &data);

	/** Add an occurrence of a term at a particular position.
	 *
	 *  Multiple occurrences of the term at the same position are
	 *  represented only once in the positional information, but do
	 *  increase the wdf.
	 *
	 *  If the term is not already in the document, it will be added to
	 *  it.
	 *
	 *  @param tname     The name of the term.
	 *  @param tpos      The position of the term.
	 *  @param wdfinc    The increment that will be applied to the wdf
	 *                   for this term.
	 */
	void add_posting(const om_termname & tname,
			 om_termpos tpos,
			 om_termcount wdfinc = 1);

	/** Add a term to the document, without specifying a position that
	 *  it occurs at.
	 *
	 *  Any existing positional information for the term will be left
	 *  unmodified.
	 *
	 *  @param tname     The name of the term.
	 *  @param wdfinc    The increment that will be applied to the wdf
	 *                   for this term.
	 */
	void add_term_nopos(const om_termname & tname,
			    om_termcount wdfinc = 1);

	/** Remove a posting of a term from the document.
	 *
	 *  Note that the term will still index the document even if all
	 *  occurrences are removed.  To remove a term from a document
	 *  completely, use remove_term().
	 *
	 *  @param tname     The name of the term.
	 *  @param tpos      The position of the term.
	 *  @param wdfdec    The decrement that will be applied to the wdf
	 *                   when removing this posting.  The wdf will not go
	 *                   below the value of 0.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if the term is
	 *  not at the position specified in the position list for this
	 *  term in this document.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if the term is
	 *  not in the document
	 */
	void remove_posting(const om_termname & tname,
			    om_termpos tpos,
			    om_termcount wdfdec = 1);

	/** Remove a term and all postings associated with it.
	 *
	 *  @param tname  The name of the term.
	 *
	 *  @exception OmInvalidArgumentError will be thrown if the term is
	 *  not in the document
	 */
	void remove_term(const om_termname & tname);

	/// Remove all terms and postings from the document.
	void clear_terms();

	om_termcount termlist_count();
	OmTermIterator termlist_begin() const;
	OmTermIterator termlist_end() const;

	om_termcount keylist_count();
	OmKeyListIterator keylist_begin() const;
	OmKeyListIterator keylist_end() const;

	/** Returns a string representing the OmDocument.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif  // OM_HGUARD_OMDOCUMENT_H
