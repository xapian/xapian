/** @file honey_alldocspostlist.cc
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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
#include "honey_alldocspostlist.h"

#include "honey_database.h"

#include "debuglog.h"
#include "str.h"
#include "wordaccess.h"

#include <string>

using namespace Honey;
using namespace std;

bool
HoneyAllDocsPostList::update_reader()
{
    Xapian::docid first_did = docid_from_key(cursor->current_key);
    if (!first_did) return false;

    cursor->read_tag();
    const string& tag = cursor->current_tag;
    reader.assign(tag.data(), tag.size(), first_did);
    return true;
}

HoneyAllDocsPostList::HoneyAllDocsPostList(const HoneyDatabase* db,
					   Xapian::doccount doccount_)
    : LeafPostList(string()),
      cursor(db->get_postlist_cursor()),
      doccount(doccount_)
{
    LOGCALL_CTOR(DB, "HoneyAllDocsPostList", db | doccount_);
    cursor->find_entry_ge(string("\0\xe0", 2));
}

HoneyAllDocsPostList::~HoneyAllDocsPostList()
{
    delete cursor;
}

Xapian::doccount
HoneyAllDocsPostList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "HoneyAllDocsPostList::get_termfreq", NO_ARGS);
    RETURN(doccount);
}

Xapian::termcount
HoneyAllDocsPostList::get_doclength() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_doclength", NO_ARGS);
    RETURN(reader.get_doclength());
}

Xapian::docid
HoneyAllDocsPostList::get_docid() const
{
    return reader.get_docid();
}

Xapian::termcount
HoneyAllDocsPostList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_wdf", NO_ARGS);
    AssertParanoid(!at_end());
    RETURN(1);
}

bool
HoneyAllDocsPostList::at_end() const
{
    return cursor == NULL;
}

PostList*
HoneyAllDocsPostList::next(double)
{
    Assert(cursor);
    if (!reader.at_end()) {
	reader.next();
	if (!reader.at_end()) return NULL;
	cursor->next();
    }

    if (!cursor->after_end()) {
	if (update_reader()) {
	    if (!reader.at_end()) return NULL;
	}
    }

    // We've reached the end.
    delete cursor;
    cursor = NULL;
    return NULL;
}

PostList*
HoneyAllDocsPostList::skip_to(Xapian::docid did, double)
{
    if (rare(!cursor)) {
	// No-op if already at_end.
	return NULL;
    }

    if (!reader.at_end()) {
	reader.skip_to(did);
	if (!reader.at_end()) return NULL;
    }

    if (!cursor->find_entry(make_doclenchunk_key(did))) {
	if (update_reader()) {
	    reader.skip_to(did);
	    if (!reader.at_end()) return NULL;
	}
	// The requested docid is between two chunks.
	cursor->next();
    }

    // Either an exact match, or in a gap before the start of a chunk.
    if (!cursor->after_end()) {
	if (update_reader()) {
	    if (!reader.at_end()) return NULL;
	}
    }

    // We've reached the end.
    delete cursor;
    cursor = NULL;
    return NULL;
}

PostList*
HoneyAllDocsPostList::check(Xapian::docid did, double, bool& valid)
{
    if (rare(!cursor)) {
	// Already at_end.
	valid = true;
	return NULL;
    }

    if (!reader.at_end()) {
	// Check for the requested docid in the current block.
	reader.skip_to(did);
	if (!reader.at_end()) {
	    valid = true;
	    return NULL;
	}
    }

    // Try moving to the appropriate chunk.
    if (!cursor->find_entry(make_doclenchunk_key(did))) {
	// We're in a chunk which might contain the docid.
	if (update_reader()) {
	    reader.skip_to(did);
	    if (!reader.at_end()) {
		valid = true;
		return NULL;
	    }
	}
	valid = false;
	return NULL;
    }

    // We had an exact match for a chunk starting with specified docid.
    Assert(!cursor->after_end());
    if (!update_reader()) {
	// We found the exact key we built so it must be a doclen chunk.
	// Therefore update_reader() "can't possibly fail".
	Assert(false);
    }

    valid = true;
    return NULL;
}

string
HoneyAllDocsPostList::get_description() const
{
    string desc = "HoneyAllDocsPostList(did=";
    desc += str(get_docid());
    desc += ",doccount=";
    desc += str(doccount);
    desc += ')';
    return desc;
}

namespace Honey {

void
DocLenChunkReader::assign(const char * p_, size_t len, Xapian::docid did_)
{
    if (len % 4 != 0 || len == 0)
	throw Xapian::DatabaseCorruptError("Doclen data length not a non-zero multiple of 4");
    if (rare(len == 0)) {
	p = NULL;
	return;
    }
    p = reinterpret_cast<const unsigned char*>(p_);
    end = p + len;
    did = did_;
    // FIXME: Alignment guarantees?
    doclen = unaligned_read4(p);
    p += 4;
}

void
DocLenChunkReader::next()
{
    if (p == end) {
	p = NULL;
	return;
    }

    // FIXME: Alignment guarantees?
    doclen = unaligned_read4(p);
    p += 4;
}

void
DocLenChunkReader::skip_to(Xapian::docid target)
{
    if (p == NULL || target <= did)
	return;

    Xapian::docid delta = target - did;
    if (delta > Xapian::docid((end - p) >> 2)) {
	p = NULL;
	return;
    }

    did = target;
    p += delta << 2;

    // FIXME: Alignment guarantees?
    doclen = unaligned_read4(p - 4);
}

}
