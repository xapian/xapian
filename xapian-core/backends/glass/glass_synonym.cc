/** @file
 * @brief Synonym data for a glass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2011,2017,2024 Olly Betts
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
#include "glass_synonym.h"

#include "xapian/error.h"

#include "glass_cursor.h"
#include "glass_database.h"
#include "debuglog.h"
#include "stringutils.h"
#include "api/vectortermlist.h"

#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

// We XOR the length values with this so that they are more likely to coincide
// with lower case ASCII letters, which are likely to be common.  This means
// that zlib should do a better job of compressing tag values.
#define MAGIC_XOR_VALUE 96

void
GlassSynonymTable::merge_changes()
{
    if (last_term.empty()) return;

    if (last_synonyms.empty()) {
	del(last_term);
    } else {
	string tag;
	for (const auto& synonym : last_synonyms) {
	    tag += uint8_t(synonym.size() ^ MAGIC_XOR_VALUE);
	    tag += synonym;
	}
	add(last_term, tag);
	last_synonyms.clear();
    }
    last_term.resize(0);
}

void
GlassSynonymTable::add_synonym(string_view term, string_view synonym)
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
		    (len = uint8_t(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		    throw Xapian::DatabaseCorruptError("Bad synonym data");
		++p;
		last_synonyms.insert(string(p, len));
		p += len;
	    }
	}
    }

    last_synonyms.emplace(synonym);
}

void
GlassSynonymTable::remove_synonym(string_view term, string_view synonym)
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
		    (len = uint8_t(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		    throw Xapian::DatabaseCorruptError("Bad synonym data");
		++p;
		last_synonyms.emplace(p, len);
		p += len;
	    }
	}
    }

#ifdef __cpp_lib_associative_heterogeneous_erasure // C++23
    last_synonyms.erase(synonym);
#else
    last_synonyms.erase(string(synonym));
#endif
}

void
GlassSynonymTable::clear_synonyms(string_view term)
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

TermList*
GlassSynonymTable::open_termlist(string_view term)
{
    vector<string> synonyms;

    if (last_term == term) {
	if (last_synonyms.empty()) return NULL;

	synonyms.reserve(last_synonyms.size());
	for (const auto& i : last_synonyms) {
	    synonyms.push_back(i);
	}
    } else {
	string tag;
	if (!get_exact_entry(term, tag)) return NULL;

	const char * p = tag.data();
	const char * end = p + tag.size();
	while (p != end) {
	    size_t len;
	    if (p == end ||
		(len = uint8_t(*p) ^ MAGIC_XOR_VALUE) >= size_t(end - p))
		throw Xapian::DatabaseCorruptError("Bad synonym data");
	    ++p;
	    synonyms.push_back(string(p, len));
	    p += len;
	}
    }

    return new VectorTermList(synonyms.begin(), synonyms.end());
}

///////////////////////////////////////////////////////////////////////////

GlassSynonymTermList::~GlassSynonymTermList()
{
    LOGCALL_DTOR(DB, "GlassSynonymTermList");
    delete cursor;
}

Xapian::termcount
GlassSynonymTermList::get_approx_size() const
{
    // This is an over-estimate, but we only use this value to build a balanced
    // or-tree, and it'll do a decent enough job for that.
    return database->synonym_table.get_entry_count();
}

Xapian::doccount
GlassSynonymTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("GlassSynonymTermList::get_termfreq() not meaningful");
}

TermList *
GlassSynonymTermList::next()
{
    LOGCALL(DB, TermList *, "GlassSynonymTermList::next", NO_ARGS);
    Assert(!cursor->after_end());

    if (!cursor->next() || !startswith(cursor->current_key, prefix)) {
	// We've reached the end of the prefixed terms.
	RETURN(this);
    }
    current_term = cursor->current_key;

    RETURN(NULL);
}

TermList*
GlassSynonymTermList::skip_to(string_view tname)
{
    LOGCALL(DB, TermList *, "GlassSynonymTermList::skip_to", tname);
    Assert(!cursor->after_end());

    if (cursor->find_entry_ge(tname)) {
	// Exact match.
	current_term = tname;
    } else {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has the right prefix.
	if (cursor->after_end() || !startswith(cursor->current_key, prefix)) {
	    // We've reached the end of the prefixed terms.
	    RETURN(this);
	}
	current_term = cursor->current_key;
    }
    RETURN(NULL);
}
