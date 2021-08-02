/** @file
 * @brief PostList in a honey database.
 */
/* Copyright (C) 2017,2018 Olly Betts
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
#include "pack.h"

#include <string>

using namespace Honey;
using namespace std;

bool
HoneyPostList::update_reader()
{
    Xapian::docid chunk_last = docid_from_key(term, cursor->current_key);
    if (!chunk_last) return false;

    cursor->read_tag();
    const string& tag = cursor->current_tag;
    reader.assign(tag.data(), tag.size(), chunk_last);
    return true;
}

// Return T with just its top bit set (for unsigned T).
#define TOP_BIT_SET(T) ((static_cast<T>(-1) >> 1) + 1)

HoneyPostList::HoneyPostList(const HoneyDatabase* db_,
			     const string& term_,
			     HoneyCursor* cursor_)
    : LeafPostList(term_), cursor(cursor_), db(db_)
{
    if (!cursor) {
	// Term not present in db.
	reader.init();
	last_did = 0;
	wdf_max = 0;
	return;
    }

    cursor->read_tag();
    const string& chunk = cursor->current_tag;

    const char* p = chunk.data();
    const char* pend = p + chunk.size();
    // FIXME: Make use of [first,last] ranges to calculate better estimates and
    // potentially to spot subqueries that can't match anything.
    Xapian::doccount tf;
    Xapian::termcount cf;
    Xapian::docid first_did;
    Xapian::termcount first_wdf;
    Xapian::docid chunk_last;
    if (!decode_initial_chunk_header(&p, pend, tf, cf,
				     first_did, last_did,
				     chunk_last, first_wdf, wdf_max))
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header");

    Xapian::termcount cf_info = cf;
    if (cf == 0) {
	// wdf must always be zero.
    } else if (tf <= 2) {
	// No further postlist data stored.
    } else if (cf == tf - 1 + first_wdf) {
	// wdf must be 1 for second and subsequent entries.
	cf_info = 1 | TOP_BIT_SET(decltype(cf_info));
    } else {
	cf_info = 1;
	// wdf_max can only be zero if cf == 0 (and
	// decode_initial_chunk_header() should ensure this).
	Assert(wdf_max != 0);
	Xapian::termcount remaining_cf_for_flat_wdf = (tf - 1) * wdf_max;
	// Check this matches and that it isn't a false match due
	// to overflow of the multiplication above.
	if (cf - first_wdf == remaining_cf_for_flat_wdf &&
	    usual(remaining_cf_for_flat_wdf / wdf_max == tf - 1)) {
	    // Set cl_info to the flat wdf value with the top bit set to
	    // signify that this is a flat wdf value.
	    cf_info = wdf_max;
	    // It shouldn't be possible for the top bit to already be set since
	    // tf > 2 so cf must be at least 2 * remaining_cf_for_flat_wdf.
	    Assert((cf_info & TOP_BIT_SET(decltype(cf_info))) == 0);
	    cf_info |= TOP_BIT_SET(decltype(cf_info));
	}
    }

    reader.init(tf, cf_info);
    reader.assign(p, pend - p, first_did, last_did, first_wdf);
}

HoneyPostList::~HoneyPostList()
{
    delete cursor;
}

Xapian::doccount
HoneyPostList::get_termfreq() const
{
    return reader.get_termfreq();
}

LeafPostList*
HoneyPostList::open_nearby_postlist(const string& term_,
				    bool need_read_pos) const
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
	// No need to consider need_read_pos for an empty posting list.
	return new HoneyPostList(db, term_, NULL);
    }

    if (need_read_pos)
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
    if (!started) {
	started = true;
	return NULL;
    }

    Assert(!reader.at_end());

    if (reader.next())
	return NULL;

    if (reader.get_docid() >= last_did) {
	// We've reached the end.
	delete cursor;
	cursor = NULL;
	return NULL;
    }

    if (rare(!cursor->next()))
	throw Xapian::DatabaseCorruptError("Hit end of table looking for "
					   "postlist chunk");

    if (rare(!update_reader()))
	throw Xapian::DatabaseCorruptError("Missing postlist chunk");

    return NULL;
}

PostList*
HoneyPostList::skip_to(Xapian::docid did, double)
{
    if (!started) {
	started = true;
    }

    if (rare(!cursor)) {
	// No-op if already at_end.
	return NULL;
    }

    Assert(!reader.at_end());

    if (reader.skip_to(did))
	return NULL;

    if (did > last_did) {
	// We've reached the end.
	delete cursor;
	cursor = NULL;
	return NULL;
    }

    // At this point we know that skip_to() must succeed since last_did
    // satisfies the requirements.

    // find_entry_ge() returns true for an exact match, which isn't interesting
    // here.
    (void)cursor->find_entry_ge(make_postingchunk_key(term, did));

    if (rare(cursor->after_end()))
	throw Xapian::DatabaseCorruptError("Hit end of table looking for "
					   "postlist chunk");

    if (rare(!update_reader()))
	throw Xapian::DatabaseCorruptError("Missing postlist chunk");

    if (rare(!reader.skip_to(did)))
	throw Xapian::DatabaseCorruptError("Postlist chunk doesn't contain "
					   "its last entry");

    return NULL;
}

Xapian::termcount
HoneyPostList::get_wdf_upper_bound() const
{
    return wdf_max;
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
PostingChunkReader::assign(const char* p_, size_t len,
			   Xapian::docid chunk_last)
{
    const char* pend = p_ + len;
    if (collfreq_info ?
	!decode_delta_chunk_header(&p_, pend, chunk_last, did, wdf) :
	!decode_delta_chunk_header_no_wdf(&p_, pend, chunk_last, did)) {
	throw Xapian::DatabaseCorruptError("Postlist delta chunk header");
    }
    p = p_;
    end = pend;
    last_did = chunk_last;
}

void
PostingChunkReader::assign(const char* p_, size_t len, Xapian::docid did_,
			   Xapian::docid last_did_in_chunk,
			   Xapian::termcount wdf_)
{
    p = p_;
    end = p_ + len;
    did = did_;
    last_did = last_did_in_chunk;
    wdf = wdf_;
}

bool
PostingChunkReader::next()
{
    if (p == end) {
	if (termfreq == 2 && did != last_did) {
	    did = last_did;
	    wdf = collfreq_info - wdf;
	    return true;
	}
	p = NULL;
	return false;
    }

    // The "constant wdf apart from maybe the first entry" case.
    if (collfreq_info & TOP_BIT_SET(decltype(collfreq_info))) {
	wdf = collfreq_info &~ TOP_BIT_SET(decltype(collfreq_info));
	collfreq_info = 0;
    }

    Xapian::docid delta;
    if (!unpack_uint(&p, end, &delta)) {
	throw Xapian::DatabaseCorruptError("postlist docid delta");
    }
    did += delta + 1;
    if (collfreq_info) {
	if (!unpack_uint(&p, end, &wdf)) {
	    throw Xapian::DatabaseCorruptError("postlist wdf");
	}
    }

    return true;
}

bool
PostingChunkReader::skip_to(Xapian::docid target)
{
    if (p == NULL)
	return false;

    if (target <= did)
	return true;

    if (target > last_did) {
	p = NULL;
	return false;
    }

    if (p == end) {
	// Given the checks above, this must be the termfreq == 2 case with the
	// current position being on the first entry, and so skip_to() must
	// move to last_did.
	AssertEq(termfreq, 2);
	did = last_did;
	wdf = collfreq_info - wdf;
	return true;
    }

    // The "constant wdf apart from maybe the first entry" case.
    if (collfreq_info & TOP_BIT_SET(decltype(collfreq_info))) {
	wdf = collfreq_info &~ TOP_BIT_SET(decltype(collfreq_info));
	collfreq_info = 0;
    }

    if (target == last_did) {
	if (collfreq_info) {
	    if (!unpack_uint_backwards(&end, p, &wdf))
		throw Xapian::DatabaseCorruptError("postlist final wdf");
	}
	did = last_did;
	p = end;
	return true;
    }

    do {
	if (rare(p == end)) {
	    // FIXME: Shouldn't happen unless last_did was wrong.
	    p = NULL;
	    return false;
	}

	Xapian::docid delta;
	if (!unpack_uint(&p, end, &delta)) {
	    throw Xapian::DatabaseCorruptError("postlist docid delta");
	}
	did += delta + 1;
	if (collfreq_info) {
	    if (!unpack_uint(&p, end, &wdf)) {
		throw Xapian::DatabaseCorruptError("postlist wdf");
	    }
	}
    } while (target > did);

    return true;
}

}
