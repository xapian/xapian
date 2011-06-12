/** @file flint_spellingwordslist.cc
 * @brief Iterator for the spelling correction words in a flint database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008 Olly Betts
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
#include "flint_spellingwordslist.h"

#include "xapian/error.h"
#include "xapian/types.h"

#include "debuglog.h"
#include "flint_utils.h"
#include "stringutils.h"

FlintSpellingWordsList::~FlintSpellingWordsList()
{
    LOGCALL_DTOR(DB, "~FlintSpellingWordsList");
    delete cursor;
}

string
FlintSpellingWordsList::get_termname() const
{
    LOGCALL(DB, string, "FlintSpellingWordsList::get_termname", NO_ARGS);
    Assert(cursor);
    Assert(!at_end());
    Assert(!cursor->current_key.empty());
    Assert(cursor->current_key[0] == 'W');
    RETURN(cursor->current_key.substr(1));
}

Xapian::doccount
FlintSpellingWordsList::get_termfreq() const
{
    LOGCALL(DB, string, "FlintSpellingWordsList::get_termfreq", NO_ARGS);
    Assert(cursor);
    Assert(!at_end());
    Assert(!cursor->current_key.empty());
    Assert(cursor->current_key[0] == 'W');
    cursor->read_tag();

    Xapian::termcount freq;
    const char *p = cursor->current_tag.data();
    if (!F_unpack_uint_last(&p, p + cursor->current_tag.size(), &freq)) {
	throw Xapian::DatabaseCorruptError("Bad spelling word freq");
    }
    return freq;
}

Xapian::termcount
FlintSpellingWordsList::get_collection_freq() const
{
    throw Xapian::InvalidOperationError("FlintSpellingWordsList::get_collection_freq() not meaningful");
}

TermList *
FlintSpellingWordsList::next()
{
    LOGCALL(DB, TermList *, "FlintSpellingWordsList::next", NO_ARGS);
    Assert(!at_end());

    cursor->next();
    if (!cursor->after_end() && !startswith(cursor->current_key, 'W')) {
	// We've reached the end of the prefixed terms.
	cursor->to_end();
    }

    RETURN(NULL);
}

TermList *
FlintSpellingWordsList::skip_to(const string &tname)
{
    LOGCALL(DB, TermList *, "FlintSpellingWordsList::skip_to", tname);
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
FlintSpellingWordsList::at_end() const
{
    LOGCALL(DB, bool, "FlintSpellingWordsList::at_end", NO_ARGS);
    RETURN(cursor->after_end());
}
