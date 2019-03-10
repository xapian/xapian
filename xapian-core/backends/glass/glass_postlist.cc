/** @file glass_postlist.cc
 * @brief Postlists in a glass database
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2007,2008,2009,2011,2013,2014,2015,2017,2019 Olly Betts
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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

#include "glass_postlist.h"

#include "glass_cursor.h"
#include "glass_database.h"
#include "debuglog.h"
#include "pack.h"
#include "str.h"
#include "unicode/description_append.h"

using Xapian::Internal::intrusive_ptr;

// Static functions

/// Report an error when reading the posting list.
[[noreturn]]
static void report_read_error(const char * position)
{
    if (position == 0) {
	// data ran out
	LOGLINE(DB, "GlassPostList data ran out");
	throw Xapian::DatabaseCorruptError("Data ran out unexpectedly when reading posting list.");
    }
    // overflow
    LOGLINE(DB, "GlassPostList value too large");
    throw Xapian::RangeError("Value in posting list too large.");
}

static inline bool
get_tname_from_key(const char **src, const char *end, string &tname)
{
    return unpack_string_preserving_sort(src, end, tname);
}

static inline bool
check_tname_in_key_lite(const char **keypos, const char *keyend, const string &tname)
{
    string tname_in_key;

    if (keyend - *keypos >= 2 && (*keypos)[0] == '\0' && (*keypos)[1] == '\xe0') {
	*keypos += 2;
    } else {
	// Read the termname.
	if (!get_tname_from_key(keypos, keyend, tname_in_key))
	    report_read_error(*keypos);
    }

    // This should only fail if the postlist doesn't exist at all.
    return tname_in_key == tname;
}

static inline bool
check_tname_in_key(const char **keypos, const char *keyend, const string &tname)
{
    if (*keypos == keyend) return false;

    return check_tname_in_key_lite(keypos, keyend, tname);
}

/// Read the start of the first chunk in the posting list.
static Xapian::docid
read_start_of_first_chunk(const char ** posptr,
			  const char * end,
			  Xapian::doccount * number_of_entries_ptr,
			  Xapian::termcount * collection_freq_ptr)
{
    LOGCALL_STATIC(DB, Xapian::docid, "read_start_of_first_chunk", (const void *)posptr | (const void *)end | (void *)number_of_entries_ptr | (void *)collection_freq_ptr);

    GlassPostList::read_number_of_entries(posptr, end,
					  number_of_entries_ptr,
					  collection_freq_ptr);
    if (number_of_entries_ptr)
	LOGVALUE(DB, *number_of_entries_ptr);
    if (collection_freq_ptr)
	LOGVALUE(DB, *collection_freq_ptr);

    Xapian::docid did;
    // Read the docid of the first entry in the posting list.
    if (!unpack_uint(posptr, end, &did))
	report_read_error(*posptr);
    ++did;
    LOGVALUE(DB, did);
    RETURN(did);
}

static inline void
read_did_increase(const char ** posptr, const char * end,
		  Xapian::docid * did_ptr)
{
    Xapian::docid did_increase;
    if (!unpack_uint(posptr, end, &did_increase)) report_read_error(*posptr);
    *did_ptr += did_increase + 1;
}

/// Read the wdf for an entry.
static inline void
read_wdf(const char ** posptr, const char * end, Xapian::termcount * wdf_ptr)
{
    if (!unpack_uint(posptr, end, wdf_ptr)) report_read_error(*posptr);
}

/// Read the start of a chunk.
static Xapian::docid
read_start_of_chunk(const char ** posptr,
		    const char * end,
		    Xapian::docid first_did_in_chunk,
		    bool * is_last_chunk_ptr)
{
    LOGCALL_STATIC(DB, Xapian::docid, "read_start_of_chunk", reinterpret_cast<const void*>(posptr) | reinterpret_cast<const void*>(end) | first_did_in_chunk | reinterpret_cast<const void*>(is_last_chunk_ptr));
    Assert(is_last_chunk_ptr);

    // Read whether this is the last chunk
    if (!unpack_bool(posptr, end, is_last_chunk_ptr))
	report_read_error(*posptr);
    LOGVALUE(DB, *is_last_chunk_ptr);

    // Read what the final document ID in this chunk is.
    Xapian::docid increase_to_last;
    if (!unpack_uint(posptr, end, &increase_to_last))
	report_read_error(*posptr);
    Xapian::docid last_did_in_chunk = first_did_in_chunk + increase_to_last;
    LOGVALUE(DB, last_did_in_chunk);
    RETURN(last_did_in_chunk);
}

void
GlassPostListTable::get_freqs(const string & term,
			      Xapian::doccount * termfreq_ptr,
			      Xapian::termcount * collfreq_ptr,
			      Xapian::termcount * wdfub_ptr) const
{
    string key = make_key(term);
    string tag;
    if (!get_exact_entry(key, tag)) {
	if (termfreq_ptr)
	    *termfreq_ptr = 0;
	if (collfreq_ptr)
	    *collfreq_ptr = 0;
	if (wdfub_ptr)
	    *wdfub_ptr = 0;
    } else {
	const char * p = tag.data();
	const char * e = p + tag.size();
	Xapian::doccount tf;
	Xapian::termcount cf;
	GlassPostList::read_number_of_entries(&p, e, &tf, &cf);
	if (termfreq_ptr)
	    *termfreq_ptr = tf;
	if (collfreq_ptr)
	    *collfreq_ptr = cf;
	if (wdfub_ptr) {
	    if (cf == 0 || tf == 1) {
		*wdfub_ptr = cf;
	    } else {
		Xapian::docid did;
		if (!unpack_uint(&p, e, &did))
		    report_read_error(p);
		bool is_last;
		(void)read_start_of_chunk(&p, e, did + 1, &is_last);
		(void)is_last;
		Xapian::termcount first_wdf;
		if (!unpack_uint(&p, e, &first_wdf))
		    report_read_error(p);
		*wdfub_ptr = max(cf - first_wdf, first_wdf);
	    }
	}
    }
}

Xapian::termcount
GlassPostListTable::get_doclength(Xapian::docid did,
				  intrusive_ptr<const GlassDatabase> db) const {
    if (!doclen_pl.get()) {
	// Don't keep a reference back to the database, since this
	// would make a reference loop.
	doclen_pl.reset(new GlassPostList(db, string(), false));
    }
    if (!doclen_pl->jump_to(did))
	throw Xapian::DocNotFoundError("Document " + str(did) + " not found");
    return doclen_pl->get_wdf();
}

bool
GlassPostListTable::document_exists(Xapian::docid did,
				    intrusive_ptr<const GlassDatabase> db) const
{
    if (!doclen_pl.get()) {
	// Don't keep a reference back to the database, since this
	// would make a reference loop.
	doclen_pl.reset(new GlassPostList(db, string(), false));
    }
    return (doclen_pl->jump_to(did));
}

// How big should chunks in the posting list be?  (They
// will grow slightly bigger than this, but not more than a
// few bytes extra) - FIXME: tune this value to try to
// maximise how well blocks are used.  Or performance.
// Or indexing speed.  Or something...
const unsigned int CHUNKSIZE = 2000;

/** PostlistChunkWriter is a wrapper which acts roughly as an
 *  output iterator on a postlist chunk, taking care of the
 *  messy details.  It's intended to be used with deletion and
 *  replacing of entries, not for adding to the end, when it's
 *  not really needed.
 */
class Glass::PostlistChunkWriter {
    public:
	PostlistChunkWriter(const string &orig_key_,
			    bool is_first_chunk_,
			    const string &tname_,
			    bool is_last_chunk_);

	/// Append an entry to this chunk.
	void append(GlassTable * table, Xapian::docid did,
		    Xapian::termcount wdf);

	/// Append a block of raw entries to this chunk.
	void raw_append(Xapian::docid first_did_, Xapian::docid current_did_,
			const string & s) {
	    Assert(!started);
	    first_did = first_did_;
	    current_did = current_did_;
	    if (!s.empty()) {
		chunk.append(s);
		started = true;
	    }
	}

	/** Flush the chunk to the buffered table.  Note: this may write it
	 *  with a different key to the original one, if for example the first
	 *  entry has changed.
	 */
	void flush(GlassTable *table);

    private:
	string orig_key;
	string tname;
	bool is_first_chunk;
	bool is_last_chunk;
	bool started;

	Xapian::docid first_did;
	Xapian::docid current_did;

	string chunk;
};

using Glass::PostlistChunkWriter;

/** PostlistChunkReader is essentially an iterator wrapper
 *  around a postlist chunk.  It simply iterates through the
 *  entries in a postlist.
 */
class Glass::PostlistChunkReader {
    string data;

    const char *pos;
    const char *end;

    bool at_end;

    Xapian::docid did;
    Xapian::termcount wdf;

  public:
    /** Initialise the postlist chunk reader.
     *
     *  @param first_did  First document id in this chunk.
     *  @param data       The tag string with the header removed.
     */
    PostlistChunkReader(Xapian::docid first_did, const string & data_)
	: data(data_), pos(data.data()), end(pos + data.length()), at_end(data.empty()), did(first_did)
    {
	if (!at_end) read_wdf(&pos, end, &wdf);
    }

    Xapian::docid get_docid() const {
	return did;
    }
    Xapian::termcount get_wdf() const {
	return wdf;
    }

    bool is_at_end() const {
	return at_end;
    }

    /** Advance to the next entry.  Set at_end if we run off the end.
     */
    void next();
};

using Glass::PostlistChunkReader;

void
PostlistChunkReader::next()
{
    if (pos == end) {
	at_end = true;
    } else {
	read_did_increase(&pos, end, &did);
	read_wdf(&pos, end, &wdf);
    }
}

PostlistChunkWriter::PostlistChunkWriter(const string &orig_key_,
					 bool is_first_chunk_,
					 const string &tname_,
					 bool is_last_chunk_)
	: orig_key(orig_key_),
	  tname(tname_), is_first_chunk(is_first_chunk_),
	  is_last_chunk(is_last_chunk_),
	  started(false)
{
    LOGCALL_CTOR(DB, "PostlistChunkWriter", orig_key_ | is_first_chunk_ | tname_ | is_last_chunk_);
}

void
PostlistChunkWriter::append(GlassTable * table, Xapian::docid did,
			    Xapian::termcount wdf)
{
    if (!started) {
	started = true;
	first_did = did;
    } else {
	Assert(did > current_did);
	// Start a new chunk if this one has grown to the threshold.
	if (chunk.size() >= CHUNKSIZE) {
	    bool save_is_last_chunk = is_last_chunk;
	    is_last_chunk = false;
	    flush(table);
	    is_last_chunk = save_is_last_chunk;
	    is_first_chunk = false;
	    first_did = did;
	    chunk.resize(0);
	    orig_key = GlassPostListTable::make_key(tname, first_did);
	} else {
	    pack_uint(chunk, did - current_did - 1);
	}
    }
    current_did = did;
    pack_uint(chunk, wdf);
}

/** Make the data to go at the start of the very first chunk.
 */
static inline string
make_start_of_first_chunk(Xapian::doccount entries,
			  Xapian::termcount collectionfreq,
			  Xapian::docid new_did)
{
    string chunk;
    pack_uint(chunk, entries);
    pack_uint(chunk, collectionfreq);
    pack_uint(chunk, new_did - 1);
    return chunk;
}

/** Make the data to go at the start of a standard chunk.
 */
static inline string
make_start_of_chunk(bool new_is_last_chunk,
		    Xapian::docid new_first_did,
		    Xapian::docid new_final_did)
{
    Assert(new_final_did >= new_first_did);
    string chunk;
    pack_bool(chunk, new_is_last_chunk);
    pack_uint(chunk, new_final_did - new_first_did);
    return chunk;
}

static void
write_start_of_chunk(string & chunk,
		     unsigned int start_of_chunk_header,
		     unsigned int end_of_chunk_header,
		     bool is_last_chunk,
		     Xapian::docid first_did_in_chunk,
		     Xapian::docid last_did_in_chunk)
{
    Assert((size_t)(end_of_chunk_header - start_of_chunk_header) <= chunk.size());

    chunk.replace(start_of_chunk_header,
		  end_of_chunk_header - start_of_chunk_header,
		  make_start_of_chunk(is_last_chunk, first_did_in_chunk,
				      last_did_in_chunk));
}

void
PostlistChunkWriter::flush(GlassTable *table)
{
    LOGCALL_VOID(DB, "PostlistChunkWriter::flush", table);

    /* This is one of the more messy parts involved with updating posting
     * list chunks.
     *
     * Depending on circumstances, we may have to delete an entire chunk
     * or file it under a different key, as well as possibly modifying both
     * the previous and next chunk of the postlist.
     */

    if (!started) {
	/* This chunk is now empty so disappears entirely.
	 *
	 * If this was the last chunk, then the previous chunk
	 * must have its "is_last_chunk" flag updated.
	 *
	 * If this was the first chunk, then the next chunk must
	 * be transformed into the first chunk.  Messy!
	 */
	LOGLINE(DB, "PostlistChunkWriter::flush(): deleting chunk");
	Assert(!orig_key.empty());
	if (is_first_chunk) {
	    LOGLINE(DB, "PostlistChunkWriter::flush(): deleting first chunk");
	    if (is_last_chunk) {
		/* This is the first and the last chunk, ie the only
		 * chunk, so just delete the tag.
		 */
		table->del(orig_key);
		return;
	    }

	    /* This is the messiest case.  The first chunk is to
	     * be removed, and there is at least one chunk after
	     * it.  Need to rewrite the next chunk as the first
	     * chunk.
	     */
	    unique_ptr<GlassCursor> cursor(table->cursor_get());

	    if (!cursor->find_entry(orig_key)) {
		throw Xapian::DatabaseCorruptError("The key we're working on has disappeared");
	    }

	    // FIXME: Currently the doclen list has a special first chunk too,
	    // which reduces special casing here.  The downside is a slightly
	    // larger than necessary first chunk and needless fiddling if the
	    // first chunk is deleted.  But really we should look at
	    // redesigning the whole postlist format with an eye to making it
	    // easier to update!

	    // Extract existing counts from the first chunk so we can reinsert
	    // them into the block we're renaming.
	    Xapian::doccount num_ent;
	    Xapian::termcount coll_freq;
	    {
		cursor->read_tag();
		const char *tagpos = cursor->current_tag.data();
		const char *tagend = tagpos + cursor->current_tag.size();

		(void)read_start_of_first_chunk(&tagpos, tagend,
						&num_ent, &coll_freq);
	    }

	    // Seek to the next chunk.
	    cursor->next();
	    if (cursor->after_end()) {
		throw Xapian::DatabaseCorruptError("Expected another key but found none");
	    }
	    const char *kpos = cursor->current_key.data();
	    const char *kend = kpos + cursor->current_key.size();
	    if (!check_tname_in_key(&kpos, kend, tname)) {
		throw Xapian::DatabaseCorruptError("Expected another key with the same term name but found a different one");
	    }

	    // Read the new first docid
	    Xapian::docid new_first_did;
	    if (!unpack_uint_preserving_sort(&kpos, kend, &new_first_did)) {
		report_read_error(kpos);
	    }

	    cursor->read_tag();
	    const char *tagpos = cursor->current_tag.data();
	    const char *tagend = tagpos + cursor->current_tag.size();

	    // Read the chunk header
	    bool new_is_last_chunk;
	    Xapian::docid new_last_did_in_chunk =
		read_start_of_chunk(&tagpos, tagend, new_first_did,
				    &new_is_last_chunk);

	    string chunk_data(tagpos, tagend);

	    // First remove the renamed tag
	    table->del(cursor->current_key);

	    // And now write it as the first chunk
	    string tag;
	    tag = make_start_of_first_chunk(num_ent, coll_freq, new_first_did);
	    tag += make_start_of_chunk(new_is_last_chunk,
					      new_first_did,
					      new_last_did_in_chunk);
	    tag += chunk_data;
	    table->add(orig_key, tag);
	    return;
	}

	LOGLINE(DB, "PostlistChunkWriter::flush(): deleting secondary chunk");
	/* This isn't the first chunk.  Check whether we're the last chunk. */

	// Delete this chunk
	table->del(orig_key);

	if (is_last_chunk) {
	    LOGLINE(DB, "PostlistChunkWriter::flush(): deleting secondary last chunk");
	    // Update the previous chunk's is_last_chunk flag.
	    unique_ptr<GlassCursor> cursor(table->cursor_get());

	    /* Should not find the key we just deleted, but should
	     * find the previous chunk. */
	    if (cursor->find_entry(orig_key)) {
		throw Xapian::DatabaseCorruptError("Glass key not deleted as we expected");
	    }
	    // Make sure this is a chunk with the right term attached.
	    const char * keypos = cursor->current_key.data();
	    const char * keyend = keypos + cursor->current_key.size();
	    if (!check_tname_in_key(&keypos, keyend, tname)) {
		throw Xapian::DatabaseCorruptError("Couldn't find chunk before delete chunk");
	    }

	    bool is_prev_first_chunk = (keypos == keyend);

	    // Now update the last_chunk
	    cursor->read_tag();
	    string tag = cursor->current_tag;

	    const char *tagpos = tag.data();
	    const char *tagend = tagpos + tag.size();

	    // Skip first chunk header
	    Xapian::docid first_did_in_chunk;
	    if (is_prev_first_chunk) {
		first_did_in_chunk = read_start_of_first_chunk(&tagpos, tagend,
							       0, 0);
	    } else {
		if (!unpack_uint_preserving_sort(&keypos, keyend, &first_did_in_chunk))
		    report_read_error(keypos);
	    }
	    bool wrong_is_last_chunk;
	    string::size_type start_of_chunk_header = tagpos - tag.data();
	    Xapian::docid last_did_in_chunk =
		read_start_of_chunk(&tagpos, tagend, first_did_in_chunk,
				    &wrong_is_last_chunk);
	    string::size_type end_of_chunk_header = tagpos - tag.data();

	    // write new is_last flag
	    write_start_of_chunk(tag,
				 start_of_chunk_header,
				 end_of_chunk_header,
				 true, // is_last_chunk
				 first_did_in_chunk,
				 last_did_in_chunk);
	    table->add(cursor->current_key, tag);
	}
    } else {
	LOGLINE(DB, "PostlistChunkWriter::flush(): updating chunk which still has items in it");
	/* The chunk still has some items in it.  Two major subcases:
	 * a) This is the first chunk.
	 * b) This isn't the first chunk.
	 *
	 * The subcases just affect the chunk header.
	 */
	string tag;

	/* First write the header, which depends on whether this is the
	 * first chunk.
	 */
	if (is_first_chunk) {
	    /* The first chunk.  This is the relatively easy case,
	     * and we just have to write this one back to disk.
	     */
	    LOGLINE(DB, "PostlistChunkWriter::flush(): rewriting the first chunk, which still has items in it");
	    string key = GlassPostListTable::make_key(tname);
	    bool ok = table->get_exact_entry(key, tag);
	    (void)ok;
	    Assert(ok);
	    Assert(!tag.empty());

	    Xapian::doccount num_ent;
	    Xapian::termcount coll_freq;
	    {
		const char * tagpos = tag.data();
		const char * tagend = tagpos + tag.size();
		(void)read_start_of_first_chunk(&tagpos, tagend,
						&num_ent, &coll_freq);
	    }

	    tag = make_start_of_first_chunk(num_ent, coll_freq, first_did);

	    tag += make_start_of_chunk(is_last_chunk, first_did, current_did);
	    tag += chunk;
	    table->add(key, tag);
	    return;
	}

	LOGLINE(DB, "PostlistChunkWriter::flush(): updating secondary chunk which still has items in it");
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
	if (!check_tname_in_key(&keypos, keyend, tname)) {
	    throw Xapian::DatabaseCorruptError("Have invalid key writing to postlist");
	}
	Xapian::docid initial_did;
	if (!unpack_uint_preserving_sort(&keypos, keyend, &initial_did)) {
	    report_read_error(keypos);
	}
	string new_key;
	if (initial_did != first_did) {
	    /* The fiddlier case:
	     * Create a new tag with the correct key, and replace
	     * the old one.
	     */
	    new_key = GlassPostListTable::make_key(tname, first_did);
	    table->del(orig_key);
	} else {
	    new_key = orig_key;
	}

	// ...and write the start of this chunk.
	tag = make_start_of_chunk(is_last_chunk, first_did, current_did);

	tag += chunk;
	table->add(new_key, tag);
    }
}

/** Read the number of entries in the posting list.
 *  This must only be called when *posptr is pointing to the start of
 *  the first chunk of the posting list.
 */
void GlassPostList::read_number_of_entries(const char ** posptr,
				   const char * end,
				   Xapian::doccount * number_of_entries_ptr,
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
 *  length : name).  Key for subsequent chunks is the same, followed by the
 *  document ID of the first document in the chunk (encoded as length of
 *  representation in first byte, and then docid).
 *
 *  A chunk (except for the first chunk) contains:
 *
 *  1)  bool - true if this is the last chunk.
 *  2)  difference between final docid in chunk and first docid.
 *  3)  wdf for the first item.
 *  4)  increment in docid to next item, followed by wdf for the item.
 *  5)  (4) repeatedly.
 *
 *  The first chunk begins with the number of entries, the collection
 *  frequency, then the docid of the first document, then has the header of a
 *  standard chunk.
 */
GlassPostList::GlassPostList(intrusive_ptr<const GlassDatabase> this_db_,
			     const string & term_,
			     bool keep_reference)
	: LeafPostList(term_),
	  this_db(keep_reference ? this_db_ : NULL),
	  have_started(false),
	  is_at_end(false),
	  cursor(this_db_->postlist_table.cursor_get())
{
    LOGCALL_CTOR(DB, "GlassPostList", this_db_.get() | term_ | keep_reference);
    init();
}

GlassPostList::GlassPostList(intrusive_ptr<const GlassDatabase> this_db_,
			     const string & term_,
			     GlassCursor * cursor_)
	: LeafPostList(term_),
	  this_db(this_db_),
	  have_started(false),
	  is_at_end(false),
	  cursor(cursor_)
{
    LOGCALL_CTOR(DB, "GlassPostList", this_db_.get() | term_ | cursor_);
    init();
}

void
GlassPostList::init()
{
    string key = GlassPostListTable::make_key(term);
    int found = cursor->find_entry(key);
    if (!found) {
	LOGLINE(DB, "postlist for term not found");
	number_of_entries = 0;
	is_at_end = true;
	pos = 0;
	end = 0;
	first_did_in_chunk = 0;
	last_did_in_chunk = 0;
	return;
    }
    cursor->read_tag();
    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    did = read_start_of_first_chunk(&pos, end, &number_of_entries, NULL);
    first_did_in_chunk = did;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk,
					    &is_last_chunk);
    read_wdf(&pos, end, &wdf);
    LOGLINE(DB, "Initial docid " << did);
}

GlassPostList::~GlassPostList()
{
    LOGCALL_DTOR(DB, "GlassPostList");
    delete positionlist;
}

LeafPostList *
GlassPostList::open_nearby_postlist(const std::string & term_,
				    bool need_read_pos) const
{
    LOGCALL(DB, LeafPostList *, "GlassPostList::open_nearby_postlist", term_ | need_read_pos);
    (void)need_read_pos;
    if (term_.empty())
	RETURN(NULL);
    if (!this_db.get() || this_db->postlist_table.is_writable())
	RETURN(NULL);
    RETURN(new GlassPostList(this_db, term_, cursor->clone()));
}

bool
GlassPostList::next_in_chunk()
{
    LOGCALL(DB, bool, "GlassPostList::next_in_chunk", NO_ARGS);
    if (pos == end) RETURN(false);

    read_did_increase(&pos, end, &did);
    read_wdf(&pos, end, &wdf);

    // Either not at last doc in chunk, or pos == end, but not both.
    Assert(did <= last_did_in_chunk);
    Assert(did < last_did_in_chunk || pos == end);
    Assert(pos != end || did == last_did_in_chunk);

    RETURN(true);
}

void
GlassPostList::next_chunk()
{
    LOGCALL_VOID(DB, "GlassPostList::next_chunk", NO_ARGS);
    if (is_last_chunk) {
	is_at_end = true;
	return;
    }

    cursor->next();
    if (cursor->after_end()) {
	is_at_end = true;
	throw Xapian::DatabaseCorruptError("Unexpected end of posting list for '" +
				     term + "'");
    }
    const char * keypos = cursor->current_key.data();
    const char * keyend = keypos + cursor->current_key.size();
    // Check we're still in same postlist
    if (!check_tname_in_key_lite(&keypos, keyend, term)) {
	is_at_end = true;
	throw Xapian::DatabaseCorruptError("Unexpected end of posting list for '" +
				     term + "'");
    }

    Xapian::docid newdid;
    if (!unpack_uint_preserving_sort(&keypos, keyend, &newdid)) {
	report_read_error(keypos);
    }
    if (newdid <= did) {
	throw Xapian::DatabaseCorruptError("Document ID in new chunk of postlist (" +
		str(newdid) +
		") is not greater than final document ID in previous chunk (" +
		str(did) + ")");
    }
    did = newdid;

    cursor->read_tag();
    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    first_did_in_chunk = did;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk,
					    &is_last_chunk);
    read_wdf(&pos, end, &wdf);
}

PositionList *
GlassPostList::read_position_list()
{
    LOGCALL(DB, PositionList *, "GlassPostList::read_position_list", NO_ARGS);
    Assert(this_db.get());
    if (rare(positionlist == NULL)) {
	// Lazily create positionlist to avoid the size cost for the common
	// case where we don't want positional data.
	positionlist = new GlassRePositionList(&this_db->position_table);
    }
    this_db->read_position_list(positionlist, did, term);
    RETURN(positionlist);
}

PositionList *
GlassPostList::open_position_list() const
{
    LOGCALL(DB, PositionList *, "GlassPostList::open_position_list", NO_ARGS);
    Assert(this_db.get());
    RETURN(this_db->open_position_list(did, term));
}

PostList *
GlassPostList::next(double w_min)
{
    LOGCALL(DB, PostList *, "GlassPostList::next", w_min);
    (void)w_min; // no warning

    if (!have_started) {
	have_started = true;
    } else {
	if (!next_in_chunk()) next_chunk();
    }

    if (is_at_end) {
	LOGLINE(DB, "Moved to end");
    } else {
	LOGLINE(DB, "Moved to docid " << did << ", wdf = " << wdf);
    }

    RETURN(NULL);
}

bool
GlassPostList::current_chunk_contains(Xapian::docid desired_did)
{
    LOGCALL(DB, bool, "GlassPostList::current_chunk_contains", desired_did);
    if (desired_did >= first_did_in_chunk &&
	desired_did <= last_did_in_chunk) {
	RETURN(true);
    }
    RETURN(false);
}

void
GlassPostList::move_to_chunk_containing(Xapian::docid desired_did)
{
    LOGCALL_VOID(DB, "GlassPostList::move_to_chunk_containing", desired_did);
    (void)cursor->find_entry(GlassPostListTable::make_key(term, desired_did));
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.data();
    const char * keyend = keypos + cursor->current_key.size();
    // Check we're still in same postlist
    if (!check_tname_in_key_lite(&keypos, keyend, term)) {
	// This should only happen if the postlist doesn't exist at all.
	is_at_end = true;
	is_last_chunk = true;
	return;
    }
    is_at_end = false;

    cursor->read_tag();
    pos = cursor->current_tag.data();
    end = pos + cursor->current_tag.size();

    if (keypos == keyend) {
	// In first chunk
#ifdef XAPIAN_ASSERTIONS
	Xapian::doccount old_number_of_entries = number_of_entries;
	did = read_start_of_first_chunk(&pos, end, &number_of_entries, NULL);
	Assert(old_number_of_entries == number_of_entries);
#else
	did = read_start_of_first_chunk(&pos, end, NULL, NULL);
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
    read_wdf(&pos, end, &wdf);

    // Possible, since desired_did might be after end of this chunk and before
    // the next.
    if (desired_did > last_did_in_chunk) next_chunk();
}

bool
GlassPostList::move_forward_in_chunk_to_at_least(Xapian::docid desired_did)
{
    LOGCALL(DB, bool, "GlassPostList::move_forward_in_chunk_to_at_least", desired_did);
    if (did >= desired_did)
	RETURN(true);

    if (desired_did <= last_did_in_chunk) {
	while (pos != end) {
	    read_did_increase(&pos, end, &did);
	    if (did >= desired_did) {
		read_wdf(&pos, end, &wdf);
		RETURN(true);
	    }
	    // It's faster to just skip over the wdf than to decode it.
	    read_wdf(&pos, end, NULL);
	}

	// If we hit the end of the chunk then last_did_in_chunk must be wrong.
	Assert(false);
    }

    pos = end;
    RETURN(false);
}

PostList *
GlassPostList::skip_to(Xapian::docid desired_did, double w_min)
{
    LOGCALL(DB, PostList *, "GlassPostList::skip_to", desired_did | w_min);
    (void)w_min; // no warning
    // We've started now - if we hadn't already, we're already positioned
    // at start so there's no need to actually do anything.
    have_started = true;

    // Don't skip back, and don't need to do anything if already there.
    if (is_at_end || desired_did <= did) RETURN(NULL);

    // Move to correct chunk
    if (!current_chunk_contains(desired_did)) {
	move_to_chunk_containing(desired_did);
	// Might be at_end now, so we need to check before trying to move
	// forward in chunk.
	if (is_at_end) RETURN(NULL);
    }

    // Move to correct position in chunk
    bool have_document = move_forward_in_chunk_to_at_least(desired_did);
    (void)have_document;
    Assert(have_document);

    if (is_at_end) {
	LOGLINE(DB, "Skipped to end");
    } else {
	LOGLINE(DB, "Skipped to docid " << did << ", wdf = " << wdf);
    }

    RETURN(NULL);
}

// Used for doclens.
bool
GlassPostList::jump_to(Xapian::docid desired_did)
{
    LOGCALL(DB, bool, "GlassPostList::jump_to", desired_did);
    // We've started now - if we hadn't already, we're already positioned
    // at start so there's no need to actually do anything.
    have_started = true;

    // If the list is empty, give up right away.
    if (pos == 0) RETURN(false);

    // Move to correct chunk, or reload the current chunk to go backwards in it
    // (FIXME: perhaps handle the latter case more elegantly, though it won't
    // happen during sequential access which is most common).
    if (is_at_end || !current_chunk_contains(desired_did) || desired_did < did) {
	// Clear is_at_end flag since we can rewind.
	is_at_end = false;

	move_to_chunk_containing(desired_did);
	// Might be at_end now, so we need to check before trying to move
	// forward in chunk.
	if (is_at_end) RETURN(false);
    }

    // Move to correct position in chunk.
    if (!move_forward_in_chunk_to_at_least(desired_did)) RETURN(false);
    RETURN(desired_did == did);
}

string
GlassPostList::get_description() const
{
    string desc;
    description_append(desc, term);
    desc += ":";
    desc += str(number_of_entries);
    return desc;
}

// Returns the last did to allow in this chunk.
Xapian::docid
GlassPostListTable::get_chunk(const string &tname,
			      Xapian::docid did, bool adding,
			      PostlistChunkReader ** from,
			      PostlistChunkWriter **to)
{
    LOGCALL(DB, Xapian::docid, "GlassPostListTable::get_chunk", tname | did | adding | from | to);
    // Get chunk containing entry
    string key = make_key(tname, did);

    // Find the right chunk
    unique_ptr<GlassCursor> cursor(cursor_get());

    (void)cursor->find_entry(key);
    Assert(!cursor->after_end());

    const char * keypos = cursor->current_key.data();
    const char * keyend = keypos + cursor->current_key.size();

    if (!check_tname_in_key(&keypos, keyend, tname)) {
	// Postlist for this termname doesn't exist.
	//
	// NB "adding" will only be true if we are adding, but it may sometimes
	// be false in some cases where we are actually adding.
	if (!adding)
	    throw Xapian::DatabaseCorruptError("Attempted to delete or modify an entry in a non-existent posting list for " + tname);

	*from = NULL;
	*to = new PostlistChunkWriter(string(), true, tname, true);
	RETURN(Xapian::docid(-1));
    }

    // See if we're appending - if so we can shortcut by just copying
    // the data part of the chunk wholesale.
    bool is_first_chunk = (keypos == keyend);
    LOGVALUE(DB, is_first_chunk);

    cursor->read_tag();
    const char * pos = cursor->current_tag.data();
    const char * end = pos + cursor->current_tag.size();
    Xapian::docid first_did_in_chunk;
    if (is_first_chunk) {
	first_did_in_chunk = read_start_of_first_chunk(&pos, end, NULL, NULL);
    } else {
	if (!unpack_uint_preserving_sort(&keypos, keyend, &first_did_in_chunk)) {
	    report_read_error(keypos);
	}
    }

    bool is_last_chunk;
    Xapian::docid last_did_in_chunk;
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk);
    *to = new PostlistChunkWriter(cursor->current_key, is_first_chunk, tname,
				  is_last_chunk);
    if (did > last_did_in_chunk) {
	// This is the shortcut.  Not very pretty, but I'll leave refactoring
	// until I've a clearer picture of everything which needs to be done.
	// (FIXME)
	*from = NULL;
	(*to)->raw_append(first_did_in_chunk, last_did_in_chunk,
			  string(pos, end));
    } else {
	*from = new PostlistChunkReader(first_did_in_chunk, string(pos, end));
    }
    if (is_last_chunk) RETURN(Xapian::docid(-1));

    // Find first did of next tag.
    cursor->next();
    if (cursor->after_end()) {
	throw Xapian::DatabaseCorruptError("Expected another key but found none");
    }
    const char *kpos = cursor->current_key.data();
    const char *kend = kpos + cursor->current_key.size();
    if (!check_tname_in_key(&kpos, kend, tname)) {
	throw Xapian::DatabaseCorruptError("Expected another key with the same term name but found a different one");
    }

    // Read the new first docid
    Xapian::docid first_did_of_next_chunk;
    if (!unpack_uint_preserving_sort(&kpos, kend, &first_did_of_next_chunk)) {
	report_read_error(kpos);
    }
    RETURN(first_did_of_next_chunk - 1);
}

void
GlassPostListTable::merge_doclen_changes(const map<Xapian::docid, Xapian::termcount> & doclens)
{
    LOGCALL_VOID(DB, "GlassPostListTable::merge_doclen_changes", doclens);

    // The cursor in the doclen_pl will no longer be valid, so reset it.
    doclen_pl.reset(0);

    LOGVALUE(DB, doclens.size());
    if (doclens.empty()) return;

    // Ensure there's a first chunk.
    string current_key = make_key(string());
    if (!key_exists(current_key)) {
	LOGLINE(DB, "Adding dummy first chunk");
	string newtag = make_start_of_first_chunk(0, 0, 0);
	newtag += make_start_of_chunk(true, 0, 0);
	add(current_key, newtag);
    }

    map<Xapian::docid, Xapian::termcount>::const_iterator j;
    j = doclens.begin();
    Assert(j != doclens.end()); // This case is caught above.

    Xapian::docid max_did;
    PostlistChunkReader *from;
    PostlistChunkWriter *to;
    max_did = get_chunk(string(), j->first, true, &from, &to);
    LOGVALUE(DB, max_did);
    for ( ; j != doclens.end(); ++j) {
	Xapian::docid did = j->first;

next_doclen_chunk:
	LOGLINE(DB, "Updating doclens, did=" << did);
	if (from) while (!from->is_at_end()) {
	    Xapian::docid copy_did = from->get_docid();
	    if (copy_did >= did) {
		if (copy_did == did) from->next();
		break;
	    }
	    to->append(this, copy_did, from->get_wdf());
	    from->next();
	}
	if ((!from || from->is_at_end()) && did > max_did) {
	    delete from;
	    to->flush(this);
	    delete to;
	    max_did = get_chunk(string(), did, false, &from, &to);
	    goto next_doclen_chunk;
	}

	Xapian::termcount new_doclen = j->second;
	if (new_doclen != static_cast<Xapian::termcount>(-1)) {
	    to->append(this, did, new_doclen);
	}
    }

    if (from) {
	while (!from->is_at_end()) {
	    to->append(this, from->get_docid(), from->get_wdf());
	    from->next();
	}
	delete from;
    }
    to->flush(this);
    delete to;
}

void
GlassPostListTable::merge_changes(const string &term,
				  const Inverter::PostingChanges & changes)
{
    {
	// Rewrite the first chunk of this posting list with the updated
	// termfreq and collfreq.
	string current_key = make_key(term);
	string tag;
	(void)get_exact_entry(current_key, tag);

	// Read start of first chunk to get termfreq and collfreq.
	const char *pos = tag.data();
	const char *end = pos + tag.size();
	Xapian::doccount termfreq;
	Xapian::termcount collfreq;
	Xapian::docid firstdid, lastdid;
	bool islast;
	if (pos == end) {
	    termfreq = 0;
	    collfreq = 0;
	    firstdid = 0;
	    lastdid = 0;
	    islast = true;
	} else {
	    firstdid = read_start_of_first_chunk(&pos, end,
						 &termfreq, &collfreq);
	    // Handle the generic start of chunk header.
	    lastdid = read_start_of_chunk(&pos, end, firstdid, &islast);
	}

	termfreq += changes.get_tfdelta();
	if (termfreq == 0) {
	    // All postings deleted!  So we can shortcut by zapping the
	    // posting list.
	    if (islast) {
		// Only one entry for this posting list.
		del(current_key);
		return;
	    }
	    MutableGlassCursor cursor(this);
	    bool found = cursor.find_entry(current_key);
	    Assert(found);
	    if (!found) return; // Reduce damage!
	    while (cursor.del()) {
		const char *kpos = cursor.current_key.data();
		const char *kend = kpos + cursor.current_key.size();
		if (!check_tname_in_key_lite(&kpos, kend, term)) break;
	    }
	    return;
	}
	collfreq += changes.get_cfdelta();

	// Rewrite start of first chunk to update termfreq and collfreq.
	string newhdr = make_start_of_first_chunk(termfreq, collfreq, firstdid);
	newhdr += make_start_of_chunk(islast, firstdid, lastdid);
	if (pos == end) {
	    add(current_key, newhdr);
	} else {
	    Assert((size_t)(pos - tag.data()) <= tag.size());
	    tag.replace(0, pos - tag.data(), newhdr);
	    add(current_key, tag);
	}
    }
    map<Xapian::docid, Xapian::termcount>::const_iterator j;
    j = changes.pl_changes.begin();
    Assert(j != changes.pl_changes.end()); // This case is caught above.

    Xapian::docid max_did;
    PostlistChunkReader *from;
    PostlistChunkWriter *to;
    max_did = get_chunk(term, j->first, false, &from, &to);
    for ( ; j != changes.pl_changes.end(); ++j) {
	Xapian::docid did = j->first;

next_chunk:
	LOGLINE(DB, "Updating term=" << term << ", did=" << did);
	if (from) while (!from->is_at_end()) {
	    Xapian::docid copy_did = from->get_docid();
	    if (copy_did >= did) {
		if (copy_did == did) {
		    from->next();
		}
		break;
	    }
	    to->append(this, copy_did, from->get_wdf());
	    from->next();
	}
	if ((!from || from->is_at_end()) && did > max_did) {
	    delete from;
	    to->flush(this);
	    delete to;
	    max_did = get_chunk(term, did, false, &from, &to);
	    goto next_chunk;
	}

	Xapian::termcount new_wdf = j->second;
	if (new_wdf != Xapian::termcount(-1)) {
	    to->append(this, did, new_wdf);
	}
    }

    if (from) {
	while (!from->is_at_end()) {
	    to->append(this, from->get_docid(), from->get_wdf());
	    from->next();
	}
	delete from;
    }
    to->flush(this);
    delete to;
}

void
GlassPostListTable::get_used_docid_range(Xapian::docid & first,
					 Xapian::docid & last) const
{
    LOGCALL(DB, Xapian::docid, "GlassPostListTable::get_used_docid_range", "&first, &used");
    unique_ptr<GlassCursor> cur(cursor_get());
    if (!cur->find_entry(pack_glass_postlist_key(string()))) {
	// Empty database.
	first = last = 0;
	return;
    }

    cur->read_tag();
    const char * p = cur->current_tag.data();
    const char * e = p + cur->current_tag.size();

    first = read_start_of_first_chunk(&p, e, NULL, NULL);

    (void)cur->find_entry(pack_glass_postlist_key(string(), GLASS_MAX_DOCID));
    Assert(!cur->after_end());

    const char * keypos = cur->current_key.data();
    const char * keyend = keypos + cur->current_key.size();
    // Check we're still in same postlist
    if (!check_tname_in_key_lite(&keypos, keyend, string())) {
	// Shouldn't happen - we already handled the empty database case above.
	Assert(false);
	first = last = 0;
	return;
    }

    cur->read_tag();
    p = cur->current_tag.data();
    e = p + cur->current_tag.size();

    Xapian::docid start_of_last_chunk;
    if (keypos == keyend) {
	start_of_last_chunk = first;
	first = read_start_of_first_chunk(&p, e, NULL, NULL);
    } else {
	// In normal chunk
	if (!unpack_uint_preserving_sort(&keypos, keyend,
					 &start_of_last_chunk)) {
	    report_read_error(keypos);
	}
    }

    bool dummy;
    last = read_start_of_chunk(&p, e, start_of_last_chunk, &dummy);
}

#ifdef DISABLE_GPL_LIBXAPIAN
# error GPL source we cannot relicense included in libxapian
#endif
