/* quartz_postlist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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
#include "omdebug.h"
#include "quartz_postlist.h"
#include "quartz_utils.h"
#include "quartz_table.h"
#include "database.h"

/** PostlistChunkReader is essentially an iterator wrapper
 *  around a postlist chunk.  It simply iterates through the
 *  entries in a postlist.
 */
class PostlistChunkReader {
    public:
	/** Initialise the postlist chunk reader.
	 *
	 *  @param keypos A pointer to the key string just after the termname.
	 *  @param keyend A pointer to one-past-the-end of the key value.
	 *  @param chunk  The string value of this chunk.
	 */
	PostlistChunkReader(const char *keypos,
			    const char *keyend,
			    const string &chunk);

	Xapian::docid get_docid() const {
	    return did;
	}
	Xapian::termcount get_wdf() const {
	    return wdf;
	}
	quartz_doclen_t get_doclength() const {
	    return doclength;
	}

	// Return global chunk data
	bool get_is_last_chunk() const {
	    return is_last_chunk;
	}

	Xapian::termcount get_collectionfreq() const {
	    return collectionfreq;
	}

	Xapian::termcount get_number_of_entries() const {
	    return number_of_entries;
	}

	bool is_at_end() const {
	    return at_end;
	}

	/** Advance to the next entry.  Set at_end if we run off the end.
	 */
	void next();

    private:
	string chunk;
	const char *pos;
	const char *end;

	bool at_end;

	Xapian::docid did;
	Xapian::termcount wdf;
	quartz_doclen_t doclength;

	Xapian::termcount collectionfreq;
	Xapian::termcount number_of_entries;
	bool is_last_chunk;
	Xapian::docid last_did_in_chunk;
};

/** PostlistChunkWriter is a wrapper which acts roughly as an
 *  output iterator on a postlist chunk, taking care of the
 *  messy details.  It's intended to be used with deletion and
 *  replacing of entries, not for adding to the end, when it's
 *  not really needed.
 */
class PostlistChunkWriter {
    public:
	PostlistChunkWriter(const string &key,
			    bool is_first_chunk_,
			    const string &tname_,
			    Xapian::termcount collectionfreq_,
			    bool is_last_chunk_,
			    Xapian::termcount number_of_entries_);

	/// Append an entry to this chunk.
	void append(Xapian::docid did, Xapian::termcount wdf, quartz_doclen_t doclen);

	/** Signal that an item being read is being skipped, so that
	 *  we can adjust the collection freqency and number of entries.
	 */
	void skip_item(Xapian::termcount wdf);

	/** Write the chunk to disk.  Note: this may write it with a
	 *  different key to the original one, if for example the first
	 *  entry has changed.
	 */
	void write_to_disk(QuartzBufferedTable *table);

    private:
	string orig_key;
	string tname;
	bool is_first_chunk;
	Xapian::termcount collectionfreq;
	bool is_last_chunk;
	Xapian::termcount number_of_entries;
	bool started;

	Xapian::docid first_did;
	Xapian::docid current_did;

	string chunk;
};

// Static functions

/// Report an error when reading the posting list.
static void report_read_error(const char * position)
{
    if (position == 0) {
	// data ran out
	throw Xapian::DatabaseCorruptError("Data ran out unexpectedly when reading posting list.");
    }
    // overflow
    throw Xapian::RangeError("Value in posting list too large.");
}

static bool get_tname_from_key(const char **src, const char *end,
			       string &tname)
{
    return unpack_string_preserving_sort(src, end, tname);
}

static bool
skip_and_check_tname_in_key(const char **keypos, const char *keyend,
			    const string &tname)
{
    string tname_in_key;

    // Read the termname.
    if (*keypos != keyend) {
	if (!get_tname_from_key(keypos, keyend, tname_in_key)) {
	    report_read_error(*keypos);
	}
    }

    // This should only fail if the postlist doesn't exist at all.
    return tname_in_key == tname;
}

/// Read the start of the first chunk in the posting list.
static Xapian::docid
read_start_of_first_chunk(const char ** posptr,
			  const char * end,
			  Xapian::termcount * number_of_entries_ptr,
			  Xapian::termcount * collection_freq_ptr)
{
    DEBUGCALL_STATIC(DB, Xapian::docid, "read_start_of_first_chunk",
		     (const void *)posptr << ", " <<
		     (const void *)end << ", " <<
		     (void *)number_of_entries_ptr << ", " <<
		     (void *)collection_freq_ptr);

    QuartzPostList::read_number_of_entries(posptr, end,
			   number_of_entries_ptr, collection_freq_ptr);
    if (number_of_entries_ptr)
	DEBUGLINE(DB, "number_of_entries = " << *number_of_entries_ptr);
    if (collection_freq_ptr)
	DEBUGLINE(DB, "collection_freq = " << *collection_freq_ptr);

    Xapian::docid did;
    // Read the docid of the first entry in the posting list.
    if (!unpack_uint(posptr, end, &did))
	report_read_error(*posptr);
    ++did;
    DEBUGLINE(DB, "doc_id = " << did);
    RETURN(did);
}

static void read_did_increase(const char ** posptr,
			      const char * end,
			      Xapian::docid * did_ptr)
{
    Xapian::docid did_increase;
    if (!unpack_uint(posptr, end, &did_increase)) report_read_error(*posptr);
    *did_ptr += did_increase + 1;
}

/// Read the wdf and the document length of an item.
static void read_wdf_and_length(const char ** posptr,
				const char * end,
				Xapian::termcount * wdf_ptr,
				quartz_doclen_t * doclength_ptr)
{
    if (!unpack_uint(posptr, end, wdf_ptr)) report_read_error(*posptr);
    if (!unpack_uint(posptr, end, doclength_ptr)) report_read_error(*posptr);
}

/// Read the start of a chunk, including the first item in it.
static Xapian::docid
read_start_of_chunk(const char ** posptr,
		    const char * end,
		    Xapian::docid first_did_in_chunk,
		    bool * is_last_chunk_ptr)
{
    DEBUGCALL_STATIC(DB, Xapian::docid, "read_start_of_chunk",
		     (void *)posptr << ", " <<
		     (void *)end << ", " <<
		     first_did_in_chunk << ", " <<
		     (void *)is_last_chunk_ptr);

    // Read whether this is the last chunk
    if (!unpack_bool(posptr, end, is_last_chunk_ptr))
	report_read_error(*posptr);
    if (is_last_chunk_ptr)
	DEBUGLINE(DB, "is_last_chunk = " << *is_last_chunk_ptr);

    // Read what the final document ID in this chunk is.
    Xapian::docid increase_to_last;
    if (!unpack_uint(posptr, end, &increase_to_last))
	report_read_error(*posptr);
    ++increase_to_last;
    Xapian::docid last_did_in_chunk = first_did_in_chunk + increase_to_last;
    DEBUGLINE(DB, "last_did_in_chunk = " << last_did_in_chunk);
    RETURN(last_did_in_chunk);
}

static string make_did_increase(Xapian::docid new_did, Xapian::docid last_did_in_chunk)
{
    Assert(new_did > last_did_in_chunk);
    return pack_uint(new_did - last_did_in_chunk - 1);
}

static string make_wdf_and_length(Xapian::termcount wdf, quartz_doclen_t doclength)
{
    return pack_uint(wdf) + pack_uint(doclength);
}

static void append_to_chunk(string & chunk,
			    Xapian::docid new_did,
			    Xapian::docid last_did_in_chunk,
			    Xapian::termcount new_wdf,
			    Xapian::termcount new_doclength)
{
    chunk += make_did_increase(new_did, last_did_in_chunk);
    chunk += make_wdf_and_length(new_wdf, new_doclength);
}

static void write_start_of_chunk(string & chunk,
				 unsigned int start_of_chunk_header,
				 unsigned int end_of_chunk_header,
				 bool is_last_chunk,
				 Xapian::docid first_did_in_chunk,
				 Xapian::docid last_did_in_chunk)
{
    Assert(last_did_in_chunk >= first_did_in_chunk);
    Xapian::docid increase_to_last = last_did_in_chunk - first_did_in_chunk;

    chunk.replace(start_of_chunk_header,
		  end_of_chunk_header - start_of_chunk_header,
		  pack_bool(is_last_chunk) + pack_uint(increase_to_last - 1));
}

PostlistChunkReader::PostlistChunkReader(const char *keypos,
					 const char *keyend,
					 const string &chunk_)
	: chunk(chunk_),
	  pos(chunk.data()), end(pos + chunk.size()),
	  at_end(false)
{
    bool is_first_chunk = (keypos == keyend);

    // Read the data for the first entry
    if (is_first_chunk) {
	did = read_start_of_first_chunk(&pos, end, &number_of_entries, &collectionfreq);
    } else {
	if (!unpack_uint_preserving_sort(&keypos, keyend, &did)) {
	    report_read_error(keypos);
	}
    }

    last_did_in_chunk = read_start_of_chunk(&pos, end, did, &is_last_chunk);

    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

void
PostlistChunkReader::next()
{
    if (pos == end) {
	at_end = true;
    } else {
	read_did_increase(&pos, end, &did);
	read_wdf_and_length(&pos, end, &wdf, &doclength);
    }
}

PostlistChunkWriter::PostlistChunkWriter(const string &key,
					 bool is_first_chunk_,
					 const string &tname_,
					 Xapian::termcount collectionfreq_,
					 bool is_last_chunk_,
					 Xapian::termcount number_of_entries_)
	: orig_key(key),
	  tname(tname_), is_first_chunk(is_first_chunk_),
	  collectionfreq(collectionfreq_),
	  is_last_chunk(is_last_chunk_),
	  number_of_entries(number_of_entries_),
	  started(false)
{
    DEBUGCALL(DB, void, "PostlistChunkWriter::PostlistChunkWriter",
	      "string(" << key << "), " << is_first_chunk_ << ", " << tname_ <<
	      ", " << collectionfreq_ << ", " << is_last_chunk_ <<
	      ", " << number_of_entries_);
}

void
PostlistChunkWriter::append(Xapian::docid did,
			    Xapian::termcount wdf,
			    quartz_doclen_t doclen)
{
    if (!started) {
	first_did = current_did = did;
    } else {
	chunk.append(make_did_increase(did, current_did));
	current_did = did;
    }
    chunk.append(make_wdf_and_length(wdf, doclen));
    started = true;
}

/** Make the data to go at the start of the very first chunk.
 */
static inline string make_start_of_first_chunk(Xapian::termcount entries,
						    Xapian::termcount collectionfreq,
						    Xapian::docid new_did)
{
    return pack_uint(entries) + pack_uint(collectionfreq) + pack_uint(new_did - 1);
}

/** Make the data to go at the start of a standard chunk.
 */
static inline string make_start_of_chunk(bool new_is_last_chunk,
					      Xapian::docid new_first_did,
					      Xapian::docid new_final_did)
{
    Assert(new_final_did >= new_first_did);
    return pack_bool(new_is_last_chunk) +
	    pack_uint(new_final_did - new_first_did - 1);
}

/// Make a key for accessing the postlist.
static void make_key(const string & tname, Xapian::docid did, string & key)
{
    key = pack_string_preserving_sort(tname);
    key += pack_uint_preserving_sort(did);
}

static void make_key(const string & tname, string & key)
{
    key = pack_string_preserving_sort(tname);
}

void
PostlistChunkWriter::write_to_disk(QuartzBufferedTable *table)
{
    /* This is one of the more messy parts involved with deleting entries.
     * Depending on circumstances, we may have to delete an entire chunk
     * or file it under a different key, as well as possibly modify both
     * the previous and next chunk of the postlist.
     */

    DEBUGCALL(DB, void, "PostlistChunkWriter::write_to_disk", table);

    if (!started) {
	/* This chunk disappears entirely, as we're now empty.
	 *
	 * If this was the last chunk, then the previous chunk
	 * must have its "is_last_chunk" flag updated.
	 *
	 * If this was the first chunk, then the next chunk must
	 * be transformed into the first chunk.  Messy!
	 */
	DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting chunk");
	if (is_first_chunk) {
	    DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting first chunk");
	    if (is_last_chunk) {
		/* This is the first and the last chunk, ie the only
		 * chunk.  Can just delete the tag.
		 */
		table->delete_tag(orig_key);
	    } else {
		/* This is the messiest case.  The first chunk has
		 * disappeared, and there is at least one chunk after
		 * this.  Need to rewrite the next chunk as the first
		 * chunk.
		 */
		AutoPtr<QuartzCursor> cursor(table->cursor_get());

		// Seek to the next chunk.
		if (cursor->find_entry(orig_key) != true) {
		    throw Xapian::DatabaseCorruptError("The key we're working on has disappeared");
		}

		cursor->next();
		if (cursor->after_end()) {
		    throw Xapian::DatabaseCorruptError("Expected another key but found none");
		}
		const char *kpos = cursor->current_key.data();
		const char *kend = kpos + cursor->current_key.size();
		if (!skip_and_check_tname_in_key(&kpos, kend, tname)) {
		    throw Xapian::DatabaseCorruptError("Expected another key with the same term name but found a different one");
		}

		// Read the new first docid
		Xapian::docid new_first_did;
		if (!unpack_uint_preserving_sort(&kpos, kend,
						 &new_first_did)) {
		    report_read_error(kpos);
		}

		const char *tagpos = cursor->current_tag.data();
		const char *tagend = tagpos + cursor->current_tag.size();

		// Read the chunk header
		bool new_is_last_chunk;
		Xapian::docid new_last_did_in_chunk =
		    read_start_of_chunk(&tagpos, tagend, new_first_did,
			    		&new_is_last_chunk);

		string chunk_data(tagpos, tagend);

		// First remove the renamed tag
		table->delete_tag(cursor->current_key);

		// And now write it as the first chunk
		string *tag = table->get_or_make_tag(orig_key);

		*tag = make_start_of_first_chunk(number_of_entries,
						       collectionfreq,
						       new_first_did);
		*tag += make_start_of_chunk(new_is_last_chunk,
						  new_first_did,
						  new_last_did_in_chunk);
		*tag += chunk_data;
	    }
	} else {
	    DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting secondary chunk");
	    /* This isn't the first chunk.  Check whether we're the last
	     * chunk.
	     */

	    // Delete this chunk
	    table->delete_tag(orig_key);

	    if (is_last_chunk) {
		DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting secondary last chunk");
		// Update the previous chunk's is_last_chunk flag.
		AutoPtr<QuartzCursor> cursor(table->cursor_get());

		/* Should not find the key we just deleted, but should
		 * find the previous chunk. */
		if (cursor->find_entry(orig_key) == true) {
		    throw Xapian::DatabaseCorruptError("Quartz key not deleted as we expected");
		}
		// Make sure this is a chunk with the right term attached.
		const char * keypos = cursor->current_key.data();
		const char * keyend = keypos + cursor->current_key.size();
		string tname_in_key;

		// Read the termname.
		if (keypos != keyend) {
		    if (!get_tname_from_key(&keypos, keyend, tname_in_key)) {
			report_read_error(keypos);
		    }
		}

		if (tname_in_key != tname) {
		    throw Xapian::DatabaseCorruptError("Couldn't find chunk before delete chunk");
		}

		// FIXME: this was declared locally here - trying to see if we
		// should be updating the member of the same name
		is_first_chunk = (keypos == keyend);

		// Now update the last_chunk
		string *tag = table->get_or_make_tag(cursor->current_key);

		Assert(tag != 0);
		Assert(tag->size() != 0);
		const char *tagpos = tag->data();
		const char *tagend = tagpos + tag->size();

		// Skip first chunk header
		Xapian::docid first_did_in_chunk;
		if (is_first_chunk) {
		    first_did_in_chunk = read_start_of_first_chunk(&tagpos, tagend,
					      0, 0);
		} else {
		    if (!unpack_uint_preserving_sort(&keypos, keyend,
						     &first_did_in_chunk))
			report_read_error(keypos);
		}
		bool wrong_is_last_chunk;
		string::size_type start_of_chunk_header = tagpos - tag->data();
		Xapian::docid last_did_in_chunk =
		    read_start_of_chunk(&tagpos, tagend, first_did_in_chunk,
					&wrong_is_last_chunk);
		string::size_type end_of_chunk_header = tagpos - tag->data();

		// write new is_last flag
		write_start_of_chunk(*tag,
				     start_of_chunk_header,
				     end_of_chunk_header,
				     true, // is_last_chunk
				     first_did_in_chunk,
				     last_did_in_chunk);
	    }
	}
    } else {
	DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting from chunk which still has items in it");
	/* The chunk still has some items in it.  Two major subcases:
	 * a) This is the first chunk.
	 * b) This isn't the first chunk.
	 *
	 * The subcases just affect the chunk header.
	 */

	string *tag = table->get_or_make_tag(orig_key);

	/* First write the header, which depends on whether this is the
	 * first chunk.
	 */
	if (is_first_chunk) {
	    /* The first chunk.  This is the relatively easy case,
	     * and we just have to write this one back to disk.
	     */
	    DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting from the first chunk, which still has items in it");
	    *tag = make_start_of_first_chunk(number_of_entries,
						   collectionfreq,
						   first_did);

	    *tag += make_start_of_chunk(is_last_chunk,
					      first_did,
					      current_did);
	} else {
	    DEBUGLINE(DB, "PostlistChunkWriter::write_to_disk(): deleting secondary chunk which still has items in it");
	    /* Not the first chunk.
	     *
	     * This has the easy sub-sub-case:
	     *   The first entry in the chunk hasn't changed
	     * ...and the hard sub-sub-case:
	     *   The first entry in the chunk has changed.  This is
	     *   harder because the key for the chunk changes, so
	     *   we've got to do a switch.
	     */

	    // First find out the initial docid
	    const char *keypos = orig_key.data();
	    const char *keyend = keypos + orig_key.size();
	    if (!skip_and_check_tname_in_key(&keypos, keyend, tname)) {
		throw Xapian::DatabaseCorruptError("Have invalid key writing to postlist");
	    }
	    Xapian::docid initial_did;
	    if (!unpack_uint_preserving_sort(&keypos, keyend, &initial_did)) {
		report_read_error(keypos);
	    }
	    if (initial_did != first_did) {
		/* The fiddlier case:
		 * Create a new tag with the correct key, and replace
		 * the old one.
		 */
		string new_key;
		make_key(tname, first_did, new_key);
		tag = table->get_or_make_tag(new_key);

		table->delete_tag(orig_key);
	    }

	    // ...and write the start of this chunk.
	    *tag = make_start_of_chunk(is_last_chunk, first_did, current_did);
	}

	*tag += chunk;
    }
}

void
PostlistChunkWriter::skip_item(Xapian::termcount wdf)
{
    number_of_entries -= 1;
    collectionfreq -= wdf;
}

static void adjust_counts(QuartzBufferedTable * bufftable,
			  const string & tname,
			  Xapian::termcount entries_increase,
			  Xapian::termcount entries_decrease,
			  Xapian::termcount collection_freq_increase,
			  Xapian::termcount collection_freq_decrease)
{
    string key;
    make_key(tname, key);
    string * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(!tag->empty());

    const char * tagpos = tag->data();
    const char * tagend = tagpos + tag->size();
    Xapian::termcount number_of_entries;
    Xapian::termcount collection_freq;
    if (!unpack_uint(&tagpos, tagend, &number_of_entries))
	report_read_error(tagpos);
    if (!unpack_uint(&tagpos, tagend, &collection_freq))
	report_read_error(tagpos);

    number_of_entries += entries_increase;
    number_of_entries -= entries_decrease;
    collection_freq += collection_freq_increase;
    collection_freq -= collection_freq_decrease;

    tag->replace(0, tagpos - tag->data(),
		 pack_uint(number_of_entries) + pack_uint(collection_freq));
}

static void new_postlist(QuartzBufferedTable * bufftable,
			 const string & tname,
			 Xapian::docid new_did,
			 Xapian::termcount new_wdf,
			 Xapian::termcount new_doclen)
{
    DEBUGCALL_STATIC(DB, void, "QuartzPostList::new_postlist",
		     bufftable << ", " <<
		     tname << ", " <<
		     new_did << ", " <<
		     new_wdf << ", " <<
		     new_doclen);
    string key;
    make_key(tname, key);
    string * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(tag->empty());

    *tag = make_start_of_first_chunk(1u, new_wdf, new_did);
    *tag += make_start_of_chunk(true, new_did, new_did);
    *tag += make_wdf_and_length(new_wdf, new_doclen);
}

static void new_chunk(QuartzBufferedTable * bufftable,
		      const string & tname,
		      Xapian::docid new_did,
		      Xapian::termcount new_wdf,
		      quartz_doclen_t new_doclen)
{
    string key;
    make_key(tname, new_did, key);
    string * tag = bufftable->get_or_make_tag(key);
    Assert(tag != 0);
    Assert(tag->size() == 0);

    *tag = make_start_of_chunk(true, new_did, new_did);
    *tag += make_wdf_and_length(new_wdf, new_doclen);
}


/** Read the number of entries in the posting list.
 *  This must only be called when *posptr is pointing to the start of
 *  the first chunk of the posting list.
 */
void QuartzPostList::read_number_of_entries(const char ** posptr,
				   const char * end,
				   Xapian::termcount * number_of_entries_ptr,
				   Xapian::termcount * collection_freq_ptr)
{
    if (!unpack_uint(posptr, end, number_of_entries_ptr))
	report_read_error(*posptr);
    if (!unpack_uint(posptr, end, collection_freq_ptr))
	report_read_error(*posptr);
}

/** The format of a postlist is:
 *
 *  Split into chunks.  Key for first chunk is the termname (encoded as
 *  length - name).  Key for subsequent chunks is the same, followed by the
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
QuartzPostList::QuartzPostList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
			       const QuartzTable * table_,
			       const QuartzTable * positiontable_,
			       const string & tname_)
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
    string key;
    make_key(tname, key);
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
    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    did = read_start_of_first_chunk(&pos, end,
			      &number_of_entries, &collection_freq);
    first_did_in_chunk = did;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk,
	    				    &is_last_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

QuartzPostList::~QuartzPostList()
{
    DEBUGCALL(DB, void, "QuartzPostList::~QuartzPostList", "");
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
	throw Xapian::DatabaseCorruptError("Unexpected end of posting list for `" +
				     tname + "'");
    }
    const char * keypos = cursor->current_key.data();
    const char * keyend = keypos + cursor->current_key.size();
    string tname_in_key;

    // Check we're still in same postlist
    if (!get_tname_from_key(&keypos, keyend, tname_in_key)) {
	report_read_error(keypos);
    }
    if (tname_in_key != tname) {
	is_at_end = true;
	throw Xapian::DatabaseCorruptError("Unexpected end of posting list for `" +
				     tname + "'");
    }

    Xapian::docid newdid;
    if (!unpack_uint_preserving_sort(&keypos, keyend, &newdid)) {
	report_read_error(keypos);
    }
    if (newdid <= did) {
	throw Xapian::DatabaseCorruptError("Document ID in new chunk of postlist (" +
		om_tostring(newdid) +
		") is not greater than final document ID in previous chunk (" +
		om_tostring(did) + ")");
    }
    did = newdid;

    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    first_did_in_chunk = did;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk,
	    				    &is_last_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

PositionList *
QuartzPostList::read_position_list()
{
    DEBUGCALL(DB, PositionList *, "QuartzPostList::read_position_list", "");

    positionlist.read_data(positiontable, did, tname);

    RETURN(&positionlist);
}

PositionList *
QuartzPostList::open_position_list() const
{
    DEBUGCALL(DB, PositionList *, "QuartzPostList::open_position_list", "");

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(positiontable, did, tname);

    RETURN(poslist.release());
}

PostList *
QuartzPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *, "QuartzPostList::next", w_min);
    (void)w_min; // no warning

    if (!have_started) {
	have_started = true;
    } else {
	if (!next_in_chunk()) next_chunk();
    }

    DEBUGLINE(DB, string("Moved to ") <<
	      (is_at_end ? string("end.") : string("docid, wdf, doclength = ") +
	       om_tostring(did) + ", " + om_tostring(wdf) + ", " +
	       om_tostring(doclength)));

    RETURN(NULL);
}

bool
QuartzPostList::current_chunk_contains(Xapian::docid desired_did)
{
    DEBUGCALL(DB, bool, "QuartzPostList::current_chunk_contains", desired_did);
    if (desired_did >= first_did_in_chunk &&
	desired_did <= last_did_in_chunk) {
	RETURN(true);
    }
    RETURN(false);
}

void
QuartzPostList::move_to_chunk_containing(Xapian::docid desired_did)
{
    DEBUGCALL(DB, void,
	      "QuartzPostList::move_to_chunk_containing", desired_did);
    string key;
    make_key(tname, desired_did, key);
    (void) cursor->find_entry(key);
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.data();
    const char * keyend = keypos + cursor->current_key.size();
    string tname_in_key;

    // Check we're still in same postlist
    if (!get_tname_from_key(&keypos, keyend, tname_in_key)) {
	report_read_error(keypos);
    }
    if (tname_in_key != tname) {
	// This should only happen if the postlist doesn't exist at all.
	is_at_end = true;
	is_last_chunk = true;
	return;
    }
    is_at_end = false;

    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    if (keypos == keyend) {
	// In first chunk
#ifdef MUS_DEBUG
	Xapian::termcount old_number_of_entries = number_of_entries;
	Xapian::termcount old_collection_freq = collection_freq;
	did = read_start_of_first_chunk(&pos, end, &number_of_entries,
					&collection_freq);
	Assert(old_number_of_entries == number_of_entries);
	Assert(old_collection_freq == collection_freq);
#else
	did = read_start_of_first_chunk(&pos, end, 0, 0);
#endif
    } else {
	// In normal chunk
	if (!unpack_uint_preserving_sort(&keypos, keyend, &did)) {
	    report_read_error(keypos);
	}
    }

    first_did_in_chunk = did;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk,
	    				    &is_last_chunk);
    read_wdf_and_length(&pos, end, &wdf, &doclength);
}

bool
QuartzPostList::move_forward_in_chunk_to_at_least(Xapian::docid desired_did)
{
    DEBUGCALL(DB, bool,
	      "QuartzPostList::move_forward_in_chunk_to_at_least", desired_did);
    if (desired_did > last_did_in_chunk) {
	pos = end;
	RETURN(false);
    }
    while (did < desired_did) {
	// FIXME: perhaps we don't need to decode the wdf and document length
	// for documents we're skipping past.
	bool at_end_of_chunk = !next_in_chunk();
	if (at_end_of_chunk) RETURN(false);
    }
    RETURN(true);
}

PostList *
QuartzPostList::skip_to(Xapian::docid desired_did, Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "QuartzPostList::skip_to", desired_did << ", " << w_min);
    (void)w_min; // no warning
    // We've started now - if we hadn't already, we're already positioned
    // at start so there's no need to actually do anything.
    have_started = true;

    // Don't skip back, and don't need to do anthing if already there.
    if (desired_did <= did) RETURN(NULL);

    move_to(desired_did);

    DEBUGLINE(DB, string("Skipped to ") <<
	      (is_at_end ? string("end.") : string("docid, wdf, doclength = ") +
	       om_tostring(did) + ", " + om_tostring(wdf) + ", " +
	       om_tostring(doclength) + "."));

    RETURN(NULL);
}

void
QuartzPostList::move_to(Xapian::docid desired_did)
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

string
QuartzPostList::get_description() const
{
    return tname + ":" + om_tostring(number_of_entries);
}

void
QuartzPostList::merge_changes(QuartzBufferedTable * bufftable,
    const map<string, map<Xapian::docid, pair<char, Xapian::termcount> > > & mod_plists,
    const map<Xapian::docid, Xapian::termcount> & doclens)
{
    map<string, map<Xapian::docid, pair<char, Xapian::termcount> > >::const_iterator i;
    for (i = mod_plists.begin(); i != mod_plists.end(); ++i) {
	string tname = i->first;
	map<Xapian::docid, pair<char, Xapian::termcount> >::const_iterator j;
	for (j = i->second.begin(); j != i->second.end(); ++j) {
	    Xapian::docid did = j->first;
	    switch (j->second.first) {
		case 'M': {
		    // Get chunk containing entry
		    string key;
		    make_key(tname, did, key);

		    // Find the right chunk
		    AutoPtr<QuartzCursor> cursor(bufftable->cursor_get());

		    cursor->find_entry(key);
		    Assert(!cursor->after_end());

		    const char * keypos = cursor->current_key.data();
		    const char * keyend = keypos + cursor->current_key.size();

		    if (!skip_and_check_tname_in_key(&keypos, keyend, tname)) {
			// Postlist for this termname doesn't exist.
			break;
		    }

		    // Get the appropriate tag and set pointers to iterate through it
		    string *tag = bufftable->get_or_make_tag(cursor->current_key);
		    Assert(tag != 0);
		    Assert(!tag->empty());

		    PostlistChunkReader from(keypos, keyend, *tag);
		    PostlistChunkWriter to(cursor->current_key, (keypos == keyend), tname,
					   from.get_collectionfreq(),
					   from.get_is_last_chunk(),
					   from.get_number_of_entries());

		    while (!from.is_at_end()) {
			if (from.get_docid() != did) {
			    to.append(from.get_docid(), from.get_wdf(), from.get_doclength());
			} else {
			    to.skip_item(from.get_wdf());
			}
			from.next();
		    }
		    to.write_to_disk(bufftable);
		    /* FALL THRU */
		}
		case 'A': {
		    map<Xapian::docid, Xapian::termcount>::const_iterator k = doclens.find(did);
		    Assert(k != doclens.end());
		    Xapian::termcount new_doclen = k->second;
		    Xapian::termcount new_wdf = j->second.second;

		    // How big should chunks in the posting list be?  (They will grow
		    // slightly bigger than this, but not more than a few bytes extra)
		    const unsigned int CHUNKSIZE = 2048;

		    string key;
		    make_key(tname, did, key);
		    AutoPtr<QuartzCursor> cursor(bufftable->cursor_get());

		    cursor->find_entry(key);
		    Assert(!cursor->after_end());

		    DEBUGLINE(DB, "cursor->current_key=`" << cursor->current_key <<
			      "', length=" << cursor->current_key.size());
		    const char * keypos = cursor->current_key.data();
		    const char * keyend = keypos + cursor->current_key.size();
		    string tname_in_key;

		    // Read the termname.
		    if (keypos != keyend)
			if (!get_tname_from_key(&keypos, keyend, tname_in_key))
			    report_read_error(keypos);

		    if (tname_in_key != tname) {
			// This should only happen if the postlist doesn't exist at all.
			new_postlist(bufftable, tname, did, new_wdf, new_doclen);
		    } else {
			bool is_first_chunk = (keypos == keyend);

			string * tag = bufftable->get_or_make_tag(cursor->current_key);
			Assert(tag != 0);
			Assert(tag->size() != 0);

			// Determine whether we're adding to the end of the chunk

			DEBUGLINE(DB, "*tag=`" << *tag << "', length=" << tag->size());
			const char * tagpos = tag->data();
			const char * tagend = tagpos + tag->size();

			Xapian::docid first_did_in_chunk;
			if (is_first_chunk) {
			    first_did_in_chunk = read_start_of_first_chunk(&tagpos, tagend, 0, 0);
			} else {
			    if (!unpack_uint_preserving_sort(&keypos, keyend,
							     &first_did_in_chunk))
				report_read_error(keypos);
			}
			bool is_last_chunk;

			unsigned int start_of_chunk_header = tagpos - tag->data();
			Xapian::docid last_did_in_chunk =
			    read_start_of_chunk(&tagpos, tagend, first_did_in_chunk,
						&is_last_chunk);
			unsigned int end_of_chunk_header = tagpos - tag->data();

			// Have read in data needed - now add item
			if (!is_last_chunk || !(last_did_in_chunk < did)) {
			    // Add in middle of postlist.
			    keypos = cursor->current_key.data();
			    keyend = keypos + cursor->current_key.size();
			    if (!skip_and_check_tname_in_key(&keypos, keyend, tname)) {
			       // Postlist for this termname doesn't exist.
			       break;
			    }
			    PostlistChunkReader from(keypos, keyend, *tag);
			    PostlistChunkWriter to(cursor->current_key, (keypos == keyend),
					   tname,
					   from.get_collectionfreq(),
					   from.get_is_last_chunk(),
					   from.get_number_of_entries());
			    while ((!from.is_at_end()) && (from.get_docid() < did)) {
				to.append(from.get_docid(),
				      from.get_wdf(),
				      from.get_doclength());
				from.next();
			    }
			    to.append(did, new_wdf, new_doclen);
			    while (!from.is_at_end()) {
				to.append(from.get_docid(),
				      from.get_wdf(),
				      from.get_doclength());
				from.next();
			    }
			    to.write_to_disk(bufftable);
			} else {
			    // Append
			    if (tag->size() > CHUNKSIZE) {
				new_chunk(bufftable, tname, did, new_wdf, new_doclen);

				// Sort out previous chunk
				write_start_of_chunk(*tag,
						     start_of_chunk_header,
						     end_of_chunk_header,
						     false,
						     first_did_in_chunk,
						     last_did_in_chunk);
			    } else {
				append_to_chunk(*tag, did, last_did_in_chunk,
						new_wdf, new_doclen);
				last_did_in_chunk = did;
				write_start_of_chunk(*tag,
						     start_of_chunk_header,
						     end_of_chunk_header,
						     is_last_chunk,
						     first_did_in_chunk,
						     last_did_in_chunk);
			    }
			}

			adjust_counts(bufftable, tname, 1, 0, new_wdf, 0);
		    }
		    break;
		}
		case 'D': {
		    // Get chunk containing entry
		    string key;
		    make_key(tname, did, key);

		    // Find the right chunk
		    AutoPtr<QuartzCursor> cursor(bufftable->cursor_get());

		    cursor->find_entry(key);
		    Assert(!cursor->after_end());

		    const char * keypos = cursor->current_key.data();
		    const char * keyend = keypos + cursor->current_key.size();

		    if (!skip_and_check_tname_in_key(&keypos, keyend, tname)) {
			// Postlist for this termname doesn't exist.
			break;
		    }

		    // Get the appropriate tag and set pointers to iterate through it
		    string *tag = bufftable->get_or_make_tag(cursor->current_key);
		    Assert(tag != 0);
		    Assert(!tag->empty());

		    PostlistChunkReader from(keypos, keyend, *tag);
		    PostlistChunkWriter to(cursor->current_key, (keypos == keyend), tname,
					   from.get_collectionfreq(),
					   from.get_is_last_chunk(),
					   from.get_number_of_entries());

		    while (!from.is_at_end()) {
			if (from.get_docid() != did) {
			    to.append(from.get_docid(), from.get_wdf(), from.get_doclength());
			} else {
			    to.skip_item(from.get_wdf());
			}
			from.next();
		    }
		    to.write_to_disk(bufftable);
		    break;
		}
	    }
	}
    }
}

