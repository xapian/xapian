/** @file honey_synonym.cc
 * @brief Synonym data for a honey database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2011,2017 Olly Betts
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
#include "honey_synonym.h"

#include "xapian/error.h"

#include "honey_cursor.h"
#include "honey_database.h"
#include "debuglog.h"
#include "stringutils.h"
#include "api/vectortermlist.h"

#include <set>
#include <string>
#include <vector>

using namespace std;

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values.
#define MAGIC_XOR_VALUE 96

void
HoneySynonymTable::merge_changes()
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
HoneySynonymTable::add_synonym(const string & term, const string & synonym)
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
HoneySynonymTable::remove_synonym(const string & term, const string & synonym)
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
HoneySynonymTable::clear_synonyms(const string & term)
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
HoneySynonymTable::open_termlist(const string & term) const
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

HoneySynonymTermList::~HoneySynonymTermList()
{
    LOGCALL_DTOR(DB, "HoneySynonymTermList");
    delete cursor;
}

Xapian::termcount
HoneySynonymTermList::get_approx_size() const
{
    // This is an over-estimate, but we only use this value to build a balanced
    // or-tree, and it'll do a decent enough job for that.
    return database->synonym_table.get_approx_entry_count();
}

string
HoneySynonymTermList::get_termname() const
{
    LOGCALL(DB, string, "HoneySynonymTermList::get_termname", NO_ARGS);
    Assert(cursor);
    Assert(!cursor->current_key.empty());
    Assert(!at_end());
    RETURN(cursor->current_key);
}

Xapian::doccount
HoneySynonymTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("HoneySynonymTermList::get_termfreq() "
					"not meaningful");
}

Xapian::termcount
HoneySynonymTermList::get_collection_freq() const
{
    throw Xapian::InvalidOperationError("HoneySynonymTermList::"
					"get_collection_freq() "
					"not meaningful");
}

TermList *
HoneySynonymTermList::next()
{
    LOGCALL(DB, TermList *, "HoneySynonymTermList::next", NO_ARGS);
    Assert(!at_end());

    if (cursor->after_end()) {
	// This is the first action on a new HoneySynonymTermList.
	if (cursor->find_entry_ge(prefix))
	    RETURN(NULL);
    } else {
	cursor->next();
    }
    if (cursor->after_end() || !startswith(cursor->current_key, prefix)) {
	// We've reached the end of the prefixed terms.
	delete cursor;
	cursor = NULL;
    }

    RETURN(NULL);
}

TermList *
HoneySynonymTermList::skip_to(const string &term)
{
    LOGCALL(DB, TermList *, "HoneySynonymTermList::skip_to", term);
    Assert(!at_end());

    if (cursor->after_end() && prefix > term) {
	// This is the first action on a new HoneySynonymTermList and we were
	// asked to skip to a term before the prefix - this ought to leave us
	// on the first term with the specified prefix.
	RETURN(skip_to(prefix));
    }

    if (!cursor->find_entry_ge(term)) {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has the right prefix.
	if (cursor->after_end() || !startswith(cursor->current_key, prefix)) {
	    // We've reached the end of the prefixed terms.
	    delete cursor;
	    cursor = NULL;
	}
    }
    RETURN(NULL);
}

bool
HoneySynonymTermList::at_end() const
{
    LOGCALL(DB, bool, "HoneySynonymTermList::at_end", NO_ARGS);
    RETURN(cursor == NULL);
}
