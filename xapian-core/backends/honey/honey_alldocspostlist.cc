/** @file
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009,2018 Olly Betts
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
#include "honey_defs.h"

#include "debuglog.h"
#include "str.h"
#include "wordaccess.h"

#include <string>

using namespace Honey;
using namespace std;

HoneyAllDocsPostList::HoneyAllDocsPostList(const HoneyDatabase* db,
					   Xapian::doccount doccount_)
    : LeafPostList(string()),
      cursor(db->get_postlist_cursor()),
      doccount(doccount_)
{
    LOGCALL_CTOR(DB, "HoneyAllDocsPostList", db | doccount_);
    static const char doclen_key_prefix[2] = {
	0, char(Honey::KEY_DOCLEN_CHUNK)
    };
    cursor->find_entry_ge(string(doclen_key_prefix, 2));
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
	if (reader.next()) return NULL;
	cursor->next();
    }

    if (!cursor->after_end()) {
	if (reader.update(cursor)) {
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

    Assert(!reader.at_end());

    if (reader.skip_to(did))
	return NULL;

    if (cursor->find_entry_ge(make_doclenchunk_key(did))) {
	// Exact match.
	if (rare(!reader.update(cursor))) {
	    // Shouldn't be possible.
	    Assert(false);
	}
	if (reader.skip_to(did)) return NULL;
	// The chunk's last docid is did, so skip_to() should always succeed.
	Assert(false);
    } else if (!cursor->after_end()) {
	if (reader.update(cursor)) {
	    if (reader.skip_to(did)) return NULL;
	    // The chunk's last docid is >= did, so skip_to() should always
	    // succeed.
	    Assert(false);
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
	if (reader.skip_to(did)) {
	    valid = true;
	    return NULL;
	}
    }

    // Try moving to the appropriate chunk.
    if (!cursor->find_entry_ge(make_doclenchunk_key(did))) {
	// We're in a chunk which might contain the docid.
	if (reader.update(cursor)) {
	    if (reader.skip_to(did)) {
		valid = true;
		return NULL;
	    }
	}
	valid = false;
	return NULL;
    }

    // We had an exact match for a chunk starting with specified docid.
    Assert(!cursor->after_end());
    if (!reader.update(cursor)) {
	// We found the exact key we built so it must be a doclen chunk.
	// Therefore reader.update() "can't possibly fail".
	Assert(false);
    }

    valid = true;
    return NULL;
}

Xapian::termcount
HoneyAllDocsPostList::get_wdf_upper_bound() const
{
    LOGCALL(DB, Xapian::termcount, "HoneyAllDocsPostList::get_wdf_upper_bound", NO_ARGS);
    RETURN(1);
}

string
HoneyAllDocsPostList::get_description() const
{
    string desc = "HoneyAllDocsPostList(doccount=";
    desc += str(doccount);
    desc += ')';
    return desc;
}

namespace Honey {

bool
DocLenChunkReader::read_doclen(const unsigned char* q)
{
    switch (width) {
	case 1:
	    doclen = *q;
	    return doclen != 0xff;
	case 2:
	    doclen = unaligned_read2(q);
	    return doclen != 0xffff;
	case 3:
	    // q - 1 is always a valid byte - either the leading byte holding
	    // the data width, or else the last byte of the previous value.
	    // unaligned_read4() uses bigendian order, so we just need to mask
	    // off the most significant byte.
	    doclen = unaligned_read4(q - 1) & 0xffffff;
	    return doclen != 0xffffff;
	default:
	    doclen = unaligned_read4(q);
	    return doclen != 0xffffffff;
    }
}

bool
DocLenChunkReader::update(HoneyCursor* cursor)
{
    Xapian::docid last_did = docid_from_key(cursor->current_key);
    if (!last_did) return false;

    cursor->read_tag();

    size_t len = cursor->current_tag.size();
    if (rare(len == 0))
	throw Xapian::DatabaseCorruptError("Doclen data chunk is empty");

    p = reinterpret_cast<const unsigned char*>(cursor->current_tag.data());
    end = p + len;
    width = *p++;
    if (((width - 8) &~ 0x18) != 0) {
	throw Xapian::DatabaseCorruptError("Invalid doclen width - currently "
					   "8, 16, 24 and 32 are supported");
    }
    width /= 8;
    if ((len - 1) % width != 0)
	throw Xapian::DatabaseCorruptError("Doclen data chunk has junk at end");
    Xapian::docid first_did = last_did - (len - 1) / width + 1;

    did = first_did;
    if (!read_doclen(p)) {
	// The first doclen value shouldn't be missing.
	throw Xapian::DatabaseCorruptError("Invalid first doclen value");
    }
    return true;
}

bool
DocLenChunkReader::next()
{
    do {
	p += width;
	if (p == end) {
	    p = NULL;
	    return false;
	}

	++did;
    } while (!read_doclen(p));
    return true;
}

bool
DocLenChunkReader::skip_to(Xapian::docid target)
{
    if (p == NULL)
	return false;

    if (target <= did)
	return true;

    Xapian::docid delta = target - did;
    if (delta >= Xapian::docid(end - p) / width) {
	p = NULL;
	return false;
    }

    did = target;
    p += delta * width;

    return read_doclen(p) || next();
}

// FIXME: Add check() method, which doesn't advance when read_doclen() returns
// false?

bool
DocLenChunkReader::find_doclength(Xapian::docid target)
{
    if (target < did)
	return false;

    Xapian::docid delta = target - did;
    Assert(width > 0);
    if (delta >= Xapian::docid(end - p) / width) {
	return false;
    }

    return read_doclen(p + delta * width);
}

}
