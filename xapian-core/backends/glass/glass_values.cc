/** @file glass_values.cc
 * @brief GlassValueManager class
 */
/* Copyright (C) 2008,2009,2010,2011,2012,2016 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
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

#include "glass_values.h"

#include "glass_cursor.h"
#include "glass_postlist.h"
#include "glass_termlist.h"
#include "debuglog.h"
#include "backends/document.h"
#include "pack.h"

#include "xapian/error.h"
#include "xapian/valueiterator.h"

#include <algorithm>
#include "autoptr.h"

using namespace Glass;
using namespace std;

// FIXME:
//  * put the "used slots" entry in the same termlist tag as the terms?
//  * multi-values?
//  * values named instead of numbered?

/** Generate a key for the "used slots" data. */
inline string
make_slot_key(Xapian::docid did)
{
    LOGCALL_STATIC(DB, string, "make_slot_key", did);
    // Add an extra character so that it can't clash with a termlist entry key
    // and will sort just after the corresponding termlist entry key.
    // FIXME: should we store this in the *same entry* as the list of terms?
    string key;
    pack_uint_preserving_sort(key, did);
    key += '\0';
    RETURN(key);
}

/** Generate a key for a value statistics item. */
inline string
make_valuestats_key(Xapian::valueno slot)
{
    LOGCALL_STATIC(DB, string, "make_valuestats_key", slot);
    string key("\0\xd0", 2);
    pack_uint_last(key, slot);
    RETURN(key);
}

void
ValueChunkReader::assign(const char * p_, size_t len, Xapian::docid did_)
{
    p = p_;
    end = p_ + len;
    did = did_;
    if (!unpack_string(&p, end, value))
	throw Xapian::DatabaseCorruptError("Failed to unpack first value");
}

void
ValueChunkReader::next()
{
    if (p == end) {
	p = NULL;
	return;
    }

    Xapian::docid delta;
    if (!unpack_uint(&p, end, &delta))
	throw Xapian::DatabaseCorruptError("Failed to unpack streamed value docid");
    did += delta + 1;
    if (!unpack_string(&p, end, value))
	throw Xapian::DatabaseCorruptError("Failed to unpack streamed value");
}

void
ValueChunkReader::skip_to(Xapian::docid target)
{
    if (p == NULL || target <= did)
	return;

    size_t value_len;
    while (p != end) {
	// Get the next docid
	Xapian::docid delta;
	if (rare(!unpack_uint(&p, end, &delta)))
	    throw Xapian::DatabaseCorruptError("Failed to unpack streamed value docid");
	did += delta + 1;

	// Get the length of the string
	if (rare(!unpack_uint(&p, end, &value_len))) {
	    throw Xapian::DatabaseCorruptError("Failed to unpack streamed value length");
	}

	// Check that it's not too long
	if (rare(value_len > size_t(end - p))) {
	    throw Xapian::DatabaseCorruptError("Failed to unpack streamed value");
	}

	// Assign the value and return only if we've reached the target
	if (did >= target) {
	    value.assign(p, value_len);
	    p += value_len;
	    return;
	}
	p += value_len;
    }
    p = NULL;
}

void
GlassValueManager::add_value(Xapian::docid did, Xapian::valueno slot,
			     const string & val)
{
    map<Xapian::valueno, map<Xapian::docid, string> >::iterator i;
    i = changes.find(slot);
    if (i == changes.end()) {
	i = changes.insert(make_pair(slot, map<Xapian::docid, string>())).first;
    }
    i->second[did] = val;
}

void
GlassValueManager::remove_value(Xapian::docid did, Xapian::valueno slot)
{
    map<Xapian::valueno, map<Xapian::docid, string> >::iterator i;
    i = changes.find(slot);
    if (i == changes.end()) {
	i = changes.insert(make_pair(slot, map<Xapian::docid, string>())).first;
    }
    i->second[did] = string();
}

Xapian::docid
GlassValueManager::get_chunk_containing_did(Xapian::valueno slot,
					    Xapian::docid did,
					    string &chunk) const
{
    LOGCALL(DB, Xapian::docid, "GlassValueManager::get_chunk_containing_did", slot | did | chunk);
    if (!cursor.get())
	cursor.reset(postlist_table->cursor_get());
    if (!cursor.get()) RETURN(0);

    bool exact = cursor->find_entry(make_valuechunk_key(slot, did));
    if (!exact) {
	// If we didn't find a chunk starting with docid did, then we need
	// to check if the chunk contains did.
	const char * p = cursor->current_key.data();
	const char * end = p + cursor->current_key.size();

	// Check that it is a value stream chunk.
	if (end - p < 2 || *p++ != '\0' || *p++ != '\xd8') RETURN(0);

	// Check that it's for the right value slot.
	Xapian::valueno v;
	if (!unpack_uint(&p, end, &v)) {
	    throw Xapian::DatabaseCorruptError("Bad value key");
	}
	if (v != slot) RETURN(0);

	// And get the first docid for the chunk so we can return it.
	if (!unpack_uint_preserving_sort(&p, end, &did) || p != end) {
	    throw Xapian::DatabaseCorruptError("Bad value key");
	}
    }

    cursor->read_tag();
    swap(chunk, cursor->current_tag);

    RETURN(did);
}

static const size_t CHUNK_SIZE_THRESHOLD = 2000;

namespace Glass {

class ValueUpdater {
    GlassPostListTable * table;

    Xapian::valueno slot;

    string ctag;

    ValueChunkReader reader;

    string tag;

    Xapian::docid prev_did;

    Xapian::docid first_did;

    Xapian::docid new_first_did;

    Xapian::docid last_allowed_did;

    void append_to_stream(Xapian::docid did, const string & value) {
	Assert(did);
	if (tag.empty()) {
	    new_first_did = did;
	} else {
	    AssertRel(did,>,prev_did);
	    pack_uint(tag, did - prev_did - 1);
	}
	prev_did = did;
	pack_string(tag, value);
	if (tag.size() >= CHUNK_SIZE_THRESHOLD) write_tag();
    }

    void write_tag() {
	// If the first docid has changed, delete the old entry.
	if (first_did && new_first_did != first_did) {
	    table->del(make_valuechunk_key(slot, first_did));
	}
	if (!tag.empty()) {
	    table->add(make_valuechunk_key(slot, new_first_did), tag);
	}
	first_did = 0;
	tag.resize(0);
    }

  public:
    ValueUpdater(GlassPostListTable * table_, Xapian::valueno slot_)
	: table(table_), slot(slot_), first_did(0), last_allowed_did(0) { }

    ~ValueUpdater() {
	while (!reader.at_end()) {
	    // FIXME: use skip_to and some splicing magic instead?
	    append_to_stream(reader.get_docid(), reader.get_value());
	    reader.next();
	}
	write_tag();
    }

    void update(Xapian::docid did, const string & value) {
	if (last_allowed_did && did > last_allowed_did) {
	    // The next change needs to go in a later existing chunk than the
	    // one we're currently updating, so we copy over the rest of the
	    // entries from the current chunk, write out the updated chunk and
	    // drop through to the case below will read in that later chunk.
	    // FIXME: use some string splicing magic instead of this loop.
	    while (!reader.at_end()) {
		// last_allowed_did should be an upper bound for this chunk.
		AssertRel(reader.get_docid(),<=,last_allowed_did);
		append_to_stream(reader.get_docid(), reader.get_value());
		reader.next();
	    }
	    write_tag();
	    last_allowed_did = 0;
	}
	if (last_allowed_did == 0) {
	    last_allowed_did = GLASS_MAX_DOCID;
	    Assert(tag.empty());
	    new_first_did = 0;
	    AutoPtr<GlassCursor> cursor(table->cursor_get());
	    if (cursor->find_entry(make_valuechunk_key(slot, did))) {
		// We found an exact match, so the first docid is the one
		// we looked for.
		first_did = did;
	    } else {
		Assert(!cursor->after_end());
		// Otherwise we need to unpack it from the key we found.
		// We may have found a non-value-chunk entry in which case
		// docid_from_key() returns 0.
		first_did = docid_from_key(slot, cursor->current_key);
	    }

	    // If there are no further chunks, then the last docid that can go
	    // in this chunk is the highest valid docid.  If there are further
	    // chunks then it's one less than the first docid of the next
	    // chunk.
	    if (first_did) {
		// We found a value chunk.
		cursor->read_tag();
		// FIXME:swap(cursor->current_tag, ctag);
		ctag = cursor->current_tag;
		reader.assign(ctag.data(), ctag.size(), first_did);
	    }
	    if (cursor->next()) {
		const string & key = cursor->current_key;
		Xapian::docid next_first_did = docid_from_key(slot, key);
		if (next_first_did) last_allowed_did = next_first_did - 1;
		Assert(last_allowed_did);
		AssertRel(last_allowed_did,>=,first_did);
	    }
	}

	// Copy over entries until we get to the one we want to
	// add/modify/delete.
	// FIXME: use skip_to and some splicing magic instead?
	while (!reader.at_end() && reader.get_docid() < did) {
	    append_to_stream(reader.get_docid(), reader.get_value());
	    reader.next();
	}
	if (!reader.at_end() && reader.get_docid() == did) reader.next();
	if (!value.empty()) {
	    // Add/update entry for did.
	    append_to_stream(did, value);
	}
    }
};

}

void
GlassValueManager::merge_changes()
{
    if (termlist_table->is_open()) {
	map<Xapian::docid, string>::const_iterator i;
	for (i = slots.begin(); i != slots.end(); ++i) {
	    const string & enc = i->second;
	    string key = make_slot_key(i->first);
	    if (!enc.empty()) {
		termlist_table->add(key, i->second);
	    } else {
		termlist_table->del(key);
	    }
	}
	slots.clear();
    }

    {
	map<Xapian::valueno, map<Xapian::docid, string> >::const_iterator i;
	for (i = changes.begin(); i != changes.end(); ++i) {
	    Xapian::valueno slot = i->first;
	    Glass::ValueUpdater updater(postlist_table, slot);
	    const map<Xapian::docid, string> & slot_changes = i->second;
	    map<Xapian::docid, string>::const_iterator j;
	    for (j = slot_changes.begin(); j != slot_changes.end(); ++j) {
		updater.update(j->first, j->second);
	    }
	}
	changes.clear();
    }
}

void
GlassValueManager::add_document(Xapian::docid did, const Xapian::Document &doc,
				map<Xapian::valueno, ValueStats> & value_stats)
{
    // FIXME: Use BitWriter and interpolative coding?  Or is it not worthwhile
    // for this?
    string slots_used;
    Xapian::valueno prev_slot = static_cast<Xapian::valueno>(-1);
    Xapian::ValueIterator it = doc.values_begin();
    while (it != doc.values_end()) {
	Xapian::valueno slot = it.get_valueno();
	string value = *it;

	// Update the statistics.
	std::pair<map<Xapian::valueno, ValueStats>::iterator, bool> i;
	i = value_stats.insert(make_pair(slot, ValueStats()));
	ValueStats & stats = i.first->second;
	if (i.second) {
	    // There were no statistics stored already, so read them.
	    get_value_stats(slot, stats);
	}

	// Now, modify the stored statistics.
	if ((stats.freq)++ == 0) {
	    // If the value count was previously zero, set the upper and lower
	    // bounds to the newly added value.
	    stats.lower_bound = value;
	    stats.upper_bound = value;
	} else {
	    // Otherwise, simply make sure they reflect the new value.
	    if (value < stats.lower_bound) {
		stats.lower_bound = value;
	    } else if (value > stats.upper_bound) {
		stats.upper_bound = value;
	    }
	}

	add_value(did, slot, value);
	if (termlist_table->is_open()) {
	    pack_uint(slots_used, slot - prev_slot - 1);
	    prev_slot = slot;
	}
	++it;
    }
    if (slots_used.empty() && slots.find(did) == slots.end()) {
	// Adding a new document with no values which we didn't just remove.
    } else {
	swap(slots[did], slots_used);
    }
}

void
GlassValueManager::delete_document(Xapian::docid did,
				   map<Xapian::valueno, ValueStats> & value_stats)
{
    Assert(termlist_table->is_open());
    map<Xapian::docid, string>::iterator it = slots.find(did);
    string s;
    if (it != slots.end()) {
	swap(s, it->second);
    } else {
	// Get from table, making a swift exit if this document has no values.
	if (!termlist_table->get_exact_entry(make_slot_key(did), s)) return;
	slots.insert(make_pair(did, string()));
    }
    const char * p = s.data();
    const char * end = p + s.size();
    Xapian::valueno prev_slot = static_cast<Xapian::valueno>(-1);
    while (p != end) {
	Xapian::valueno slot;
	if (!unpack_uint(&p, end, &slot)) {
	    throw Xapian::DatabaseCorruptError("Value slot encoding corrupt");
	}
	slot += prev_slot + 1;
	prev_slot = slot;

	std::pair<map<Xapian::valueno, ValueStats>::iterator, bool> i;
	i = value_stats.insert(make_pair(slot, ValueStats()));
	ValueStats & stats = i.first->second;
	if (i.second) {
	    // There were no statistics stored already, so read them.
	    get_value_stats(slot, stats);
	}

	// Now, modify the stored statistics.
	AssertRelParanoid(stats.freq, >, 0);
	if (--(stats.freq) == 0) {
	    stats.lower_bound.resize(0);
	    stats.upper_bound.resize(0);
	}

	remove_value(did, slot);
    }
}

void
GlassValueManager::replace_document(Xapian::docid did,
				    const Xapian::Document &doc,
				    map<Xapian::valueno, ValueStats> & value_stats)
{
    if (doc.get_docid() == did) {
	// If we're replacing a document with itself, but the optimisation for
	// this higher up hasn't kicked in (e.g. because we've added/replaced
	// a document since this one was read) and the values haven't changed,
	// then the call to delete_document() below will remove the values
	// before the subsequent add_document() can read them.
	//
	// The simplest way to handle this is to force the document to read its
	// values, which we only need to do this is the docid matches.  Note that
	// this check can give false positives as we don't also check the
	// database, so for example replacing document 4 in one database with
	// document 4 from another will unnecessarily trigger this, but forcing
	// the values to be read is fairly harmless, and this is unlikely to be
	// a common case.
	doc.internal->need_values();
    }
    delete_document(did, value_stats);
    add_document(did, doc, value_stats);
}

string
GlassValueManager::get_value(Xapian::docid did, Xapian::valueno slot) const
{
    map<Xapian::valueno, map<Xapian::docid, string> >::const_iterator i;
    i = changes.find(slot);
    if (i != changes.end()) {
	map<Xapian::docid, string>::const_iterator j;
	j = i->second.find(did);
	if (j != i->second.end()) return j->second;
    }

    // Read it from the table.
    string chunk;
    Xapian::docid first_did;
    first_did = get_chunk_containing_did(slot, did, chunk);
    if (first_did == 0) return string();

    ValueChunkReader reader(chunk.data(), chunk.size(), first_did);
    reader.skip_to(did);
    if (reader.at_end() || reader.get_docid() != did) return string();
    return reader.get_value();
}

void
GlassValueManager::get_all_values(map<Xapian::valueno, string> & values,
				  Xapian::docid did) const
{
    Assert(values.empty());
    if (!termlist_table->is_open()) {
	// Either the database has been closed, or else there's no termlist table.
	// Check if the postlist table is open to determine which is the case.
	if (!postlist_table->is_open())
	    GlassTable::throw_database_closed();
	throw Xapian::FeatureUnavailableError("Database has no termlist");
    }
    map<Xapian::docid, string>::const_iterator i = slots.find(did);
    string s;
    if (i != slots.end()) {
	s = i->second;
    } else {
	// Get from table.
	if (!termlist_table->get_exact_entry(make_slot_key(did), s)) return;
    }
    const char * p = s.data();
    const char * end = p + s.size();
    Xapian::valueno prev_slot = static_cast<Xapian::valueno>(-1);
    while (p != end) {
	Xapian::valueno slot;
	if (!unpack_uint(&p, end, &slot)) {
	    throw Xapian::DatabaseCorruptError("Value slot encoding corrupt");
	}
	slot += prev_slot + 1;
	prev_slot = slot;
	values.insert(make_pair(slot, get_value(did, slot)));
    }
}

void
GlassValueManager::get_value_stats(Xapian::valueno slot) const
{
    LOGCALL_VOID(DB, "GlassValueManager::get_value_stats", slot);
    // Invalidate the cache first in case an exception is thrown.
    mru_slot = Xapian::BAD_VALUENO;
    get_value_stats(slot, mru_valstats);
    mru_slot = slot;
}

void
GlassValueManager::get_value_stats(Xapian::valueno slot, ValueStats & stats) const
{
    LOGCALL_VOID(DB, "GlassValueManager::get_value_stats", slot | Literal("[stats]"));
    // Invalidate the cache first in case an exception is thrown.
    mru_slot = Xapian::BAD_VALUENO;

    string tag;
    if (postlist_table->get_exact_entry(make_valuestats_key(slot), tag)) {
	const char * pos = tag.data();
	const char * end = pos + tag.size();

	if (!unpack_uint(&pos, end, &(stats.freq))) {
	    if (pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Frequency statistic in value table is too large");
	}
	if (!unpack_string(&pos, end, stats.lower_bound)) {
	    if (pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Lower bound in value table is too large");
	}
	size_t len = end - pos;
	if (len == 0) {
	    stats.upper_bound = stats.lower_bound;
	} else {
	    stats.upper_bound.assign(pos, len);
	}
    } else {
	stats.clear();
    }

    mru_slot = slot;
}

void
GlassValueManager::set_value_stats(map<Xapian::valueno, ValueStats> & value_stats)
{
    LOGCALL_VOID(DB, "GlassValueManager::set_value_stats", value_stats);
    map<Xapian::valueno, ValueStats>::const_iterator i;
    for (i = value_stats.begin(); i != value_stats.end(); ++i) {
	string key = make_valuestats_key(i->first);
	const ValueStats & stats = i->second;
	if (stats.freq != 0) {
	    string new_value;
	    pack_uint(new_value, stats.freq);
	    pack_string(new_value, stats.lower_bound);
	    // We don't store or count empty values, so neither of the bounds
	    // can be empty.  So we can safely store an empty upper bound when
	    // the bounds are equal.
	    if (stats.lower_bound != stats.upper_bound)
		new_value += stats.upper_bound;
	    postlist_table->add(key, new_value);
	} else {
	    postlist_table->del(key);
	}
    }
    value_stats.clear();
    mru_slot = Xapian::BAD_VALUENO;
}
