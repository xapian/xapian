/** @file
 * @brief ProtoMSet class
 */
/* Copyright (C) 2004,2007,2017,2018,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_PROTOMSET_H
#define XAPIAN_INCLUDED_PROTOMSET_H

#include "api/enquireinternal.h"
#include "api/result.h"
#include "collapser.h"
#include "heap.h"
#include "matchtimeout.h"
#include "msetcmp.h"
#include "omassert.h"
#include "spymaster.h"
#include "stdclamp.h"

#include <algorithm>

using Xapian::Internal::intrusive_ptr;

class ProtoMSet {
    /// Adapt MSetCmp to be usable with min_heap.
    class MCmpAdaptor {
	ProtoMSet* protomset;

      public:
	explicit MCmpAdaptor(ProtoMSet* protomset_) : protomset(protomset_) {}

	bool operator()(Xapian::doccount a, Xapian::doccount b) const {
	    return protomset->mcmp(protomset->results[a],
				   protomset->results[b]);
	}
    };

    friend class MCmpAdaptor;

    /** Maximum size the ProtoMSet needs to grow to.
     *
     *  This is the maximum rank we care about.
     */
    Xapian::doccount max_size;

    Xapian::doccount check_at_least;

    Xapian::Enquire::Internal::sort_setting sort_by;

    MSetCmp mcmp;

    /** Minimum threshold on the weight.
     *
     *  If the primary result ordering is by decreasing relevance (i.e. @a
     *  sort_by is REL or REL_VAL) then once the min_heap kicks in this
     *  threshold is raised to the lowest weight in the proto-mset.
     *
     *  Enquire::set_cutoff() can also affect min_weight - an absolute
     *  threshold determines the initial value; a percentage threshold raises
     *  the threshold each time max_weight increases (unless it's already
     *  higher than the value the percentage threshold results in).
     */
    double min_weight = 0.0;

    /** The highest document weight seen.
     *
     * This weight may not actually be present in @a results if we're not
     * sorting primarily by relevance, or if min_weight > max_weight.
     */
    double max_weight = 0.0;

    bool min_weight_pending = false;

    /** Count of how many known matching documents have been processed so far.
     *
     *  Used to implement "check_at_least".
     */
    Xapian::doccount known_matching_docs = 0;

    /// The items in the proto-MSet.
    std::vector<Result> results;

    /** A heap of offsets into @a results.
     *
     *  Created lazily once we actually need it.
     */
    std::vector<Xapian::doccount> min_heap;

    /// First entry wanted in MSet.
    Xapian::doccount first;

    /** How many weighted leaf subqueries there are.
     *
     *  Used for scaling percentages when the highest weighted document doesn't
     *  "match all terms".
     */
    Xapian::termcount total_subqs;

    /// The number of subqueries which matched to give max_weight.
    Xapian::termcount max_weight_subqs_matched = 0;

    int percent_threshold;

    double percent_threshold_factor;

    double percent_scale = 0.0;

    PostListTree& pltree;

    Collapser collapser;

    double max_possible;

    bool stop_once_full;

    TimeOut timeout;

  public:
    ProtoMSet(Xapian::doccount first_,
	      Xapian::doccount max_items,
	      Xapian::doccount check_at_least_,
	      MSetCmp mcmp_,
	      Xapian::Enquire::Internal::sort_setting sort_by_,
	      Xapian::termcount total_subqs_,
	      PostListTree& pltree_,
	      Xapian::valueno collapse_key,
	      Xapian::doccount collapse_max,
	      int percent_threshold_,
	      double percent_threshold_factor_,
	      double max_possible_,
	      bool stop_once_full_,
	      double time_limit)
	: max_size(first_ + max_items),
	  check_at_least(check_at_least_),
	  sort_by(sort_by_),
	  mcmp(mcmp_),
	  first(first_),
	  total_subqs(total_subqs_),
	  percent_threshold(percent_threshold_),
	  percent_threshold_factor(percent_threshold_factor_),
	  pltree(pltree_),
	  collapser(collapse_key, collapse_max, results, mcmp),
	  max_possible(max_possible_),
	  stop_once_full(stop_once_full_),
	  timeout(time_limit)
    {
	results.reserve(max_size);
    }

    ProtoMSet(const ProtoMSet&) = delete;

    ProtoMSet& operator=(const ProtoMSet&) = delete;

    Collapser& get_collapser() { return collapser; }

    bool full() const { return results.size() == max_size; }

    double get_min_weight() const { return min_weight; }

    void update_max_weight(double weight) {
	if (weight <= max_weight)
	    return;

	max_weight = weight;
	max_weight_subqs_matched = pltree.count_matching_subqs();
	if (percent_threshold) {
	    set_new_min_weight(weight * percent_threshold_factor);
	}
    }

    bool checked_enough() {
	if (known_matching_docs >= check_at_least) {
	    return true;
	}
	if (known_matching_docs >= max_size && timeout.timed_out()) {
	    check_at_least = max_size;
	    return true;
	}
	return false;
    }

    /** Resolve a pending min_weight change.
     *
     *  Only called when there's a percentage weight cut-off.
     */
    bool handle_min_weight_pending(bool finalising = false) {
	// min_weight_pending shouldn't get set when unweighted.
	Assert(sort_by != Xapian::Enquire::Internal::DOCID);
	min_weight_pending = false;
	bool weight_first = (sort_by == Xapian::Enquire::Internal::REL ||
			     sort_by == Xapian::Enquire::Internal::REL_VAL);
	double new_min_weight = HUGE_VAL;
	size_t j = 0;
	size_t min_elt = 0;
	for (size_t i = 0; i != results.size(); ++i) {
	    if (results[i].get_weight() < min_weight) {
		continue;
	    }
	    if (i != j) {
		results[j] = std::move(results[i]);
		if (collapser) {
		    collapser.result_has_moved(i, j);
		}
	    }
	    if (weight_first && results[j].get_weight() < new_min_weight) {
		new_min_weight = results[j].get_weight();
		min_elt = j;
	    }
	    ++j;
	}
	if (weight_first) {
	    if (finalising) {
		if (known_matching_docs >= check_at_least)
		    min_weight = new_min_weight;
	    } else {
		if (checked_enough())
		    min_weight = new_min_weight;
	    }
	}
	if (j != results.size()) {
	    results.erase(results.begin() + j, results.end());
	    if (!finalising) {
		return false;
	    }
	}
	if (!finalising && min_elt != 0 && !collapser) {
	    // Install the correct element at the tip of the heap, so
	    // that Heap::make() has less to do.  NB Breaks collapsing.
	    std::swap(results[0], results[min_elt]);
	}
	return true;
    }

    bool early_reject(Result& new_item,
		      bool calculated_weight,
		      SpyMaster& spymaster,
		      const Xapian::Document& doc) {
	if (min_heap.empty())
	    return false;

	// We're sorting by value (in part at least), so compare the item
	// against the lowest currently in the proto-mset.  If sort_by is VAL,
	// then new_item.get_weight() won't be set yet, but that doesn't matter
	// since it's not used by the sort function.
	Xapian::doccount worst_idx = min_heap.front();
	if (mcmp(new_item, results[worst_idx]))
	    return false;

	// The candidate isn't good enough to make the proto-mset, but there
	// are still things we may need to do with it.

	// If we're collapsing, we need to check if this would have been
	// collapsed before incrementing known_matching_docs.
	if (!collapser) {
	    ++known_matching_docs;
	    double weight =
		calculated_weight ? new_item.get_weight() : pltree.get_weight();
	    spymaster(doc, weight);
	    update_max_weight(weight);
	    return true;
	}

	if (checked_enough()) {
	    // We've seen enough items so can drop this one.
	    double weight =
		calculated_weight ? new_item.get_weight() : pltree.get_weight();
	    update_max_weight(weight);
	    return true;
	}

	// We can't drop the item because we need to test whether it would be
	// collapsed.
	return false;
    }

    /** Process new_item.
     *
     *  Conceptually this is "add new_item", but taking into account
     *  collapsing.
     */
    bool process(Result&& new_item,
		 ValueStreamDocument& vsdoc) {
	update_max_weight(new_item.get_weight());

	if (!collapser) {
	    // No collapsing, so just add the item.
	    add(std::move(new_item));
	} else {
	    auto res = collapser.check(new_item, vsdoc);
	    switch (res) {
		case REJECT:
		    return true;

		case REPLACE:
		    // There was a previous item in the collapse tab so the
		    // MSet can't be empty.
		    Assert(!results.empty());

		    // This is one of the best collapse_max potential MSet
		    // entries with this key which we've seen so far.  The
		    // entry with this key which it displaced is still in the
		    // proto-MSet so replace it.
		    replace(collapser.old_item, std::move(new_item));
		    return true;

		default:
		    break;
	    }

	    auto elt = add(std::move(new_item));
	    if (res != EMPTY && elt != Xapian::doccount(-1)) {
		collapser.process(res, elt);
	    }
	}

	if (stop_once_full) {
	    if (full() && checked_enough()) {
		return false;
	    }
	}

	return true;
    }

    // Returns the new item's index, or Xapian::doccount(-1) if not added.
    Xapian::doccount add(Result&& item) {
	++known_matching_docs;

	if (item.get_weight() < min_weight) {
	    return Xapian::doccount(-1);
	}

	if (item.get_weight() > max_weight) {
	    update_max_weight(item.get_weight());
	}

	if (results.size() < max_size) {
	    // We're still filling, or just about to become full.
	    results.push_back(std::move(item));
	    Assert(min_heap.empty());
	    return results.size() - 1;
	}

	if (min_heap.empty()) {
	    // This breaks if we're collapsing because it moves elements around
	    // but can be used if we aren't (and could be for elements with
	    // no collapse key too - FIXME).
	    if (min_weight_pending) {
		if (!handle_min_weight_pending()) {
		    results.push_back(std::move(item));
		    return results.size() - 1;
		}
	    }

	    if (results.size() == 0) {
		// E.g. get_mset(0, 0, 10);
		return Xapian::doccount(-1);
	    }
	    min_heap.reserve(results.size());
	    for (Xapian::doccount i = 0; i != results.size(); ++i)
		min_heap.push_back(i);
	    Heap::make(min_heap.begin(), min_heap.end(), MCmpAdaptor(this));
	    if (sort_by == Xapian::Enquire::Internal::REL ||
		sort_by == Xapian::Enquire::Internal::REL_VAL) {
		if (checked_enough()) {
		    min_weight = results[min_heap.front()].get_weight();
		}
	    }
	}

	Xapian::doccount worst_idx = min_heap.front();
	if (!mcmp(item, results[worst_idx])) {
	    // The new item is less than what we already had.
	    return Xapian::doccount(-1);
	}

	results[worst_idx] = std::move(item);
	Heap::replace(min_heap.begin(), min_heap.end(), MCmpAdaptor(this));
	if (sort_by == Xapian::Enquire::Internal::REL ||
	    sort_by == Xapian::Enquire::Internal::REL_VAL) {
	    if (checked_enough()) {
		min_weight = results[min_heap.front()].get_weight();
	    }
	}
	return worst_idx;
    }

    void replace(Xapian::doccount old_item, Result&& b) {
	results[old_item] = std::move(b);
	if (min_heap.empty())
	    return;

	// We need to find the entry in min_heap corresponding to old_item.
	// The simplest way is just to linear-scan for it, and that's actually
	// fairly efficient as we're just searching for an integer in a
	// vector of integers.  The heap structure means that the lowest ranked
	// entry is first and lower ranked entries will tend to be nearer the
	// start, so intuitively scanning forwards for an entry which we're
	// removing because we found a higher ranking one seems sensible, but
	// I've not actually profiled this.
	auto it = std::find(min_heap.begin(), min_heap.end(), old_item);
	if (rare(it == min_heap.end())) {
	    // min_heap should contain all indices of results.
	    Assert(false);
	    return;
	}

	// siftdown() here is correct (because it's on a min-heap).
	Heap::siftdown(min_heap.begin(), min_heap.end(), it, MCmpAdaptor(this));
    }

    void set_new_min_weight(double min_wt) {
	if (min_wt <= min_weight)
	    return;

	min_weight = min_wt;

	if (results.empty()) {
	    // This method gets called before we start matching to set the
	    // fixed weight_threshold threshold.
	    return;
	}

#if 0
	// FIXME: Is this possible?  set_new_min_weight() from a percentage
	// threshold can't do this...
	if (min_wt > max_weight) {
	    // The new threshold invalidates all current entries.
	    results.resize(0);
	    min_heap.resize(0);
	    return;
	}
#endif

	if (!min_heap.empty()) {
	    // If sorting primarily by weight, we could pop the heap while the
	    // tip's weight is < min_wt, but each pop needs 2*log(n)
	    // comparisons, and then pushing replacements for each of those
	    // items needs log(n) comparisons.
	    //
	    // Instead we just discard the heap - if we need to rebuild it,
	    // that'll require 3*n comparisons.  The break even is about 3
	    // discarded items for n=10 or about 5 for n=100, but we may never
	    // need to rebuild the heap.
	    min_heap.clear();
	}

	// Note that we need to check items against min_weight at some point.
	min_weight_pending = true;
    }

    void finalise_percentages() {
	if (results.empty() || max_weight == 0.0)
	    return;

	percent_scale = max_weight_subqs_matched / double(total_subqs);
	percent_scale /= max_weight;
	Assert(percent_scale > 0);
	if (!percent_threshold) {
	    return;
	}

	// Truncate the results if necessary.
	set_new_min_weight(percent_threshold_factor / percent_scale);
	if (min_weight_pending) {
	    handle_min_weight_pending(true);
	}
    }

    Xapian::MSet finalise(const Xapian::MatchDecider* mdecider,
			  Xapian::doccount matches_lower_bound,
			  Xapian::doccount matches_estimated,
			  Xapian::doccount matches_upper_bound) {
	finalise_percentages();

	AssertRel(matches_estimated, >=, matches_lower_bound);
	AssertRel(matches_estimated, <=, matches_upper_bound);

	Xapian::doccount uncollapsed_lower_bound = matches_lower_bound;
	Xapian::doccount uncollapsed_estimated = matches_estimated;
	Xapian::doccount uncollapsed_upper_bound = matches_upper_bound;

	if (!full()) {
	    // We didn't get all the results requested, so we know that we've
	    // got all there are, and the bounds and estimate are all equal to
	    // that number.
	    matches_lower_bound = results.size();
	    matches_estimated = matches_lower_bound;
	    matches_upper_bound = matches_lower_bound;

	    // And that should equal known_matching_docs, unless a percentage
	    // threshold caused some matches to be excluded.
	    if (!percent_threshold) {
		AssertEq(matches_estimated, known_matching_docs);
	    } else {
		AssertRel(matches_estimated, <=, known_matching_docs);
	    }
	} else if (!collapser && known_matching_docs < check_at_least) {
	    // Similar to the above, but based on known_matching_docs.
	    matches_lower_bound = known_matching_docs;
	    matches_estimated = matches_lower_bound;
	    matches_upper_bound = matches_lower_bound;
	} else {
	    // We can end up scaling the estimate more than once, so collect
	    // the scale factors and apply them in one go to avoid rounding
	    // more than once.
	    double estimate_scale = 1.0;
	    double unique_rate = 1.0;

	    if (collapser) {
		matches_lower_bound = collapser.get_matches_lower_bound();

		Xapian::doccount docs_considered =
		    collapser.get_docs_considered();
		Xapian::doccount dups_ignored = collapser.get_dups_ignored();
		if (docs_considered > 0) {
		    // Scale the estimate by the rate at which we've been
		    // finding unique documents.
		    double unique = double(docs_considered - dups_ignored);
		    unique_rate = unique / double(docs_considered);
		}

		// We can safely reduce the upper bound by the number of
		// duplicates we've ignored.
		matches_upper_bound -= dups_ignored;
	    }

	    if (mdecider) {
		if (!percent_threshold && !collapser) {
		    if (known_matching_docs > matches_lower_bound) {
			// We're not collapsing or doing a percentage
			// threshold, so known_matching_docs is a lower bound
			// on the total number of matches.
			matches_lower_bound = known_matching_docs;
		    }
		}

		Xapian::doccount decider_denied = mdecider->docs_denied_;
		Xapian::doccount decider_considered =
		    mdecider->docs_allowed_ + mdecider->docs_denied_;

		// Scale the estimate by the rate at which the MatchDecider has
		// been accepting documents.
		if (decider_considered > 0) {
		    double accept = double(decider_considered - decider_denied);
		    double accept_rate = accept / double(decider_considered);
		    estimate_scale *= accept_rate;
		}

		// If a document is denied by the MatchDecider, it can't be
		// found to be a duplicate, so it is safe to also reduce the
		// upper bound by the number of documents denied by the
		// MatchDecider.
		matches_upper_bound -= decider_denied;
		if (collapser) uncollapsed_upper_bound -= decider_denied;
	    }

	    if (percent_threshold) {
		// Scale the estimate assuming that document weights are evenly
		// distributed from 0 to the maximum weight seen.
		estimate_scale *= (1.0 - percent_threshold_factor);

		// This is all we can be sure of without additional work.
		matches_lower_bound = results.size();

		if (collapser) {
		    uncollapsed_lower_bound = matches_lower_bound;
		}
	    }

	    if (collapser && estimate_scale != 1.0) {
		uncollapsed_estimated =
		    Xapian::doccount(uncollapsed_estimated * estimate_scale +
				     0.5);
	    }

	    estimate_scale *= unique_rate;

	    if (estimate_scale != 1.0) {
		matches_estimated =
		    Xapian::doccount(matches_estimated * estimate_scale + 0.5);
		if (matches_estimated < matches_lower_bound)
		    matches_estimated = matches_lower_bound;
	    }

	    if (collapser || mdecider) {
		// Clamp the estimate the range given by the bounds.
		AssertRel(matches_lower_bound, <=, matches_upper_bound);
		matches_estimated = STD_CLAMP(matches_estimated,
					      matches_lower_bound,
					      matches_upper_bound);
	    } else if (!percent_threshold) {
		AssertRel(known_matching_docs, <=, matches_upper_bound);
		if (known_matching_docs > matches_lower_bound)
		    matches_lower_bound = known_matching_docs;
		if (known_matching_docs > matches_estimated)
		    matches_estimated = known_matching_docs;
	    }

	    if (collapser && !mdecider && !percent_threshold) {
		AssertRel(known_matching_docs, <=, uncollapsed_upper_bound);
		if (known_matching_docs > uncollapsed_lower_bound)
		    uncollapsed_lower_bound = known_matching_docs;
	    }
	}

	if (collapser && matches_lower_bound > uncollapsed_lower_bound) {
	    // Clamp the uncollapsed bound to be at least the collapsed one.
	    uncollapsed_lower_bound = matches_lower_bound;
	}

	if (collapser) {
	    // Clamp the estimate to lie within the known bounds.
	    if (uncollapsed_estimated < uncollapsed_lower_bound) {
		uncollapsed_estimated = uncollapsed_lower_bound;
	    } else if (uncollapsed_estimated > uncollapsed_upper_bound) {
		uncollapsed_estimated = uncollapsed_upper_bound;
	    }
	} else {
	    // When not collapsing the uncollapsed bounds are just the same.
	    uncollapsed_lower_bound = matches_lower_bound;
	    uncollapsed_estimated = matches_estimated;
	    uncollapsed_upper_bound = matches_upper_bound;
	}

	// FIXME: Profile using min_heap here (when it's been created) to
	// handle "first" and perform the sort.
	if (first != 0) {
	    if (first > results.size()) {
		results.clear();
	    } else {
		// We perform nth_element() on reverse iterators so that the
		// unwanted elements end up at the end of items, which means
		// that the call to erase() to remove them doesn't have to copy
		// any elements.
		auto nth = results.rbegin() + first;
		std::nth_element(results.rbegin(), nth, results.rend(), mcmp);
		// Discard the unwanted elements.
		results.erase(results.end() - first, results.end());
	    }
	}

	std::sort(results.begin(), results.end(), mcmp);

	collapser.finalise(min_weight, percent_threshold);

	// The estimates should lie between the bounds.
	AssertRel(matches_lower_bound, <=, matches_estimated);
	AssertRel(matches_estimated, <=, matches_upper_bound);
	AssertRel(uncollapsed_lower_bound, <=, uncollapsed_estimated);
	AssertRel(uncollapsed_estimated, <=, uncollapsed_upper_bound);

	// Collapsing should only reduce the bounds and estimate.
	AssertRel(matches_lower_bound, <=, uncollapsed_lower_bound);
	AssertRel(matches_estimated, <=, uncollapsed_estimated);
	AssertRel(matches_upper_bound, <=, uncollapsed_upper_bound);

	return Xapian::MSet(new Xapian::MSet::Internal(first,
						       matches_upper_bound,
						       matches_lower_bound,
						       matches_estimated,
						       uncollapsed_upper_bound,
						       uncollapsed_lower_bound,
						       uncollapsed_estimated,
						       max_possible,
						       max_weight,
						       std::move(results),
						       percent_scale * 100.0));
    }
};

#endif // XAPIAN_INCLUDED_PROTOMSET_H
