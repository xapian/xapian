/* glass_alltermslist.cc: A termlist containing all terms in a glass database.
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

#include "glass_alltermslist.h"
#include "glass_postlist.h"

#include "debuglog.h"
#include "pack.h"
#include "stringutils.h"

void
GlassAllTermsList::read_termfreq_and_collfreq() const
{
    LOGCALL_VOID(DB, "GlassAllTermsList::read_termfreq_and_collfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());

    // Unpack the termfreq and collfreq from the tag.  Only do this if
    // one or other is actually read.
    cursor->read_tag();
    const char *p = cursor->current_tag.data();
    const char *pend = p + cursor->current_tag.size();
    GlassPostList::read_number_of_entries(&p, pend, &termfreq, &collfreq);
}

GlassAllTermsList::~GlassAllTermsList()
{
    LOGCALL_DTOR(DB, "GlassAllTermsList");
    delete cursor;
}

string
GlassAllTermsList::get_termname() const
{
    LOGCALL(DB, string, "GlassAllTermsList::get_termname", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());
    RETURN(current_term);
}

Xapian::doccount
GlassAllTermsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "GlassAllTermsList::get_termfreq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());
    if (termfreq == 0) read_termfreq_and_collfreq();
    RETURN(termfreq);
}

Xapian::termcount
GlassAllTermsList::get_collection_freq() const
{
    LOGCALL(DB, Xapian::termcount, "GlassAllTermsList::get_collection_freq", NO_ARGS);
    Assert(!current_term.empty());
    Assert(!at_end());
    if (termfreq == 0) read_termfreq_and_collfreq();
    RETURN(collfreq);
}

TermList *
GlassAllTermsList::next()
{
    LOGCALL(DB, TermList *, "GlassAllTermsList::next", NO_ARGS);
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
	    const string & key = pack_glass_postlist_key(prefix);
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
	cursor->to_end();
	current_term.resize(0);
    }

    RETURN(NULL);
}

TermList *
GlassAllTermsList::skip_to(const string &term)
{
    LOGCALL(DB, TermList *, "GlassAllTermsList::skip_to", term);
    Assert(!at_end());
    // Set termfreq to 0 to indicate no termfreq/collfreq have been read for
    // the current term.
    termfreq = 0;

    if (rare(!cursor)) {
	cursor = database->postlist_table.cursor_get();
	Assert(cursor); // The postlist table isn't optional.
    }

    string key = pack_glass_postlist_key(term);
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
GlassAllTermsList::at_end() const
{
    LOGCALL(DB, bool, "GlassAllTermsList::at_end", NO_ARGS);
    RETURN(cursor && cursor->after_end());
}
