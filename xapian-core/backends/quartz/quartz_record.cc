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

// Tag used to store next free docid and total length
static const string METAINFO_TAG("\000\000", 2);

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

    if (entries < 1) RETURN(0);
    RETURN(entries - 1);
}

om_docid
QuartzRecordManager::get_newdocid(QuartzBufferedTable & table)
{
    DEBUGCALL_STATIC(DB, om_docid, "QuartzRecordManager::get_newdocid", "[table]");
    string * tag = table.get_or_make_tag(METAINFO_TAG);

    om_docid did;
    quartz_totlen_t totlen;
    if (tag->empty()) {
	did = 1u;
	totlen = 0u;
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	if (!unpack_uint(&data, end, &did)) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (!unpack_uint(&data, end, &totlen)) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (data != end) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
	++did;
	if (did == 0) {
	    throw OmRangeError("Next document number is out of range.");
	}
    }
    *tag = pack_uint(did);
    *tag += pack_uint(totlen);

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
    string * tag = table.get_or_make_tag(METAINFO_TAG);

    om_docid did;
    quartz_totlen_t totlen;
    if (tag->empty()) {
	did = 1u;
	totlen = 0u;
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	if (!unpack_uint(&data, end, &did)) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (!unpack_uint(&data, end, &totlen)) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (data != end) {
	    throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
	}
    }
    
    if (totlen < old_doclen)
	throw OmDatabaseCorruptError("Total document length is less than claimed old document length");

    totlen -= old_doclen;
    quartz_totlen_t newlen = totlen + new_doclen;

    if (newlen < totlen)
	throw OmRangeError("New total document length is out of range.");

    *tag = pack_uint(did);
    *tag += pack_uint(newlen);
}

om_totlength
QuartzRecordManager::get_total_length(QuartzTable & table)
{
    DEBUGCALL_STATIC(DB, om_totlength, "QuartzRecordManager::get_total_length", "QuartzTable &");
    string tag;
    if (!table.get_exact_entry(METAINFO_TAG, tag)) RETURN(0u);

    om_docid did;
    quartz_totlen_t totlen;
    const char * data = tag.data();
    const char * end = data + tag.size();
    if (!unpack_uint(&data, end, &did)) {
	throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
    }
    if (!unpack_uint(&data, end, &totlen)) {
	throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
    }
    if (data != end) {
	throw OmDatabaseCorruptError("Record containing meta information is corrupt.");
    }
    RETURN((om_totlength)totlen);
}

void
QuartzRecordManager::delete_record(QuartzBufferedTable & table,
				   om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::delete_record", "[table], " << did);
    table.delete_tag(quartz_docid_to_key(did));
}
