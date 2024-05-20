/** @file
 * @brief Iterator for the spelling correction words in a glass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2017,2024 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#include "glass_spellingwordslist.h"

#include "xapian/error.h"
#include "xapian/types.h"

#include "debuglog.h"
#include "glass_database.h"
#include "pack.h"
#include "stringutils.h"

using namespace std;

GlassSpellingWordsList::~GlassSpellingWordsList()
{
    LOGCALL_DTOR(DB, "GlassSpellingWordsList");
    delete cursor;
}

Xapian::termcount
GlassSpellingWordsList::get_approx_size() const
{
    // This is an over-estimate, but we only use this value to build a balanced
    // or-tree, and it'll do a decent enough job for that.
    return database->spelling_table.get_entry_count();
}

Xapian::doccount
GlassSpellingWordsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "GlassSpellingWordsList::get_termfreq", NO_ARGS);
    Assert(cursor);
    Assert(!cursor->after_end());
    Assert(!cursor->current_key.empty());
    Assert(cursor->current_key[0] == 'W');
    cursor->read_tag();

    Xapian::termcount freq;
    const char *p = cursor->current_tag.data();
    if (!unpack_uint_last(&p, p + cursor->current_tag.size(), &freq)) {
	throw Xapian::DatabaseCorruptError("Bad spelling word freq");
    }
    RETURN(freq);
}

TermList *
GlassSpellingWordsList::next()
{
    LOGCALL(DB, TermList *, "GlassSpellingWordsList::next", NO_ARGS);
    Assert(!cursor->after_end());

    if (!cursor->next() || cursor->current_key[0] != 'W') {
	// We've reached the end of the prefixed terms.
	RETURN(this);
    }
    current_term.assign(cursor->current_key, 1);
    RETURN(NULL);
}

TermList*
GlassSpellingWordsList::skip_to(string_view tname)
{
    LOGCALL(DB, TermList *, "GlassSpellingWordsList::skip_to", tname);
    Assert(!cursor->after_end());

    if (cursor->find_entry_ge("W"s.append(tname))) {
	// Exact match.
	current_term = tname;
    } else {
	// The exact term we asked for isn't there, so check if the next term
	// after it also has a W prefix.
	if (cursor->after_end() || cursor->current_key[0] != 'W') {
	    // We've reached the end of the prefixed terms.
	    RETURN(this);
	}
	current_term.assign(cursor->current_key, 1);
    }
    RETURN(NULL);
}
