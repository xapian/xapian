/** @file
 * @brief Subclass of HoneyTable which holds postlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014,2015,2016,2017,2018,2019 Olly Betts
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

#include "honey_alldocspostlist.h"
#include "honey_cursor.h"
#include "honey_database.h"
#include "honey_defs.h"
#include "honey_postlist.h"
#include "honey_postlist_encodings.h"

#include <memory>

using namespace Honey;
using namespace std;

HoneyPostList*
HoneyPostListTable::open_post_list(const HoneyDatabase* db,
				   const std::string& term,
				   bool need_read_pos) const
{
    Assert(!term.empty());
    // Try to position cursor first so we avoid creating HoneyPostList objects
    // for terms which don't exist.
    unique_ptr<HoneyCursor> cursor(cursor_get());
    if (!cursor->find_exact(Honey::make_postingchunk_key(term))) {
	// FIXME: Return NULL here and handle that in Query::Internal
	// postlist() methods as we build the PostList tree.
	// return NULL;
	return new HoneyPostList(db, term, NULL);
    }

    if (need_read_pos)
	return new HoneyPosPostList(db, term, cursor.release());
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
    if (!decode_initial_chunk_header_freqs(&p, pend, tf, cf))
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header");
    if (termfreq_ptr) *termfreq_ptr = tf;
    if (collfreq_ptr) *collfreq_ptr = cf;
}

void
HoneyPostListTable::get_used_docid_range(Xapian::doccount doccount,
					 Xapian::docid& first,
					 Xapian::docid& last) const
{
    unique_ptr<HoneyCursor> cursor(cursor_get());
    Assert(cursor.get());

    static const char doclen_key_prefix[2] = {
	0, char(Honey::KEY_DOCLEN_CHUNK)
    };
    if (cursor->find_entry_ge(string(doclen_key_prefix, 2))) {
	first = 1;
    } else {
	// doccount == 0 should be handled by our caller.
	Assert(!cursor->after_end());
	Xapian::docid last_in_first_chunk = docid_from_key(cursor->current_key);
	if (last_in_first_chunk == 0) {
	    // Note that our caller checks for doccount == 0 and handles that.
	    throw Xapian::DatabaseCorruptError("Bad first doclen chunk key");
	}
	cursor->read_tag();
	unsigned width = cursor->current_tag[0] / 8;
	first = last_in_first_chunk - (cursor->current_tag.size() - 2) / width;
    }

    // We know the last docid is at least first - 1 + doccount, so seek
    // to there and then scan forwards.  If we match exactly, then that
    // is exactly the last docid (our caller handles this case when
    // first == 1, but not otherwise).
    last = first - 1 + doccount;
    if (cursor->find_entry_ge(make_doclenchunk_key(last)))
	return;

    if (cursor->after_end())
	throw Xapian::DatabaseCorruptError("Missing doclen chunk");

    do {
	Xapian::docid new_last = docid_from_key(cursor->current_key);
	if (new_last == 0) {
	    // We've hit a non-doclen item.
	    return;
	}
	last = new_last;
	cursor->next();
    } while (!cursor->after_end());

    // We've reached the end of the table (only possible if there are no terms
    // at all!)
}

Xapian::termcount
HoneyPostListTable::get_wdf_upper_bound(const std::string& term) const
{
    string chunk;
    if (!get_exact_entry(Honey::make_postingchunk_key(term), chunk)) {
	// Term not present.
	return 0;
    }

    const char* p = chunk.data();
    const char* pend = p + chunk.size();
    Xapian::doccount tf;
    Xapian::termcount cf;
    Xapian::docid first;
    Xapian::docid last;
    Xapian::docid chunk_last;
    Xapian::termcount first_wdf;
    Xapian::termcount wdf_max;
    if (!decode_initial_chunk_header(&p, pend, tf, cf, first, last, chunk_last,
				     first_wdf, wdf_max))
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header");
    return wdf_max;
}
