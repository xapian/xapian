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
 *
 *  2)  bool - true if this is the last chunk.
 *  3)  wdf, then doclength of first item.
 *  4)  increment in docid to next item, followed by wdf and doclength of item
 *  5)  (4) repeatedly.
 * 
 *  The first chunk begins with the number of entries, then the docid of the
 *  first document, then continues from (2) as other chunks.
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
	  is_at_end(false),
	  have_started(false)
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

    read_number_of_entries();
    read_first_docid();
    read_start_of_chunk();
    read_wdf_and_length();
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
    // Read whether this is the last chunk
    if (!unpack_bool(&pos, end, &is_last_chunk)) report_read_error(pos);

    // Remember the first document ID
    first_did_in_chunk = did;

    // Read what the final document ID in this chunk is.
    om_docid increase_to_last;
    if (!unpack_uint(&pos, end, &increase_to_last)) report_read_error(pos);
    last_did_in_chunk = first_did_in_chunk + increase_to_last;
}

void
QuartzPostList::read_wdf_and_length()
{
    // Read the wdf and length of an item
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

    read_wdf_and_length();

    // Either not at last doc in chunk, or pos == end, but not both.
    Assert(did < last_did_in_chunk || pos == end);
    Assert(pos != end || did == last_did_in_chunk);

    return true;
}

void
QuartzPostList::next_chunk()
{
    if (is_last_chunk) {
	is_at_end = true;
	return;
    }

    cursor->next();
    if (cursor->after_end()) {
	is_at_end = true;
	throw OmDatabaseCorruptError("Unexpected end of posting list (for `" +
				     tname + "'.");
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
	throw OmDatabaseCorruptError("Unexpected end of posting list (for `" +
				     tname + "'.");
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
    read_wdf_and_length();
}

PositionList *
QuartzPostList::get_position_list()
{
    throw OmUnimplementedError("FIXME");
}

PostList *
QuartzPostList::next(om_weight w_min)
{
    if (!have_started) {
	have_started = true;
    } else {
	if (!next_in_chunk()) next_chunk();
    }
    return NULL;
}

bool
QuartzPostList::current_chunk_contains(om_docid desired_did)
{
    if (desired_did >= first_did_in_chunk &&
	desired_did <= last_did_in_chunk) {
	return true;
    } else {
	return false;
    }
}

void
QuartzPostList::move_to_chunk_containing(om_docid desired_did)
{
    QuartzDbKey key;
    make_key(tname, desired_did, key);
    (void) cursor->find_entry(key);
    Assert(!cursor->after_end());

    
    
    throw OmUnimplementedError("QuartzPostList::move_to_chunk_containing() not yet implemented");
}

bool
QuartzPostList::move_forward_in_chunk_to_at_least(om_docid desired_did)
{
    if (desired_did > last_did_in_chunk) {
	pos = end;
	return false;
    }
    while (did < desired_did) {
	// FIXME: perhaps we don't need to decode the wdf and documnet length
	// for documents we're skipping past.
	bool at_end_of_chunk = !next_in_chunk();
	if (at_end_of_chunk) return false;
    }
    return true;
}

PostList *
QuartzPostList::skip_to(om_docid desired_did, om_weight w_min)
{
    // Don't skip back, and don't need to do anthing if already there.
    if (desired_did <= did) return NULL;

    // We've started now - we're already positioned at start so there's no
    // need to actually do anything.
    have_started = true;

    // Move to correct chunk
    if (!current_chunk_contains(desired_did)) {
	move_to_chunk_containing(desired_did);
	if (!current_chunk_contains(desired_did)) {
	    // Possible, since desired_did might be after end of
	    // this chunk and before the next.
	    next_chunk();
	    // Might be at_end now - this is why we need the test before moving forward in chunk.
	}
    }

    // Move to correct position in chunk
    if (!is_at_end) {
#ifdef MUS_DEBUG
	bool have_document =
#else
	(void)
#endif
		move_forward_in_chunk_to_at_least(desired_did);
	Assert(have_document);
    }

    return NULL;
}

std::string
QuartzPostList::get_description() const
{
    return tname + ":" + om_tostring(number_of_entries);
}


// Methods modifying position lists

