/* quartz_record.cc: Records in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include "om/omerror.h"
#include "omassert.h"
#include "omdebug.h"
using std::string;

#define NEXTDOCID_TAG std::string("\000\000", 2)
#define TOTLEN_TAG std::string("\000\001", 2)

string
QuartzRecordManager::get_record(QuartzTable & table, om_docid did)
{
    DEBUGCALL_STATIC(DB, string, "QuartzRecordManager::get_record", "[table], " << did);
    string key(quartz_docid_to_key(did));
    string tag;

    if (!table.get_exact_entry(key, tag)) {
	throw OmDocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    RETURN(tag);
}


om_doccount
QuartzRecordManager::get_doccount(QuartzTable & table)
{   
    DEBUGCALL_STATIC(DB, om_doccount, "QuartzRecordManager::get_doccount", "[table]");
    // FIXME: check that the sizes of these types (om_doccount and
    // quartz_tablesize_t) are compatible.
    om_doccount entries = table.get_entry_count();

    Assert(entries != 1);
    if (entries < 2) RETURN(0);
    RETURN(entries - 2);
}

om_docid
QuartzRecordManager::get_newdocid(QuartzBufferedTable & table)
{
    DEBUGCALL_STATIC(DB, om_docid, "QuartzRecordManager::get_newdocid", "[table]");
    string key;
    key = NEXTDOCID_TAG;

    string * tag = table.get_or_make_tag(key);

    om_docid did;
    if (tag->empty()) {
	did = 1u;

	// Ensure that other informational tag is present.
	string key2;
	key2 = TOTLEN_TAG;
	(void) table.get_or_make_tag(key2);
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	bool success = unpack_uint(&data, end, &did);
	if (!success) {
	    if (data == end) { // Overflow
		throw OmRangeError("Next document number is out of range.");
	    } else { // Number ran out
		throw OmDatabaseCorruptError("Record containing next free docid is corrupt.");
	    }
	}
	if (data != end) {
	    // Junk data at end of record
	    throw OmDatabaseCorruptError("Junk data at end of record containing next free docid.");
	}
    }

    *tag = pack_uint(did + 1);

    RETURN(did);
}

om_docid
QuartzRecordManager::add_record(QuartzBufferedTable & table,
				const string & data)
{
    DEBUGCALL_STATIC(DB, om_docid, "QuartzRecordManager::add_record", "[table], " << data);
    om_docid did = get_newdocid(table);

    string key(quartz_docid_to_key(did));
    string * tag = table.get_or_make_tag(key);
    *tag = data;

    RETURN(did);
}

void
QuartzRecordManager::replace_record(QuartzBufferedTable & table,
				    const string & data,
				    om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::replace_record", "[table], " << data << ", " << did);
    string key(quartz_docid_to_key(did));
    string * tag = table.get_or_make_tag(key);
    *tag = data;
}

void
QuartzRecordManager::modify_total_length(QuartzBufferedTable & table,
					 quartz_doclen_t old_doclen,
					 quartz_doclen_t new_doclen)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::modify_total_length", "[table], " << old_doclen << ", " << new_doclen);
    string key;
    key = TOTLEN_TAG;
    string * tag = table.get_or_make_tag(key);

    quartz_totlen_t totlen;
    if (tag->empty()) {
	totlen = 0u;

	// Ensure that other informational tag is present.
	string key2;
	key2 = NEXTDOCID_TAG;
	(void) table.get_or_make_tag(key2);
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	bool success = unpack_uint(&data, end, &totlen);
	if (!success) {
	    if (data == end) { // Overflow
		throw OmRangeError("Total document length is out of range.");
	    } else { // Number ran out
		throw OmDatabaseCorruptError("Record containing total document length is corrupt.");
	    }
	}
	if (data != end) {
	    // Junk data at end of record
	    throw OmDatabaseCorruptError("Junk data at end of record containing total document length.");
	}
    }

    quartz_totlen_t newlen = totlen + new_doclen;
    if (newlen < totlen)
	throw OmRangeError("New total document length is out of range.");
    if (newlen < old_doclen)
	throw OmDatabaseCorruptError("Total document length is less than claimed old document length");
    newlen -= old_doclen;
    *tag = pack_uint(newlen);
}

om_totlength
QuartzRecordManager::get_total_length(QuartzTable & table)
{
    DEBUGCALL_STATIC(DB, om_totlength, "QuartzRecordManager::get_total_length", "QuartzTable &");
    string key;
    key = TOTLEN_TAG;
    string tag;

    if (!table.get_exact_entry(key, tag)) {
	RETURN(0u);
    }

    quartz_totlen_t totlen;
    const char * data = tag.data();
    const char * end = data + tag.size();
    bool success = unpack_uint(&data, end, &totlen);
    if (!success) {
	if (data == end) { // Overflow
	    throw OmRangeError("Total document length is out of range.");
	} else { // Number ran out
	    throw OmDatabaseCorruptError("Record containing total document length is corrupt.");
	}
    }

    RETURN((om_totlength) totlen);
}

void
QuartzRecordManager::delete_record(QuartzBufferedTable & table,
				   om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::delete_record", "[table], " << did);
    table.delete_tag(quartz_docid_to_key(did));
}

