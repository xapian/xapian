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

// Static functions

/// Make a key for accessing the postlist.
static void make_key(const om_termname & tname, om_docid did, QuartzDbKey & key)
{
    key.value = pack_string(tname);
    key.value += pack_uint_preserving_sort(did);
}

/** Make the data to go at the start of the very first chunk.
 */
static inline std::string make_start_of_first_chunk(om_termcount entries,
						    om_termcount collectionfreq,
						    om_docid new_did)
{
    return pack_uint(entries) + pack_uint(collectionfreq) + pack_uint(new_did);
}

static std::string make_wdf_and_length(om_termcount wdf,
				       quartz_doclen_t doclength)
{
    return pack_uint(wdf) + pack_uint(doclength);
}

/** Make the data to go at the start of a standard chunk.
 */
static inline std::string make_start_of_chunk(bool new_is_last_chunk,
					      om_docid new_first_did,
					      om_docid new_final_did)
{
    Assert(new_final_did >= new_first_did);
    return pack_bool(new_is_last_chunk) +
	    pack_uint(new_final_did - new_first_did);
}

static void new_postlist(QuartzBufferedTable * bufftable,
			 const om_termname & tname,
			 om_docid new_did,
			 om_termcount new_wdf,
			 om_termcount new_doclen)
{
    QuartzDbKey key;
    key.value = pack_string(tname);
    QuartzDbTag * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(tag->value.size() == 0);

    tag->value = make_start_of_first_chunk(1u, new_wdf, new_did);
    tag->value += make_start_of_chunk(true, new_did, new_did);
    tag->value += make_wdf_and_length(new_wdf, new_doclen);
}

static void new_chunk(QuartzBufferedTable * bufftable,
		      const om_termname & tname,
		      bool is_last_chunk,
		      om_docid new_did,
		      om_termcount new_wdf,
		      quartz_doclen_t new_doclen)
{
    QuartzDbKey key;
    make_key(tname, new_did, key);
    QuartzDbTag * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(tag->value.size() == 0);

    tag->value = make_start_of_chunk(true, new_did, new_did);
    tag->value += make_wdf_and_length(new_wdf, new_doclen);
}

static void write_start_of_chunk(std::string & chunk,
				 unsigned int start_of_chunk_header,
				 unsigned int end_of_chunk_header,
				 bool is_last_chunk,
				 om_docid first_did_in_chunk,
				 om_docid last_did_in_chunk)
{
    Assert(last_did_in_chunk >= first_did_in_chunk);
    om_docid increase_to_last = last_did_in_chunk - first_did_in_chunk;

    chunk.replace(start_of_chunk_header,
		  end_of_chunk_header - start_of_chunk_header,
		  pack_bool(is_last_chunk) + pack_uint(increase_to_last));
}

static std::string make_did_increase(om_docid new_did,
				     om_docid last_did_in_chunk)
{
    Assert(new_did > last_did_in_chunk);
    return pack_uint(new_did - last_did_in_chunk);
}

static void append_to_chunk(std::string & chunk,
			    om_docid new_did,
			    om_docid last_did_in_chunk,
			    om_termcount new_wdf,
			    om_termcount new_doclength)
{
    chunk += make_did_increase(new_did, last_did_in_chunk);
    chunk += make_wdf_and_length(new_wdf, new_doclength);
}

/// Report an error when reading the posting list.
static void report_read_error(const char * position)
{
    if (position == 0) {
	// data ran out
	throw OmDatabaseCorruptError("Data ran out unexpectedly when reading posting list.");
    } else {
	// overflow
	throw OmRangeError("Value in posting list too large.");
    }
}

static void adjust_counts(QuartzBufferedTable * bufftable,
			  const om_termname & tname,
			  om_termcount entries_increase,
			  om_termcount entries_decrease,
			  om_termcount collection_freq_increase,
			  om_termcount collection_freq_decrease)
{
    QuartzDbKey key;
    key.value = pack_string(tname);
    QuartzDbTag * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(tag->value.size() != 0);

    const char * tagpos = tag->value.data();
    const char * tagend = tagpos + tag->value.size();
    om_termcount number_of_entries;
    om_termcount collection_freq;
    if (!unpack_uint(&tagpos, tagend, &number_of_entries))
	report_read_error(tagpos);
    if (!unpack_uint(&tagpos, tagend, &collection_freq))
	report_read_error(tagpos);

    number_of_entries += entries_increase;
    number_of_entries -= entries_decrease;
    collection_freq += collection_freq_increase;
    collection_freq -= collection_freq_decrease;

    tag->value.replace(0, tagpos - tag->value.data(), 
		       pack_uint(number_of_entries) +
		       pack_uint(collection_freq));
}


/** Read the number of entries in the posting list. 
 *  This must only be called when *posptr is pointing to the start of
 *  the first chunk of the posting list.
 */
static void read_number_of_entries(const char ** posptr,
				   const char * end,
				   om_termcount * number_of_entries_ptr,
				   om_termcount * collection_freq_ptr)
{
    if (!unpack_uint(posptr, end, number_of_entries_ptr))
	report_read_error(*posptr);
    if (!unpack_uint(posptr, end, collection_freq_ptr))
	report_read_error(*posptr);
}

/// Read the docid of the first entry in the posting list.
static void read_first_docid(const char ** posptr,
			     const char * end,
			     om_docid * did_ptr)
{
    if (!unpack_uint(posptr, end, did_ptr))
	report_read_error(*posptr);
}

/// Read the start of the first chunk in the posting list.
static void read_start_of_first_chunk(const char ** posptr,
				      const char * end,
				      om_termcount * number_of_entries_ptr,
				      om_termcount * collection_freq_ptr,
				      om_docid * did_ptr)
{
    read_number_of_entries(posptr, end,
			   number_of_entries_ptr, collection_freq_ptr);
    read_first_docid(posptr, end, did_ptr);
}

static void read_did_increase(const char ** posptr,
			      const char * end,
			      om_docid * did_ptr)
{
    om_docid did_increase;
    if (!unpack_uint(posptr, end, &did_increase)) report_read_error(*posptr);
    *did_ptr += did_increase;
}

/// Read the wdf and the document length of an item.
static void read_wdf_and_length(const char ** posptr,
				const char * end,
				om_termcount * wdf_ptr,
				quartz_doclen_t * doclength_ptr)
{
    if (!unpack_uint(posptr, end, wdf_ptr)) report_read_error(*posptr);
    if (!unpack_uint(posptr, end, doclength_ptr)) report_read_error(*posptr);
}

/// Read the start of a chunk, including the first item in it.
static void read_start_of_chunk(const char ** posptr,
				const char * end,
				om_docid first_did_in_chunk,
				bool * is_last_chunk_ptr,
				om_docid * last_did_in_chunk_ptr)
{
    // Read whether this is the last chunk
    if (!unpack_bool(posptr, end, is_last_chunk_ptr))
	report_read_error(*posptr);

    // Read what the final document ID in this chunk is.
    om_docid increase_to_last;
    if (!unpack_uint(posptr, end, &increase_to_last))
	report_read_error(*posptr);
    *last_did_in_chunk_ptr = first_did_in_chunk + increase_to_last;
}

/** The format of a postlist is:
 *
 *  Split into chunks.  Key for first chunk is the termname (encoded as
 *  length - name).  Key for susequent chunks is the same, followed by the
 *  document ID of the first document in the chunk (encoded as length of
 *  representation in first byte, and then docid).
 *
 *  A chunk (except for the first chunk) contains:
 *
 *  1)  bool - true if this is the last chunk.
 *  2)  difference between final docid in chunk and first docid.
 *  3)  wdf, then doclength of first item.
 *  4)  increment in docid to next item, followed by wdf and doclength of item
 *  5)  (4) repeatedly.
 * 
 *  The first chunk begins with the number of entries, then the docid of the
 *  first document, then has the header of a standard chunk.
 */
QuartzPostList::QuartzPostList(RefCntPtr<const Database> this_db_,
			       const QuartzTable * table_,
			       const QuartzTable * positiontable_,
			       const om_termname & tname_)
	: this_db(this_db_),
	  table(table_),
	  positiontable(positiontable_),
	  tname(tname_),
	  cursor(table->cursor_get()),
	  is_at_end(false),
	  have_started(false)
{
    DEBUGCALL(DB, void, "QuartzPostList::QuartzPostList",
	      this_db_.get() << ", " << table_ << ", " <<
	      positiontable_ << ", " << tname_);
    QuartzDbKey key;
    key.value = pack_string(tname);
    int found = cursor->find_entry(key);
    if (!found) {
	number_of_entries = 0;
	collection_freq = 0;
	is_at_end = true;
	pos = 0;
	end = 0;
	first_did_in_chunk = 0;
	last_did_in_chunk = 0;
	return;
    }
    pos = cursor->current_tag.value.data();
    end = pos + cursor->current_tag.value.size();

    read_start_of_first_chunk(&pos, end,
			      &number_of_entries, &collection_freq, &did);
    first_did_in_chunk = did;
    read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk,
			&last_did_in_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

QuartzPostList::~QuartzPostList()
{
    DEBUGCALL(DB, void, "QuartzPostList::~QuartzPostList", "");
    delete cursor;
}



bool
QuartzPostList::next_in_chunk()
{
    DEBUGCALL(DB, bool, "QuartzPostList::next_in_chunk", "");
    if (pos == end) RETURN(false);

    read_did_increase(&pos, end, &did);
    read_wdf_and_length(&pos, end, &wdf, &doclength);

    // Either not at last doc in chunk, or pos == end, but not both.
    Assert(did <= last_did_in_chunk);
    Assert(did < last_did_in_chunk || pos == end);
    Assert(pos != end || did == last_did_in_chunk);

    RETURN(true);
}

void
QuartzPostList::next_chunk()
{
    DEBUGCALL(DB, void, "QuartzPostList::next_chunk", "");
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
				     tname + "').");
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

    first_did_in_chunk = did;
    read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk,
			&last_did_in_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

PositionList *
QuartzPostList::get_position_list()
{
    DEBUGCALL(DB, PositionList *, "QuartzPostList::get_position_list", "");

    positionlist.read_data(positiontable, did, tname);

    RETURN(&positionlist);
}

PostList *
QuartzPostList::next(om_weight w_min)
{
    DEBUGCALL(DB, PostList *, "QuartzPostList::next", w_min);

    if (!have_started) {
	have_started = true;
    } else {
	if (!next_in_chunk()) next_chunk();
    }

    DEBUGLINE(DB, std::string("Moved to ") <<
	      (is_at_end ? std::string("end.") : std::string("docid, wdf, doclength = ") +
	       om_tostring(did) + ", " + om_tostring(wdf) + ", " +
	       om_tostring(doclength) + "."));
    
    RETURN(NULL);
}

bool
QuartzPostList::current_chunk_contains(om_docid desired_did)
{
    DEBUGCALL(DB, bool, "QuartzPostList::current_chunk_contains", desired_did);
    if (desired_did >= first_did_in_chunk &&
	desired_did <= last_did_in_chunk) {
	RETURN(true);
    } else {
	RETURN(false);
    }
}

void
QuartzPostList::move_to_chunk_containing(om_docid desired_did)
{
    DEBUGCALL(DB, void,
	      "QuartzPostList::move_to_chunk_containing", desired_did);
    QuartzDbKey key;
    make_key(tname, desired_did, key);
    (void) cursor->find_entry(key);
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.value.data();
    const char * keyend = keypos + cursor->current_key.value.size();
    std::string tname_in_key;

    // Check we're still in same postlist
    if (!unpack_string(&keypos, keyend, tname_in_key)) {
	report_read_error(keypos);
    }
    if (tname_in_key != tname) {
	// This should only happen if the postlist doesn't exist at all.
	is_at_end = true;
	is_last_chunk = true;
	return;
    }
    is_at_end = false;

    pos = cursor->current_tag.value.data();
    end = pos + cursor->current_tag.value.size();

    if (keypos == keyend) {
	// In first chunk
#ifdef MUS_DEBUG
	om_termcount old_number_of_entries = number_of_entries;
	om_termcount old_collection_freq = collection_freq;
	read_start_of_first_chunk(&pos, end,
				  &number_of_entries, &collection_freq, &did);
#else
	read_start_of_first_chunk(&pos, end, 0, 0, &did);
#endif
	Assert(old_number_of_entries == number_of_entries);
	Assert(old_collection_freq == collection_freq);
    } else {
	// In normal chunk
	if (!unpack_uint_preserving_sort(&keypos, keyend, &did)) {
	    report_read_error(keypos);
	}
    }

    first_did_in_chunk = did;
    read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk,
			&last_did_in_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

bool
QuartzPostList::move_forward_in_chunk_to_at_least(om_docid desired_did)
{
    DEBUGCALL(DB, bool,
	      "QuartzPostList::move_forward_in_chunk_to_at_least", desired_did);
    if (desired_did > last_did_in_chunk) {
	pos = end;
	RETURN(false);
    }
    while (did < desired_did) {
	// FIXME: perhaps we don't need to decode the wdf and documnet length
	// for documents we're skipping past.
	bool at_end_of_chunk = !next_in_chunk();
	if (at_end_of_chunk) RETURN(false);
    }
    RETURN(true);
}

PostList *
QuartzPostList::skip_to(om_docid desired_did, om_weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "QuartzPostList::skip_to", desired_did << ", " << w_min);
    // We've started now - if we hadn't already, we're already positioned
    // at start so there's no need to actually do anything.
    have_started = true;

    // Don't skip back, and don't need to do anthing if already there.
    if (desired_did <= did) RETURN(NULL);

    move_to(desired_did);

    DEBUGLINE(DB, std::string("Skipped to ") <<
	      (is_at_end ? std::string("end.") : std::string("docid, wdf, doclength = ") +
	       om_tostring(did) + ", " + om_tostring(wdf) + ", " +
	       om_tostring(doclength) + "."));
    
    RETURN(NULL);
}

void
QuartzPostList::move_to(om_docid desired_did)
{
    DEBUGCALL(DB, void, "QuartzPostList::move_to", desired_did);

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
}

std::string
QuartzPostList::get_description() const
{
    return tname + ":" + om_tostring(number_of_entries);
}


void
QuartzPostList::add_entry(QuartzBufferedTable * bufftable,
			  const om_termname & tname,
			  om_docid new_did,
			  om_termcount new_wdf,
			  quartz_doclen_t new_doclen)
{
    DEBUGCALL_STATIC(DB, void, "QuartzPostList::add_entry",
		     bufftable << ", " <<
		     tname << ", " <<
		     new_did << ", " <<
		     new_wdf << ", " <<
		     new_doclen);
    
    // How big should chunks in the posting list be?  (They will grow
    // slightly bigger than this, but not more than a few bytes extra)
    unsigned int chunksize = 2048;

    QuartzDbKey key;
    make_key(tname, new_did, key);
    AutoPtr<QuartzCursor> cursor(bufftable->cursor_get());

    cursor->find_entry(key);
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.value.data();
    const char * keyend = keypos + cursor->current_key.value.size();
    std::string tname_in_key;

    // Read the termname.
    if (keypos != keyend)
	if (!unpack_string(&keypos, keyend, tname_in_key))
	    report_read_error(keypos);

    if (tname_in_key != tname) {
	// This should only happen if the postlist doesn't exist at all.
	new_postlist(bufftable, tname, new_did, new_wdf, new_doclen);
    } else {
	bool is_first_chunk = (keypos == keyend);

	QuartzDbTag * tag = bufftable->get_or_make_tag(cursor->current_key);
	Assert(tag != 0);
	Assert(tag->value.size() != 0);

	// Determine whether we're adding to the end of the chunk

	const char * tagpos = tag->value.data();
	const char * tagend = tagpos + tag->value.size();

	om_docid first_did_in_chunk;
	if (is_first_chunk) {
	    read_start_of_first_chunk(&tagpos, tagend,
				      0, 0, &first_did_in_chunk);
	} else {
	    if (!unpack_uint_preserving_sort(&keypos, keyend,
					     &first_did_in_chunk))
		report_read_error(keypos);
	}
	bool is_last_chunk;
	om_docid last_did_in_chunk;

	unsigned int start_of_chunk_header = tagpos - tag->value.data();
	read_start_of_chunk(&tagpos, tagend, first_did_in_chunk,
			    &is_last_chunk, &last_did_in_chunk);
	unsigned int end_of_chunk_header = tagpos - tag->value.data();

	// Have read in data needed - now add item
	if (!is_last_chunk ||
	    !(last_did_in_chunk < new_did)) {
	    // Add in middle of postlist.
	    throw OmUnimplementedError("Setting entries only currently implemented at end of postlist.");
	} else {
	    // Append
	    if (tag->value.size() > chunksize) {
		new_chunk(bufftable, tname, is_last_chunk,
			  new_did, new_wdf, new_doclen);

		// Sort out previous chunk
		write_start_of_chunk(tag->value,
				     start_of_chunk_header,
				     end_of_chunk_header,
				     false,
				     first_did_in_chunk,
				     last_did_in_chunk);
	    } else {
		append_to_chunk(tag->value, new_did, last_did_in_chunk,
				new_wdf, new_doclen);
		last_did_in_chunk = new_did;
		write_start_of_chunk(tag->value,
				     start_of_chunk_header,
				     end_of_chunk_header,
				     is_last_chunk,
				     first_did_in_chunk,
				     last_did_in_chunk);
	    }
	}

	adjust_counts(bufftable, tname, 1, 0, new_wdf, 0);
    }
}

void
QuartzPostList::delete_entry(QuartzBufferedTable * bufftable,
			     const om_termname & tname,
			     om_docid did_to_delete)
{
    DEBUGCALL_STATIC(DB, void, "QuartzPostList::delete_entry",
		     bufftable << ", " <<
		     tname << ", " <<
		     did_to_delete);

    // Get chunk containing entry
    QuartzDbKey key;
    make_key(tname, did_to_delete, key);
    AutoPtr<QuartzCursor> cursor(bufftable->cursor_get());

    cursor->find_entry(key);
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.value.data();
    const char * keyend = keypos + cursor->current_key.value.size();
    std::string tname_in_key;

    // Read the termname.
    if (keypos != keyend)
	if (!unpack_string(&keypos, keyend, tname_in_key))
	    report_read_error(keypos);

    if (tname_in_key != tname) {
	// This should only happen if the postlist doesn't exist at all.
	return;
    } else {
	bool is_first_chunk = (keypos == keyend);

	// Get the appropriate tag and set pointers to iterate through it
	QuartzDbTag * tag = bufftable->get_or_make_tag(cursor->current_key);
	Assert(tag != 0);
	Assert(tag->value.size() != 0);
	const char * tagpos = tag->value.data();
	const char * tagend = tagpos + tag->value.size();

	// Get the first document ID in the chunk.
	om_docid first_did_in_chunk;
	if (is_first_chunk) {
	    read_start_of_first_chunk(&tagpos, tagend,
				      0, 0, &first_did_in_chunk);
	} else {
	    if (!unpack_uint_preserving_sort(&keypos, keyend,
					     &first_did_in_chunk))
		report_read_error(keypos);
	}

	// Read the chunk header
	bool is_last_chunk;
	om_docid last_did_in_chunk;
	unsigned int start_of_chunk_header = tagpos - tag->value.data();
	read_start_of_chunk(&tagpos, tagend, first_did_in_chunk,
			    &is_last_chunk, &last_did_in_chunk);
	unsigned int end_of_chunk_header = tagpos - tag->value.data();

	// Read first wdf and length
	om_termcount wdf;
	quartz_doclen_t doclength;
	read_wdf_and_length(&tagpos, tagend, &wdf, &doclength);

	// Check if item is in the chunk's range
	if (last_did_in_chunk < did_to_delete) return; // Entry not in range

	// Find item to delete
	om_docid currdid = first_did_in_chunk;
	om_docid prevdid = currdid;
	unsigned int start_of_item;

	while (currdid < did_to_delete) {
	    start_of_item = tagpos - tag->value.data();
	    prevdid = currdid;

	    read_did_increase(&tagpos, tagend, &currdid);
	    read_wdf_and_length(&tagpos, tagend, &wdf, &doclength);

	    // Either not at last doc in chunk, or tagpos == tagend, but not
	    // both.
	    Assert(currdid <= last_did_in_chunk);
	    Assert(currdid < last_did_in_chunk || tagpos == tagend);
	    Assert(tagpos != tagend || currdid == last_did_in_chunk);
	}

	unsigned int end_of_item = tagpos - tag->value.data();

	// Delete item
	// FIXME: implement
	
	// Sort out chunk header
	write_start_of_chunk(tag->value,
			     start_of_chunk_header,
			     end_of_chunk_header,
			     is_last_chunk,
			     first_did_in_chunk,
			     last_did_in_chunk);

	adjust_counts(bufftable, tname, 0, 1, 0, wdf);
    }
}

