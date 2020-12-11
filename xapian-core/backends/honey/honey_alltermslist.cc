/** @file
 * @brief A termlist containing all terms in a honey database.
 */
/* Copyright (C) 2005,2007,2008,2009,2010,2017,2018 Olly Betts
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

#include "honey_alltermslist.h"
#include "honey_cursor.h"
#include "honey_postlist.h"
#include "honey_postlist_encodings.h"

#include "debuglog.h"
#include "pack.h"
#include "stringutils.h"

#include "xapian/error.h"

using namespace std;

void
HoneyAllTermsList::read_termfreq() const
{
    LOGCALL_VOID(DB, "HoneyAllTermsList::read_termfreq", NO_ARGS);
    Assert(!at_end());

    // Unpack the termfreq from the tag.
    Xapian::termcount collfreq;
    cursor->read_tag();
    const char* p = cursor->current_tag.data();
    const char* pend = p + cursor->current_tag.size();
    if (!decode_initial_chunk_header_freqs(&p, pend,
					   termfreq, collfreq)) {
	throw Xapian::DatabaseCorruptError("Postlist initial chunk header not "
					   "as expected");
    }
    // Not used.
    (void)collfreq;
}

HoneyAllTermsList::~HoneyAllTermsList()
{
    LOGCALL_DTOR(DB, "HoneyAllTermsList");
    delete cursor;
}

Xapian::termcount
HoneyAllTermsList::get_approx_size() const
{
    // This is an over-estimate and not entirely proportional between shards,
    // but we only use this value to build a balanced or-tree, and it'll at
    // least tend to distinguish large databases from small ones.
    return database->postlist_table.get_approx_entry_count();
}

string
HoneyAllTermsList::get_termname() const
{
    LOGCALL(DB, string, "HoneyAllTermsList::get_termname", NO_ARGS);
    Assert(!at_end());
    RETURN(current_term);
}

Xapian::doccount
HoneyAllTermsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "HoneyAllTermsList::get_termfreq", NO_ARGS);
    Assert(!at_end());
    if (termfreq == 0) read_termfreq();
    RETURN(termfreq);
}

TermList*
HoneyAllTermsList::next()
{
    LOGCALL(DB, TermList*, "HoneyAllTermsList::next", NO_ARGS);
    // Set termfreq to 0 to indicate no termfreq has been read for the current
    // term.
    termfreq = 0;

    if (rare(!cursor)) {
	Assert(database.get());
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.

	if (prefix.empty()) {
	    (void)cursor->find_entry_ge(string("\x00\xff", 2));
	} else {
	    const string& key = pack_honey_postlist_key(prefix);
	    if (cursor->find_entry_ge(key)) {
		// The exact term we asked for is there, so just copy it rather
		// than wasting effort unpacking it from the key.
		current_term = prefix;
		RETURN(NULL);
	    }
	}
	if (cursor->after_end()) {
	    delete cursor;
	    cursor = NULL;
	    database = NULL;
	    RETURN(NULL);
	}
	goto first_time;
    }

    while (true) {
	if (!cursor->next()) {
	    delete cursor;
	    cursor = NULL;
	    database = NULL;
	    RETURN(NULL);
	}

first_time:
	const char* p = cursor->current_key.data();
	const char* pend = p + cursor->current_key.size();
	if (!unpack_string_preserving_sort(&p, pend, current_term)) {
	    throw Xapian::DatabaseCorruptError("PostList table key has "
					       "unexpected format");
	}

	// If this key is for the first chunk of a postlist, we're done.
	// Otherwise we need to skip past continuation chunks until we find the
	// first chunk of the next postlist.
	if (p == pend) break;
    }

    if (!startswith(current_term, prefix)) {
	// We've reached the end of the prefixed terms.
	delete cursor;
	cursor = NULL;
	database = NULL;
    }

    RETURN(NULL);
}

TermList*
HoneyAllTermsList::skip_to(const string& term)
{
    LOGCALL(DB, TermList*, "HoneyAllTermsList::skip_to", term);
    // Set termfreq to 0 to indicate no termfreq has been read for the current
    // term.
    termfreq = 0;

    if (rare(!cursor)) {
	if (!database.get()) {
	    // skip_to() once at_end() is allowed but a no-op.
	    RETURN(NULL);
	}
	if (rare(term.empty())) {
	    RETURN(next());
	}
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.
    }

    if (rare(term.empty())) {
	RETURN(NULL);
    }

    string key = pack_honey_postlist_key(term);
    if (cursor->find_entry_ge(key)) {
	// The exact term we asked for is there, so just copy it rather than
	// wasting effort unpacking it from the key.
	current_term = term;
    } else {
	if (cursor->after_end()) {
	    delete cursor;
	    cursor = NULL;
	    database = NULL;
	    RETURN(NULL);
	}

	const char* p = cursor->current_key.data();
	const char* pend = p + cursor->current_key.size();
	if (!unpack_string_preserving_sort(&p, pend, current_term) ||
	    p != pend) {
	    throw Xapian::DatabaseCorruptError("PostList table key has "
					       "unexpected format");
	}
    }

    if (!startswith(current_term, prefix)) {
	// We've reached the end of the prefixed terms.
	delete cursor;
	cursor = NULL;
	database = NULL;
    }

    RETURN(NULL);
}

bool
HoneyAllTermsList::at_end() const
{
    LOGCALL(DB, bool, "HoneyAllTermsList::at_end", NO_ARGS);
    // Either next() or skip_to() should be called before at_end().
    Assert(!(cursor == NULL && database.get() != NULL));
    RETURN(cursor == NULL);
}
