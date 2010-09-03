/** @file flint_alldocspostlist.h
 * @brief A PostList which iterates over all documents in a FlintDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_FLINT_ALLDOCSPOSTLIST_H
#define XAPIAN_INCLUDED_FLINT_ALLDOCSPOSTLIST_H

#include <string>

#include "autoptr.h"
#include "flint_database.h"
#include "leafpostlist.h"

class FlintCursor;

class FlintAllDocsPostList : public LeafPostList {
    /// Don't allow assignment.
    void operator=(const FlintAllDocsPostList &);

    /// Don't allow copying.
    FlintAllDocsPostList(const FlintAllDocsPostList &);

    /// Set @a current_did from @a cursor->current_key.
    PostList * read_did_from_current_key();

    /// The database we're iterating over.
    Xapian::Internal::RefCntPtr<const FlintDatabase> db;

    /// The number of documents in the database.
    Xapian::doccount doccount;

    /// Cursor running over termlist table keys.
    AutoPtr<FlintCursor> cursor;

    /// The current document id.
    Xapian::docid current_did;

  public:
    FlintAllDocsPostList(Xapian::Internal::RefCntPtr<const FlintDatabase> db_,
			 Xapian::doccount doccount_)
      : LeafPostList(std::string()),
	db(db_), doccount(doccount_), cursor(db->termlist_table.cursor_get()),
	current_did(0)
    {
	cursor->find_entry(std::string());
    }

    Xapian::doccount get_termfreq() const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_doclength() const;

    Xapian::termcount get_wdf() const;

    PostList * next(Xapian::weight w_min);

    PostList * skip_to(Xapian::docid desired_did, Xapian::weight w_min);

    bool at_end() const;

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_FLINT_ALLDOCSPOSTLIST_H
