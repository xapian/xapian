/* chert_alltermslist.cc: A termlist containing all terms in a chert database.
 *
 * Copyright (C) 2005,2007,2008,2009,2010 Olly Betts
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

#include "chert_alltermslist.h"
#include "chert_postlist.h"

#include "debuglog.h"
#include "pack.h"
#include "stringutils.h"

void
ChertAllTermsList::read_termfreq() const
{
    LOGCALL_VOID(DB, "ChertAllTermsList::read_termfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());

    // Unpack the termfreq from the tag.
    Xapian::termcount collfreq;
    cursor->read_tag();
    const char *p = cursor->current_tag.data();
    const char *pend = p + cursor->current_tag.size();
    ChertPostList::read_number_of_entries(&p, pend, &termfreq, &collfreq);
    // Not used.
    (void)collfreq;
}

ChertAllTermsList::~ChertAllTermsList()
{
    LOGCALL_DTOR(DB, "ChertAllTermsList");
    delete cursor;
}

string
ChertAllTermsList::get_termname() const
{
    LOGCALL(DB, string, "ChertAllTermsList::get_termname", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());
    RETURN(current_term);
}

Xapian::doccount
ChertAllTermsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "ChertAllTermsList::get_termfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());
    if (termfreq == 0) read_termfreq();
    RETURN(termfreq);
}

TermList *
ChertAllTermsList::next()
{
    LOGCALL(DB, TermList *, "ChertAllTermsList::next", NO_ARGS);
    Assert(!at_end());
    // Set termfreq to 0 to indicate no termfreq/collfreq have been read for
    // the current term.
    termfreq = 0;

    if (rare(!cursor)) {
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.

	if (prefix.empty()) {
	    (void)cursor->find_entry_ge(string("\x00\xff", 2));
	} else {
	    const string & key = pack_chert_postlist_key(prefix);
	    if (cursor->find_entry_ge(key)) {
		// The exact term we asked for is there, so just copy it rather
		// than wasting effort unpacking it from the key.
		current_term = prefix;
		RETURN(NULL);
	    }
	}
	goto first_time;
    }

    while (true) {
	cursor->next();
first_time:
	if (cursor->after_end()) {
	    current_term.resize(0);
	    RETURN(NULL);
	}

	// Fast check for terms without any zero bytes.  ~8.4% faster for
	// glass.
	auto nul = cursor->current_key.find('\0');
	if (nul == string::npos) {
	    current_term = cursor->current_key;
	    break;
	}
	if (cursor->current_key[nul + 1] != '\xff') {
	    continue;
	}

	const char* p = cursor->current_key.data();
	const char* pend = p + cursor->current_key.size();
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
	cursor->to_end();
	current_term.resize(0);
    }

    RETURN(NULL);
}

TermList *
ChertAllTermsList::skip_to(const string &term)
{
    LOGCALL(DB, TermList *, "ChertAllTermsList::skip_to", term);
    Assert(!at_end());
    // Set termfreq to 0 to indicate no termfreq has been read for the current
    // term.
    termfreq = 0;

    if (rare(!cursor)) {
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.
    }

    string key = pack_chert_postlist_key(term);
    if (cursor->find_entry_ge(key)) {
	// The exact term we asked for is there, so just copy it rather than
	// wasting effort unpacking it from the key.
	current_term = term;
    } else {
	if (cursor->after_end()) {
	    current_term.resize(0);
	    RETURN(NULL);
	}

	const char *p = cursor->current_key.data();
	const char *pend = p + cursor->current_key.size();
	if (!unpack_string_preserving_sort(&p, pend, current_term)) {
	    throw Xapian::DatabaseCorruptError("PostList table key has unexpected format");
	}
    }

    if (!startswith(current_term, prefix)) {
	// We've reached the end of the prefixed terms.
	cursor->to_end();
	current_term.resize(0);
    }

    RETURN(NULL);
}

bool
ChertAllTermsList::at_end() const
{
    LOGCALL(DB, bool, "ChertAllTermsList::at_end", NO_ARGS);
    RETURN(cursor && cursor->after_end());
}
