/* omdocumentinternal.h: internal class representing a modified document
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
#include <xapian/base.h>
#include <xapian/types.h>
#include "document.h"
#include "documentterm.h"

using namespace std;

// A document - holds values, terms, and document data
// Data can be in a database (accessed via a Xapian::Document::Internal) or
// held by this class (or some combination if a document from a database is
// being amended).
class ModifiedDocument : public Xapian::Document::Internal {
    public:
	/// Reference counted pointer to the underlying
	// Xapian::Document::Internal instance
	Xapian::Internal::RefCntPtr<Xapian::Document::Internal> ptr;

	bool data_here;
	mutable bool values_here; // FIXME mutable is a hack
	bool terms_here;

	/// The (user defined) data associated with this document.
	string data;

	/// Type to store values in.
	typedef map<om_valueno, string> document_values;

	/// The values associated with this document.
	mutable document_values values; // FIXME mutable is a hack

	/// Type to store terms in.
	typedef map<string, OmDocumentTerm> document_terms;

	/// The terms (and their frequencies and positions) in this document.
	document_terms terms;

	explicit ModifiedDocument(Xapian::Document::Internal *ld)
		: ptr(ld), data_here(false), values_here(false),
		  terms_here(false) { }

	string get_value(om_valueno valueid) const;
	map<om_valueno, string> get_all_values() const;
	string get_data() const;
	void set_data(const string &data_);
	TermList * open_term_list() const;
	void add_value(om_valueno valueno, const string &value);
	void remove_value(om_valueno valueno);
	void clear_values();
	void add_posting(const string & tname, om_termpos tpos, om_termcount wdfinc);
	void add_term_nopos(const string & tname, om_termcount wdfinc);
	void remove_posting(const string & tname, om_termpos tpos,
			    om_termcount wdfdec);
	void remove_term(const string & tname);
	void clear_terms();
	om_termcount termlist_count();
	om_termcount values_count();
	OmValueIterator::Internal * values_begin() const;
	OmValueIterator::Internal * values_end() const;
	/** Returns a string representing the object.
	 *  Introspection method.
	 */
	string get_description() const;
        /* calls ptr->get_all_values(); or whatever, as so many
         * methods seem to be doing this independantly.
         */
        void need_values() const/*FIXME const is a hack*/;
	void need_terms();
	Xapian::Document::Internal * modify();
};

#endif  // OM_HGUARD_OMDOCUMENTINTERNAL_H
