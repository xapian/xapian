/** @file honey_postlisttable.cc
 * @brief Subclass of HoneyTable which holds postlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014,2015,2016,2017 Olly Betts
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

#include "honey_postlisttable.h"

#include "honey_cursor.h"
#include "honey_database.h"
#include "honey_postlist.h"
#include "honey_postlist_encodings.h"

#include <memory>

using namespace std;

HoneyPostList*
HoneyPostListTable::open_post_list(const HoneyDatabase* db,
				   const std::string& term) const
{
    // Try to position cursor first so we avoid creating HoneyPostList objects
    // for terms which don't exist.
    unique_ptr<HoneyCursor> cursor(cursor_get());
    if (!cursor->find_exact(Honey::make_postingchunk_key(term))) {
	// FIXME: Return NULL here and handle that in Query::Internal
	// postlist() methods as we build the PostList tree.
	// return NULL;
	return new HoneyPostList(db, term, NULL);
    }

    return new HoneyPostList(db, term, cursor.release());
}

void
HoneyPostListTable::get_freqs(const std::string& term,
			      Xapian::doccount* termfreq_ptr,
			      Xapian::termcount* collfreq_ptr) const
{
    string chunk;
    if (!get_exact_entry(Honey::make_postingchunk_key(term), chunk)) {
	if (termfreq_ptr) *termfreq_ptr = 0;
	if (collfreq_ptr) *collfreq_ptr = 0;
	return;
    }

    const char* p = chunk.data();
    const char* pend = p + chunk.size();
    Xapian::doccount tf;
    Xapian::termcount cf;
    Xapian::docid first, last;
    if (!decode_initial_chunk_header(&p, pend, tf, cf, first, last))
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header");
    if (termfreq_ptr) *termfreq_ptr = tf;
    if (collfreq_ptr) *collfreq_ptr = cf;
}

void
HoneyPostListTable::get_used_docid_range(Xapian::docid& first,
					 Xapian::docid& last) const
{
    unique_ptr<HoneyCursor> cursor;
    if (cursor->find_entry_ge(string("\0\xe0", 2))) {
	first = 1;
    } else {
	const char* p = cursor->current_key.data();
	const char* pend = p + cursor->current_key.size();
	p += 2;
	if (p[-2] != '\0' ||
	    p[-1] != '\xe0' ||
	    !unpack_uint_preserving_sort(&p, pend, &first) ||
	    p != pend) {
	    // Note that our caller checks for doccount == 0 and handles that.
	    throw Xapian::DatabaseCorruptError("Bad first doclen chunk");
	}
    }

    cursor->find_entry_lt(string("\0\xe1", 2));
    cursor->read_tag();
    if (cursor->current_key.size() == 2) {
	// Must be a single doclen chunk starting at 1.
	AssertEq(cursor->current_key[0], '\0');
	AssertEq(cursor->current_key[1], '\xe0');
	last = cursor->current_tag.size() >> 2;
    } else {
	const char* p = cursor->current_key.data();
	const char* pend = p + cursor->current_key.size();
	p += 2;
	if (p[-2] != '\0' ||
	    p[-1] != '\xe0' ||
	    !unpack_uint_preserving_sort(&p, pend, &last) ||
	    p != pend) {
	    throw Xapian::DatabaseCorruptError("Bad final doclen chunk");
	}
	last += (cursor->current_tag.size() >> 2) - 1;
    }
}
