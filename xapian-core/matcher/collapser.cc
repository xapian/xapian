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

#include <config.h>

#include "collapser.h"

#include "omassert.h"

#include <algorithm>

using namespace std;

collapse_result
CollapseData::add_item(const Xapian::Internal::MSetItem & item,
		       Xapian::doccount collapse_max, MSetCmp mcmp,
		       Xapian::Internal::MSetItem & old_item)
{
    if (items.size() < collapse_max) {
	items.push_back(item);
	items.back().collapse_key = string();
	return ADDED;
    }

    // We already have collapse_max items better than item so we need to
    // eliminate the lowest ranked.
    if (collapse_count == 0 && collapse_max != 1) {
	// Be lazy about calling make_heap - if we see <= collapse_max
	// items with a particular collapse key, we never need to use
	// the heap.
	make_heap(items.begin(), items.end(), mcmp);
    }
    ++collapse_count;

    if (mcmp(items.front(), item)) {
	// If this is the "best runner-up", update next_best_weight.
	if (item.wt > next_best_weight) next_best_weight = item.wt;
	return REJECTED;
    }

    next_best_weight = items.front().wt;

    items.push_back(item);
    push_heap(items.begin(), items.end(), mcmp);
    pop_heap(items.begin(), items.end(), mcmp);
    swap(old_item, items.back());
    items.pop_back();

    return REPLACED;
}

collapse_result
Collapser::process(Xapian::Internal::MSetItem & item,
		   PostList * postlist,
		   Xapian::Document::Internal & vsdoc,
		   MSetCmp mcmp)
{
    ++docs_considered;
    // The postlist will supply the collapse key for a remote match.
    const string * key_ptr = postlist->get_collapse_key();
    if (key_ptr) {
	item.collapse_key = *key_ptr;
    } else {
	// Otherwise use the Document object to get the value.
	item.collapse_key = vsdoc.get_value(slot);
    }

    if (item.collapse_key.empty()) {
	// We don't collapse items with an empty collapse key.
	++no_collapse_key;
	return EMPTY;
    }

    auto oldkey = table.find(item.collapse_key);
    if (oldkey == table.end()) {
	// We've not seen this collapse key before.
	table.insert(make_pair(item.collapse_key, CollapseData(item)));
	++entry_count;
	return ADDED;
    }

    collapse_result res;
    CollapseData & collapse_data = oldkey->second;
    res = collapse_data.add_item(item, collapse_max, mcmp, old_item);
    if (res == ADDED) {
	++entry_count;
    } else if (res == REJECTED || res == REPLACED) {
	++dups_ignored;
    }
    return res;
}

Xapian::doccount
Collapser::get_collapse_count(const string & collapse_key, int percent_cutoff,
			      double min_weight) const
{
    auto key = table.find(collapse_key);
    // If a collapse key is present in the MSet, it must be in our table.
    Assert(key != table.end());

    if (!percent_cutoff) {
	// The recorded collapse_count is correct.
	return key->second.get_collapse_count();
    }

    if (key->second.get_next_best_weight() < min_weight) {
	// We know for certain that all collapsed items would have failed the
	// percentage cutoff, so collapse_count should be 0.
	return 0;
    }

    // We know that the highest weighted collapsed item would have survived the
    // percentage cutoff, but it's possible all other collapsed items would
    // have failed it.  Since collapse_count is a lower bound, we must set it
    // to 1.
    return 1;
}

Xapian::doccount
Collapser::get_matches_lower_bound() const
{
    // We've seen this many matches, but all other documents matching the query
    // could be collapsed onto values already seen.
    Xapian::doccount matches_lower_bound = no_collapse_key + entry_count;
    return matches_lower_bound;
    // FIXME: *Unless* we haven't achieved collapse_max occurrences of *any*
    // collapse key value, so we can increase matches_lower_bound like the
    // code below, but it isn't quite that simple - there may not be that
    // many documents.
#if 0
    Xapian::doccount max_kept = 0;
    for (auto i = table.begin(); i != table.end(); ++i) {
	if (i->second.get_collapse_count() > max_kept) {
	    max_kept = i->second.get_collapse_count();
	    if (max_kept == collapse_max) {
		return matches_lower_bound;
	    }
	}
    }
    return matches_lower_bound + (collapse_max - max_kept);
#endif
}
