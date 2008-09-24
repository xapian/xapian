/** @file chert_values.cc
 * @brief ChertValueManager class
 */
/* Copyright (C) 2008 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#include "chert_values.h"

#include "chert_cursor.h"
#include "chert_postlist.h"
#include "chert_termlist.h"
#include "chert_utils.h"
#include "omdebug.h"

#include "xapian/error.h"
#include "xapian/valueiterator.h"

#include <algorithm>
#include "autoptr.h"

using namespace std;

// FIXME:
//  * put the "used slots" entry in the same termlist tag as the terms?
//  * multi-values?
//  * values named instead of numbered?

/** Generate a key for a value stream. */
inline string
make_key(Xapian::valueno slot, Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, string, "make_key", slot << ", " << did);
    RETURN(string("\0\xd8", 2) + pack_uint(slot) + pack_uint_preserving_sort(did));
}

inline Xapian::docid
docid_from_key(Xapian::valueno required_slot, const string & key)
{
    DEBUGCALL_STATIC(DB, Xapian::docid, "docid_from_key", required_slot << ", " << key);
    const char * p = key.data();
    const char * end = p + key.length();
    // Fail if not a value chunk key.
    if (end - p < 2 || *p++ != '\0' || *p++ != '\xd8') RETURN(0);
    Xapian::valueno slot;
    if (!unpack_uint(&p, end, &slot))
       	throw Xapian::DatabaseCorruptError("bad value key");
    // Fail if for a different slot.
    if (slot != required_slot) RETURN(0);
    Xapian::docid did;
    if (!unpack_uint_preserving_sort(&p, end, &did))
       	throw Xapian::DatabaseCorruptError("bad value key");
    RETURN(did);
}

/** Generate a key for the "used slots" data. */
inline string
make_slot_key(Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, string, "make_slot_key", did);
    // Add an extra character so that it can't clash with a termlist entry key
    // and will sort just after the corresponding termlist entry key.
    // FIXME: should we store this in the *same entry* as the list of terms?
    RETURN(chert_docid_to_key(did) + string(1, '\0'));
}

/** Generate a key for a value statistics item. */
inline string
make_valuestats_key(Xapian::valueno slot)
{
    DEBUGCALL_STATIC(DB, string, "make_valuestats_key", slot);
    RETURN(string("\0\xd0", 2) + pack_uint_last(slot));
}

class ValueChunkReader {
    const char *p;
    const char *end;

    Xapian::docid did;

    string value;

  public:
    ValueChunkReader() : p(NULL) { }

    ValueChunkReader(const char * p_, size_t len, Xapian::docid did_) {
	assign(p_, len, did_);
    }

    void assign(const char * p_, size_t len, Xapian::docid did_) {
	p = p_;
	end = p_ + len;
	did = did_;
	if (!unpack_string(&p, end, value))
	    throw Xapian::DatabaseCorruptError("Failed to unpack first value");
    }

    bool at_end() const { return p == NULL; }

    Xapian::docid get_docid() const { return did; }

    const string & get_value() const { return value; }

    void next() { 
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

    void skip_to(Xapian::docid target) {
	while (!at_end() && target > did) {
	    next();
	}
    }
};

void
ChertValueManager::add_value(Xapian::docid did, Xapian::valueno slot,
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
ChertValueManager::remove_value(Xapian::docid did, Xapian::valueno slot)
{
    map<Xapian::valueno, map<Xapian::docid, string> >::iterator i;
    i = changes.find(slot);
    if (i == changes.end()) {
	i = changes.insert(make_pair(slot, map<Xapian::docid, string>())).first;
    }
    i->second[did] = string();
}

Xapian::docid
ChertValueManager::get_chunk_containing_did(Xapian::valueno slot,
					    Xapian::docid did,
					    string &chunk) const
{
    DEBUGCALL(DB, Xapian::docid, "ChertValueManager::get_chunk_containing_did", slot << ", " << did << "[chunk]");
    AutoPtr<ChertCursor> cursor(postlist_table->cursor_get());
    if (!cursor.get()) return 0;

    bool exact = cursor->find_entry(make_key(slot, did));
    if (!exact) {
	// If we didn't find a chunk starting with docid did, then we need
	// to check that the chunk:
	const char * p = cursor->current_key.data();
	const char * end = p + cursor->current_key.size();

	// Check that it is a value stream chunk.
	if (end - p < 2 || *p++ != '\0' || *p++ != '\xd8') return 0;

	// Check that it's for the right value slot.
	Xapian::valueno v;
	if (!unpack_uint(&p, end, &v)) {
	    throw Xapian::DatabaseCorruptError("Bad value key");
	}
	if (v != slot) return 0;

	// And get the first docid for the chunk so we can return it.
	if (!unpack_uint_preserving_sort(&p, end, &did) || p != end) {
	    throw Xapian::DatabaseCorruptError("Bad value key");
	}
    }

    cursor->read_tag();
    swap(chunk, cursor->current_tag);

    return did;
}

const size_t CHUNK_SIZE_THRESHOLD = 2000;

class ValueUpdater {
    ChertPostListTable * table;

    Xapian::valueno slot;

    string ctag;

    ValueChunkReader reader;

    string tag;

    Xapian::docid prev_did;

    Xapian::docid first_did;

    Xapian::docid new_first_did;

    Xapian::docid last_allowed_did;

    void append_to_stream(Xapian::docid did, const string & value) {
	if (tag.empty())
	    new_first_did = did;
	else
	    tag += pack_uint(did - prev_did - 1);
	prev_did = did;
	tag += pack_string(value);
	if (tag.size() >= CHUNK_SIZE_THRESHOLD) write_tag();
    }

    void write_tag() {
	// If the first docid has changed, delete the old entry.
	if (first_did && new_first_did != first_did) {
	    table->del(make_key(slot, first_did));
	}
	if (!tag.empty()) {
	    table->add(make_key(slot, new_first_did), tag);
	}
	first_did = 0;
	tag.resize(0);
    }

  public:
    ValueUpdater(ChertPostListTable * table_, Xapian::valueno slot_)
       	: table(table_), slot(slot_), first_did(0) { }

    ~ValueUpdater() {
	while (!reader.at_end()) {
	    // FIXME: use skip_to and some splicing magic instead?
	    append_to_stream(reader.get_docid(), reader.get_value());
	    reader.next();
	}
	if (first_did) write_tag();
    }

    void update(Xapian::docid did, const string & value) {
	if (first_did && did > last_allowed_did) write_tag();
	if (first_did == 0) {
	    AutoPtr<ChertCursor> cursor(table->cursor_get());
	    bool exact = cursor->find_entry(make_key(slot, did));
	    if (!cursor->current_key.empty()) {
		if (!exact)
		    first_did = docid_from_key(slot, cursor->current_key);
		if (exact || first_did) {
		    cursor->read_tag();
		    if (cursor->current_tag.size() < CHUNK_SIZE_THRESHOLD) {
			swap(cursor->current_tag, ctag);
		    } else {
			first_did = did;
		    }
		} else {
		    first_did = did;
		}
		if (!ctag.empty()) {
		    reader.assign(ctag.data(), ctag.size(), first_did);
		}
		last_allowed_did = static_cast<Xapian::docid>(-1);
		if (cursor->next()) {
		    Xapian::docid next_first_did;
		    next_first_did = docid_from_key(slot, cursor->current_key);
		    if (next_first_did) last_allowed_did = next_first_did - 1;
		}
	    } else {
		first_did = did;
	    }
	}
	while (!reader.at_end() && reader.get_docid() <= did) {
	    if (reader.get_docid() != did) {
		// FIXME: use skip_to and some splicing magic instead?
		append_to_stream(reader.get_docid(), reader.get_value());
	    }
	    reader.next();
	}
	if (!value.empty()) {
	    // Add/update entry for did.
	    append_to_stream(did, value);
	}
    }
};

void
ChertValueManager::merge_changes()
{
    {
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
	    ValueUpdater updater(postlist_table, slot);
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
ChertValueManager::add_document(Xapian::docid did, const Xapian::Document &doc,
				map<Xapian::valueno, ValueStats> & value_stats)
{
    // FIXME: Use BitWriter and interpolative coding?  Or is it not worthwhile
    // for this?
    string slots_used;
    Xapian::ValueIterator it = doc.values_begin();
    Xapian::valueno prev_slot = static_cast<Xapian::valueno>(-1);
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
	slots_used += pack_uint(slot - prev_slot - 1);
	prev_slot = slot;
	++it;
    }
    swap(slots[did], slots_used);
}

void
ChertValueManager::delete_document(Xapian::docid did,
				   map<Xapian::valueno, ValueStats> & value_stats)
{
    map<Xapian::docid, string>::iterator it = slots.find(did);
    string s;
    if (it != slots.end()) {
	s = it->second;
    } else {
	// Get from table, making a swift exit if this document has no values.
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
ChertValueManager::replace_document(Xapian::docid did,
				    const Xapian::Document &doc,
				    map<Xapian::valueno, ValueStats> & value_stats)
{
    // FIXME: We do this to force the values to be cached in the Document
    // object in case we're asked to replace a document with itself with
    // unmodified values.  Really we should trap this case and avoid doing
    // needless work.
    (void)doc.values_count();

    delete_document(did, value_stats);
    add_document(did, doc, value_stats);
}

string
ChertValueManager::get_value(Xapian::docid did, Xapian::valueno slot) const
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
ChertValueManager::get_all_values(map<Xapian::valueno, string> & values,
				  Xapian::docid did) const
{
    Assert(values.empty());
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
ChertValueManager::get_value_stats(Xapian::valueno slot) const
{
    DEBUGCALL(DB, void, "ChertValueManager::get_value_stats", slot);
    // Invalidate the cache first in case an exception is thrown.
    mru_valno = Xapian::BAD_VALUENO;
    get_value_stats(slot, mru_valstats);
    mru_valno = slot;
}

void
ChertValueManager::get_value_stats(Xapian::valueno slot, ValueStats & stats) const
{
    DEBUGCALL(DB, void, "ChertValueManager::get_value_stats", slot << ", [stats]");
    // Invalidate the cache first in case an exception is thrown.
    mru_valno = Xapian::BAD_VALUENO;

    string tag;
    if (postlist_table->get_exact_entry(make_valuestats_key(slot), tag)) {
	const char * pos = tag.data();
	const char * end = pos + tag.size();

	if (!unpack_uint(&pos, end, &(stats.freq))) {
	    if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Frequency statistic in value table is too large");
	}
	if (!unpack_string(&pos, end, stats.lower_bound)) {
	    if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
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

    mru_valno = slot;
}

void
ChertValueManager::set_value_stats(map<Xapian::valueno, ValueStats> & value_stats)
{
    DEBUGCALL(DB, void, "ChertValueManager::set_value_stats", "[value_stats]");
    map<Xapian::valueno, ValueStats>::const_iterator i;
    for (i = value_stats.begin(); i != value_stats.end(); ++i) {
	string key = make_valuestats_key(i->first);
	const ValueStats & stats = i->second;
	if (stats.freq != 0) {
	    string new_value = pack_uint(stats.freq);
	    new_value += pack_string(stats.lower_bound);
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
    mru_valno = Xapian::BAD_VALUENO;
}
