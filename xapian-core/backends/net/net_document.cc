/* net_document.cc: C++ class for storing net documents
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

#include <config.h>
#include "net_document.h"
#include "omdebug.h"

NetworkDocument::NetworkDocument(const Xapian::Database::Internal *db_,
				 Xapian::docid did_,
				 const string & doc_,
				 const map<Xapian::valueno, string> &values_)
	: Xapian::Document::Internal(db_, did_), doc(doc_), values(values_)
{
}

string
NetworkDocument::do_get_value(Xapian::valueno valueid) const
{
    DEBUGCALL(DB, string, "NetworkDocument::get_value", valueid);
    map<Xapian::valueno, string>::const_iterator k = values.find(valueid);
    if (k == values.end()) RETURN("");
    RETURN(k->second);
}

map<Xapian::valueno, string>
NetworkDocument::do_get_all_values() const
{
    return values;
}

string
NetworkDocument::do_get_data() const
{
    return doc;
}
