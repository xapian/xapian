/** @file
 * @brief A termlist containing all terms in a glass database.
 */
/* Copyright (C) 2005,2007,2008,2009,2010,2017,2024 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "glass_alltermslist.h"
#include "glass_postlist.h"

#include "debuglog.h"
#include "pack.h"
#include "stringutils.h"

#include <string_view>

using namespace std;

void
GlassAllTermsList::read_termfreq() const
{
    LOGCALL_VOID(DB, "GlassAllTermsList::read_termfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(cursor);
    Assert(!cursor->after_end());

    // Unpack the termfreq from the tag.
    cursor->read_tag();
    const char *p = cursor->current_tag.data();
    const char *pend = p + cursor->current_tag.size();
    GlassPostList::read_freqs(&p, pend, &termfreq, NULL);
}

GlassAllTermsList::~GlassAllTermsList()
{
    LOGCALL_DTOR(DB, "GlassAllTermsList");
    delete cursor;
}

Xapian::termcount
GlassAllTermsList::get_approx_size() const
{
    // This is an over-estimate and not entirely proportional between shards,
    // but we only use this value to build a balanced or-tree, and it'll at
    // least tend to distinguish large databases from small ones.
    return database->postlist_table.get_entry_count();
}

Xapian::doccount
GlassAllTermsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "GlassAllTermsList::get_termfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(cursor);
    Assert(!cursor->after_end());
    if (termfreq == 0) read_termfreq();
    RETURN(termfreq);
}

TermList *
GlassAllTermsList::next()
{
    LOGCALL(DB, TermList *, "GlassAllTermsList::next", NO_ARGS);
    // Set termfreq to 0 to indicate no termfreq has been read for the current
    // term.
    termfreq = 0;

    if (rare(!cursor)) {
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.

	if (prefix.empty()) {
	    (void)cursor->find_entry_ge(string("\x00\xff", 2));
	} else {
	    const string & key = pack_glass_postlist_key(prefix);
	    if (cursor->find_entry_ge(key)) {
		// The exact term we asked for is there, so just copy it rather
		// than wasting effort unpacking it from the key.
		current_term = prefix;
		RETURN(NULL);
	    }
	}
	if (cursor->after_end()) {
	    RETURN(this);
	}
	goto first_time;
    }

    Assert(!cursor->after_end());
    while (true) {
	if (!cursor->next()) {
	    RETURN(this);
	}

first_time:
	// Fast check for terms without any zero bytes.  ~8.4% faster.
	auto nul = cursor->current_key.find('\0');
	if (nul == string::npos) {
	    current_term = cursor->current_key;
	    break;
	}
	if (cursor->current_key[nul + 1] != '\xff') {
	    continue;
	}

	const char *p = cursor->current_key.data();
	const char *pend = p + cursor->current_key.size();
	if (!unpack_string_preserving_sort(&p, pend, current_term)) {
	    throw Xapian::DatabaseCorruptError("PostList table key has unexpected format");
	}

	// If this key is for the first chunk of a postlist, we're done.
	// Otherwise we need to skip past continuation chunks until we find the
	// first chunk of the next postlist.
	if (p == pend) break;
    }

    if (!startswith(current_term, prefix)) {
	// We've reached the end of the prefixed terms.
	RETURN(this);
    }

    RETURN(NULL);
}

TermList*
GlassAllTermsList::skip_to(string_view term)
{
    LOGCALL(DB, TermList *, "GlassAllTermsList::skip_to", term);
    // Set termfreq to 0 to indicate no termfreq has been read for the current
    // term.
    termfreq = 0;

    if (rare(!cursor)) {
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.
    }
    Assert(!cursor->after_end());

    string key = pack_glass_postlist_key(term);
    if (cursor->find_entry_ge(key)) {
	// The exact term we asked for is there, so just copy it rather than
	// wasting effort unpacking it from the key.
	current_term = term;
    } else {
	if (cursor->after_end()) {
	    RETURN(this);
	}

	const char *p = cursor->current_key.data();
	const char *pend = p + cursor->current_key.size();
	if (!unpack_string_preserving_sort(&p, pend, current_term)) {
	    throw Xapian::DatabaseCorruptError("PostList table key has unexpected format");
	}
    }

    if (!startswith(current_term, prefix)) {
	// We've reached the end of the prefixed terms.
	RETURN(this);
    }

    RETURN(NULL);
}
