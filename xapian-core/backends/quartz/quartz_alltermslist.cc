/* quartzalltermslist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "quartz_alltermslist.h"
#include "quartz_utils.h"
#include "quartz_postlist.h"

QuartzAllTermsList::QuartzAllTermsList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
				       AutoPtr<Bcursor> pl_cursor_,
				       quartz_tablesize_t size_,
				       const string & prefix_)
	: database(database_), pl_cursor(pl_cursor_), size(size_), 
	  started(false), prefix(prefix_)
{
    DEBUGCALL(DB, void, "QuartzAllTermsList", "[database_], [pl_cursor_]");
    if (prefix.empty()) {
	/* Seek to the first term */
	pl_cursor->find_entry(string());

	if (pl_cursor->current_key.empty()) {
	    pl_cursor->next();
	}
    } else {
	// Seek to the first key which is at or after the desired prefix.
	if (!pl_cursor->find_entry(pack_string_preserving_sort(prefix))) {
	    // Found the last key which is before the prefix - advance to the
	    // first one following the prefix.
	    pl_cursor->next();
	}
    }

    is_at_end = pl_cursor->after_end();
    if (!is_at_end) {
	const char *start = pl_cursor->current_key.data();
	const char *end = start + pl_cursor->current_key.length();
	if (!unpack_string_preserving_sort(&start, end, current_term)) {
	    throw Xapian::DatabaseCorruptError("Failed to read the key field from a Bcursor's key");
	}
    }

    if (current_term.substr(0, prefix.size()) != prefix)
	is_at_end = true;

    have_stats = false;
}

QuartzAllTermsList::~QuartzAllTermsList()
{
    DEBUGCALL(DB, void, "~QuartzAllTermsList", "");
}

Xapian::termcount
QuartzAllTermsList::get_approx_size() const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzAllTermsList::get_approx_size", "");
    RETURN(size);
}

string
QuartzAllTermsList::get_termname() const
{
    DEBUGCALL(DB, string, "QuartzAllTermsList::get_termname", "");
    Assert(started);
    RETURN(current_term);
}

void QuartzAllTermsList::get_stats() const
{
    pl_cursor->read_tag();
    const char *start = pl_cursor->current_tag.data();
    const char *end = start + pl_cursor->current_tag.length();
    QuartzPostList::read_number_of_entries(&start, end,
					   &termfreq, &collection_freq);

    have_stats = true;
}

Xapian::doccount
QuartzAllTermsList::get_termfreq() const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzAllTermsList::get_termfreq", "");
    Assert(started);
    if (have_stats) {
	RETURN(termfreq);
    } else if (!is_at_end) {
	get_stats();
	RETURN(termfreq);
    }
    throw Xapian::InvalidArgumentError("Attempt to get termfreq after end");
}

Xapian::termcount
QuartzAllTermsList::get_collection_freq() const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzAllTermsList::get_collection_freq", "");
    Assert(started);
    if (have_stats) {
	RETURN(collection_freq);
    } else if (!is_at_end) {
	get_stats();
	RETURN(collection_freq);
    }
    throw Xapian::InvalidArgumentError("Attempt to get collection_freq after end");
}

TermList *
QuartzAllTermsList::skip_to(const string &tname)
{
    DEBUGCALL(DB, TermList *, "QuartzAllTermsList::skip_to", tname);
    DEBUGLINE(DB, "QuartzAllTermList::skip_to(" << tname << ")");
    started = true;
    string key;
    key = pack_string_preserving_sort(tname);

    have_stats = false;

    if (!pl_cursor->find_entry(key)) {
	if (pl_cursor->after_end()) {
	    is_at_end = true;
	} else {
	    next();
	    // next() checks the prefix, so we don't need to do that here.
	}
    } else {
	// This assertion isn't true if key contains zero bytes.
	// Assert(key == pl_cursor->current_key);
	current_term = tname;

	// Check that we haven't gone past the prefix.
	if (current_term.substr(0, prefix.size()) != prefix)
	    is_at_end = true;
    }
    RETURN(NULL);
}

TermList *
QuartzAllTermsList::next()
{
    DEBUGCALL(DB, TermList *, "QuartzAllTermsList::next", "");
    if (!started) {
	started = true;
    } else {
	while (true) {
	    pl_cursor->next();

	    is_at_end = pl_cursor->after_end();

	    if (is_at_end) break;

	    const char *start = pl_cursor->current_key.data();
	    const char *end = start + pl_cursor->current_key.length();
	    if (!unpack_string_preserving_sort(&start, end, current_term)) {
		throw Xapian::DatabaseCorruptError("Failed to read the key field from a Bcursor's key");
	    }
	    if (current_term.substr(0, prefix.size()) != prefix) {
		is_at_end = true;
		break;
	    }
	    // Check if this is the first chunk of a postlist, skip otherwise
	    if (start == end) break;
	}

	have_stats = false;
    }
    RETURN(NULL);
}

bool
QuartzAllTermsList::at_end() const
{
    DEBUGCALL(DB, bool, "QuartzAllTermsList::at_end", "");
    RETURN(is_at_end);
}
