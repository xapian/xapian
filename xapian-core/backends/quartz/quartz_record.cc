/* quartz_record.cc: Records in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "quartz_record.h"
#include "quartz_utils.h"
#include "utils.h"
#include "om/omerror.h"

#define NEXTDOCID_TAG std::string("\000\000", 2)
#define TOTLEN_TAG std::string("\000\001", 2)

OmData
QuartzRecordManager::get_record(QuartzTable & table,
				om_docid did)
{
    QuartzDbKey key(quartz_docid_to_key(did));
    QuartzDbTag tag;

    if (!table.get_exact_entry(key, tag)) {
	throw OmDocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    return OmData(tag.value);
}


om_doccount
QuartzRecordManager::get_doccount(QuartzTable & table)
{   
    // FIXME: check that the sizes of these types (om_doccount and
    // quartz_tablesize_t) are compatible.
    return table.get_entry_count() - 2;
}

om_docid
QuartzRecordManager::get_newdocid(QuartzBufferedTable & table)
{
    QuartzDbKey key;
    key.value = NEXTDOCID_TAG;

    QuartzDbTag * tag = table.get_tag(key);
    if (tag == 0) {
	throw OmDatabaseCorruptError("Record containing next free docid not present.");
    }

    om_docid did;
    const char * data = tag->value.data();
    const char * end = data + tag->value.size();
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

    tag->value = pack_uint(did + 1);

    return did;
}

void
QuartzRecordManager::initialise(QuartzDiskTable & table,
				QuartzRevisionNumber new_revision)
{
    std::map<QuartzDbKey, QuartzDbTag *> entries;
    QuartzDbKey key;
    QuartzDbTag tag_nextdoc;
    QuartzDbTag tag_totallen;

    key.value = NEXTDOCID_TAG;
    tag_nextdoc.value = pack_uint(1u);
    entries[key] = &tag_nextdoc;

    key.value = TOTLEN_TAG;
    tag_totallen.value = pack_uint(0u);
    entries[key] = &tag_totallen;

    table.set_entries(entries, new_revision);
}

om_docid
QuartzRecordManager::add_record(QuartzBufferedTable & table,
				const OmData & data,
				om_doclength new_doclen)
{
    om_docid did = get_newdocid(table);

    QuartzDbKey key(quartz_docid_to_key(did));
    QuartzDbTag * tag = table.get_or_make_tag(key);
    tag->value = data.value;

    return did;
}

void
QuartzRecordManager::modify_total_length(QuartzBufferedTable & table,
					 quartz_doclen_t old_doclen,
					 quartz_doclen_t new_doclen)
{
    QuartzDbKey key;
    key.value = TOTLEN_TAG;
    QuartzDbTag * tag = table.get_or_make_tag(key);

    quartz_totlen_t totlen;
    const char * data = tag->value.data();
    const char * end = data + tag->value.size();
    bool success = unpack_uint(&data, end, &totlen);
    if (!success) {
	if (data == end) { // Overflow
	    throw OmRangeError("Total document length is out of range.");
	} else { // Number ran out
	    throw OmDatabaseCorruptError("Record containing total document length is corrupt.");
	}
    }

    quartz_totlen_t newlen = totlen + new_doclen;
    if (newlen < totlen)
	throw OmRangeError("New total document length is out of range.");
    if (newlen < old_doclen)
	throw OmDatabaseCorruptError("Total document length is less than claimed old document length");
    newlen -= old_doclen;
    tag->value = pack_uint(newlen);
}

om_totlength
QuartzRecordManager::get_total_length(QuartzTable & table)
{
    QuartzDbKey key;
    key.value = TOTLEN_TAG;
    QuartzDbTag tag;

    if (!table.get_exact_entry(key, tag)) {
	throw OmDatabaseCorruptError("Record containing total document length is missing.");
    }

    quartz_totlen_t totlen;
    const char * data = tag.value.data();
    const char * end = data + tag.value.size();
    bool success = unpack_uint(&data, end, &totlen);
    if (!success) {
	if (data == end) { // Overflow
	    throw OmRangeError("Total document length is out of range.");
	} else { // Number ran out
	    throw OmDatabaseCorruptError("Record containing total document length is corrupt.");
	}
    }

    return (om_totlength) totlen;
}

void
QuartzRecordManager::delete_record(QuartzBufferedTable & table,
				   om_docid did)
{
    table.delete_tag(quartz_docid_to_key(did));
}

