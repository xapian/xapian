/* quartz_postlist.h: Postlists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#ifndef OM_HGUARD_QUARTZ_POSTLIST_H
#define OM_HGUARD_QUARTZ_POSTLIST_H

#include <string>

#include "leafpostlist.h"
#include "om/omtypes.h"
#include "quartz_table_entries.h"
#include "omassert.h"
#include "quartz_types.h"
#include "quartz_positionlist.h"

using namespace std;

class QuartzTable;
class QuartzCursor;
class QuartzBufferedTable;
class QuartzDatabase;
class Database;

/** A postlist in a quartz database.
 */
class QuartzPostList : public LeafPostList {
    private:
	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us.
	 */
	RefCntPtr<const Database> this_db;

	/// The table containing the postlist.
	const QuartzTable * table;

	/// The table containing positionlists.
	const QuartzTable * positiontable;

	/// The termname for this postlist.
	string tname;


	/// Cursor pointing to current chunk of postlist. 
	AutoPtr<QuartzCursor> cursor;

	/// True if this is the last chunk.
	bool is_last_chunk;

	/// The first document id in this chunk;
	om_docid first_did_in_chunk;

	/// The last document id in this chunk;
	om_docid last_did_in_chunk;

	/// Position of iteration through current chunk.
	const char * pos;

	/// Byte after end of current chunk.
	const char * end;


        /// Document id we're currently at.
	om_docid did;

	/// The (absolute) length of the current document, 
	quartz_doclen_t doclength;

	/// The wdf of the current document.
	om_termcount wdf;


	/// Whether we've run off the end of the list yet.
	bool is_at_end;

	/// Whether we've run off the end of the list yet.
	bool have_started;

	/// The number of entries in the posting list.
	om_doccount number_of_entries;

	/// The number of occurences of the term in the posting list.
	om_termcount collection_freq;

	/// The position list object for this posting list.
	QuartzPositionList positionlist;

        /// Copying is not allowed.
        QuartzPostList(const QuartzPostList &);

        /// Assignment is not allowed.
        void operator=(const QuartzPostList &);

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
	bool current_chunk_contains(om_docid desired_did);

	/** Move to chunk containing the specified document ID.
	 *
	 *  This moves to the chunk whose starting document ID is
	 *  <= desired_did, but such that the next chunks starting
	 *  document ID is > desired_did.
	 *
	 *  It is thus possible that current_chunk_contains(desired_did)
	 *  will return false after this call, since the document ID
	 *  might lie after the end of this chunk, but before the start
	 *  of the next chunk.
	 */
	void move_to_chunk_containing(om_docid desired_did);

	/** Scan forward in the current chunk for the specified document ID.
	 *
	 *  This is particularly efficient if the desired document ID is
	 *  greater than the last in the chunk - it then skips straight
	 *  to the end.
	 *
	 *  @return true if we moved to a valid document,
	 *          false if we reached the end of the chunk.
	 */
	bool move_forward_in_chunk_to_at_least(om_docid desired_did);

	/** Move to the desired document ID, or the next document ID if it
	 *  doesn't exist, whether it's before or after the current position.
	 */
	void move_to(om_docid desired_did);

    public:
        /// Default constructor.
        QuartzPostList(RefCntPtr<const Database> this_db_,
		       const QuartzTable * table_,
		       const QuartzTable * positiontable_,
		       const string & tname);

        /// Destructor.
        ~QuartzPostList();

	/** Returns number of docs indexed by this term.
	 *
	 *  This is the length of the postlist.
	 */
	om_doccount   get_termfreq() const { return number_of_entries; }

	/** Returns the number of occurences of the term in the database.
	 *
	 *  This is the sum of the wdfs in the postlist.
	 */
	om_termcount get_collection_freq() const { return collection_freq; }

	/// Returns the current docid.
	om_docid     get_docid() const { Assert(have_started); return did; }

	/// Returns the length of current document.
	om_doclength get_doclength() const {
	    Assert(have_started);
	    return static_cast<om_doclength>(doclength);
	}

	/** Returns the Within Document Frequency of the term in the current
	 *  document.
	 */
	om_termcount get_wdf() const { Assert(have_started); return wdf; }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList *read_position_list();

	/** Get the list of positions of the term in the current document.
	 */
	PositionList * open_position_list() const;

	/// Move to the next document.
	PostList * next(om_weight w_min);

	/// Skip to next document with docid >= docid.
	PostList * skip_to(om_docid desired_did, om_weight w_min);

	/// Return true if and only if we're off the end of the list.
	bool       at_end() const { return is_at_end; }

	/// Get a description of the document.
	std::string get_description() const;

	/// Insert an entry
	static void add_entry(QuartzBufferedTable * bufftable,
			      const string & tname,
			      om_docid new_did,
			      om_termcount new_wdf,
			      quartz_doclen_t new_doclen);

	/// Delete an entry
	static void delete_entry(QuartzBufferedTable * bufftable,
				 const string & tname,
				 om_docid did);

	static void read_number_of_entries(const char ** posptr,
					   const char * end,
					   om_termcount * number_of_entries_ptr,
					   om_termcount * collection_freq_ptr);
};

#endif /* OM_HGUARD_QUARTZ_POSTLIST_H */
