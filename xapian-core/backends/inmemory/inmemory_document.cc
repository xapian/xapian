/* inmemory_document.cc: C++ class for storing inmemory documents
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2008 Olly Betts
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

#include <config.h>
#include "inmemory_document.h"

InMemoryDocument::InMemoryDocument(const Xapian::Database::Internal *db_,
				   Xapian::docid did_,
				   const string & data_,
				   const map<Xapian::valueno, string> &values_)
	: Xapian::Document::Internal(db_, did_), data(data_), values(values_)
{
}

string
InMemoryDocument::do_get_value(Xapian::valueno valueid) const
{
    map<Xapian::valueno, string>::const_iterator k = values.find(valueid);
    if (k == values.end()) return "";
    return k->second;
}

void
InMemoryDocument::do_get_all_values(map<Xapian::valueno, string> & values_) const
{
    values_ = values;
}

string
InMemoryDocument::do_get_data() const
{
    return data;
}
