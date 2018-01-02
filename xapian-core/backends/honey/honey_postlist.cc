/** @file honey_postlist.cc
 * @brief PostList in a honey database.
 */
/* Copyright (C) 2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "honey_postlist.h"

#include "honey_cursor.h"
#include "honey_database.h"
#include "honey_positionlist.h"
#include "honey_postlist_encodings.h"
#include "wordaccess.h"

#include <string>

using namespace Honey;
using namespace std;

bool
HoneyPostList::update_reader()
{
    Xapian::docid first_did = docid_from_key(term, cursor->current_key);
    if (!first_did) return false;

    cursor->read_tag();
    const string& tag = cursor->current_tag;
    reader.assign(tag.data(), tag.size(), first_did);
    return true;
}

HoneyPostList::HoneyPostList(const HoneyDatabase* db_,
			     const string& term_,
			     HoneyCursor* cursor_)
    : LeafPostList(term_), cursor(cursor_), db(db_)
{
    if (!cursor) {
	// Term not present in db.
	termfreq = 0;
	last_did = 0;
	return;
    }
    cursor->read_tag();
    const string& chunk = cursor->current_tag;

    const char* p = chunk.data();
    const char* pend = p + chunk.size();
    // FIXME: Make use of [first,last] ranges to calculate better estimates and
    // potentially to spot subqueries that can't match anything.
    Xapian::termcount cf;
    Xapian::docid first_did;
    if (!decode_initial_chunk_header(&p, pend, termfreq, cf,
				     first_did, last_did))
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header");
    reader.assign(p, pend - p, first_did);
}

HoneyPostList::~HoneyPostList()
{
    delete cursor;
}

Xapian::doccount
HoneyPostList::get_termfreq() const
{
    return termfreq;
}

LeafPostList*
HoneyPostList::open_nearby_postlist(const string& term_, bool need_pos) const
{
    Assert(!term_.empty());
    if (!cursor) return NULL;
    // FIXME: Once Honey supports writing, we need to return NULL here if the
    // DB is writable and has uncommitted modifications.

    unique_ptr<HoneyCursor> new_cursor(new HoneyCursor(*cursor));
    if (!new_cursor->find_exact(Honey::make_postingchunk_key(term_))) {
	// FIXME: Return NULL here and handle that in Query::Internal
	// postlist() methods as we build the PostList tree.
	// We also need to distinguish this case from "open_nearby_postlist()
	// not supported" though.
	// return NULL;
	//
	// No need to consider need_pos for an empty posting list.
	return new HoneyPostList(db, term_, NULL);
    }

    if (need_pos)
	return new HoneyPosPostList(db, term_, new_cursor.release());
    return new HoneyPostList(db, term_, new_cursor.release());
}

Xapian::docid
HoneyPostList::get_docid() const
{
    return reader.get_docid();
}

Xapian::termcount
HoneyPostList::get_wdf() const
{
    return reader.get_wdf();
}

bool
HoneyPostList::at_end() const
{
    return cursor == NULL;
}

PositionList*
HoneyPostList::open_position_list() const
{
    return new HoneyPositionList(db->position_table, get_docid(), term);
}

PostList*
HoneyPostList::next(double)
{
    if (rare(!cursor)) {
	// This happens for terms not present in db.
	AssertEq(termfreq, 0);
	return NULL;
    }

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
HoneyPostList::skip_to(Xapian::docid did, double)
{
    if (rare(!cursor)) {
	// No-op if already at_end.
	return NULL;
    }

    if (!reader.at_end()) {
	reader.skip_to(did);
	if (!reader.at_end()) return NULL;
    }

    if (did > last_did) goto set_at_end;

    if (!cursor->find_entry(make_postingchunk_key(term, did))) {
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

set_at_end:
    // We've reached the end.
    delete cursor;
    cursor = NULL;
    return NULL;
}

PostList*
HoneyPostList::check(Xapian::docid did, double, bool& valid)
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

    if (did > last_did) goto set_at_end;

    // Try moving to the appropriate chunk.
    if (!cursor->find_entry(make_postingchunk_key(term, did))) {
	// We're in a chunk which might contain the docid.
	if (update_reader()) {
	    reader.skip_to(did);
	    if (!reader.at_end()) {
		valid = true;
		return NULL;
	    }
	}

set_at_end:
	// We've reached the end.
	delete cursor;
	cursor = NULL;
	valid = true;
	return NULL;
    }

    // We had an exact match for a chunk starting with specified docid.
    Assert(!cursor->after_end());
    if (!update_reader()) {
	// We found the exact key we built so it must be a posting chunk.
	// Therefore update_reader() "can't possibly fail".
	Assert(false);
    }

    valid = true;
    return NULL;
}

string
HoneyPostList::get_description() const
{
    string desc = "HoneyPostList(";
    desc += term;
    desc += ')';
    return desc;
}

HoneyPosPostList::HoneyPosPostList(const HoneyDatabase* db_,
				   const std::string& term_,
				   HoneyCursor* cursor_)
    : HoneyPostList(db_, term_, cursor_),
      position_list(db_->position_table) {}

PositionList*
HoneyPosPostList::read_position_list()
{
    position_list.read_data(HoneyPostList::get_docid(), term);
    // FIXME: Consider returning NULL if there's no positional data - callers
    // need fixing up, but this may be a rare case and the costs of checking
    // for NULL may outweigh any gains.  Need to profile.
    return &position_list;
}

string
HoneyPosPostList::get_description() const
{
    string desc = "HoneyPosPostList(";
    desc += term;
    desc += ')';
    return desc;
}

namespace Honey {

void
PostingChunkReader::assign(const char * p_, size_t len, Xapian::docid did_)
{
    Xapian::docid last_did_in_chunk;
    const char* pend = p_ + len;
    if (!decode_delta_chunk_header(&p_, pend, did_, last_did_in_chunk)) {
	throw Xapian::DatabaseCorruptError("Postlist delta chunk header");
    }
    if ((pend - p_) % 8 != 4)
	throw Xapian::DatabaseCorruptError("Doclen data length not 4 more than a multiple of 8");
    if (rare(p_ == pend)) {
	p = NULL;
	return;
    }
    p = reinterpret_cast<const unsigned char*>(p_);
    end = reinterpret_cast<const unsigned char*>(pend);
    did = did_;
    last_did = last_did_in_chunk;
}

void
PostingChunkReader::next()
{
    if ((end - p) % 8 != 0) {
	// FIXME: Alignment guarantees?  Hard with header.
	wdf = unaligned_read4(p);
	p += 4;
	return;
    }

    if (p == end) {
	p = NULL;
	return;
    }

    // FIXME: Alignment guarantees?  Hard with header.
    did += unaligned_read4(p) + 1;
    wdf = unaligned_read4(p + 4);
    p += 8;
}

void
PostingChunkReader::skip_to(Xapian::docid target)
{
    if (p == NULL)
	return;

    if ((end - p) % 8 != 0) {
	p += 4;
	if (target <= did) {
	    // FIXME: Alignment guarantees?  Hard with header.
	    wdf = unaligned_read4(p - 4);
	    return;
	}
    }

    if (target <= did)
	return;

    if (target > last_did) {
	p = NULL;
	return;
    }

    // FIXME: Special case target == last_did to just decode the wdf from the
    // end?

    do {
	if (rare(p == end)) {
	    // FIXME: Shouldn't happen unless last_did was wrong.
	    p = NULL;
	    return;
	}

	// FIXME: Alignment guarantees?  Hard with header.
	did += unaligned_read4(p) + 1;
	p += 8;
    } while (target > did);
    wdf = unaligned_read4(p - 4);
}

}
