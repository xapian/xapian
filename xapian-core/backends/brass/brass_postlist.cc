/* brass_postlist.cc: Postlists in a brass database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2007,2008,2009,2011,2013,2014 Olly Betts
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

#include "brass_postlist.h"

#include "brass_cursor.h"
#include "brass_database.h"
#include "debuglog.h"
#include "noreturn.h"
#include "pack.h"
#include "str.h"
#include <queue>
#include "unicode/description_append.h"
#include "math.h"

using Xapian::Internal::intrusive_ptr;

void
BrassPostListTable::get_freqs(const string & term,
			      Xapian::doccount * termfreq_ptr,
			      Xapian::termcount * collfreq_ptr) const
{
    string key = make_key(term);
    string tag;
    if (!get_exact_entry(key, tag)) {
	if (termfreq_ptr)
	    *termfreq_ptr = 0;
	if (collfreq_ptr)
	    *collfreq_ptr = 0;
    } else {
	const char * p = tag.data();
	BrassPostList::read_number_of_entries(&p, p + tag.size(),
					      termfreq_ptr, collfreq_ptr);
    }
}

Xapian::termcount
BrassPostListTable::get_doclength(Xapian::docid did,
				  intrusive_ptr<const BrassDatabase> db) const {
    if (!doclen_pl.get()) {
	// Don't keep a reference back to the database, since this
	// would make a reference loop.
	doclen_pl.reset(new BrassPostList(db, string(), false));
    }
    if (!doclen_pl->jump_to(did))
	throw Xapian::DocNotFoundError("Document " + str(did) + " not found");
    return doclen_pl->get_wdf();
}

bool
BrassPostListTable::document_exists(Xapian::docid did,
				    intrusive_ptr<const BrassDatabase> db) const
{
    if (!doclen_pl.get()) {
	// Don't keep a reference back to the database, since this
	// would make a reference loop.
	doclen_pl.reset(new BrassPostList(db, string(), false));
    }
    return (doclen_pl->jump_to(did));
}

// Static functions

/// Report an error when reading the posting list.
XAPIAN_NORETURN(static void report_read_error(const char * position));
static void report_read_error(const char * position)
{
    if (position == 0) {
	// data ran out
	LOGLINE(DB, "BrassPostList data ran out");
	throw Xapian::DatabaseCorruptError("Data ran out unexpectedly when reading posting list.");
    }
    // overflow
    LOGLINE(DB, "BrassPostList value too large");
    throw Xapian::RangeError("Value in posting list too large.");
}

static inline bool get_tname_from_key(const char **src, const char *end,
			       string &tname)
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

    BrassPostList::read_number_of_entries(posptr, end,
			   number_of_entries_ptr, collection_freq_ptr);
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


/// Calculate the number of bytes we need to encode an integer
static inline
unsigned get_max_bytes(unsigned n)
{
	if (n== 0) {
		///we need at least one byte
		return 1;
	}
	unsigned l=0;
	while (n) {
		l++;
		n >>= 8;
	}
	return l;
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


/** Read the number of entries in the posting list.
 *  This must only be called when *posptr is pointing to the start of
 *  the first chunk of the posting list.
 */
void BrassPostList::read_number_of_entries(const char ** posptr,
				   const char * end,
				   Xapian::doccount * number_of_entries_ptr,
				   Xapian::termcount * collection_freq_ptr)
{
    if (!unpack_uint(posptr, end, number_of_entries_ptr))
	report_read_error(*posptr);
    if (!unpack_uint(posptr, end, collection_freq_ptr))
	report_read_error(*posptr);
}

/* @pos_ : a pointer to the end of the header of the chunk.
 * @end_ : a pointer to the end of the chunk.
 * @first_did_in_chunk_ : first doc id in this chunk */
FixedWidthChunk::FixedWidthChunk( map<Xapian::docid,Xapian::termcount>::const_iterator pl_start_, 
								 map<Xapian::docid,Xapian::termcount>::const_iterator pl_end_)
								 : pl_start(pl_start_), pl_end(pl_end_)
{}

///encode the post list
bool 
FixedWidthChunk::encode(string& desired_chunk)
{
	if (pl_start == pl_end) {
		//the map is empty.
		LOGLINE( DB, "Desired postlist is empty!" );
		return false;
	}

	/* we use @it to go through the map, 
	 * if we aren't in continuous block, we will encode doc id and doc length normally
	 * but if we are, we do some special operations, 
	 * and apply fixed width format. 
	 * @start_pos : it tracks the start of the continuous block we are in currently. 
	 * @docid_before_start_pos : doc id before current continuous block */
	map<Xapian::docid,Xapian::termcount>::const_iterator it = pl_start, start_pos;
	Xapian::docid docid_before_start_pos = it->first;

	//go throuth the whole map
	while (it != pl_end)
	{
		unsigned length_contiguous = 1;
		Xapian::docid last_docid = it->first, cur_docid = 0;

		//number of bytes to encode a length.
		unsigned max_bytes = get_max_bytes(it->second);

		/* Since different number of bytes is needed for different length
		 * and we must select the max number of bytes
		 * to make sure all lengths in this continuous block can be encoded, 
		 * some bytes may be wasted. 
		 * @used_bytes: number of bytes in all
		 * @good_bytes: number of bytes which isn't wasted.
		 * We require the ratio good_bytes/used_bytes must be bigger than a certain value,
		 * in case much space is wasted. */
		unsigned used_bytes = 0;
		unsigned good_bytes = 0;

		start_pos = it;
		it++;
		while (it != pl_end)
		{
			cur_docid = it->first;

			//number of bytes to encode current doc length
			unsigned cur_bytes = get_max_bytes(it->second);
			if (cur_docid != last_docid+1 || cur_bytes > max_bytes) {
				/* if current id isn't continuous with last id,
				 * or the number of bytes we need to store this doc length 
				 * is bigger than the max bytes needed in this continuous block,
				 * we don't encode current doc id into current block. */
				break;
			}

			/* used_bytes is all bytes to be used,
			 * but only @cur_bytes isn't wasted. */
			used_bytes += max_bytes;
			good_bytes += cur_bytes;
			if ((double)good_bytes/used_bytes < DOCLEN_CHUNK_MIN_GOOD_BYTES_RATIO) {
				/* the ratio is too small, which means too much space is wasted,
				 * so we will start a new block. */
				used_bytes = 0;
				good_bytes = 0;
				break;
			}

			length_contiguous++;
			last_docid = cur_docid;
			it++;
		}

		if (length_contiguous > DOCLEN_CHUNK_MIN_CONTIGUOUS_LENGTH) {
			//this block is long enough, fixed width format will be used.

			//an indicator indicating the start of fixed width format chunk
			pack_uint(desired_chunk, SEPERATOR);

			//the delta of doc id
			pack_uint(desired_chunk, start_pos->first-docid_before_start_pos);

			//the length of the block
			pack_uint_in_bytes(length_contiguous, 2, desired_chunk);
			
			//number of bytes to encode each doc length in this block
			pack_uint_in_bytes(max_bytes, 1, desired_chunk);

			while (start_pos != it) {
				pack_uint_in_bytes(start_pos->second, max_bytes, desired_chunk);
				docid_before_start_pos = start_pos->first;
				start_pos++;
			}
		} else {
			//this block isn't long enough, we encode it normally.
			while (start_pos != it)
			{
				//the delta of doc id
				pack_uint(desired_chunk, start_pos->first-docid_before_start_pos);

				//the doc length
				pack_uint(desired_chunk, start_pos->second);
				docid_before_start_pos = start_pos->first;
				start_pos++;
			}
		}

	}
	return true;
}

/* move to next did in the chunk. 
 * If no more did, set is_at_end=true. */
bool 
FixedWidthChunkReader::next()
{
	LOGCALL(DB, bool, "FixedWidthChunkReader::next", NO_ARGS);
	if (is_at_end) {
		RETURN(false);
	}
	if (pos == end) {
		//have run out the chunk
		is_at_end = true;
		RETURN(false);
	}
	if (is_in_block && len_info) {
		//we are in a continuous block and there are some entries haven't been read in this block

		//the delta of doc id in continuous block is always 1
		cur_did++;

		//number of entries decreases
		len_info--;
		if (len_info == 0) {
			//all entries in this chunk have been read, so we aren't in a continuous block now
			is_in_block = false;
		}
		//get doc length
		unpack_uint_in_bytes(&pos, bytes_info, &cur_length);
		RETURN(true);
	}

	//we aren't in a continuous block
	Xapian::docid incre_did = 0;

	//store the start of a continuous block
	pos_of_block = pos;

	//get the increment of doc id
	unpack_uint(&pos, end, &incre_did);
	if (incre_did != SEPERATOR)	{
		//we are still in normal format
		is_in_block = false;
		cur_did += incre_did;
		unpack_uint(&pos, end, &cur_length);
		RETURN(true);
	}

	//we will go into fixed width format

	//we are in a continuous block now
	is_in_block = true;
	unpack_uint(&pos, end, &incre_did);

	//get basic info of the continuous block
	unpack_uint_in_bytes(&pos, 2, &len_info);
	unpack_uint_in_bytes(&pos, 1, &bytes_info);

	//store the doc id before this block
	did_before_block = cur_did;

	//get first doc id in this block
	cur_did += incre_did;

	//get first doc length in this block
	unpack_uint_in_bytes(&pos, bytes_info, &cur_length);
	len_info--;
	if (len_info == 0) {
		// if there is no other entries, we have run out the block
		is_in_block = false;
	}
	RETURN(true);
}


/* jump to desired did, 
 * if it fails, it will arrive at the exact did just after @desired_did */
bool 
FixedWidthChunkReader::jump_to(Xapian::docid desired_did)
{

	LOGCALL(DB, bool, "FixedWidthChunkReader::jump_to", desired_did);

	if (cur_did == desired_did)	{
		RETURN(true);
	}
	if (is_in_block) {
		/* we are in a continuous block, 
		 * if desired doc id is smaller than the doc id just before this block,
		 * we should go back to the start of the whole chunk,
		 * otherwise, we just go back to the start of this continuous block. */
		if (did_before_block >= desired_did) {
			pos = ori_pos;
			cur_did = first_did_in_chunk;
		} else {
			pos = pos_of_block;
			cur_did = did_before_block;
		}
	} else if (cur_did > desired_did)
	{
		/* we aren't in a continuous block, 
		 * and desired doc id is smaller than current doc id,
		 * so we have to go back to the start of the whole chunk. */
		pos = ori_pos;
		cur_did = first_did_in_chunk;
	}

	Xapian::docid incre_did = 0;
	while (pos!=end)
	{
		//store the start of current block
		pos_of_block = pos;
		
		//get the increment of doc id
		unpack_uint(&pos, end, &incre_did);

		if (incre_did != SEPERATOR)	{
			// we aren't in a continuous block.
			is_in_block = false;
			cur_did += incre_did;
			unpack_uint(&pos, end, &cur_length);
			if (cur_did == desired_did) {
				RETURN(true);
			}
			if (cur_did > desired_did) {
				//In this case, desired doc id doesn't exist.
				RETURN(false);
			}
			continue;
		} else {
			// we are in a continuous block
			is_in_block = true;

			//get the increment of doc id
			unpack_uint(&pos, end, &incre_did);

			//get the number of entries in this block
			unpack_uint_in_bytes(&pos, 2, &len_info);

			//get the fixed number of bytes in this block
			unpack_uint_in_bytes(&pos, 1, &bytes_info);

			//store the doc id just before this block
			did_before_block = cur_did;

			cur_did += incre_did;
			if (desired_did < cur_did) {
				//In this case, desired doc id doesn't exist.
				unpack_uint_in_bytes(&pos, bytes_info, &cur_length);
				len_info--;
				RETURN(false);
			}

			/* As the increment of doc id in a continuous block is always 1,
			 * the min doc id in this block is @cur_did,
			 * the max doc id in this block is @cur_did+@len_info-1. */
			if (desired_did <= cur_did+len_info-1) {
				//desired doc id is in this block, move the pointer to the length of desired doc id
				pos += bytes_info*(desired_did-cur_did);
				unpack_uint_in_bytes(&pos, bytes_info, &cur_length);

				//update number of remaining entries in this block 
				len_info -= (desired_did-cur_did+1);
				if (len_info == 0) {
					//no other entries, we aren't in a block now
					is_in_block = false;
				}

				//update current doc id
				cur_did = desired_did;
				RETURN(true);
			}

			//desired doc id isn't in this block, we just skip this block
			pos += bytes_info*len_info;
			cur_did += len_info-1;
			is_in_block = false;
		}

	}
	RETURN(false);
}

//merge old map and new map, the @new_doclen is valid.
bool 
DoclenChunkWriter::get_new_doclen( )
{
	LOGCALL(DB, bool, "DoclenChunkWriter::get_new_doclen", NO_ARGS);
	const char* pos = chunk_from.data();
	const char* end = pos+chunk_from.size();

	//deal with the header of the chunk
	if (is_first_chunk) {
		read_start_of_first_chunk(&pos, end, NULL, NULL);
	}
	read_start_of_chunk(&pos, end, 0, &is_last_chunk); 

	if (pos == end) {
		//original chunk is empty, we just do some deleting according to changes
		LOGLINE( DB, "empty chunk!" );
		map<Xapian::docid,Xapian::termcount>::const_iterator it = changes_start;
		for (; it!=changes_end ; ++it) {
			//the doc length is -1, which means we should delete this doc id
			if (it->second != SEPERATOR) {
				new_doclen.insert(new_doclen.end(), *it);
			}
		}
		if (new_doclen.empty()) {
			LOGLINE(DB, "new_doclen is empty! ");
			RETURN(false);
		}
	} else {
		//read old map of <docid,length> from original chunk
		Xapian::docid cur_did = 0, inc_did = 0;
		Xapian::termcount doc_len = 0;
		cur_did = first_did_in_chunk;
		while (pos!=end)
		{
			//get the increment of the doc id
			unpack_uint(&pos, end, &inc_did);
			if (inc_did != SEPERATOR) {
				//normal format
				cur_did += inc_did;
				unpack_uint(&pos, end, &doc_len);
				new_doclen.insert(new_doclen.end(), make_pair<Xapian::docid,Xapian::termcount>(cur_did,doc_len));
				continue;
			} else {
				//fixed width format
				unpack_uint(&pos, end, &inc_did);
				unsigned len=0, bytes=0;

				//get the number of entries in this block
				unpack_uint_in_bytes(&pos, 2, &len);

				//get the fixed number of bytes in this block
				unpack_uint_in_bytes(&pos, 1, &bytes);
				cur_did += inc_did;
				while (len--) {
					unpack_uint_in_bytes(&pos, bytes, &doc_len);
					new_doclen.insert(new_doclen.end(), make_pair<Xapian::docid,Xapian::termcount>(cur_did,doc_len));

					//the increment of doc id in a continuous block is always 1
					cur_did++;
				}
				
				//just a compensation for the loop above
				cur_did--;
			}

		}

		LOGVALUE(DB, new_doclen.size());

		/* merge old map with changes, get new map of <docid,length>
		 * we can do this job in a more easy way, 
		 * for example, new_doclen[chg_it->first]=chg_it->second, 
		 * but it will cost more time.
		 * the following code takes the advantages of a fact
		 * that all elements in a map is in order. 
		 * so we can merge these changes in a more efficient way. */
		map<Xapian::docid,Xapian::termcount>::const_iterator chg_it = changes_start;
		map<Xapian::docid,Xapian::termcount>::iterator ori_it = new_doclen.begin();

		while (chg_it != changes_end)
		{
			while (chg_it->first > ori_it->first)
			{
				++ori_it;
				if (ori_it == new_doclen.end()){
					break;
				}
			}
			if (ori_it == new_doclen.end()) {
				new_doclen.insert(ori_it, *chg_it);
				++chg_it;
				while (chg_it != changes_end)
				{
					new_doclen.insert(ori_it, *chg_it);
					++chg_it;
				}
				break;
			}
			if (ori_it->first == chg_it->first)	{
				if (chg_it->second != SEPERATOR) {
					ori_it->second = chg_it->second;
				} else {
					new_doclen.erase(ori_it++);
				}
			} else {
				new_doclen.insert(ori_it, *chg_it);
			}
			++chg_it;
		}

	}
	LOGVALUE(DB, new_doclen.size());
	RETURN(true);
}

bool 
DoclenChunkWriter::merge_doclen_changes( )
{
	//get new map of <docid,length>
	get_new_doclen();

	//build new chunk from new doclen map.
	map<Xapian::docid,Xapian::termcount>::const_iterator start_pos, end_pos;
	start_pos = end_pos = new_doclen.begin();
	if (new_doclen.size() == 0) {
		return true;
	}


	//If the number of entries in new doclen map is less than a certain value,
	//one chunk is enough.
	//Otherwise we need to split it into many chunks.
	if (new_doclen.size() <= MAX_ENTRIES_IN_CHUNK) {
		//only one chunk
		
		string cur_chunk;
		FixedWidthChunk fwc(new_doclen.begin(), new_doclen.end());

		//make the standard header of the chunk
		end_pos = new_doclen.end();
		end_pos--;
		string head_of_chunk = make_start_of_chunk(is_last_chunk,start_pos->first,end_pos->first);
		cur_chunk = head_of_chunk+cur_chunk;

		//encode new map<doc id, doc length>
		fwc.encode(cur_chunk);

		if (is_first_chunk) {
			//make the header for first chunk
			string head_of_first_chunk = 
				make_start_of_first_chunk(0, 0, start_pos->first);
			cur_chunk = head_of_first_chunk+cur_chunk;
		}

		//make key for this chunk
		string cur_key;
		if (!is_first_chunk) {
			cur_key = BrassPostListTable::make_key(string(), start_pos->first);
		} else {
			cur_key = BrassPostListTable::make_key(string());
		}

		//insert this chunk
		postlist_table->add(cur_key,cur_chunk);
	} else {
		//we need more than one chunk

		//track the number of entries in current chunk
		int count = 0;

		bool is_done = false;
		while (!is_done)
		{
			end_pos++;
			if (end_pos==new_doclen.end()) {
				is_done = true;
			}
			count++;
			if (is_done || count == MAX_ENTRIES_IN_CHUNK) {
				//current chunk is full, or no more entry. 
				string cur_chunk, cur_key;

				//make standard header for this chunk
				map<Xapian::docid,Xapian::termcount>::const_iterator it = end_pos;
				it--;
				if (end_pos==new_doclen.end() && is_last_chunk) {
					//this chunk is last chunk
					cur_chunk = make_start_of_chunk(true, start_pos->first, it->first);
				} else {
					cur_chunk = make_start_of_chunk(false,start_pos->first, it->first);
				}

				FixedWidthChunk fwc(start_pos, end_pos);
				fwc.encode(cur_chunk);

				if (start_pos==new_doclen.begin() && is_first_chunk) {
					//this chunk is first chunk
					//make header for first chunk
					string head_of_first_chunk = make_start_of_first_chunk(0, 0, start_pos->first);
					cur_chunk = head_of_first_chunk+cur_chunk;
					cur_key = postlist_table->make_key(string());
				} else {
					cur_key = postlist_table->make_key(string(), start_pos->first);
				}

				//insert current chunk
				postlist_table->add(cur_key,cur_chunk);

				count = 0;
				start_pos = end_pos;
			}
		}
	}			
	return true;
}


/* This class is just a wrapper of FixedWidthChunkReader,
 * This class just deals with the header of the chunk. */
DoclenChunkReader::DoclenChunkReader(const string& chunk_, bool is_first_chunk, 
									 Xapian::docid first_did_in_chunk)
	: chunk(chunk_), p_fwcr(0)
{
	LOGCALL_CTOR(DB, "DoclenChunkReader", chunk_.size() | is_first_chunk | first_did_in_chunk);
	const char* pos = chunk.data();
	const char* end = pos+chunk.size();
	if (is_first_chunk)	{
		read_start_of_first_chunk(&pos, end, NULL, NULL);
	}
	bool is_last_chunk;
	read_start_of_chunk(&pos, end, 0, &is_last_chunk);
	LOGVALUE(DB,is_last_chunk);
	p_fwcr.reset(new FixedWidthChunkReader(pos,end,first_did_in_chunk));
}

inline bool
read_increase( const char** p, const char* end, Xapian::docid* incre_did, Xapian::termcount* wdf )
{
	unsigned tmp;
	if ( incre_did == NULL )
	{
		incre_did = &tmp;
	}
	if ( wdf == NULL )
	{
		wdf = &tmp;
	}
	unpack_uint( p, end, incre_did );
	unpack_uint( p, end, wdf );
	return true;
}

int SkipList::cal_level( unsigned size )
{
	return (int)(log10(size)/0.6);
}

void SkipList::genDiffVector()
{
	map<Xapian::docid,Xapian::termcount>::const_iterator it = start, pre_it = start;
	for ( ; it!=end ; ++it )
	{
		src.push_back(it->first-pre_it->first);
		src.push_back(it->second);
		pre_it = it;
	}
}

int SkipList::encodeLength( unsigned n )
{
	int len = 0;
	while ( n >= 128 )
	{
		len++;
		n >>= 7;
	}
	len++;
	return len;
}

void SkipList::addLevel ( int ps, int &pe, int& pinfo1_, int& pinfo2_, int curLevel )
{
	int size = pe-ps;
	int pinfo1 = ps;
	int pinfo2 = ps+3+size/2;
    
	if ( pinfo2%2 == pinfo1%2 )
	{
		pinfo2--;
	}
    
	int value1 = 0, value2 = 0;
	for ( int i = ps ; i<=pinfo2-5 ; i+=2 )
	{
		value1 += src[i];
	}
	for ( int i = pinfo2-3 ; i<= pe-2 ; i+=2 )
	{
		value2 += src[i];
	}
    
	src.insert(src.begin()+pinfo1,0);
	src.insert(src.begin()+pinfo1,0);
	src.insert(src.begin()+pinfo1,SEPERATOR);
    
	src.insert(src.begin()+pinfo2,0);
	src.insert(src.begin()+pinfo2,0);
	src.insert(src.begin()+pinfo2,SEPERATOR);
    
	pe += 6;
	src[pinfo2+2]=value2;
	int offset2 = 0;
	for ( int i = pinfo2+3 ; i<pe-2 ; ++i )
	{
		offset2 += encodeLength(src[i]);
	}
	src[pinfo2+1] = offset2;
    
	src[pinfo1+2]=value1;
	int offset1 = 0;
	for ( int i = pinfo1+3 ; i<pinfo2-2 ; ++i )
	{
		offset1 += encodeLength(src[i]);
	}
	src[pinfo1+1]=offset1;
    
	int longer = 0;
	for ( int i = 0 ; i<3 ; ++i )
	{
		longer += encodeLength(src[pinfo1+i]);
		longer += encodeLength(src[pinfo2+i]);
	}
    
	int updateCount = 0;
	unsigned offset = 0;
	for( int i = ps-3 ; i>=0 ; i-- )
	{
		offset += encodeLength(src[i]);
		if ( src[i]==SEPERATOR && src[i+1]>=offset )
		{
			if ( updateCount >= curLevel )
			{
				break;
			}
			int tmp = src[i+1];
			src[i+1] += longer;
			longer += encodeLength(src[i+1])-encodeLength(tmp);
			updateCount++;
		}
	}
    
	pinfo1_ = pinfo1;
	pinfo2_ = pinfo2;
    
    
}

void SkipList::buildSkipList( int level )
{
	if ( level == 0 )
	{
		return;
	}
	int curLevel = 0;
	queue<int> positions;
	positions.push(0);
	positions.push((int)src.size());
	while ( curLevel < level )
	{
		int n = (int)pow(2,curLevel);
		for ( int i = 0 ; i<n ; ++i )
		{
			int ps=0, pe=0, pinfo1=0, pinfo2=0;
			ps = positions.front();
			positions.pop();
			pe = positions.front();
			positions.pop();
			addLevel(ps,pe,pinfo1,pinfo2,curLevel);
			positions.push(pinfo2+3);
			positions.push(pe);
			positions.push(pinfo1+3);
			positions.push(pinfo2);
            
		}
		for ( int i = (int)positions.size()-1 ; i>=0 ; --i )
		{
			int tmp = positions.front();
			tmp += i/4*6;
			positions.pop();
			positions.push(tmp);
		}
		curLevel++;
	}
    
}


SkipList::SkipList(map<Xapian::docid,Xapian::termcount>:: const_iterator start_,
                   map<Xapian::docid,Xapian::termcount>:: const_iterator end_)
: start(start_), end(end_)
{
	genDiffVector();
    int s = 0;
    map<Xapian::docid,Xapian::termcount>:: const_iterator it1(start_), it2(end_);
    for (; it1!=it2; ++it1) {
        ++s;
    };
    buildSkipList(cal_level(s));
}

void SkipList::encode( string& chunk ) const
{
	for ( int i = 0 ; i<(int)src.size() ; ++i )
	{
		pack_uint( chunk,src[i] );
	}
    
}

SkipListReader::SkipListReader(const char* pos_, const char* end_, Xapian::docid first_did_)
: ori_pos(pos_), pos(pos_), end(end_), first_did(first_did_) {
    if (pos != end){
        at_end = false;
        did = first_did;
        next();
    } else {
        at_end = true;
        did = -1;
        wdf = -1;
    }
}

bool SkipListReader::jump_to(Xapian::docid desired_did) {
    if (did == desired_did) {
        return true;
    }
    if (did > desired_did) {
        pos = ori_pos;
        did = first_did;
        next();
    }
    while (pos != end) {
        if (did > desired_did) {
            return false;
        }
        if (did == desired_did) {
            return true;
        }
        Xapian::termcount incre_did = 0;
        unpack_uint(&pos, end, &incre_did);
        if (incre_did == SEPERATOR) {
            unsigned p_offset = 0;
            unsigned d_offset = 0;
            read_increase(&pos, end, &p_offset, &d_offset);
            if (desired_did >= did+d_offset) {
                pos += p_offset;
                did += d_offset;
                read_increase(&pos, end, NULL, &wdf);
            } else {
                
            }
        } else {
            did += incre_did;
            unpack_uint(&pos, end, &wdf);
        }
    }
    return true;
    
    
    
}

bool SkipListReader::next() {
    if (pos == end) {
        at_end = true;
        return false;
    }
    if (at_end) {
        return false;
    }
    Xapian::termcount incre_did = 0;
    unpack_uint(&pos, end, &incre_did);
    while (incre_did == SEPERATOR) {
        read_increase(&pos, end, NULL, NULL);
        if (pos == end) {
            at_end = true;
            return true;
        }
        unpack_uint(&pos, end, &incre_did);
    }
    did += incre_did;
    unpack_uint(&pos, end, &wdf);
    return true;
}

SkipListWriter::SkipListWriter(string& chunk_from_, bool is_first_chunk_, Xapian::docid first_did_,
                               BrassPostListTable* postlist_table_,
                               map<Xapian::docid,Xapian::termcount>::const_iterator& changes_start_,
                               map<Xapian::docid,Xapian::termcount>::const_iterator& changes_end_)
: chunk_from(chunk_from_), first_did(first_did_), is_first_chunk(is_first_chunk_), postlist_table(postlist_table_),
    changes_start(changes_start_), changes_end(changes_end_)
{
    
}

bool
SkipListWriter::get_new_postlist()
{
    LOGCALL(DB, bool, "SkipListWriter::get_new_doclen", NO_ARGS);
	const char* pos = chunk_from.data();
    const char* end = pos+chunk_from.size();
    
	//deal with the header of the chunk
	if (is_first_chunk) {
		read_start_of_first_chunk(&pos, end, &num_of_entries, &coll_fre);
	}
	read_start_of_chunk(&pos, end, first_did, &is_last_chunk);
    
	if (pos == end) {
		//original chunk is empty, we just do some deleting according to changes
		LOGLINE( DB, "empty chunk!" );
		map<Xapian::docid,Xapian::termcount>::const_iterator it = changes_start;
		for (; it!=changes_end ; ++it) {
			//the doc length is -1, which means we should delete this doc id
			if (it->second != SEPERATOR) {
				new_postlist.insert(new_postlist.end(), *it);
			}
		}
		if (new_postlist.empty()) {
			LOGLINE(DB, "new_postlist is empty! ");
			RETURN(false);
		}
	} else {
		//read old map of <docid,length> from original chunk
        Xapian::docid cur_did = 0, incre_did = 0;
        Xapian::termcount wdf = 0;
        cur_did = first_did;
        while ( pos != end )
        {
            unpack_uint( &pos, end, &incre_did );
            while ( incre_did == SEPERATOR )
            {
                read_increase( &pos, end, NULL, NULL );
                unpack_uint( &pos, end, &incre_did );
            }
            unpack_uint( &pos, end, &wdf );
            cur_did += incre_did;
            new_postlist.insert(new_postlist.end(), make_pair<Xapian::docid,Xapian::termcount>(cur_did, wdf));
        }

        
		LOGVALUE(DB, new_postlist.size());
        
		/* merge old map with changes, get new map of <docid,length>
		 * we can do this job in a more easy way,
		 * for example, new_doclen[chg_it->first]=chg_it->second,
		 * but it will cost more time.
		 * the following code takes the advantages of a fact
		 * that all elements in a map is in order.
		 * so we can merge these changes in a more efficient way. */
		map<Xapian::docid,Xapian::termcount>::const_iterator chg_it = changes_start;
		map<Xapian::docid,Xapian::termcount>::iterator ori_it = new_postlist.begin();
        
		while (chg_it != changes_end)
		{
			while (chg_it->first > ori_it->first)
			{
				++ori_it;
				if (ori_it == new_postlist.end()){
					break;
				}
			}
			if (ori_it == new_postlist.end()) {
				new_postlist.insert(ori_it, *chg_it);
				++chg_it;
				while (chg_it != changes_end)
				{
					new_postlist.insert(ori_it, *chg_it);
					++chg_it;
				}
				break;
			}
			if (ori_it->first == chg_it->first)	{
				if (chg_it->second != SEPERATOR) {
					ori_it->second = chg_it->second;
				} else {
					new_postlist.erase(ori_it++);
				}
			} else {
				new_postlist.insert(ori_it, *chg_it);
			}
			++chg_it;
		}
        
	}
	LOGVALUE(DB, new_postlist.size());
	RETURN(true);
}

bool SkipListWriter::merge_postlist_changes(const string& term)
{
	//get new map of <docid,length>
	get_new_postlist();
    
	//build new chunk from new postlist map.
	map<Xapian::docid,Xapian::termcount>::const_iterator start_pos, end_pos;
	start_pos = end_pos = new_postlist.begin();
	if (new_postlist.size() == 0) {
		return true;
	}
    
    
	//If the number of entries in new postlist map is less than a certain value,
	//one chunk is enough.
	//Otherwise we need to split it into many chunks.
	if (new_postlist.size() <= MAX_ENTRIES_IN_CHUNK) {
		//only one chunk
		
		string cur_chunk;
		SkipList sl(new_postlist.begin(), new_postlist.end());
        
		//make the standard header of the chunk
		end_pos = new_postlist.end();
		end_pos--;
		string head_of_chunk = make_start_of_chunk(is_last_chunk,start_pos->first,end_pos->first);
		cur_chunk = head_of_chunk+cur_chunk;
        
		//encode new map<doc id, doc length>
		sl.encode(cur_chunk);
        
		if (is_first_chunk) {
			//make the header for first chunk
			string head_of_first_chunk =
            make_start_of_first_chunk(num_of_entries, coll_fre, start_pos->first);
			cur_chunk = head_of_first_chunk+cur_chunk;
		}
        
		//make key for this chunk
		string cur_key;
		if (!is_first_chunk) {
			cur_key = BrassPostListTable::make_key(term, start_pos->first);
		} else {
			cur_key = BrassPostListTable::make_key(term);
		}
        
		//insert this chunk
		postlist_table->add(cur_key,cur_chunk);
	} else {
		//we need more than one chunk
        
		//track the number of entries in current chunk
		int count = 0;
        
		bool is_done = false;
		while (!is_done)
		{
			end_pos++;
			if (end_pos==new_postlist.end()) {
				is_done = true;
			}
			count++;
			if (is_done || count == MAX_ENTRIES_IN_CHUNK) {
				//current chunk is full, or no more entry.
				string cur_chunk, cur_key;
                
				//make standard header for this chunk
				map<Xapian::docid,Xapian::termcount>::const_iterator it = end_pos;
				it--;
				if (end_pos==new_postlist.end() && is_last_chunk) {
					//this chunk is last chunk
					cur_chunk = make_start_of_chunk(true, start_pos->first, it->first);
				} else {
					cur_chunk = make_start_of_chunk(false,start_pos->first, it->first);
				}
                
				SkipList sl(start_pos, end_pos);
				sl.encode(cur_chunk);
                
				if (start_pos==new_postlist.begin() && is_first_chunk) {
					//this chunk is first chunk
					//make header for first chunk
					string head_of_first_chunk = make_start_of_first_chunk(num_of_entries, coll_fre, start_pos->first);
					cur_chunk = head_of_first_chunk+cur_chunk;
					cur_key = postlist_table->make_key(term);
				} else {
					cur_key = postlist_table->make_key(term, start_pos->first);
				}
                
				//insert current chunk
				postlist_table->add(cur_key,cur_chunk);
                
				count = 0;
				start_pos = end_pos;
			}
		}
	}			
	return true;

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
BrassPostList::BrassPostList(intrusive_ptr<const BrassDatabase> this_db_,
			     const string & term_,
			     bool keep_reference)
	: LeafPostList(term_),
	  this_db(keep_reference ? this_db_ : NULL),
	  have_started(false),
	  is_at_end(false),
	  cursor(this_db_->postlist_table.cursor_get()),
      p_skip_list_reader(0),
	  p_doclen_chunk_reader(0),
	  is_doclen_list(term_.empty()?true:false)
{
	LOGCALL_CTOR(DB, "BrassPostList", this_db_.get() | term_ | keep_reference);
	LOGVALUE( DB, is_doclen_list );
    init();
}

BrassPostList::BrassPostList(intrusive_ptr<const BrassDatabase> this_db_,
			     const string & term_,
			     BrassCursor * cursor_)
	: LeafPostList(term_),
	  this_db(this_db_),
	  have_started(false),
	  is_at_end(false),
	  cursor(cursor_),
      p_skip_list_reader(0),
	  p_doclen_chunk_reader(0),
	  is_doclen_list(term_.empty()?true:false)
{
	LOGCALL_CTOR(DB, "BrassPostList", this_db_.get() | term_ | cursor_);
	LOGVALUE( DB, is_doclen_list );
    init();
}

void
BrassPostList::init()
{
	string key = BrassPostListTable::make_key(term);
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

	is_first_chunk = true;

	did = read_start_of_first_chunk(&pos, end, &number_of_entries, NULL);
	first_did_in_chunk = did;
	last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk);
	if (!is_doclen_list) {
		p_skip_list_reader.reset(new SkipListReader(pos, end, first_did_in_chunk));
        did = p_skip_list_reader->get_docid();
        wdf = p_skip_list_reader->get_wdf();
        is_at_end = p_skip_list_reader->is_at_end();
		p_doclen_chunk_reader.reset(0);
	}
	if (is_doclen_list)	{
		LOGLINE(DB, "This brass_postlist is for doclen info.");
		p_doclen_chunk_reader.reset(new DoclenChunkReader(cursor->current_tag,true,first_did_in_chunk));
		did = p_doclen_chunk_reader->get_docid();
		wdf = p_doclen_chunk_reader->get_doclen();
		is_at_end = p_doclen_chunk_reader->at_end();
		LOGVALUE(DB, did);
		LOGVALUE(DB, wdf);
		LOGVALUE(DB, is_at_end);
        p_skip_list_reader.reset(0);
	}

	LOGLINE(DB, "Initial docid " << did);
}

BrassPostList::~BrassPostList()
{
    LOGCALL_DTOR(DB, "BrassPostList");
	LOGVALUE(DB, is_doclen_list);
}

LeafPostList *
BrassPostList::open_nearby_postlist(const std::string & term_) const
{
    LOGCALL(DB, LeafPostList *, "BrassPostList::open_nearby_postlist", term_);
    if (term_.empty())
	return NULL;
    if (!this_db.get() || this_db->postlist_table.is_writable())
	return NULL;
    return new BrassPostList(this_db, term_, cursor->clone());
}

Xapian::termcount
BrassPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "BrassPostList::get_doclength", NO_ARGS);
    Assert(have_started);
    Assert(this_db.get());
    RETURN(this_db->get_doclength(did));
}

bool
BrassPostList::next_in_chunk()
{
    LOGCALL(DB, bool, "BrassPostList::next_in_chunk", NO_ARGS);
	if (is_doclen_list) {
		LOGLINE(DB, "next_in_chunk() for doclen list");
		if (p_doclen_chunk_reader->next()) {
			did = p_doclen_chunk_reader->get_docid();
			wdf = p_doclen_chunk_reader->get_doclen();
			is_at_end = p_doclen_chunk_reader->at_end();
			LOGVALUE(DB, did);
			LOGVALUE(DB, wdf);
			LOGVALUE(DB, is_at_end);
			RETURN(true);
		}
		is_at_end = p_doclen_chunk_reader->at_end();
		RETURN(false);
	}
    
    if (p_skip_list_reader->next()) {
        did = p_skip_list_reader->get_docid();
        wdf = p_skip_list_reader->get_wdf();
        is_at_end = p_skip_list_reader->is_at_end();
        RETURN(true);
    }
    
    is_at_end = p_skip_list_reader->is_at_end();
    RETURN(false);
    
    /*if (pos == end) RETURN(false);*/

    /*read_did_increase(&pos, end, &did);
    read_wdf(&pos, end, &wdf);*/

    // Either not at last doc in chunk, or pos == end, but not both.
    /*Assert(did <= last_did_in_chunk);
    Assert(did < last_did_in_chunk || pos == end);
    Assert(pos != end || did == last_did_in_chunk);

    RETURN(true);*/
}

void
BrassPostList::next_chunk()
{
	LOGCALL_VOID(DB, "BrassPostList::next_chunk", NO_ARGS);
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

	is_first_chunk = false;

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
	if (!is_doclen_list) {
		p_skip_list_reader.reset(new SkipListReader(pos,end,first_did_in_chunk));
        did = p_skip_list_reader->get_docid();
        wdf = p_skip_list_reader->get_wdf();
        is_at_end = p_skip_list_reader->is_at_end();
	}
	if (is_doclen_list) {
		LOGLINE(DB, "next_chunk() for doclen list");
		LOGLINE(DB, "build new doclen_chunk_reader"); 
		p_doclen_chunk_reader.reset(new DoclenChunkReader(cursor->current_tag,is_first_chunk,first_did_in_chunk));
		did = p_doclen_chunk_reader->get_docid();
		wdf = p_doclen_chunk_reader->get_doclen();
		is_at_end = p_doclen_chunk_reader->at_end();
		LOGVALUE(DB, did);
		LOGVALUE(DB, wdf);
		LOGVALUE(DB, is_at_end);
	}

}

PositionList *
BrassPostList::read_position_list()
{
    LOGCALL(DB, PositionList *, "BrassPostList::read_position_list", NO_ARGS);
    Assert(this_db.get());
    positionlist.read_data(&this_db->position_table, did, term);
    RETURN(&positionlist);
}

PositionList *
BrassPostList::open_position_list() const
{
    LOGCALL(DB, PositionList *, "BrassPostList::open_position_list", NO_ARGS);
    Assert(this_db.get());
    RETURN(new BrassPositionList(&this_db->position_table, did, term));
}

PostList *
BrassPostList::next(double w_min)
{
    LOGCALL(DB, PostList *, "BrassPostList::next", w_min);
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
BrassPostList::current_chunk_contains(Xapian::docid desired_did)
{
    LOGCALL(DB, bool, "BrassPostList::current_chunk_contains", desired_did);
    if (desired_did >= first_did_in_chunk &&
	desired_did <= last_did_in_chunk) {
	RETURN(true);
    }
    RETURN(false);
}

void
BrassPostList::move_to_chunk_containing(Xapian::docid desired_did)
{
    LOGCALL_VOID(DB, "BrassPostList::move_to_chunk_containing", desired_did);
    (void)cursor->find_entry(BrassPostListTable::make_key(term, desired_did));
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

	is_first_chunk = keypos==keyend;

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
    last_did_in_chunk = read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk);
	if (!is_doclen_list) {
		//read_wdf(&pos, end, &wdf);
        p_skip_list_reader.reset(new SkipListReader(pos,end,first_did_in_chunk));
        did = p_skip_list_reader->get_docid();
        wdf = p_skip_list_reader->get_wdf();
        is_at_end = p_skip_list_reader->is_at_end();
	}
	if (is_doclen_list) {
		LOGLINE(DB, "build new doclen_chunk_reader " ); 
		p_doclen_chunk_reader.reset(new DoclenChunkReader(cursor->current_tag,is_first_chunk,first_did_in_chunk));
		did = p_doclen_chunk_reader->get_docid();
		wdf = p_doclen_chunk_reader->get_doclen();
		is_at_end = p_doclen_chunk_reader->at_end();
		LOGVALUE(DB, did);
		LOGVALUE(DB, wdf);
		LOGVALUE(DB, is_at_end);
	}
    

    // Possible, since desired_did might be after end of this chunk and before
    // the next.
    if (desired_did > last_did_in_chunk) next_chunk();
}

PostList *
BrassPostList::skip_to(Xapian::docid desired_did, double w_min)
{
    LOGCALL(DB, PostList *, "BrassPostList::skip_to", desired_did | w_min);
    (void)w_min; // no warning
	if (is_doclen_list)	{
		// If current chunk is doc length chunk, 
		// you should call jump_to rather than skip_to. 
		LOGLINE( DB, "skip_to isn't designed for doclen list!" );
		jump_to(desired_did);
		RETURN(NULL);
	}
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
    
    p_skip_list_reader->jump_to(desired_did);
    did = p_skip_list_reader->get_docid();
    wdf = p_skip_list_reader->get_wdf();
    is_at_end = p_skip_list_reader->is_at_end();
    

    if (is_at_end) {
	LOGLINE(DB, "Skipped to end");
    } else {
	LOGLINE(DB, "Skipped to docid " << did << ", wdf = " << wdf);
    }

    RETURN(NULL);
}

// Used for doclens.
bool
BrassPostList::jump_to(Xapian::docid desired_did)
{
	LOGCALL(DB, bool, "BrassPostList::jump_to", desired_did);
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

		LOGLINE( DB, "get exact chunk for jump_to " );
		move_to_chunk_containing(desired_did);
		// Might be at_end now, so we need to check before trying to move
		// forward in chunk.
		if (is_at_end) RETURN(false);
	}

	// Move to correct position in chunk.
	wdf = p_doclen_chunk_reader->get_doclen(desired_did);
	is_at_end = p_doclen_chunk_reader->at_end();
	if (wdf == (Xapian::termcount)-1) {
		LOGLINE(DB, "No doc with desired_id!");
		wdf = p_doclen_chunk_reader->get_doclen();
		did = p_doclen_chunk_reader->get_docid();
		LOGVALUE(DB,wdf);
		LOGVALUE(DB,did);
		RETURN(false);
	}
	did = p_doclen_chunk_reader->get_docid();
	LOGVALUE(DB,did);
	LOGVALUE(DB,wdf);
	LOGVALUE(DB,is_at_end);
	RETURN(true);
}

string
BrassPostList::get_description() const
{
    string desc;
    description_append(desc, term);
    desc += ":";
    desc += str(number_of_entries);
    return desc;
}


void
BrassPostListTable::merge_doclen_changes(const map<Xapian::docid, Xapian::termcount> & doclens)
{
	LOGCALL_VOID(DB, "BrassPostListTable::merge_doclen_changes", doclens);

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

	map<Xapian::docid, Xapian::termcount>::const_iterator it, pre_it;
	pre_it = it = doclens.begin();
	Assert(it != doclens.end());

	LOGVALUE(DB, doclens.size());
	while (it!=doclens.end())
	{
		string key = make_key(string(), it->first);

		AutoPtr<BrassCursor> cursor(cursor_get());

		(void)cursor->find_entry(key);
		Assert(!cursor->after_end());

		const char * keypos = cursor->current_key.data();
		const char * keyend = keypos + cursor->current_key.size();

		check_tname_in_key(&keypos, keyend, string());

		bool is_first_chunk = (keypos == keyend);
		LOGVALUE(DB, is_first_chunk);

		cursor->read_tag();

		//Store current key and chunk, as they will be deleted later.
		string ori_key(cursor->current_key);
		string desired_chunk(cursor->current_tag);

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
		read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk);
		LOGVALUE(DB,is_last_chunk);
		LOGVALUE(DB,first_did_in_chunk);


		Xapian::docid first_did_in_next_chunk = 0;
		if (is_last_chunk) {
			it = doclens.end();
		} else {
			//If this chunk isn't last chunk, get first doc id in next chunk.
			cursor->next();
			Assert(!cursor->after_end());
			const char * kpos = cursor->current_key.data();
			const char * kend = kpos + cursor->current_key.size();
			Assert(check_tname_in_key(&kpos, kend, string()));
			unpack_uint_preserving_sort(&kpos, kend, &first_did_in_next_chunk);
			Assert(first_did_in_next_chunk);
		}
		LOGVALUE(DB,first_did_in_next_chunk);

		// All changes among first doc id in this chunk and first doc id in next chunk 
		// should be applied to this chunk.
		while (it!=doclens.end() && it->first < first_did_in_next_chunk) 	{
			++it;
		}

		// Delete current chunk to insert new chunk later.
		del(ori_key);
		DoclenChunkWriter writer(desired_chunk, pre_it, it, this, is_first_chunk,first_did_in_chunk);
		writer.merge_doclen_changes();
		pre_it = it;
	}
}


void
BrassPostListTable::merge_changes(const string &term,
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
	    MutableBrassCursor cursor(this);
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
    
    
    map<Xapian::docid, Xapian::termcount>::const_iterator it, pre_it;
	pre_it = it = changes.pl_changes.begin();
	Assert(it != changes.pl_changes.end());
    
	LOGVALUE(DB, changes.pl_changes.size());
	while (it!=changes.pl_changes.end())
	{
		string key = make_key(term, it->first);
        
		AutoPtr<BrassCursor> cursor(cursor_get());
        
		(void)cursor->find_entry(key);
		Assert(!cursor->after_end());
        
		const char * keypos = cursor->current_key.data();
		const char * keyend = keypos + cursor->current_key.size();
        
		check_tname_in_key(&keypos, keyend, string());
        
		bool is_first_chunk = (keypos == keyend);
		LOGVALUE(DB, is_first_chunk);
        
		cursor->read_tag();
        
		//Store current key and chunk, as they will be deleted later.
		string ori_key(cursor->current_key);
		string desired_chunk(cursor->current_tag);
        
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
		read_start_of_chunk(&pos, end, first_did_in_chunk, &is_last_chunk);
		LOGVALUE(DB,is_last_chunk);
		LOGVALUE(DB,first_did_in_chunk);
        
        
		Xapian::docid first_did_in_next_chunk = 0;
		if (is_last_chunk) {
			it = changes.pl_changes.end();
		} else {
			//If this chunk isn't last chunk, get first doc id in next chunk.
			cursor->next();
			Assert(!cursor->after_end());
			const char * kpos = cursor->current_key.data();
			const char * kend = kpos + cursor->current_key.size();
			Assert(check_tname_in_key(&kpos, kend, string()));
			unpack_uint_preserving_sort(&kpos, kend, &first_did_in_next_chunk);
			Assert(first_did_in_next_chunk);
		}
		LOGVALUE(DB,first_did_in_next_chunk);
        
		// All changes among first doc id in this chunk and first doc id in next chunk
		// should be applied to this chunk.
		while (it!=changes.pl_changes.end() && it->first < first_did_in_next_chunk) 	{
			++it;
		}
        
		// Delete current chunk to insert new chunk later.
		del(ori_key);
		SkipListWriter writer(desired_chunk, is_first_chunk, first_did_in_chunk, this, pre_it, it);
		writer.merge_postlist_changes(term);
		pre_it = it;
	}
}
