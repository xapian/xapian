/** @file brass_postlist.h
 * @brief Postlists in brass databases
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2007,2008,2009,2011,2013,2014 Olly Betts
 * Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2014 Shangtong Zhang
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

#ifndef OM_HGUARD_BRASS_POSTLIST_H
#define OM_HGUARD_BRASS_POSTLIST_H

#include <xapian/database.h>

#include "brass_inverter.h"
#include "brass_types.h"
#include "brass_positionlist.h"
#include "api/leafpostlist.h"
#include "omassert.h"

#include "debuglog.h"

#include "autoptr.h"
#include <map>
#include <string>
#include <vector>

//the following four macros is for fixed width doclen chunk.

//an indicator used in fixed width doclen chunk
#define SEPERATOR ((unsigned)-1)

/* If the length of a continuous block is more than this value, 
 * fixed width format will be applied. */
#define DOCLEN_CHUNK_MIN_CONTIGUOUS_LENGTH 5

/* The ratio is good bytes / used bytes in a block of fixed width format. 
 * we require good bytes ratio must be bigger than this value 
 * if we want to apply fixed width format. */
#define DOCLEN_CHUNK_MIN_GOOD_BYTES_RATIO 0.8

/* A fixed width doclen chunk has at most so many entries. */
#define MAX_ENTRIES_IN_CHUNK 2000

using namespace std;

class BrassCursor;
class BrassDatabase;

namespace Brass {
    class PostlistChunkReader;
    class PostlistChunkWriter;
}

/* The Format Of Fixed Width Doclen Chunk
 *
 * In normal format of postlist chunk for doc length,
 * we first encode the increment of did, then the doc length is encoded.
 * But when doc ids are continuous, we have more efficient ways to encode them.
 * In the following, we refer some continuous ids as a continuous block. 
 * When encoding doc lengths, if we discover a continuous block,
 * we first encode -1 as an indicator, then the increment of did, 
 * the number of entries in this block, the bytes to encode a length.
 * for example, if we have some tuples < did, len >,
 * <1,7>,<2,5>,<3,6>,<4,15>,<5,257>, 
 * then the chunk will be 
 * -1 1 5 2 7 5 6 15 257 */


/* This class is used to encode a map<did,len> to a chunk.
 * It doesn't deal with the header of the chunk. */
class FixedWidthChunk {

	///
	map<Xapian::docid,Xapian::termcount>::const_iterator pl_start, pl_end;

	///
	vector<unsigned> src;

	///
	bool buildVector( );

public:

	///
	FixedWidthChunk( map<Xapian::docid,Xapian::termcount>::const_iterator pl_start_, 
		map<Xapian::docid,Xapian::termcount>::const_iterator pl_end_);

	///The encoded map will be appended to @chunk.
	bool encode( string& chunk ) const;
};

/* This class is used to read fixed width format doclen chunk.
 * It doesn't deal with the header of the chunk. */
class FixedWidthChunkReader {

	///
	const char* ori_pos;

	///
	const char* pos;

	///
	const char* pos_of_block;

	///
	const char* end;

	///
	Xapian::docid cur_did;

	///
	Xapian::termcount cur_length;

	///
	bool is_at_end;

	//indicating whether we are in a continuous block.
	bool is_in_block;

	/* If we are in a continuous block,
	 * the following two variables is the basic info for this continuous block. */
	unsigned len_info;
	unsigned bytes_info;

	///
	Xapian::docid did_before_block;
	
	///first doc id in this chunk
	Xapian::docid first_did_in_chunk;


	public:
	/* @pos_ : a pointer to the end of the header of the chunk.
	 * @end_ : a pointer to the end of the chunk. */
	FixedWidthChunkReader( const char* pos_, const char* end_, Xapian::docid first_did_in_chunk_ )
		: ori_pos(pos_), pos(pos_), pos_of_block(NULL), end(end_), cur_did(0), cur_length(0),
		is_at_end(false),is_in_block(false),len_info(0),bytes_info(0),
		did_before_block(0),first_did_in_chunk(first_did_in_chunk_)
	{
		LOGCALL_CTOR(DB, "FixedWidthChunkReader", first_did_in_chunk_ );
		if (pos == end) {
			is_at_end = true;
			LOGLINE(DB, "empty chunk" ); 
			return;
		}
		cur_did = first_did_in_chunk_;
		next();
		LOGVALUE(DB, is_at_end );
		LOGVALUE(DB, cur_did );
		LOGVALUE(DB, cur_length );
	};

	/* jump to desired did, 
	 * if it fails, it will arrive at the exact did just after @desired_did */
	bool jump_to( Xapian::docid desired_did );

	/* move to next did in the chunk. 
	 * If no more did, set is_at_end=true. */
	bool next();

	/// return current docid
	Xapian::docid get_docid() const {
		return cur_did;
	}

	/// return length of doc with current doc id
	Xapian::termcount get_doclength() const {
		return cur_length;
	}

	/// return is_at_end
	bool at_end() const {
		return is_at_end;
	}
};


///This class is used to update fixed width doclen chunk.
class DoclenChunkWriter {

	///the original chunk
	const string& chunk_from;

	///the changes of doc length
	map<Xapian::docid,Xapian::termcount>::const_iterator changes_start, changes_end;

	///
	BrassPostListTable* postlist_table;

	///
	bool is_first_chunk;

	///
	bool is_last_chunk;

	///
	Xapian::docid first_did_in_chunk;

	//new map of doc length
	map<Xapian::docid,Xapian::termcount> new_doclen;

	//merge old map and new map
	bool get_new_doclen( );

	public:

	/* @chunk_from_ : original chunk
	 * @changes_start_  @changes_end_ : iterator of map of changes */
	DoclenChunkWriter( const string& chunk_from_, 
		map<Xapian::docid,Xapian::termcount>::const_iterator& changes_start_,
		map<Xapian::docid,Xapian::termcount>::const_iterator& changes_end_,
		BrassPostListTable* postlist_table_,
		bool is_first_chunk_, Xapian::docid first_did_in_chunk_ )
		: chunk_from(chunk_from_), changes_start(changes_start_), changes_end(changes_end_), 
		postlist_table(postlist_table_), is_first_chunk(is_first_chunk_),
		first_did_in_chunk(first_did_in_chunk_)
	{
		LOGCALL_CTOR(DB, "DoclenChunkWriter", is_first_chunk_ | first_did_in_chunk_ );
		is_last_chunk = true;
	}

	/* it will build and insert new chunk,
	 * make sure old chunk is deleted before call this function.*/
	bool merge_doclen_changes( );
};

/* This class is just a wrapper of FixedWidthChunkReader,
 * This class just deals with the header of the chunk. */
class DoclenChunkReader {

	///
	const string& chunk;

	///
	AutoPtr<FixedWidthChunkReader> p_fwcr;

	public:

	///
	DoclenChunkReader( const string& chunk_, bool is_first_chunk, Xapian::docid first_did_in_chunk );

	///
	~DoclenChunkReader() {
		LOGCALL_DTOR(DB, "DoclenChunkReader");
	}

	///
	Xapian::termcount get_doclen( Xapian::docid desired_did ) {
		if (p_fwcr->jump_to(desired_did)) {
			return p_fwcr->get_doclength();
		}
		return -1;
	}

	///
	Xapian::termcount get_doclen() {
		return p_fwcr->get_doclength();
	}

	///
	Xapian::docid get_docid() {
		return p_fwcr->get_docid();
	}

	///
	bool next() {
		return p_fwcr->next();
	}

	///
	bool at_end() {
		return p_fwcr->at_end();
	}
};

class BrassPostList;

class BrassPostListTable : public BrassTable {
	/// PostList for looking up document lengths.
	mutable AutoPtr<BrassPostList> doclen_pl;

    public:
	/** Create a new table object.
	 *
	 *  This does not create the table on disk - the create() method must
	 *  be called before the table is created on disk
	 *
	 *  This also does not open the table - the open() method must be
	 *  called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 */
	BrassPostListTable(const string & path_, bool readonly_)
	    : BrassTable("postlist", path_ + "/postlist.", readonly_),
	      doclen_pl()
	{ }

	bool open(int flags_, brass_revision_number_t revno) {
	    doclen_pl.reset(0);
	    return BrassTable::open(flags_, revno);
	}

	/// Merge changes for a term.
	void merge_changes(const string &term, const Inverter::PostingChanges & changes);

	/// Merge document length changes.
	void merge_doclen_changes(const map<Xapian::docid, Xapian::termcount> & doclens);

	Xapian::docid get_chunk(const string &tname,
		Xapian::docid did, bool adding,
		Brass::PostlistChunkReader ** from,
		Brass::PostlistChunkWriter **to);

	/// Compose a key from a termname and docid.
	static string make_key(const string & term, Xapian::docid did) {
	    return pack_brass_postlist_key(term, did);
	}

	/// Compose a key from a termname.
	static string make_key(const string & term) {
	    return pack_brass_postlist_key(term);
	}

	bool term_exists(const string & term) const {
	    return key_exists(make_key(term));
	}

	/** Returns frequencies for a term.
	 *
	 *  @param term		The term to get frequencies for
	 *  @param termfreq_ptr	Point to return number of docs indexed by @a
	 *			term (or NULL not to return)
	 *  @param collfreq_ptr	Point to return number of occurrences of @a
	 *			term in the database (or NULL not to return)
	 */
	void get_freqs(const std::string & term,
		       Xapian::doccount * termfreq_ptr,
		       Xapian::termcount * collfreq_ptr) const;

	/** Returns the length of document @a did. */
	Xapian::termcount get_doclength(Xapian::docid did,
					Xapian::Internal::intrusive_ptr<const BrassDatabase> db) const;

	/** Check if document @a did exists. */
	bool document_exists(Xapian::docid did,
			     Xapian::Internal::intrusive_ptr<const BrassDatabase> db) const;
};

/** A postlist in a brass database.
 */
class BrassPostList : public LeafPostList {
	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us, and also to give us access
	 *  to the position_table.
	 */
	Xapian::Internal::intrusive_ptr<const BrassDatabase> this_db;

	/// The position list object for this posting list.
	BrassPositionList positionlist;

	/// Whether we've started reading the list yet.
	bool have_started;

	/// True if this is the last chunk.
	bool is_last_chunk;

	/// Whether we've run off the end of the list yet.
	bool is_at_end;

	/// Cursor pointing to current chunk of postlist.
	AutoPtr<BrassCursor> cursor;

	/// The first document id in this chunk.
	Xapian::docid first_did_in_chunk;

	/// The last document id in this chunk.
	Xapian::docid last_did_in_chunk;

	/// Position of iteration through current chunk.
	const char * pos;

	/// Pointer to byte after end of current chunk.
	const char * end;

	/// Document id we're currently at.
	Xapian::docid did;

	/// The wdf of the current document.
	Xapian::termcount wdf;

	/// The number of entries in the posting list.
	Xapian::doccount number_of_entries;

	/// To read doclen chunk, when is_doclen_list is true.
	AutoPtr<DoclenChunkReader> p_doclen_chunk_reader;

	/// True when this postlist is to store documents length.
	bool is_doclen_list;

	/// True when current chunk is first chunk.
	/// This variable is valid only when this chunk is for doc length.
	bool is_first_chunk;

	/// Copying is not allowed.
	BrassPostList(const BrassPostList &);

	/// Assignment is not allowed.
	void operator=(const BrassPostList &);

	/** Move to the next item in the chunk, if possible.
	 *  If already at the end of the chunk, returns false.
	 */
	bool next_in_chunk();

	/** Move to the next chunk.
	 *
	 *  If there are no more chunks in this postlist, this will set
	 *  is_at_end to true.
	 */
	void next_chunk();

	/** Return true if the given document ID lies in the range covered
	 *  by the current chunk.  This does not say whether the document ID
	 *  is actually present.  It will return false if the document ID
	 *  is greater than the last document ID in the chunk, even if it is
	 *  less than the first document ID in the next chunk: it is possible
	 *  for no chunk to contain a particular document ID.
	 */
	bool current_chunk_contains(Xapian::docid desired_did);

	/** Move to chunk containing the specified document ID.
	 *
	 *  This moves to the chunk whose starting document ID is
	 *  <= desired_did, but such that the next chunk's starting
	 *  document ID is > desired_did.
	 *
	 *  It is thus possible that current_chunk_contains(desired_did)
	 *  will return false after this call, since the document ID
	 *  might lie after the end of this chunk, but before the start
	 *  of the next chunk.
	 */
	void move_to_chunk_containing(Xapian::docid desired_did);

	/** Scan forward in the current chunk for the specified document ID.
	 *
	 *  This is particularly efficient if the desired document ID is
	 *  greater than the last in the chunk - it then skips straight
	 *  to the end.
	 *
	 *  @return true if we moved to a valid document,
	 *	    false if we reached the end of the chunk.
	 */
	bool move_forward_in_chunk_to_at_least(Xapian::docid desired_did);

	BrassPostList(Xapian::Internal::intrusive_ptr<const BrassDatabase> this_db_,
		      const string & term,
		      BrassCursor * cursor_);

	void init();

    public:
	/// Default constructor.
	BrassPostList(Xapian::Internal::intrusive_ptr<const BrassDatabase> this_db_,
		      const string & term,
		      bool keep_reference);

	/// Destructor.
	~BrassPostList();

	LeafPostList * open_nearby_postlist(const std::string & term_) const;

	/** Used for looking up doclens.
	 *
	 *  @return true if docid @a desired_did has a document length.
	 */
	bool jump_to(Xapian::docid desired_did);

	/** Returns number of docs indexed by this term.
	 *
	 *  This is the length of the postlist.
	 */
	Xapian::doccount get_termfreq() const { return number_of_entries; }

	/// Returns the current docid.
	Xapian::docid get_docid() const { Assert(have_started); return did; }

	/// Returns the length of current document.
	Xapian::termcount get_doclength() const;

	/** Returns the Within Document Frequency of the term in the current
	 *  document.
	 */
	Xapian::termcount get_wdf() const { Assert(have_started); return wdf; }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList *read_position_list();

	/** Get the list of positions of the term in the current document.
	 */
	PositionList * open_position_list() const;

	/// Move to the next document.
	PostList * next(double w_min);

	/// Skip to next document with docid >= docid.
	PostList * skip_to(Xapian::docid desired_did, double w_min);

	/// Return true if and only if we're off the end of the list.
	bool at_end() const { return is_at_end; }

	/// Get a description of the document.
	std::string get_description() const;

	/// Read the number of entries and the collection frequency.
	static void read_number_of_entries(const char ** posptr,
					   const char * end,
					   Xapian::doccount * number_of_entries_ptr,
					   Xapian::termcount * collection_freq_ptr);
};

#endif /* OM_HGUARD_BRASS_POSTLIST_H */
