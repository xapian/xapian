/* document.h: class with document data
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

#ifndef OM_HGUARD_DOCUMENT_H
#define OM_HGUARD_DOCUMENT_H

#include "om/omtypes.h"
#include "refcnt.h"
#include "termlist.h"
#include <map>
using std::map;
#include <string>
using std::string;

class OmValue;
class Database;

/// A document in the database - holds values, terms, postings, etc
class Document : public RefCntBase {
    private:
	/// Copies are not allowed.
	Document(const Document &, om_docid did);

	/// Assignment is not allowed.
	void operator=(const Document &);

	const Database *database;

	/// The virtual implementation of get_value().
	virtual OmValue do_get_value(om_valueno valueid) const = 0;
	/// The virtual implementation of get_all_values().
	virtual map<om_valueno, OmValue> do_get_all_values() const = 0;
	/// The virtual implementation of get_data().
	virtual string do_get_data() const = 0;

    protected:
	/// The document ID of the document.
	om_docid did;
	
    public:
	/** Get value by value number.
	 *
	 *  Values are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of values, each of which
	 *  having a different valueid.  Duplicate values with the same valueid
	 *  are not supported in a single document.
	 *
	 *  Value numbers are any integer >= 0, but particular database
	 *  backends may impose a more restrictive range than that.
	 *
	 *  @param valueid  The value number requested.
	 *
	 *  @return       An OmValue object containing the specified value.  If
	 *  the value is not present in this document, the value's value will
	 *  be a zero length string
	 */
	OmValue get_value(om_valueno valueid) const;

	/** Get all values for this document
	 *
	 *  Values are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of values, each of which
	 *  having a different valueid.  Duplicate values with the same valueid
	 *  are not supported in a single document.
	 *
	 *  @return   A map of OmValue objects containing all the values.
	 */
	map<om_valueno, OmValue> get_all_values() const;

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
	 *  @return       An string containing the data for this document.
	 */
	string get_data() const;	

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @return       A pointer to the newly created term list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	LeafTermList * open_term_list() const;
	
	/** Constructor.  In derived classes, this will typically be a
	 *  private method, and only be called by database objects of the
	 *  corresponding type.
	 */
	Document(const Database *database_, om_docid did_)
	    : database(database_), did(did_) {};

	/** Destructor.  Note that the database object which created this
	 *  document must still exist at the time this is called.
	 */
	virtual ~Document() {}
};

#endif  // OM_HGUARD_DOCUMENT_H
