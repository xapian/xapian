/** @file inmemory_document.cc
 * @brief A document read from a InMemoryDatabase.
 */
/* Copyright (C) 2008 Olly Betts
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

#include "inmemory_document.h"

#include "debuglog.h"
#include "omassert.h"

string
InMemoryDocument::do_get_value(Xapian::valueno) const
{
    LOGCALL(DB, string, "InMemoryDocument::do_get_value", "[slot]");
    // Our ctor sets the values, so we should never get here.
    Assert(false);
    RETURN(string());
}

void
InMemoryDocument::do_get_all_values(map<Xapian::valueno, string> &) const
{
    LOGCALL_VOID(DB, "InMemoryDocument::do_get_all_values", "[&values_]");
    // Our ctor sets the values, so we should never get here.
    Assert(false);
}

string
InMemoryDocument::do_get_data() const
{
    LOGCALL(DB, string, "InMemoryDocument::do_get_data", "");
    // Our ctor sets the data, so we should never get here.
    Assert(false);
    RETURN(string());
}
