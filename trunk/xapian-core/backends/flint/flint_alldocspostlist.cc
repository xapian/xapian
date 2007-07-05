/* flint_alldocspostlist.cc: All-document postlists in flint databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "omdebug.h"
#include "flint_alldocspostlist.h"
#include "flint_utils.h"
#include "flint_values.h"
#include "flint_cursor.h"
#include "database.h"

/** The format of a postlist is:
 */
FlintAllDocsPostList::FlintAllDocsPostList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
                                           const FlintTable * table_,
                                           Xapian::doccount doccount_)
	: this_db(this_db_),
	  table(table_),
	  cursor(table->cursor_get()),
          did(0),
	  is_at_end(false),
          doccount(doccount_)
{
    DEBUGCALL(DB, void, "FlintAllDocsPostList::FlintAllDocsPostList",
	      this_db_.get() << ", " << table_ << ", " << doccount_);

    // Move to initial NULL entry.
    cursor->find_entry("");
}

FlintAllDocsPostList::~FlintAllDocsPostList()
{
    DEBUGCALL(DB, void, "FlintAllDocsPostList::~FlintAllDocsPostList", "");
}

PostList *
FlintAllDocsPostList::next(Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *, "FlintAllDocsPostList::next", w_min);
    (void)w_min; // no warning

    cursor->next();
    if (cursor->after_end()) {
        is_at_end = true;
    } else {
        string key = cursor->current_key;
        const char * keystr = key.c_str();
        if (!unpack_uint_preserving_sort(&keystr, keystr + key.length(), &did)) {
            if (*keystr == 0)
                throw Xapian::DatabaseCorruptError("Unexpected end of data when reading from termlist table");
            else
                throw Xapian::RangeError("Document number in value table is too large");
        }
    }

    DEBUGLINE(DB, string("Moved to ") <<
              (is_at_end ? string("end.") : string("docid = ") +
               om_tostring(did)));

    RETURN(NULL);
}

PostList *
FlintAllDocsPostList::skip_to(Xapian::docid desired_did, Xapian::weight w_min)
{
    DEBUGCALL(DB, PostList *,
	      "FlintAllDocsPostList::skip_to", desired_did << ", " << w_min);
    (void)w_min; // no warning

    // Don't skip back, and don't need to do anything if already there.
    if (desired_did <= did) RETURN(NULL);
    if (is_at_end) RETURN(NULL);

    string desired_key = pack_uint_preserving_sort(desired_did);
    bool exact_match = cursor->find_entry(desired_key);
    if (!exact_match)
        cursor->next();
    if (cursor->after_end()) {
        is_at_end = true;
    } else {
        string key = cursor->current_key;
        const char * keystr = key.c_str();
        if (!unpack_uint_preserving_sort(&keystr, keystr + key.length(), &did)) {
            if (*keystr == 0)
                throw Xapian::DatabaseCorruptError("Unexpected end of data when reading from termlist table");
            else
                throw Xapian::RangeError("Document number in value table is too large");
        }
    }

    DEBUGLINE(DB, string("Skipped to ") <<
              (is_at_end ? string("end.") : string("docid = ") +
               om_tostring(did)));

    RETURN(NULL);
}

string
FlintAllDocsPostList::get_description() const
{
    return ":" + om_tostring(doccount);
}
