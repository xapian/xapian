/* flint_record.cc: Records in flint databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
#include "flint_record.h"
#include "flint_utils.h"
#include "utils.h"
#include <xapian/error.h>
#include "omassert.h"
#include "omdebug.h"

using std::string;

string
FlintRecordTable::get_record(Xapian::docid did) const
{
    DEBUGCALL(DB, string, "FlintRecordTable::get_record", did);
    string tag;

    if (!get_exact_entry(flint_docid_to_key(did), tag)) {
	throw Xapian::DocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    RETURN(tag);
}


Xapian::doccount
FlintRecordTable::get_doccount() const
{   
    DEBUGCALL(DB, Xapian::doccount, "FlintRecordTable::get_doccount", "");
    // Check that we can't overflow (the unsigned test is actually too
    // strict as we can typically assign an unsigned short to a signed long,
    // but this shouldn't actually matter here).
    CASSERT(sizeof(Xapian::doccount) >= sizeof(flint_tablesize_t));
    CASSERT_TYPE_UNSIGNED(Xapian::doccount);
    CASSERT_TYPE_UNSIGNED(flint_tablesize_t);
    RETURN(get_entry_count());
}

void
FlintRecordTable::replace_record(const string & data, Xapian::docid did)
{
    DEBUGCALL(DB, void, "FlintRecordTable::replace_record", data << ", " << did);
    string key(flint_docid_to_key(did));
    add(key, data);
}

void
FlintRecordTable::delete_record(Xapian::docid did)
{
    DEBUGCALL(DB, void, "FlintRecordTable::delete_record", did);
    if (!del(flint_docid_to_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + om_tostring(did));
}
