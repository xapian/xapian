/** @file glass_spellingwordslist.cc
 * @brief Iterator for the spelling correction words in a glass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009 Olly Betts
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
#include "pack.h"
#include "stringutils.h"

GlassSpellingWordsList::~GlassSpellingWordsList()
{
    LOGCALL_DTOR(DB, "GlassSpellingWordsList");
    delete cursor;
}

string
GlassSpellingWordsList::get_termname() const
{
    LOGCALL(DB, string, "GlassSpellingWordsList::get_termname", NO_ARGS);
    Assert(cursor);
    Assert(!at_end());
    Assert(!cursor->current_key.empty());
    Assert(cursor->current_key[0] == 'W');
    RETURN(cursor->current_key.substr(1));
}

Xapian::doccount
GlassSpellingWordsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "GlassSpellingWordsList::get_termfreq", NO_ARGS);
    Assert(cursor);
    Assert(!at_end());
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

Xapian::termcount
GlassSpellingWordsList::get_collection_freq() const
{
    throw Xapian::InvalidOperationError("GlassSpellingWordsList::get_collection_freq() not meaningful");
}

TermList *
GlassSpellingWordsList::next()
{
    LOGCALL(DB, TermList *, "GlassSpellingWordsList::next", NO_ARGS);
    Assert(!at_end());

    cursor->next();
    if (!cursor->after_end() && !startswith(cursor->current_key, 'W')) {
	// We've reached the end of the prefixed terms.
	cursor->to_end();
    }

    RETURN(NULL);
}

TermList *
GlassSpellingWordsList::skip_to(const string &tname)
{
    LOGCALL(DB, TermList *, "GlassSpellingWordsList::skip_to", tname);
    Assert(!at_end());

    if (!cursor->find_entry_ge("W" + tname)) {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has a W prefix.
	if (!cursor->after_end() && !startswith(cursor->current_key, 'W')) {
	    // We've reached the end of the prefixed terms.
	    cursor->to_end();
	}
    }
    RETURN(NULL);
}

bool
GlassSpellingWordsList::at_end() const
{
    LOGCALL(DB, bool, "GlassSpellingWordsList::at_end", NO_ARGS);
    RETURN(cursor->after_end());
}
