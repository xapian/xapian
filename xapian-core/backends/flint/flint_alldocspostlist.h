/* flint_alldocspostlist.h: All document postlists in flint databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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

#ifndef OM_HGUARD_FLINT_ALLDOCSPOSTLIST_H
#define OM_HGUARD_FLINT_ALLDOCSPOSTLIST_H

#include <map>
#include <string>

#include "leafpostlist.h"
#include "database.h"
#include "omassert.h"
#include "flint_types.h"

using namespace std;

class FlintCursor;
class FlintTable;

/** A postlist in a flint database.
 */
class FlintAllDocsPostList : public LeafPostList {
    private:
	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us.
	 */
	Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db;

	/// The table containing the values.
	const FlintTable * table;

	/// Cursor pointing to current values.
	AutoPtr<FlintCursor> cursor;

	/// Document id we're currently at.
	Xapian::docid did;

	/// Whether we've run off the end of the list yet.
	bool is_at_end;

        /// Number of documents in the database.
        Xapian::doccount doccount;

	/// Copying is not allowed.
	FlintAllDocsPostList(const FlintAllDocsPostList &);

	/// Assignment is not allowed.
	void operator=(const FlintAllDocsPostList &);


    public:
	/// Default constructor.
	FlintAllDocsPostList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
                             const FlintTable * table_,
                             Xapian::doccount doccount_);

	/// Destructor.
	~FlintAllDocsPostList();

	/** Returns length of the all documents postlist.
         *
         *  This is also the number of documents in the database.
	 */
	Xapian::doccount get_termfreq() const { return doccount; }

	/// Returns the current docid.
	Xapian::docid get_docid() const { Assert(did != 0); return did; }

	/// Returns the length of current document.
	Xapian::doclength get_doclength() const {
	    DEBUGCALL(DB, Xapian::doclength, "FlintAllDocsPostList::get_doclength", "");
	    Assert(did != 0);
	    RETURN(this_db->get_doclength(did));
	}

	/** Returns the Within Document Frequency of the term in the current
	 *  document.
	 */
	Xapian::termcount get_wdf() const { Assert(did != 0); return static_cast<Xapian::termcount>(1); }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList *read_position_list() {
            throw Xapian::InvalidOperationError("Can't read position list from all docs postlist.");
        }

	/** Get the list of positions of the term in the current document.
	 */
	PositionList * open_position_list() const {
            throw Xapian::InvalidOperationError("Can't read position list from all docs postlist.");
        }

	/// Move to the next document.
	PostList * next(Xapian::weight w_min);

	/// Skip to next document with docid >= docid.
	PostList * skip_to(Xapian::docid desired_did, Xapian::weight w_min);

	/// Return true if and only if we're off the end of the list.
	bool at_end() const { return is_at_end; }

	/// Get a description of the postlist.
	std::string get_description() const;
};

#endif /* OM_HGUARD_FLINT_ALLDOCSPOSTLIST_H */
