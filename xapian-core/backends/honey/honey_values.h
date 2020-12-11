/** @file
 * @brief HoneyValueManager class
 */
/* Copyright (C) 2008,2009,2011,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_VALUES_H
#define XAPIAN_INCLUDED_HONEY_VALUES_H

#include "honey_cursor.h"
#include "backends/valuestats.h"
#include "pack.h"
#include "xapian/error.h"
#include "xapian/types.h"

#include <map>
#include <memory>
#include <string>

namespace Honey {

/** Generate a key for a value stream chunk. */
inline std::string
make_valuechunk_key(Xapian::valueno slot, Xapian::docid last_did)
{
    std::string key(1, '\0');
    if (slot < (Honey::KEY_VALUE_CHUNK_HI - Honey::KEY_VALUE_CHUNK) >> 3) {
	key += char(Honey::KEY_VALUE_CHUNK + slot);
    } else {
	key += char(Honey::KEY_VALUE_CHUNK_HI);
	pack_uint_preserving_sort(key, slot);
    }
    pack_uint_preserving_sort(key, last_did);
    return key;
}

inline Xapian::docid
docid_from_key(Xapian::valueno required_slot, const std::string& key)
{
    const char* p = key.data();
    const char* end = p + key.length();
    if (end - p < 3 || *p++ != '\0') {
	// Not a value chunk key.
	return 0;
    }
    unsigned char code = *p++;
    if (code < Honey::KEY_VALUE_CHUNK || code > Honey::KEY_VALUE_CHUNK_HI) {
	// Also not a value chunk key.
	return 0;
    }

    Xapian::valueno slot;
    if (code < Honey::KEY_VALUE_CHUNK_HI) {
	slot = code - Honey::KEY_VALUE_CHUNK;
    } else {
	if (!unpack_uint_preserving_sort(&p, end, &slot))
	    throw Xapian::DatabaseCorruptError("Bad value key");
    }
    // Fail if for a different slot.
    if (slot != required_slot) return 0;
    Xapian::docid did;
    if (!unpack_uint_preserving_sort(&p, end, &did))
	throw Xapian::DatabaseCorruptError("Bad value key");
    return did;
}

inline std::string
make_valuestats_key(Xapian::valueno slot)
{
    std::string key(1, '\0');
    if (slot <= 7) {
	key += char(Honey::KEY_VALUE_STATS + slot);
    } else {
	key += char(Honey::KEY_VALUE_STATS + 7);
	pack_uint_preserving_sort(key, slot);
    }
    return key;
}

inline static std::string
encode_valuestats(Xapian::doccount freq,
		  const std::string& lbound,
		  const std::string& ubound)
{
    std::string value;
    pack_uint(value, freq);
    pack_string(value, lbound);
    // We don't store or count empty values, so neither of the bounds
    // can be empty.  So we can safely store an empty upper bound when
    // the bounds are equal.
    if (lbound != ubound) value += ubound;
    return value;
}

}

namespace Xapian {
    class Document;
}

class HoneyPostListTable;
class HoneyTermListTable;
struct ValueStats;

class HoneyValueManager {
    /** The value number for the most recently used value statistics.
     *
     *  Set to Xapian::BAD_VALUENO if no value statistics are currently
     *  cached.
     */
    mutable Xapian::valueno mru_slot;

    /** The most recently used value statistics. */
    mutable ValueStats mru_valstats;

    HoneyPostListTable& postlist_table;

    HoneyTermListTable& termlist_table;

    std::map<Xapian::docid, std::string> slots;

    std::map<Xapian::valueno, std::map<Xapian::docid, std::string>> changes;

    mutable std::unique_ptr<HoneyCursor> cursor;

    void add_value(Xapian::docid did, Xapian::valueno slot,
		   const std::string& val);

    void remove_value(Xapian::docid did, Xapian::valueno slot);

    /** Move the cursor to the chunk containing did.
     *
     *  @return The last docid in the chunk, or 0 if off the end of the stream.
     */
    Xapian::docid get_chunk_containing_did(Xapian::valueno slot,
					   Xapian::docid did,
					   std::string& chunk) const;

    /** Get the statistics for value slot @a slot. */
    void get_value_stats(Xapian::valueno slot) const;

    void get_value_stats(Xapian::valueno slot, ValueStats& stats) const;

  public:
    /** Create a new HoneyValueManager object. */
    HoneyValueManager(HoneyPostListTable& postlist_table_,
		      HoneyTermListTable& termlist_table_)
	: mru_slot(Xapian::BAD_VALUENO),
	  postlist_table(postlist_table_),
	  termlist_table(termlist_table_) { }

    // Merge in batched-up changes.
    void merge_changes();

    std::string add_document(Xapian::docid did, const Xapian::Document& doc,
			     std::map<Xapian::valueno, ValueStats>& val_stats);

    void delete_document(Xapian::docid did,
			 std::map<Xapian::valueno, ValueStats>& val_stats);

    std::string replace_document(Xapian::docid did,
				 const Xapian::Document& doc,
				 std::map<Xapian::valueno, ValueStats>& val_stats);

    std::string get_value(Xapian::docid did, Xapian::valueno slot) const;

    void get_all_values(std::map<Xapian::valueno, std::string>& values,
			Xapian::docid did) const;

    Xapian::doccount get_value_freq(Xapian::valueno slot) const {
	if (mru_slot != slot) get_value_stats(slot);
	return mru_valstats.freq;
    }

    std::string get_value_lower_bound(Xapian::valueno slot) const {
	if (mru_slot != slot) get_value_stats(slot);
	return mru_valstats.lower_bound;
    }

    std::string get_value_upper_bound(Xapian::valueno slot) const {
	if (mru_slot != slot) get_value_stats(slot);
	return mru_valstats.upper_bound;
    }

    /** Write the updated statistics to the table.
     *
     *  If the @a freq member of the statistics for a particular slot is 0, the
     *  statistics for that slot will be cleared.
     *
     *  @param val_stats The statistics to set.
     */
    void set_value_stats(std::map<Xapian::valueno, ValueStats>& val_stats);

    void reset() {
	/// Ignore any old cached valuestats.
	mru_slot = Xapian::BAD_VALUENO;
    }

    bool is_modified() const {
	return !changes.empty();
    }

    void cancel() {
	// Discard batched-up changes.
	slots.clear();
	changes.clear();
    }
};

namespace Honey {

class ValueChunkReader {
    const char* p;
    const char* end;

    Xapian::docid did;

    std::string value;

  public:
    /// Create a ValueChunkReader which is already at_end().
    ValueChunkReader() : p(NULL) { }

    ValueChunkReader(const char* p_, size_t len, Xapian::docid last_did) {
	assign(p_, len, last_did);
    }

    void assign(const char* p_, size_t len, Xapian::docid last_did);

    bool at_end() const { return p == NULL; }

    Xapian::docid get_docid() const { return did; }

    const std::string& get_value() const { return value; }

    void next();

    void skip_to(Xapian::docid target);
};

}

#endif // XAPIAN_INCLUDED_HONEY_VALUES_H
