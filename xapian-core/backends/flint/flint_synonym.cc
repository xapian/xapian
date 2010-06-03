/** @file flint_synonym.cc
 * @brief Synonym data for a flint database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009 Olly Betts
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
#include "flint_synonym.h"

#include "xapian/error.h"

#include "debuglog.h"
#include "flint_cursor.h"
#include "flint_utils.h"
#include "stringutils.h"
#include "vectortermlist.h"

#include <set>
#include <string>
#include <vector>

using namespace std;

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values.
#define MAGIC_XOR_VALUE 96

void
FlintSynonymTable::merge_changes()
{
    if (last_term.empty()) return;

    if (last_synonyms.empty()) {
	del(last_term);
    } else {
	string tag;

	set<string>::const_iterator i;
	for (i = last_synonyms.begin(); i != last_synonyms.end(); ++i) {
	    const string & synonym = *i;
	    tag += byte(synonym.size() ^ MAGIC_XOR_VALUE);
	    tag += synonym;
	}

	add(last_term, tag);
	last_synonyms.clear();
    }
    last_term.resize(0);
}

void
FlintSynonymTable::add_synonym(const string & term, const string & synonym)
{
    if (last_term != term) {
	merge_changes();
	last_term = term;

	string tag;
	if (get_exact_entry(term, tag)) {
	    const char * p = tag.data();
	    const char * end = p + tag.size();
	    while (p != end) {
		size_t len;
		if (p == end ||
		    (len = byte(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		    throw Xapian::DatabaseCorruptError("Bad synonym data");
		++p;
		last_synonyms.insert(string(p, len));
		p += len;
	    }
	}
    }

    last_synonyms.insert(synonym);
}

void
FlintSynonymTable::remove_synonym(const string & term, const string & synonym)
{
    if (last_term != term) {
	merge_changes();
	last_term = term;

	string tag;
	if (get_exact_entry(term, tag)) {
	    const char * p = tag.data();
	    const char * end = p + tag.size();
	    while (p != end) {
		size_t len;
		if (p == end ||
		    (len = byte(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		    throw Xapian::DatabaseCorruptError("Bad synonym data");
		++p;
		last_synonyms.insert(string(p, len));
		p += len;
	    }
	}
    }

    last_synonyms.erase(synonym);
}

void
FlintSynonymTable::clear_synonyms(const string & term)
{
    // We don't actually ever need to merge_changes() here, but it's quite
    // likely that someone might clear_synonyms() and then add_synonym() for
    // the same term.  The alternative we could otherwise optimise for (modify
    // synonyms for a term, then clear those for another, then modify those for
    // the first term again) seems much less likely.
    if (last_term == term) {
	last_synonyms.clear();
    } else {
	merge_changes();
	last_term = term;
    }
}

TermList *
FlintSynonymTable::open_termlist(const string & term)
{
    vector<string> synonyms;

    if (last_term == term) {
	if (last_synonyms.empty()) return NULL;

	synonyms.reserve(last_synonyms.size());
	set<string>::const_iterator i;
	for (i = last_synonyms.begin(); i != last_synonyms.end(); ++i) {
	    synonyms.push_back(*i);
	}
    } else {
	string tag;
	if (!get_exact_entry(term, tag)) return NULL;

	const char * p = tag.data();
	const char * end = p + tag.size();
	while (p != end) {
	    size_t len;
	    if (p == end ||
		(len = byte(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		throw Xapian::DatabaseCorruptError("Bad synonym data");
	    ++p;
	    synonyms.push_back(string(p, len));
	    p += len;
	}
    }

    return new VectorTermList(synonyms.begin(), synonyms.end());
}

///////////////////////////////////////////////////////////////////////////

FlintSynonymTermList::~FlintSynonymTermList()
{
    LOGCALL_DTOR(DB, "FlintSynonymTermList");
    delete cursor;
}

string
FlintSynonymTermList::get_termname() const
{
    LOGCALL(DB, string, "FlintSynonymTermList::get_termname", NO_ARGS);
    Assert(cursor);
    Assert(!cursor->current_key.empty());
    Assert(!at_end());
    RETURN(cursor->current_key);
}

Xapian::doccount
FlintSynonymTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("FlintSynonymTermList::get_termfreq() not meaningful");
}

Xapian::termcount
FlintSynonymTermList::get_collection_freq() const
{
    throw Xapian::InvalidOperationError("FlintSynonymTermList::get_collection_freq() not meaningful");
}

TermList *
FlintSynonymTermList::next()
{
    LOGCALL(DB, TermList *, "FlintSynonymTermList::next", NO_ARGS);
    Assert(!at_end());

    cursor->next();
    if (!cursor->after_end() && !startswith(cursor->current_key, prefix)) {
	// We've reached the end of the end of the prefixed terms.
	cursor->to_end();
    }

    RETURN(NULL);
}

TermList *
FlintSynonymTermList::skip_to(const string &tname)
{
    LOGCALL(DB, TermList *, "FlintSynonymTermList::skip_to", tname);
    Assert(!at_end());

    if (!cursor->find_entry_ge(tname)) {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has the right prefix.
	if (!cursor->after_end() && !startswith(cursor->current_key, prefix)) {
	    // We've reached the end of the prefixed terms.
	    cursor->to_end();
	}
    }
    RETURN(NULL);
}

bool
FlintSynonymTermList::at_end() const
{
    LOGCALL(DB, bool, "FlintSynonymTermList::at_end", NO_ARGS);
    RETURN(cursor->after_end());
}
