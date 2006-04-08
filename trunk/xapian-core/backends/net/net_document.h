/* net_document.h: C++ class definition for accessing a net document
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

#ifndef OM_HGUARD_NET_DOCUMENT_H
#define OM_HGUARD_NET_DOCUMENT_H

#include "document.h"

class NetworkDocument : public Xapian::Document::Internal {
    friend class NetworkDatabase;
    private:
	string doc;

	map<Xapian::valueno, string> values;

	NetworkDocument(const Xapian::Database::Internal *database_, Xapian::docid did_,
			const string & doc_,
			const map<Xapian::valueno, string> &values_);

	// Stop copying
	NetworkDocument(const NetworkDocument &);
	NetworkDocument & operator = (const NetworkDocument &);
    public:
	string do_get_value(Xapian::valueno valueid) const;
	map<Xapian::valueno, string> do_get_all_values() const;
	string do_get_data() const;
};

#endif /* OM_HGUARD_NET_DOCUMENT_H */
