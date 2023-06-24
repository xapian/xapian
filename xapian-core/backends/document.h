/** @file document.h
 * @brief class with document data
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2007,2008,2009,2010,2011 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_DOCUMENT_H
#define OM_HGUARD_DOCUMENT_H

#include "xapian/intrusive_ptr.h"
#include <xapian/types.h>
#include "api/termlist.h"
#include "backends/database.h"
#include "api/documentterm.h"
#include <map>
#include <string>

using std::map;
using std::string;
using std::swap;

class DocumentValueList;
class ValueStreamDocument;

/// A document in the database, possibly plus modifications.
class Xapian::Document::Internal : public Xapian::Internal::intrusive_base {
    friend class ::DocumentValueList;
    friend class ::ValueStreamDocument;
    public:
	/// Type to store values in.
	typedef map<Xapian::valueno, string> document_values;

	/// Type to store terms in.
	typedef map<string, OmDocumentTerm> document_terms;

    protected:
	/// The database this document is in.
	Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database;

    private:
	// Prevent copying
	Internal(const Internal &);
	Internal & operator=(const Internal &);

	bool data_here;
	mutable bool values_here; // FIXME mutable is a hack
	mutable bool terms_here;
	mutable bool positions_modified;

	/// The (user defined) data associated with this document.
	string data;

	/// The values associated with this document.
	mutable document_values values; // FIXME mutable is a hack

	/// The terms (and their frequencies and positions) in this document.
	mutable document_terms terms;

	/** The number of distinct terms in @a terms.
	 *
	 *  Only valid when terms_here is true.
	 *
	 *  This may be less than terms.size() if any terms have been deleted.
	 */
	mutable Xapian::termcount termlist_size;

    protected:
	/** The document ID of the document in that database.
	 *
	 *  If we're using multiple databases together this may not be the
	 *  same as the docid in the combined database.
	 */
	Xapian::docid did;

    private:
	// Functions for backend to implement
	virtual string do_get_value(Xapian::valueno /*valueno*/) const { return string(); }
	virtual void do_get_all_values(map<Xapian::valueno, string> & values_) const {
	    values_.clear();
	}
	virtual string do_get_data() const { return string(); }

    public:
	/** Get value by value number.
	 *
	 *  Values are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of values, each of which
	 *  having a different value number.  Duplicate values with the same
	 *  value number are not supported in a single document.
	 *
	 *  Value numbers are any integer >= 0, but particular database
	 *  backends may impose a more restrictive range than that.
	 *
	 *  @param slot  The value number requested.
	 *
	 *  @return       A string containing the specified value.  If
	 *  the value is not present in this document, the value's value will
	 *  be a zero length string
	 */
	string get_value(Xapian::valueno slot) const;

	/** Set all the values.
	 *
	 *  @param values_	The values to set - passed by non-const reference, and
	 *			may be modified by the call.
	 */
	void set_all_values(map<Xapian::valueno, string> & values_) {
	    // For efficiency we just swap the old and new value maps.
	    swap(values, values_);
	    values_here = true;
	}

	Xapian::valueno values_count() const;
	void add_value(Xapian::valueno, const string &);
	void remove_value(Xapian::valueno);
	void clear_values();
	void add_posting(const string &, Xapian::termpos, Xapian::termcount);
	void add_term(const string &, Xapian::termcount);
	void remove_posting(const string &, Xapian::termpos, Xapian::termcount);
	Xapian::termpos remove_postings(const string &,
					Xapian::termpos, Xapian::termpos,
					Xapian::termcount);
	void remove_term(const string &);
	void clear_terms();
	Xapian::termcount termlist_count() const;

	/** Get data stored in document.
	 *
	 *  This is a general piece of data associated with a document, and
	 *  will typically be used to store such information as text to be
	 *  displayed in the result list, and a pointer in some form
	 *  (eg, URL) to the full text of the document.
	 *
	 *  This operation can be expensive, and shouldn't normally be used
	 *  during the match operation (such as in a match decider functor):
	 *  use a value instead, if at all possible.
	 *
	 *  @return       A string containing the data for this document.
	 */
	string get_data() const;

	void set_data(const string &);

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @return       A pointer to the newly created term list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	TermList * open_term_list() const;

	void need_values() const;
	void need_terms() const;

	/** Return true if the data in the document may have been modified.
	 */
	bool data_modified() const {
	    return data_here;
	}

	/** Return true if the values in the document may have been modified.
	 */
	bool values_modified() const {
	    return values_here;
	}

	/** Return true if the terms in the document may have been modified.
	 */
	bool terms_modified() const {
	    return terms_here;
	}

	/// Return true if term positions may have been modified.
	bool term_positions_modified() const {
	    return positions_modified;
	}

	/// Return true if the document may have been modified.
	bool modified() const {
	    return terms_here || values_here || data_here;
	}

	/** Get the docid which is associated with this document (if any).
	 *
	 *  NB If multiple databases are being searched together, then this
	 *  will be the document id in the individual database, not the merged
	 *  database!
	 *
	 *  @return If this document came from a database, return the document
	 *	    id in that database.  Otherwise, return 0.
	 */
	Xapian::docid get_docid() const { return did; }

	/// Return a string describing this object.
	string get_description() const;

	/** Constructor.
	 *
	 *  In derived classes, this will typically be a private method, and
	 *  only be called by database objects of the corresponding type.
	 */
	Internal(Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database_,
		 Xapian::docid did_)
	    : database(database_), data_here(false), values_here(false),
	      terms_here(false), positions_modified(false), did(did_) { }

	Internal()
	    : database(), data_here(false), values_here(false),
	      terms_here(false), positions_modified(false), did(0) { }

	/** Destructor.
	 *
	 *  Note that the database object which created this document must
	 *  still exist at the time this is called.
	 */
	virtual ~Internal();
};

#endif  // OM_HGUARD_DOCUMENT_H
