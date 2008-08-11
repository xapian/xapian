/** @file chert_values.cc
 * @brief ChertValueTable class
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
#include "chert_utils.h"
#include "omdebug.h"

#include "xapian/error.h"
#include "xapian/valueiterator.h"

#include <algorithm>
#include "autoptr.h"

using namespace std;

// FIXME:
//  * put the "used slots" entry in the termlist table, perhaps in the same
//    entry as the terms.
//  * put the value chunks in the postlist table.
//  * put the value stats in the postlist table?
//  * multi-values?
//  * values named instead of numbered?

/** Generate a key for a value stream. */
inline string
make_key(Xapian::valueno slot, Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, string, "make_key", slot << ", " << did);
    // FIXME: sort out exactly what the key format is.
    RETURN(string(1, '\x01') + pack_uint(slot) + pack_uint_preserving_sort(did));
}

/** Generate a key for the "used slots" data. */
inline string
make_slot_key(Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, string, "make_slot_key", did);
    // FIXME: sort out exactly what the key format is.
    RETURN(string(1, '\0') + pack_uint_last(did));
}

/** Generate a key for a value statistics item. */
inline string
make_valuestats_key(Xapian::valueno slot)
{
    DEBUGCALL_STATIC(DB, string, "make_valuestats_key", slot);
    // FIXME: sort out exactly what the key format is.
    RETURN(string(1, '\xff') + pack_uint_last(slot));
}

class ValueChunkReader {
    const char *p;
    const char *end;

    Xapian::docid did;

    string value;

  public:
    ValueChunkReader(const char * p_, size_t len, Xapian::docid did_)
	: p(p_), end(p_ + len), did(did_)
    {
	//if (!unpack_string(&p, end, value))
	//    throw Xapian::DatabaseCorruptError("Failed to unpack first value");
	value.assign(p, len);
	p = end;
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
ChertValueTable::add_value(Xapian::docid did, Xapian::valueno slot,
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
ChertValueTable::remove_value(Xapian::docid did, Xapian::valueno slot)
{
    map<Xapian::valueno, map<Xapian::docid, string> >::iterator i;
    i = changes.find(slot);
    if (i == changes.end()) {
	i = changes.insert(make_pair(slot, map<Xapian::docid, string>())).first;
    }
    i->second[did] = string();
}

Xapian::docid
ChertValueTable::get_chunk_containing_did(Xapian::valueno slot,
					  Xapian::docid did,
					  string &chunk) const
{
    AutoPtr<ChertCursor> cursor(cursor_get());
    if (!cursor.get() || !cursor->find_entry(make_key(slot, did))) {
	return 0;
    }

    const char * p = cursor->current_key.data();
    const char * end = p + cursor->current_key.size();

    if (*p++ != '\x01') return 0;

    Xapian::valueno v;
    if (!unpack_uint(&p, end, &v)) {
	throw Xapian::DatabaseCorruptError("Bad value key");
    }
    if (v != slot) return 0;

    Xapian::docid first_did;
    if (!unpack_uint_preserving_sort(&p, end, &first_did) || p != end) {
	throw Xapian::DatabaseCorruptError("Bad value key");
    }

    cursor->read_tag();
    swap(chunk, cursor->current_tag);

    return first_did;
}

void
ChertValueTable::merge_changes()
{
    {
	map<Xapian::docid, string>::const_iterator i;
	for (i = slots.begin(); i != slots.end(); ++i) {
	    add(make_slot_key(i->first), i->second);
	}
	slots.clear();
    }

    {
	map<Xapian::valueno, map<Xapian::docid, string> >::const_iterator i;
	for (i = changes.begin(); i != changes.end(); ++i) {
	    Xapian::valueno slot = i->first;
	    const map<Xapian::docid, string> & slot_changes = i->second;
	    map<Xapian::docid, string>::const_iterator j;
	    for (j = slot_changes.begin(); j != slot_changes.end(); ++j) {
		// FIXME: but in chunks...
		add(make_key(slot, j->first), j->second);
	    }
	}
	changes.clear();
    }
}

void
ChertValueTable::add_document(Xapian::docid did, const Xapian::Document &doc,
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
ChertValueTable::delete_document(Xapian::docid did,
				 map<Xapian::valueno, ValueStats> & value_stats)
{
    map<Xapian::docid, string>::iterator it = slots.find(did);
    string s;
    if (it != slots.end()) {
	s = it->second;
    } else {
	// Get from table, making a swift exit if this document has no values.
	if (!get_exact_entry(make_slot_key(did), s)) return;
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
ChertValueTable::replace_document(Xapian::docid did,
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
ChertValueTable::get_value(Xapian::docid did, Xapian::valueno slot) const
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
ChertValueTable::get_all_values(map<Xapian::valueno, string> & values,
				Xapian::docid did) const
{
    Assert(values.empty());
    map<Xapian::docid, string>::const_iterator i = slots.find(did);
    string s;
    if (i != slots.end()) {
	s = i->second;
    } else {
	// Get from table.
	if (!get_exact_entry(make_slot_key(did), s)) return;
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
ChertValueTable::get_value_stats(Xapian::valueno slot) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_value_stats", slot);
    // Invalidate the cache first in case an exception is thrown.
    mru_valno = Xapian::BAD_VALUENO;
    get_value_stats(slot, mru_valstats);
    mru_valno = slot;
}

void
ChertValueTable::get_value_stats(Xapian::valueno slot, ValueStats & stats) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_value_stats", slot << ", [stats]");
    // Invalidate the cache first in case an exception is thrown.
    mru_valno = Xapian::BAD_VALUENO;

    string tag;
    if (get_exact_entry(make_valuestats_key(slot), tag)) {
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
ChertValueTable::set_value_stats(map<Xapian::valueno, ValueStats> & value_stats)
{
    DEBUGCALL(DB, void, "ChertValueTable::set_value_stats", "[value_stats]");
    map<Xapian::valueno, ValueStats>::const_iterator i;
    for (i = value_stats.begin(); i != value_stats.end(); ++i) {
	string key = make_valuestats_key(i->first);
	const ValueStats & stats = i->second;
	if (stats.freq != 0) {
	    string new_value;
	    new_value += pack_uint(stats.freq);
	    new_value += pack_string(stats.lower_bound);
	    // We don't store or count empty values, so neither of the bounds
	    // can be empty.  So we can safely store an empty upper bound when
	    // the bounds are equal.
	    if (stats.lower_bound != stats.upper_bound)
		new_value += stats.upper_bound;
	    add(key, new_value);
	} else {
	    del(key);
	}
    }
    value_stats.clear();
    mru_valno = Xapian::BAD_VALUENO;
}

