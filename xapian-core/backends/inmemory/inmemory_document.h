/* inmemory_document.h: C++ class definition for accessing a inmemory document
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

#ifndef OM_HGUARD_INMEMORY_DOCUMENT_H
#define OM_HGUARD_INMEMORY_DOCUMENT_H

#include "document.h"

class InMemoryDocument : public Document {
    friend class InMemoryDatabase;
    private:
	string doc;
	map<om_valueno, string> values;

	InMemoryDocument(const Database *database_, om_docid did_,
			 const string & doc_,
			 const map<om_valueno, string> &values_);

	// Stop copying
	InMemoryDocument(const InMemoryDocument &);
	InMemoryDocument & operator = (const InMemoryDocument &);
    public:
	string do_get_value(om_valueno valueid) const;
	map<om_valueno, string> do_get_all_values() const;
	string do_get_data() const;
};

#endif /* OM_HGUARD_INMEMORY_DOCUMENT_H */
