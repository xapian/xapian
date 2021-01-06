/** @file
 * @brief Collapse documents with the same collapse key during the match.
 */
/* Copyright (C) 2009,2011,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_COLLAPSER_H
#define XAPIAN_INCLUDED_COLLAPSER_H

#include "backends/documentinternal.h"
#include "msetcmp.h"
#include "omassert.h"
#include "api/result.h"

#include <unordered_map>
#include <vector>

/// Enumeration reporting how a result will be handled by the Collapser.
typedef enum {
    EMPTY,
    NEW,
    ADD,
    REJECT,
    REPLACE
} collapse_result;

/// Class tracking information for a given value of the collapse key.
class CollapseData {
    /** Currently kept MSet entries for this value of the collapse key.
     *
     *  If collapse_max > 1, then this is a min-heap once collapse_count > 0.
     *
     *  The first member of the pair is the index into proto_mset.results
     *  and the second is the docid of the entry (used to detect if the
     *  entry in proto_mset.results we were referring to has been dropped).
     *
     *  FIXME: We expect collapse_max to be small, so perhaps we should
     *  preallocate space for that many entries and/or allocate space in
     *  larger blocks to divvy up?
     */
    std::vector<std::pair<Xapian::doccount, Xapian::docid>> items;

    /// The highest weight of a document we've rejected.
    double next_best_weight;

    /// The number of documents we've rejected.
    Xapian::doccount collapse_count;

  public:
    /// Construct with the given item.
    CollapseData(Xapian::doccount item, Xapian::docid did)
	: items(1, { item, did }), next_best_weight(0), collapse_count(0) {
    }

    /** Check a new result with this collapse key value.
     *
     *  If this method determines the action to take is ADD or REPLACE, then
     *  the proto-mset should be updated and then add_item() called to complete
     *  the update of the CollapseData (if the result doesn't actually get
     *  added, then it's OK not to follow up with a call to add_item()).
     *
     *  @param results		The results so far.
     *  @param result		The new result.
     *  @param collapse_max	Max no. of items for each collapse key value.
     *  @param mcmp		Result comparison functor.
     *  @param[out] old_item	Item to be replaced (when REPLACE is returned).
     *
     *  @return How to handle @a result: ADD, REJECT or REPLACE.
     */
    collapse_result check_item(const std::vector<Result>& results,
			       const Result& result,
			       Xapian::doccount collapse_max,
			       MSetCmp mcmp,
			       Xapian::doccount& old_item);

    /** Set item after constructing with a placeholder.
     *
     *  @param item		The new item (index into results).
     */
    void set_item(Xapian::doccount item);

    /** Complete update of new result with this collapse key value.
     *
     *  @param results		The results so far.
     *  @param item		The new item (index into results).
     *  @param collapse_max	Max no. of items for each collapse key value.
     *  @param mcmp		Result comparison functor.
     */
    void add_item(const std::vector<Result>& results,
		  Xapian::doccount item,
		  Xapian::doccount collapse_max,
		  MSetCmp mcmp);

    /** Process relocation of entry in results.
     *
     *  @param from	The old item (index into results).
     *  @param to  	The new item (index into results).
     */
    void result_has_moved(Xapian::doccount from, Xapian::doccount to) {
	for (auto&& item : items) {
	    if (item.first == from) {
		item.first = to;
		return;
	    }
	}
	// The entry ought to be present.
	Assert(false);
    }

    /// The highest weight of a document we've rejected.
    double get_next_best_weight() const { return next_best_weight; }

    /// The number of documents we've rejected.
    Xapian::doccount get_collapse_count() const { return collapse_count; }
};

/// The Collapser class tracks collapse keys and the documents they match.
class Collapser {
    /// Map from collapse key values to the items we're keeping for them.
    std::unordered_map<std::string, CollapseData> table;

    /// How many items we're currently keeping in @a table.
    Xapian::doccount entry_count = 0;

    /** How many documents have we seen without a collapse key?
     *
     *  We use this statistic to improve matches_lower_bound.
     */
    Xapian::doccount no_collapse_key = 0;

    /** How many documents with duplicate collapse keys we have ignored.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing) and matches_upper_bound.
     */
    Xapian::doccount dups_ignored = 0;

    /** How many documents we've considered for collapsing.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing).
     */
    Xapian::doccount docs_considered = 0;

    /** The value slot we're getting collapse keys from. */
    Xapian::valueno slot;

    /** The maximum number of items to keep for each collapse key value. */
    Xapian::doccount collapse_max;

    std::vector<Result>& results;

    MSetCmp mcmp;

    /** Pointer to CollapseData when NEW or ADD is in progress. */
    CollapseData* ptr = NULL;

    /// Adapt @a mcmp to be usable with min_heap.
    bool operator()(Xapian::doccount a, Xapian::doccount b) const {
	return mcmp(results[a], results[b]);
    }

  public:
    /// Replaced item when REPLACE is returned by @a collapse().
    Xapian::doccount old_item = 0;

    Collapser(Xapian::valueno slot_,
	      Xapian::doccount collapse_max_,
	      std::vector<Result>& results_,
	      MSetCmp mcmp_)
	: slot(slot_),
	  collapse_max(collapse_max_),
	  results(results_),
	  mcmp(mcmp_) { }

    /// Return true if collapsing is active for this match.
    operator bool() const { return collapse_max != 0; }

    /** Check a new result.
     *
     *  If this method determines the action to take is NEW or ADD then the
     *  proto-mset should be updated and then process() called to complete the
     *  update (if the result doesn't actually get added, then it's OK not
     *  to follow up with a call to process()).
     *
     *  @param result	The new result.
     *  @param vsdoc	Document for getting values.
     *
     *  @return How to handle @a result: EMPTY, NEW, ADD, REJECT or REPLACE.
     */
    collapse_result check(Result& result,
			  Xapian::Document::Internal & vsdoc);

    /** Handle a new Result.
     *
     *  @param action	The collapse_result returned by check().
     *  @param item	The new item (index into results).
     */
    void process(collapse_result action, Xapian::doccount item);

    /** Process relocation of entry in results.
     *
     *  @param from		The old item (index into results).
     *  @param to  		The new item (index into results).
     */
    void result_has_moved(Xapian::doccount from, Xapian::doccount to) {
	const std::string& collapse_key = results[to].get_collapse_key();
	if (collapse_key.empty()) {
	    return;
	}
	auto it = table.find(collapse_key);
	if (rare(it == table.end())) {
	    // The entry ought to be present.
	    Assert(false);
	    return;
	}

	CollapseData& collapse_data = it->second;
	collapse_data.result_has_moved(from, to);
    }

    Xapian::doccount get_collapse_count(const std::string & collapse_key,
					int percent_threshold,
					double min_weight) const;

    Xapian::doccount get_docs_considered() const { return docs_considered; }

    Xapian::doccount get_dups_ignored() const { return dups_ignored; }

    Xapian::doccount get_entries() const { return entry_count; }

    Xapian::doccount get_matches_lower_bound() const;

    void finalise(double min_weight, int percent_threshold);
};

/** Simpler version of Collapser used when merging MSet objects.
 *
 *  We merge results in descending rank order, so collapsing is much simpler
 *  than during the match - we just need to discard documents if we've already
 *  seen collapse_max with the same key.
 */
class CollapserLite {
    /// Map from collapse key values to collapse counts.
    std::unordered_map<std::string, Xapian::doccount> table;

    /// How many items we're currently keeping in @a table.
    Xapian::doccount entry_count = 0;

    /** How many documents have we seen without a collapse key?
     *
     *  We use this statistic to improve matches_lower_bound.
     */
    Xapian::doccount no_collapse_key = 0;

    /** How many documents with duplicate collapse keys we have ignored.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing) and matches_upper_bound.
     */
    Xapian::doccount dups_ignored = 0;

    /** How many documents we've considered for collapsing.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing).
     */
    Xapian::doccount docs_considered = 0;

    /** The maximum number of items to keep for each collapse key value. */
    Xapian::doccount collapse_max;

  public:
    CollapserLite(Xapian::doccount collapse_max_)
	: collapse_max(collapse_max_) {}

    /// Return true if collapsing is active for this match.
    operator bool() const { return collapse_max != 0; }

    /** Try to add a new key.
     *
     *  @return true if accepted; false if rejected.
     */
    bool add(const std::string& key) {
	++docs_considered;

	if (key.empty()) {
	    ++no_collapse_key;
	    return true;
	}

	auto r = table.emplace(key, 1);
	if (r.second) {
	    // New entry, set to 1.
	} else if (r.first->second == collapse_max) {
	    // Already seen collapse_max with this key so reject.
	    ++dups_ignored;
	    return false;
	} else {
	    // Increment count.
	    ++r.first->second;
	}
	++entry_count;
	return true;
    }

    Xapian::doccount get_docs_considered() const { return docs_considered; }

    Xapian::doccount get_dups_ignored() const { return dups_ignored; }

    Xapian::doccount get_entries() const { return entry_count; }

    Xapian::doccount get_matches_lower_bound() const {
	return no_collapse_key + entry_count;
    }

    void finalise(std::vector<Result>& results, int percent_threshold) {
	if (table.empty() || results.empty())
	    return;

	// We need to fill in collapse_count values in results using the
	// information stored in table.
	Xapian::doccount todo = entry_count;
	for (Result& result : results) {
	    const std::string& key = result.get_collapse_key();
	    if (key.empty())
		continue;

	    // Adjust collapse_count.
	    if (percent_threshold) {
		// FIXME: We can probably do better here.
		result.set_collapse_count(1);
	    } else {
		auto c = result.get_collapse_count() + table[key];
		result.set_collapse_count(c);
	    }

	    if (--todo == 0) {
		// Terminate early if we've handled all non-empty entries.
		break;
	    }
	}
    }
};

#endif // XAPIAN_INCLUDED_COLLAPSER_H
