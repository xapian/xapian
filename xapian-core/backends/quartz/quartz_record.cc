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
    key.value = std::string("\000\000", 2);

    QuartzDbTag * tag = table.get_tag(key);
    if (tag == 0) {
	throw OmDatabaseCorruptError("Record containing next free docid not present.");
    }

    return 1;
}

void
QuartzRecordManager::initialise(QuartzDiskTable & table,
				QuartzRevisionNumber new_revision)
{
    std::map<QuartzDbKey, QuartzDbTag *> entries;
    QuartzDbKey key;
    QuartzDbTag tag_nextdoc;
    QuartzDbTag tag_totallen;

    tag_nextdoc.value = pack_uint(1u);
    tag_totallen.value = pack_uint(0u);
    
    key.value = string("\000\000", 2);
    entries[key] = &tag_nextdoc;

    key.value = string("\000\001", 2);
    entries[key] = &tag_totallen;

    table.set_entries(entries, new_revision);
}

om_docid
QuartzRecordManager::add_record(QuartzBufferedTable & table,
				const OmData & data,
				om_doclength doclen)
{
    om_docid did = get_newdocid(table);

    QuartzDbKey key(quartz_docid_to_key(did));

    QuartzDbTag * tag = table.get_or_make_tag(key);

    tag->value = data.value;

    return did;
}

void
QuartzRecordManager::delete_record(QuartzBufferedTable & table,
				   om_docid did)
{
    table.delete_tag(quartz_docid_to_key(did));
}

