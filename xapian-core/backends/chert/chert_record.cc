/* chert_record.cc: Records in chert databases
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

#include "chert_record.h"

#include <xapian/error.h>
#include "debuglog.h"
#include "omassert.h"
#include "pack.h"
#include "str.h"

using std::string;

inline string
make_key(Xapian::docid did)
{
    string key;
    pack_uint_preserving_sort(key, did);
    return key;
}

string
ChertRecordTable::get_record(Xapian::docid did) const
{
    LOGCALL(DB, string, "ChertRecordTable::get_record", did);
    string tag;

    if (!get_exact_entry(make_key(did), tag)) {
	throw Xapian::DocNotFoundError("Document " + str(did) + " not found.");
    }

    RETURN(tag);
}

Xapian::doccount
ChertRecordTable::get_doccount() const
{   
    LOGCALL(DB, Xapian::doccount, "ChertRecordTable::get_doccount", NO_ARGS);
    chert_tablesize_t count = get_entry_count();
    if (rare(count > chert_tablesize_t(Xapian::doccount(-1)))) {
	// If we've got more entries than there are possible docids, the
	// database is in an odd state.
	throw Xapian::DatabaseCorruptError("Impossibly many entries in the record table");
    }
    RETURN(Xapian::doccount(count));
}

void
ChertRecordTable::replace_record(const string & data, Xapian::docid did)
{
    LOGCALL_VOID(DB, "ChertRecordTable::replace_record", data | did);
    add(make_key(did), data);
}

void
ChertRecordTable::delete_record(Xapian::docid did)
{
    LOGCALL_VOID(DB, "ChertRecordTable::delete_record", did);
    if (!del(make_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + str(did));
}
