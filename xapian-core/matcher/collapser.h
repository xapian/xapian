/** @file
 * @brief Collapse documents with the same collapse key during the match.
 */
/* Copyright (C) 2009,2011 Olly Betts
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

#include "backends/document.h"
#include "msetcmp.h"
#include "api/omenquireinternal.h"
#include "api/postlist.h"

#include <map>
#include <vector>

/// Enumeration reporting how a document was handled by the Collapser.
typedef enum {
    EMPTY,
    ADDED,
    REJECTED,
    REPLACED
} collapse_result;

/// Class tracking information for a given value of the collapse key.
class CollapseData {
    /** Currently kept MSet entries for this value of the collapse key.
     *
     *  If collapse_max > 1, then this is a min-heap once collapse_count > 0.
     *
     *  FIXME: We expect collapse_max to be small, so perhaps we should
     *  preallocate space for that many entries and/or allocate space in
     *  larger blocks to divvy up?
     */
    std::vector<Xapian::Internal::MSetItem> items;

    /// The highest weight of a document we've rejected.
    double next_best_weight;

    /// The number of documents we've rejected.
    Xapian::doccount collapse_count;

  public:
    /// Construct with the given MSetItem @a item.
    explicit CollapseData(const Xapian::Internal::MSetItem & item)
	: items(1, item), next_best_weight(0), collapse_count(0) {
	items[0].collapse_key = std::string();
    }

    /** Handle a new MSetItem with this collapse key value.
     *
     *  @param item		The new item.
     *  @param collapse_max	Max no. of items for each collapse key value.
     *  @param mcmp		MSetItem comparison functor.
     *  @param[out] old_item	Replaced item (when REPLACED is returned).
     *
     *  @return How @a item was handled: ADDED, REJECTED or REPLACED.
     */
    collapse_result add_item(const Xapian::Internal::MSetItem & item,
			     Xapian::doccount collapse_max,
			     MSetCmp mcmp,
			     Xapian::Internal::MSetItem & old_item);

    /// The highest weight of a document we've rejected.
    double get_next_best_weight() const { return next_best_weight; }

    /// The number of documents we've rejected.
    Xapian::doccount get_collapse_count() const { return collapse_count; }
};

/// The Collapser class tracks collapse keys and the documents they match.
class Collapser {
    /// Map from collapse key values to the items we're keeping for them.
    std::map<std::string, CollapseData> table;

    /// How many items we're currently keeping in @a table.
    Xapian::doccount entry_count;

    /** How many documents have we seen without a collapse key?
     *
     *  We use this statistic to improve matches_lower_bound.
     */
    Xapian::doccount no_collapse_key;

    /** How many documents with duplicate collapse keys we have ignored.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing) and matches_upper_bound.
     */
    Xapian::doccount dups_ignored;

    /** How many documents we've considered for collapsing.
     *
     *  We use this statistic to improve matches_estimated (by considering
     *  the rate of collapsing).
     */
    Xapian::doccount docs_considered;

    /** The value slot we're getting collapse keys from. */
    Xapian::valueno slot;

    /** The maximum number of items to keep for each collapse key value. */
    Xapian::doccount collapse_max;

  public:
    /// Replaced item when REPLACED is returned by @a collapse().
    Xapian::Internal::MSetItem old_item;

    Collapser(Xapian::valueno slot_, Xapian::doccount collapse_max_)
	: entry_count(0), no_collapse_key(0), dups_ignored(0),
	  docs_considered(0), slot(slot_), collapse_max(collapse_max_),
	  old_item(0, 0) { }

    /// Return true if collapsing is active for this match.
    operator bool() const { return collapse_max != 0; }

    /** Handle a new MSetItem.
     *
     *  @param item		The new item.
     *  @param postlist		PostList to try to get collapse key from
     *				(this happens for a remote match).
     *  @param doc		Document for getting values.
     *  @param mcmp		MSetItem comparison functor.
     *
     *  @return How @a item was handled: EMPTY, ADDED, REJECTED or REPLACED.
     */
    collapse_result process(Xapian::Internal::MSetItem & item,
			    PostList * postlist,
			    Xapian::Document::Internal & vsdoc,
			    MSetCmp mcmp);

    Xapian::doccount get_collapse_count(const std::string & collapse_key,
					int percent_cutoff,
					double min_weight) const;

    Xapian::doccount get_docs_considered() const { return docs_considered; }

    Xapian::doccount get_dups_ignored() const { return dups_ignored; }

    Xapian::doccount entries() const { return entry_count; }

    Xapian::doccount get_matches_lower_bound() const;

    bool empty() const { return table.empty(); }
};

#endif // XAPIAN_INCLUDED_COLLAPSER_H
