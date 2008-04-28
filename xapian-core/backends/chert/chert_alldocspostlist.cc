/** @file chert_alldocspostlist.cc
 * @brief A PostList which iterates over all documents in a ChertDatabase.
 */
/* Copyright (C) 2006,2007,2008 Olly Betts
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

#include <string>

#include "chert_database.h"
#include "chert_alldocspostlist.h"

#include "utils.h"

using namespace std;

Xapian::doccount
ChertAllDocsPostList::get_termfreq() const
{
    return doccount;
}

Xapian::docid
ChertAllDocsPostList::get_docid() const
{
    return current_did;
}

Xapian::doclength
ChertAllDocsPostList::get_doclength() const
{
    DEBUGCALL(DB, Xapian::doclength, "ChertAllDocsPostList::get_doclength", "");
    Assert(current_did);

    cursor->read_tag();

    if (cursor->current_tag.empty()) RETURN(0);

    const char * pos = cursor->current_tag.data();
    const char * end = pos + cursor->current_tag.size();

    chert_doclen_t doclen;
    if (!unpack_uint(&pos, end, &doclen)) {
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
ChertAllDocsPostList::get_wdf() const
{
    DEBUGCALL(DB, Xapian::termcount, "ChertAllDocsPostList::get_wdf", "");
    Assert(current_did);
    RETURN(1);
}

PostList *
ChertAllDocsPostList::read_did_from_current_key()
{
    DEBUGCALL(DB, PostList *, "ChertAllDocsPostList::read_did_from_current_key",
	      "");
    const string & key = cursor->current_key;
    const char * pos = key.data();
    const char * end = pos + key.size();
    if (!unpack_uint_preserving_sort(&pos, end, &current_did)) {
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
ChertAllDocsPostList::next(Xapian::weight /*w_min*/)
{
    DEBUGCALL(DB, PostList *, "ChertAllDocsPostList::next", "/*w_min*/");
    Assert(!at_end());
    if (!cursor->next()) RETURN(NULL);
    RETURN(read_did_from_current_key());
}

PostList *
ChertAllDocsPostList::skip_to(Xapian::docid did, Xapian::weight /*w_min*/)
{
    DEBUGCALL(DB, PostList *, "ChertAllDocsPostList::skip_to",
	      did << ", /*w_min*/");

    if (did <= current_did || at_end()) RETURN(NULL);

    if (cursor->find_entry_ge(pack_uint_preserving_sort(did))) {
	// The exact docid that was asked for exists.
	current_did = did;
	RETURN(NULL);
    }
    if (cursor->after_end()) RETURN(NULL);

    RETURN(read_did_from_current_key());
}

bool
ChertAllDocsPostList::at_end() const {
    DEBUGCALL(DB, bool, "ChertAllDocsPostList::at_end", "");
    RETURN(cursor->after_end());
}

string
ChertAllDocsPostList::get_description() const
{
    string desc = "ChertAllDocsPostList(did=";
    desc += om_tostring(current_did);
    desc += ",doccount=";
    desc += om_tostring(doccount);
    desc += ')';
    return desc;
}
