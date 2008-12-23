/* quartz_record.cc: Records in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
#include "quartz_record.h"
#include "quartz_utils.h"
#include "utils.h"
#include <xapian/error.h>
#include "omassert.h"
#include "omdebug.h"

using std::string;

// Magic key (which corresponds to an invalid docid) is used to store the
// next free docid and total length of all documents
static const string METAINFO_KEY("", 1);

string
QuartzRecordTable::get_record(Xapian::docid did) const
{
    DEBUGCALL(DB, string, "QuartzRecordTable::get_record", did);
    string key(quartz_docid_to_key(did));
    string tag;

    if (!get_exact_entry(key, tag)) {
	throw Xapian::DocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    RETURN(tag);
}


Xapian::doccount
QuartzRecordTable::get_doccount() const
{   
    DEBUGCALL(DB, Xapian::doccount, "QuartzRecordTable::get_doccount", "");
    // Check that we can't overflow (the unsigned test is actually too
    // strict as we can typically assign an unsigned short to a signed long,
    // but this shouldn't actually matter here).
    CASSERT(sizeof(Xapian::doccount) >= sizeof(quartz_tablesize_t));
    CASSERT_TYPE_UNSIGNED(Xapian::doccount);
    CASSERT_TYPE_UNSIGNED(quartz_tablesize_t);
    Xapian::doccount entries = get_entry_count();
    RETURN(entries ? entries - 1 : 0);
}

Xapian::docid
QuartzRecordTable::get_lastdocid() const
{
    DEBUGCALL(DB, Xapian::docid, "QuartzRecordTable::get_lastdocid", "");

    string tag;
    if (!get_exact_entry(METAINFO_KEY, tag)) RETURN(0u);

    Xapian::docid did;
    const char * data = tag.data();
    const char * end = data + tag.size();
    if (!unpack_uint(&data, end, &did)) {
	throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
    }
    RETURN(did);
}

void
QuartzRecordTable::replace_record(const string & data, Xapian::docid did)
{
    DEBUGCALL(DB, void, "QuartzRecordTable::replace_record", data << ", " << did);
    string key(quartz_docid_to_key(did));
    add(key, data);
}

void
QuartzRecordTable::set_total_length_and_lastdocid(quartz_totlen_t totlen,
						  Xapian::docid did)
{
    DEBUGCALL(DB, void, "QuartzRecordTable::set_total_length_and_lastdocid",
			totlen << ", " << did);
    string tag = pack_uint(did);
    tag += pack_uint_last(totlen);
    add(METAINFO_KEY, tag);
}

quartz_totlen_t
QuartzRecordTable::get_total_length() const
{
    DEBUGCALL(DB, quartz_totlen_t, "QuartzRecordTable::get_total_length", "");

    string tag;
    if (!get_exact_entry(METAINFO_KEY, tag)) RETURN(0);

    Xapian::docid did;
    quartz_totlen_t totlen;
    const char * data = tag.data();
    const char * end = data + tag.size();
    if (!unpack_uint(&data, end, &did)) {
	throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
    }
    if (!unpack_uint_last(&data, end, &totlen)) {
	throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
    }
    RETURN(totlen);
}

void
QuartzRecordTable::delete_record(Xapian::docid did)
{
    DEBUGCALL(DB, void, "QuartzRecordTable::delete_record", did);
    if (!del(quartz_docid_to_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + om_tostring(did));
}
