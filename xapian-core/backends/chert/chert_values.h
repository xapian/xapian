/** @file chert_values.h
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

#ifndef XAPIAN_INCLUDED_CHERT_VALUES_H
#define XAPIAN_INCLUDED_CHERT_VALUES_H

#include "valuestats.h"

#include "xapian/types.h"

#include <map>
#include <string>

namespace Xapian {
    class Document;
}

class ChertPostListTable;
class ChertTermListTable;
struct ValueStats;

class ChertValueManager {
    /** The value number for the most recently used value statistics.
     *
     *  Set to Xapian::BAD_VALUENO if no value statistics are currently
     *  cached.
     */
    mutable Xapian::valueno mru_valno;

    /** The most recently used value statistics. */
    mutable ValueStats mru_valstats;

    ChertPostListTable * postlist_table;

    ChertTermListTable * termlist_table;

    std::map<Xapian::docid, std::string> slots;

    std::map<Xapian::valueno, std::map<Xapian::docid, std::string> > changes;

    void add_value(Xapian::docid did, Xapian::valueno slot,
		   const std::string & val);

    void remove_value(Xapian::docid did, Xapian::valueno slot);

    Xapian::docid get_chunk_containing_did(Xapian::valueno slot,
					   Xapian::docid did,
					   std::string &chunk) const;

    /** Get the statistics for value slot @a slot. */
    void get_value_stats(Xapian::valueno slot) const;

    void get_value_stats(Xapian::valueno slot, ValueStats & stats) const;

  public:
    /** Create a new ChertValueManager object. */
    ChertValueManager(ChertPostListTable * postlist_table_,
		      ChertTermListTable * termlist_table_)
	: mru_valno(Xapian::BAD_VALUENO),
	  postlist_table(postlist_table_),
	  termlist_table(termlist_table_) { }

    // Merge in batched-up changes.
    void merge_changes();

    void add_document(Xapian::docid did, const Xapian::Document &doc,
		      std::map<Xapian::valueno, ValueStats> & value_stats);

    void delete_document(Xapian::docid did,
			 std::map<Xapian::valueno, ValueStats> & value_stats);

    void replace_document(Xapian::docid did, const Xapian::Document &doc,
			  std::map<Xapian::valueno, ValueStats> & value_stats);

    std::string get_value(Xapian::docid did, Xapian::valueno slot) const;

    void get_all_values(std::map<Xapian::valueno, std::string> & values,
			Xapian::docid did) const;

    Xapian::doccount get_value_freq(Xapian::valueno slot) const {
	if (mru_valno != slot) get_value_stats(slot);
	return mru_valstats.freq;
    }

    std::string get_value_lower_bound(Xapian::valueno slot) const {
	if (mru_valno != slot) get_value_stats(slot);
	return mru_valstats.lower_bound;
    }

    std::string get_value_upper_bound(Xapian::valueno slot) const {
	if (mru_valno != slot) get_value_stats(slot);
	return mru_valstats.upper_bound;
    }

    /** Write the updated statistics to the table.
     *
     *  If the @a freq member of the statistics for a particular slot is 0, the
     *  statistics for that slot will be cleared.
     *
     *  @param value_stats The statistics to set.
     */
    void set_value_stats(std::map<Xapian::valueno, ValueStats> & value_stats);

    void reset() {
	/// Ignore any old cached valuestats.
	mru_valno = Xapian::BAD_VALUENO;
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

#endif // XAPIAN_INCLUDED_CHERT_VALUES_H
