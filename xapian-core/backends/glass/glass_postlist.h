/** @file glass_postlist.h
 * @brief Postlists in glass databases
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2007,2008,2009,2011,2013,2014,2015 Olly Betts
 * Copyright 2007,2009 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_GLASS_POSTLIST_H
#define OM_HGUARD_GLASS_POSTLIST_H

#include <xapian/database.h>

#include "glass_defs.h"
#include "glass_inverter.h"
#include "glass_positionlist.h"
#include "api/leafpostlist.h"
#include "omassert.h"

#include "autoptr.h"
#include <map>
#include <string>

using namespace std;

class GlassCursor;
class GlassDatabase;

namespace Glass {
    class PostlistChunkReader;
    class PostlistChunkWriter;
    class RootInfo;
}

using Glass::RootInfo;

class GlassPostList;

class GlassPostListTable : public GlassTable {
	/// PostList for looking up document lengths.
	mutable AutoPtr<GlassPostList> doclen_pl;

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
	GlassPostListTable(const string & path_, bool readonly_)
	    : GlassTable("postlist", path_ + "/postlist.", readonly_),
	      doclen_pl()
	{ }

	GlassPostListTable(int fd, off_t offset_, bool readonly_)
	    : GlassTable("postlist", fd, offset_, readonly_),
	      doclen_pl()
	{ }

	void open(int flags_, const RootInfo & root_info,
		  glass_revision_number_t rev) {
	    doclen_pl.reset(0);
	    GlassTable::open(flags_, root_info, rev);
	}

	/// Merge changes for a term.
	void merge_changes(const string &term, const Inverter::PostingChanges & changes);

	/// Merge document length changes.
	void merge_doclen_changes(const map<Xapian::docid, Xapian::termcount> & doclens);

	Xapian::docid get_chunk(const string &tname,
		Xapian::docid did, bool adding,
		Glass::PostlistChunkReader ** from,
		Glass::PostlistChunkWriter **to);

	/// Compose a key from a termname and docid.
	static string make_key(const string & term, Xapian::docid did) {
	    return pack_glass_postlist_key(term, did);
	}

	/// Compose a key from a termname.
	static string make_key(const string & term) {
	    return pack_glass_postlist_key(term);
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
					Xapian::Internal::intrusive_ptr<const GlassDatabase> db) const;

	/** Check if document @a did exists. */
	bool document_exists(Xapian::docid did,
			     Xapian::Internal::intrusive_ptr<const GlassDatabase> db) const;

	void get_used_docid_range(Xapian::docid & first,
				  Xapian::docid & last) const;
};

/** A postlist in a glass database.
 */
class GlassPostList : public LeafPostList {
	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us, and also to give us access
	 *  to the position_table.
	 */
	Xapian::Internal::intrusive_ptr<const GlassDatabase> this_db;

	/// The position list object for this posting list.
	GlassPositionList positionlist;

	/// Whether we've started reading the list yet.
	bool have_started;

	/// True if this is the last chunk.
	bool is_last_chunk;

	/// Whether we've run off the end of the list yet.
	bool is_at_end;

	/// Cursor pointing to current chunk of postlist.
	AutoPtr<GlassCursor> cursor;

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

	/// Copying is not allowed.
	GlassPostList(const GlassPostList &);

	/// Assignment is not allowed.
	void operator=(const GlassPostList &);

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

	GlassPostList(Xapian::Internal::intrusive_ptr<const GlassDatabase> this_db_,
		      const string & term,
		      GlassCursor * cursor_);

	void init();

    public:
	/// Default constructor.
	GlassPostList(Xapian::Internal::intrusive_ptr<const GlassDatabase> this_db_,
		      const string & term,
		      bool keep_reference);

	/// Destructor.
	~GlassPostList();

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

	/// Returns the number of unique terms in the current document.
	Xapian::termcount get_unique_terms() const;

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

#endif /* OM_HGUARD_GLASS_POSTLIST_H */
