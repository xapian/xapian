/* quartz_positionlist.cc
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

#include "omdebug.h"
#include "quartz_postlist.h"
#include "quartz_utils.h"
#include "quartz_table.h"
#include "database.h"

/** The format of a postlist is:
 *
 *  Split into chunks.  Key for first chunk is the termname (encoded as
 *  length - name).  Key for susequent chunks is the same, followed by the
 *  document ID of the first document in the chunk (encoded as length of
 *  representation in first byte, and then docid).
 *
 *  A chunk (except for the first chunk) contains:
 *
 *  1)  difference between final docid in chunk and first docid.
 *  2)  wdf, then doclength of first item.
 *  3)  increment in docid to next item, followed by wdf and doclength of item
 *  4)  (3) repeatedly.
 * 
 *  The first chunk begins with the number of entries, then the docid of the
 *  first document, then continues as other chunks.
 */
QuartzPostList::QuartzPostList(RefCntPtr<const Database> this_db_,
			       om_doclength avlength_,
			       const QuartzTable * table_,
			       const om_termname & tname_)
	: this_db(this_db_),
	  avlength(avlength_),
	  table(table_),
	  tname(tname_),
	  cursor(table->cursor_get()),
	  is_at_end(false)
{
    QuartzDbKey key;
    key.value = pack_string(tname);
    int found = cursor->find_entry(key);
    if (!found) {
	throw OmInvalidDataError("Attempt to open a postlist for term `" +
				 tname +
				 "', which can't be found in database.");
    }
    pos = cursor->current_tag.value.data();
    end = pos + cursor->current_tag.value.size();

    did = 1;

    read_number_of_entries();
    read_first_docid();
    read_start_of_chunk();
}

void
QuartzPostList::make_key(const om_termname & tname,
			 om_docid did,
			 QuartzDbKey & key)
{
    key.value = pack_string(tname);
    key.value += pack_uint_preserving_sort(did);
}

QuartzPostList::~QuartzPostList()
{
    delete cursor;
}


void
QuartzPostList::report_read_error(const char * position)
{
    if (position == 0) {
	// data ran out
	throw OmDatabaseCorruptError("Data ran out unexpectedly when reading posting list.");
    } else {
	// overflow
	throw OmRangeError("Value in posting list too large.");
    }
}

void
QuartzPostList::read_number_of_entries()
{
    if (!unpack_uint(&pos, end, &number_of_entries)) report_read_error(pos);
}

void
QuartzPostList::read_first_docid()
{
    if (!unpack_uint(&pos, end, &did)) report_read_error(pos);
}

void
QuartzPostList::read_start_of_chunk()
{
    om_docid increase_to_last;
    if (!unpack_uint(&pos, end, &increase_to_last)) report_read_error(pos);
    last_did_in_chunk = did + increase_to_last;

    if (!unpack_uint(&pos, end, &wdf)) report_read_error(pos);
    if (!unpack_uint(&pos, end, &doclength)) report_read_error(pos);
}

bool
QuartzPostList::next_in_chunk()
{
    if (pos == end) return false;
    om_docid did_increase;
    if (!unpack_uint(&pos, end, &did_increase)) report_read_error(pos);
    did += did_increase;
    Assert(did <= last_did_in_chunk);

    if (!unpack_uint(&pos, end, &wdf)) report_read_error(pos);
    if (!unpack_uint(&pos, end, &doclength)) report_read_error(pos);

    // Either not at last doc in chunk, or pos == end, but not both.
    Assert(did < last_did_in_chunk || pos == end);
    Assert(pos != end || did == last_did_in_chunk);

    return true;
}

void
QuartzPostList::next_chunk()
{
    cursor->next();
    if (cursor->after_end()) {
	is_at_end = true;
	return;
    }
    const char * keypos = cursor->current_key.value.data();
    const char * keyend = keypos + cursor->current_key.value.size();
    std::string tname_in_key;

    // Check we're still in same postlist
    if (!unpack_string(&keypos, keyend, tname_in_key)) {
	report_read_error(keypos);
    }
    if (tname_in_key != tname) {
	is_at_end = true;
	return;
    }

    om_docid newdid;
    if (!unpack_uint_preserving_sort(&keypos, keyend, &newdid)) {
	report_read_error(keypos);
    }
    if (newdid <= did) {
	throw OmDatabaseCorruptError("Document ID in new chunk of postlist (" +
		om_tostring(newdid) +
		") is not greater than final document ID in previous chunk (" +
		om_tostring(did) + ").");
    }
    did = newdid;

    pos = cursor->current_tag.value.data();
    end = pos + cursor->current_tag.value.size();
    read_start_of_chunk();
}

PositionList *
QuartzPostList::get_position_list()
{
    throw OmUnimplementedError("FIXME");
}

PostList *
QuartzPostList::next(om_weight w_min)
{
    if (!next_in_chunk()) next_chunk();
    return NULL;
}

PostList *
QuartzPostList::skip_to(om_docid did, om_weight w_min)
{
    throw OmUnimplementedError("FIXME");
}



// Methods modifying position lists

