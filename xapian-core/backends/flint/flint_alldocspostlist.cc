/** @file flint_alldocspostlist.cc
 * @brief A PostList which iterates over all documents in a FlintDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
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

#include <config.h>
#include "flint_alldocspostlist.h"

#include <string>

#include "debuglog.h"
#include "flint_database.h"
#include "str.h"

using namespace std;

Xapian::doccount
FlintAllDocsPostList::get_termfreq() const
{
    return doccount;
}

Xapian::docid
FlintAllDocsPostList::get_docid() const
{
    return current_did;
}

Xapian::termcount
FlintAllDocsPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "FlintAllDocsPostList::get_doclength", NO_ARGS);
    Assert(current_did);

    cursor->read_tag();

    if (cursor->current_tag.empty()) RETURN(0);

    const char * pos = cursor->current_tag.data();
    const char * end = pos + cursor->current_tag.size();

    flint_doclen_t doclen;
    if (!F_unpack_uint(&pos, end, &doclen)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for doclen in termlist";
	} else {
	    msg = "Overflowed value for doclen in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    RETURN(doclen);
}

Xapian::termcount
FlintAllDocsPostList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "FlintAllDocsPostList::get_wdf", NO_ARGS);
    Assert(current_did);
    RETURN(1);
}

PostList *
FlintAllDocsPostList::read_did_from_current_key()
{
    LOGCALL(DB, PostList *, "FlintAllDocsPostList::read_did_from_current_key", NO_ARGS);
    const string & key = cursor->current_key;
    const char * pos = key.data();
    const char * end = pos + key.size();
    if (!F_unpack_uint_preserving_sort(&pos, end, &current_did)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data in termlist key";
	} else {
	    msg = "Overflowed value in termlist key";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    // Return NULL to help the compiler tail-call optimise our callers.
    RETURN(NULL);
}

PostList *
FlintAllDocsPostList::next(Xapian::weight /*w_min*/)
{
    LOGCALL(DB, PostList *, "FlintAllDocsPostList::next", Literal("/*w_min*/"));
    Assert(!at_end());
    if (!cursor->next()) RETURN(NULL);
    RETURN(read_did_from_current_key());
}

PostList *
FlintAllDocsPostList::skip_to(Xapian::docid did, Xapian::weight /*w_min*/)
{
    LOGCALL(DB, PostList *, "FlintAllDocsPostList::skip_to", did | Literal("/*w_min*/"));

    if (did <= current_did || at_end()) RETURN(NULL);

    if (cursor->find_entry_ge(F_pack_uint_preserving_sort(did))) {
	// The exact docid that was asked for exists.
	current_did = did;
	RETURN(NULL);
    }
    if (cursor->after_end()) RETURN(NULL);

    RETURN(read_did_from_current_key());
}

bool
FlintAllDocsPostList::at_end() const {
    LOGCALL(DB, bool, "FlintAllDocsPostList::at_end", NO_ARGS);
    RETURN(cursor->after_end());
}

string
FlintAllDocsPostList::get_description() const
{
    string desc = "FlintAllDocsPostList(did=";
    desc += str(current_did);
    desc += ",doccount=";
    desc += str(doccount);
    desc += ')';
    return desc;
}
