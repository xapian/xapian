/** @file
 * @brief A document read from a RemoteDatabase.
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

#include "remote-document.h"

#include "debuglog.h"
#include "omassert.h"

string
RemoteDocument::do_get_value(Xapian::valueno) const
{
    LOGCALL(DB, string, "RemoteDocument::do_get_value", Literal("[slot]"));
    // Our ctor sets the values, so we should never get here.
    Assert(false);
    RETURN(string());
}

void
RemoteDocument::do_get_all_values(map<Xapian::valueno, string> &) const
{
    LOGCALL_VOID(DB, "RemoteDocument::do_get_all_values", Literal("[&values_]"));
    // Our ctor sets the values, so we should never get here.
    Assert(false);
}

string
RemoteDocument::do_get_data() const
{
    LOGCALL(DB, string, "RemoteDocument::do_get_data", NO_ARGS);
    // Our ctor sets the data, so we should never get here.
    Assert(false);
    RETURN(string());
}
