/** @file brass_document.cc
 * @brief A document read from a BrassDatabase.
 */
/* Copyright (C) 2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "brass_document.h"

#include "debuglog.h"

using namespace std;

string
BrassDocument::do_get_value(Xapian::valueno slot) const
{
    LOGCALL(DB, string, "BrassDocument::do_get_value", slot);
    RETURN(value_manager.get_value(did, slot));
}

void
BrassDocument::do_get_all_values(map<Xapian::valueno, string> & values_) const
{
    LOGCALL_VOID(DB, "BrassDocument::do_get_all_values", "[values_]");
    value_manager.get_all_values(did, values_);
}

string
BrassDocument::do_get_data() const
{
    LOGCALL(DB, string, "BrassDocument::do_get_data", NO_ARGS);
    RETURN(record_table.get_document_data(did));
}
