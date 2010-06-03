/* flint_record.cc: Records in flint databases
 *
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2008 Olly Betts
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
#include "flint_record.h"
#include "flint_utils.h"
#include "str.h"
#include <xapian/error.h>
#include "debuglog.h"
#include "omassert.h"

using std::string;

string
FlintRecordTable::get_record(Xapian::docid did) const
{
    LOGCALL(DB, string, "FlintRecordTable::get_record", did);
    string tag;

    if (!get_exact_entry(flint_docid_to_key(did), tag)) {
	throw Xapian::DocNotFoundError("Document " + str(did) + " not found.");
    }

    RETURN(tag);
}

Xapian::doccount
FlintRecordTable::get_doccount() const
{   
    LOGCALL(DB, Xapian::doccount, "FlintRecordTable::get_doccount", NO_ARGS);
    STATIC_ASSERT_TYPE_DOMINATES(Xapian::doccount, flint_tablesize_t);
    RETURN(get_entry_count());
}

void
FlintRecordTable::replace_record(const string & data, Xapian::docid did)
{
    LOGCALL_VOID(DB, "FlintRecordTable::replace_record", data | did);
    add(flint_docid_to_key(did), data);
}

void
FlintRecordTable::delete_record(Xapian::docid did)
{
    LOGCALL_VOID(DB, "FlintRecordTable::delete_record", did);
    if (!del(flint_docid_to_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + str(did));
}
