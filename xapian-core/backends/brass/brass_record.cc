/* brass_record.cc: Records in brass databases
 *
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2008,2009 Olly Betts
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

#include "brass_record.h"

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
BrassRecordTable::get_record(Xapian::docid did) const
{
    LOGCALL(DB, string, "BrassRecordTable::get_record", did);
    string tag;

    if (!get_exact_entry(make_key(did), tag)) {
	throw Xapian::DocNotFoundError("Document " + str(did) + " not found.");
    }

    RETURN(tag);
}

Xapian::doccount
BrassRecordTable::get_doccount() const
{   
    LOGCALL(DB, Xapian::doccount, "BrassRecordTable::get_doccount", NO_ARGS);
    brass_tablesize_t count = get_entry_count();
    if (rare(count > brass_tablesize_t(Xapian::doccount(-1)))) {
	// If we've got more entries than there are possible docids, the
	// database is in an odd state.
	throw Xapian::DatabaseCorruptError("Impossibly many entries in the record table");
    }
    RETURN(Xapian::doccount(count));
}

void
BrassRecordTable::replace_record(const string & data, Xapian::docid did)
{
    LOGCALL_VOID(DB, "BrassRecordTable::replace_record", data | did);
    add(make_key(did), data);
}

void
BrassRecordTable::delete_record(Xapian::docid did)
{
    LOGCALL_VOID(DB, "BrassRecordTable::delete_record", did);
    if (!del(make_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + str(did));
}
