/** @file collapser.cc
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

#include <config.h>

#include "collapser.h"

#include "heap.h"
#include "omassert.h"

#include <algorithm>

using namespace std;

collapse_result
CollapseData::check_item(const vector<Result>& results,
			 const Result& result,
			 Xapian::doccount collapse_max, MSetCmp mcmp,
			 Xapian::doccount& old_item)
{
    if (items.size() < collapse_max) {
	return ADD;
    }

    // We already have collapse_max items better than result so we need to
    // eliminate the lowest ranked.
    if (collapse_count == 0 && collapse_max != 1) {
	// Be lazy about calling make_heap - if we see <= collapse_max
	// items with a particular collapse key, we never need to use
	// the heap.
	Heap::make(items.begin(), items.end(),
		   [&](pair<Xapian::doccount, Xapian::docid> a,
		       pair<Xapian::doccount, Xapian::docid> b) {
		       return mcmp(results[a.first], results[b.first]);
		   });
    }
    ++collapse_count;

    Xapian::doccount old_item_candidate = items.front().first;
    const Result& old_result = results[old_item_candidate];
    if (items.front().second != old_result.get_docid()) {
	// The previous result with this collapse key we were going to replace
	// has been pushed out of the protomset by higher ranking results.
	//
	// Awkwardly that means we don't know its exact weight, but we
	// only need next_best_weight to know if we can zero collapse_count
	// when there's a percentage cut-off, so setting it to an overestimate
	// is OK, and it's not critically important as it makes no difference
	// at all unless there's a percentage cut-off set.
	//
	// For now use the new result's weight.  FIXME: The lowest weight in
	// the current MSet would be a tighter bound if we're sorting primarily
	// by weight (if we're not, then is next_best_weight useful at all?)
	// We could also check other entries in items to find an upper bound
	// weight.
	next_best_weight = result.get_weight();

	return REPLACE;
    }

    if (mcmp(old_result, result)) {
	// If this is the "best runner-up", update next_best_weight.
	if (result.get_weight() > next_best_weight)
	    next_best_weight = result.get_weight();
	return REJECT;
    }

    old_item = old_item_candidate;
    next_best_weight = old_result.get_weight();

    items.front() = { old_item, result.get_docid() };
    Heap::replace(items.begin(), items.end(),
		  [&](pair<Xapian::doccount, Xapian::docid> a,
		      pair<Xapian::doccount, Xapian::docid> b) {
		      return mcmp(results[a.first], results[b.first]);
		  });

    return REPLACE;
}

void
CollapseData::set_item(Xapian::doccount item)
{
    AssertEq(items.size(), 1);
    AssertEq(items[0].first, 0);
    items[0].first = item;
}

void
CollapseData::add_item(const vector<Result>& results,
		       Xapian::doccount item,
		       Xapian::doccount collapse_max,
		       MSetCmp mcmp)
{
    const Result& result = results[item];
    if (items.size() < collapse_max) {
	items.emplace_back(item, result.get_docid());
	return;
    }

    items.front() = { item, result.get_docid() };
    Heap::replace(items.begin(), items.end(),
		  [&](pair<Xapian::doccount, Xapian::docid> a,
		      pair<Xapian::doccount, Xapian::docid> b) {
		      return mcmp(results[a.first], results[b.first]);
		  });
}

collapse_result
Collapser::check(Result& result,
		 Xapian::Document::Internal& vsdoc)
{
    ptr = NULL;
    ++docs_considered;
    result.set_collapse_key(vsdoc.get_value(slot));

    if (result.get_collapse_key().empty()) {
	// We don't collapse results with an empty collapse key.
	++no_collapse_key;
	return EMPTY;
    }

    // Use dummy value 0 for item - if process() is called, this will get
    // updated to the appropriate value, and if it isn't then the docid won't
    // match and we'll know the item isn't in the current proto-mset.
    auto r = table.emplace(result.get_collapse_key(),
			   CollapseData(0, result.get_docid()));
    ptr = &r.first->second;
    if (r.second) {
	// We've not seen this collapse key before.
	++entry_count;
	return NEW;
    }

    collapse_result res;
    CollapseData& collapse_data = *ptr;
    res = collapse_data.check_item(results, result, collapse_max, mcmp,
				   old_item);
    if (res == ADD) {
	++entry_count;
    } else if (res == REJECT || res == REPLACE) {
	++dups_ignored;
    }
    return res;
}

void
Collapser::process(collapse_result action,
		   Xapian::doccount item)
{
    switch (action) {
	case NEW:
	    // We've not seen this collapse key before.
	    Assert(ptr);
	    ptr->set_item(item);
	    return;
	case ADD: {
	    Assert(ptr);
	    ptr->add_item(results, item, collapse_max, mcmp);
	    break;
	}
	default:
	    // Shouldn't be called for other actions.
	    Assert(false);
    }
}

Xapian::doccount
Collapser::get_collapse_count(const string & collapse_key,
			      int percent_threshold,
			      double min_weight) const
{
    auto key = table.find(collapse_key);
    // If a collapse key is present in the MSet, it must be in our table.
    Assert(key != table.end());

    if (!percent_threshold) {
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

void
Collapser::finalise(double min_weight, int percent_threshold)
{
    if (table.empty() || results.empty())
	return;

    // We need to fill in collapse_count values in results using the
    // information stored in table.
    Xapian::doccount todo = entry_count;
    for (Result& result : results) {
	const string& key = result.get_collapse_key();
	if (key.empty())
	    continue;

	// Fill in collapse_count.
	result.set_collapse_count(get_collapse_count(key, percent_threshold,
						     min_weight));
	if (--todo == 0) {
	    // Terminate early if we've handled all non-empty entries.
	    break;
	}
    }
}
