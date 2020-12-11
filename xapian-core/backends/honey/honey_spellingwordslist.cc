/** @file
 * @brief Iterator for the spelling correction words in a honey database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2017,2018 Olly Betts
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

#include "honey_spellingwordslist.h"

#include "xapian/error.h"
#include "xapian/types.h"

#include "debuglog.h"
#include "honey_database.h"
#include "pack.h"
#include "stringutils.h"

using namespace std;

HoneySpellingWordsList::~HoneySpellingWordsList()
{
    LOGCALL_DTOR(DB, "HoneySpellingWordsList");
    delete cursor;
}

Xapian::termcount
HoneySpellingWordsList::get_approx_size() const
{
    // This is an over-estimate, but we only use this value to build a balanced
    // or-tree, and it'll do a decent enough job for that.
    return database->spelling_table.get_approx_entry_count();
}

string
HoneySpellingWordsList::get_termname() const
{
    LOGCALL(DB, string, "HoneySpellingWordsList::get_termname", NO_ARGS);
    Assert(cursor);
    Assert(!cursor->after_end());
    const string& key = cursor->current_key;
    Assert(!key.empty());
    unsigned char first = key[0];
    AssertRel(first, >=, Honey::KEY_PREFIX_WORD);
    RETURN(first > Honey::KEY_PREFIX_WORD ? key : key.substr(1));
}

Xapian::doccount
HoneySpellingWordsList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "HoneySpellingWordsList::get_termfreq", NO_ARGS);
    Assert(cursor);
    Assert(!cursor->after_end());
    Assert(!cursor->current_key.empty());
    AssertRel(static_cast<unsigned char>(cursor->current_key[0]), >=,
	      Honey::KEY_PREFIX_WORD);
    cursor->read_tag();

    Xapian::termcount freq;
    const char* p = cursor->current_tag.data();
    if (!unpack_uint_last(&p, p + cursor->current_tag.size(), &freq)) {
	throw Xapian::DatabaseCorruptError("Bad spelling word freq");
    }
    RETURN(freq);
}

TermList*
HoneySpellingWordsList::next()
{
    LOGCALL(DB, TermList*, "HoneySpellingWordsList::next", NO_ARGS);
    Assert(cursor);

    if (cursor->after_end()) {
	// This is the first action on a new HoneySpellingWordsList.
	(void)cursor->find_entry_ge(string(1, char(Honey::KEY_PREFIX_WORD)));
    } else {
	cursor->next();
    }
    if (cursor->after_end()) {
	// We've reached the end of the prefixed terms.
	delete cursor;
	cursor = NULL;
    }

    RETURN(NULL);
}

TermList*
HoneySpellingWordsList::skip_to(const string& term)
{
    LOGCALL(DB, TermList*, "HoneySpellingWordsList::skip_to", term);
    Assert(cursor);

    if (!cursor->find_entry_ge(Honey::make_spelling_wordlist_key(term))) {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has a W prefix.
	if (cursor->after_end()) {
	    // We've reached the end of the prefixed terms.
	    delete cursor;
	    cursor = NULL;
	}
    }
    RETURN(NULL);
}

bool
HoneySpellingWordsList::at_end() const
{
    LOGCALL(DB, bool, "HoneySpellingWordsList::at_end", NO_ARGS);
    RETURN(cursor == NULL);
}
