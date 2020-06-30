/** @file mset.cc
 * @brief Xapian::MSet class
 */
/* Copyright (C) 2017 Olly Betts
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

#include "msetinternal.h"
#include "xapian/mset.h"

#include "net/serialise.h"
#include "matcher/msetcmp.h"
#include "pack.h"
#include "roundestimate.h"
#include "serialise-double.h"
#include "str.h"
#include "unicode/description_append.h"

#include <algorithm>
#include <cfloat>
#include <string>

using namespace std;

namespace Xapian {

MSet::MSet(const MSet&) = default;

MSet&
MSet::operator=(const MSet&) = default;

MSet::MSet(MSet&&) = default;

MSet&
MSet::operator=(MSet&&) = default;

MSet::MSet() : internal(new MSet::Internal) {}

MSet::MSet(Internal* internal_) : internal(internal_) {}

MSet::~MSet() {}

void
MSet::fetch_(Xapian::doccount first, Xapian::doccount last) const
{
    internal->fetch(first, last);
}

void
MSet::set_item_weight(Xapian::doccount i, double weight)
{
    internal->set_item_weight(i, weight);
}

void
MSet::sort_by_relevance()
{
    std::sort(internal->items.begin(), internal->items.end(),
	      get_msetcmp_function(Enquire::Internal::REL, true, false));
}

int
MSet::convert_to_percent(double weight) const
{
    return internal->convert_to_percent(weight);
}

Xapian::doccount
MSet::get_termfreq(const std::string& term) const
{
    // Check the cached data for query terms first.
    Xapian::doccount termfreq;
    if (usual(internal->stats && internal->stats->get_stats(term, termfreq))) {
	return termfreq;
    }

    if (rare(internal->enquire.get() == NULL)) {
	// Consistent with get_termfreq() on an empty database which always
	// returns 0.
	return 0;
    }

    // Fall back to asking the database via enquire.
    return internal->enquire->get_termfreq(term);
}

double
MSet::get_termweight(const std::string& term) const
{
    // A term not in the query has no termweight, so 0.0 makes sense as the
    // answer in such cases.
    double weight = 0.0;
    if (usual(internal->stats)) {
	(void)internal->stats->get_termweight(term, weight);
    }
    return weight;
}

Xapian::doccount
MSet::get_firstitem() const
{
    return internal->first;
}

Xapian::doccount
MSet::get_matches_lower_bound() const
{
    return internal->matches_lower_bound;
}

Xapian::doccount
MSet::get_matches_estimated() const
{
    // Doing this here avoids calculating if the estimate is never looked at,
    // though does mean we recalculate if this method is called more than once.
    return round_estimate(internal->matches_lower_bound,
			  internal->matches_upper_bound,
			  internal->matches_estimated);
}

Xapian::doccount
MSet::get_matches_upper_bound() const
{
    return internal->matches_upper_bound;
}

Xapian::doccount
MSet::get_uncollapsed_matches_lower_bound() const
{
    return internal->uncollapsed_lower_bound;
}

Xapian::doccount
MSet::get_uncollapsed_matches_estimated() const
{
    // Doing this here avoids calculating if the estimate is never looked at,
    // though does mean we recalculate if this method is called more than once.
    return round_estimate(internal->uncollapsed_lower_bound,
			  internal->uncollapsed_upper_bound,
			  internal->uncollapsed_estimated);
}

Xapian::doccount
MSet::get_uncollapsed_matches_upper_bound() const
{
    return internal->uncollapsed_upper_bound;
}

double
MSet::get_max_attained() const
{
    return internal->max_attained;
}

double
MSet::get_max_possible() const
{
    return internal->max_possible;
}

Xapian::doccount
MSet::size() const
{
    Assert(internal.get());
    return internal->items.size();
}

std::string
MSet::snippet(const std::string& text,
	      size_t length,
	      const Xapian::Stem& stemmer,
	      unsigned flags,
	      const std::string& hi_start,
	      const std::string& hi_end,
	      const std::string& omit) const
{
    // The actual implementation is in queryparser/termgenerator_internal.cc.
    return internal->snippet(text, length, stemmer, flags,
			     hi_start, hi_end, omit);
}

std::string
MSet::get_description() const
{
    return internal->get_description();
}

Document
MSet::Internal::get_document(Xapian::doccount index) const
{
    if (index >= items.size()) {
	string msg = "Requested index ";
	msg += str(index);
	msg += " in MSet of size ";
	msg += str(items.size());
	throw Xapian::RangeError(msg);
    }
    Assert(enquire.get());
    return enquire->get_document(items[index].get_docid());
}

void
MSet::Internal::fetch(Xapian::doccount first_, Xapian::doccount last) const
{
    if (items.empty() || enquire.get() == NULL) {
	return;
    }
    if (last > items.size() - 1) {
	last = items.size() - 1;
    }
    if (first_ <= last) {
	Xapian::doccount n = last - first_;
	for (Xapian::doccount i = 0; i <= n; ++i) {
	    enquire->request_document(items[i].get_docid());
	}
    }
}

void
MSet::Internal::set_item_weight(Xapian::doccount i, double weight)
{
    // max_attained is updated assuming that set_item_weight is called on every
    // MSet item from 0 up. While assigning new weights max_attained is updated
    // as the maximum of the new weights set till Xapian::doccount i.
    if (i == 0)
	max_attained = weight;
    else
	max_attained = max(max_attained, weight);
    // Ideally the max_possible should be the maximum possible weight that
    // can be assigned by the reranking algorithm, but since it is not always
    // possible to calculate the max possible weight for a reranking algorithm
    // we use this approach.
    max_possible = max(max_possible, max_attained);
    items[i].set_weight(weight);
}

int
MSet::Internal::convert_to_percent(double weight) const
{
    int percent;
    if (percent_scale_factor == 0.0) {
	// For an unweighted search, give all matches 100%.
	percent = 100;
    } else if (weight <= 0.0) {
	// Some weighting schemes can return zero relevance while matching,
	// so give such matches 0%.
	percent = 0;
    } else {
	// Adding on 100 * DBL_EPSILON was a hack to work around excess
	// precision (e.g. on x86 when not using SSE), but this code seems like
	// it's generally asking for problems with floating point rounding
	// issues - maybe we ought to carry through the matching and total
	// number of subqueries and calculate using those instead.
	//
	// There are corresponding hacks in matcher/matcher.cc.
	percent = int(weight * percent_scale_factor + 100.0 * DBL_EPSILON);
	if (percent <= 0) {
	    // Make any non-zero weight give a non-zero percentage.
	    percent = 1;
	} else if (percent > 100) {
	    // Make sure we don't ever exceed 100%.
	    percent = 100;
	}
	// FIXME: Ideally we should also make sure any non-exact match gives
	// < 100%.
    }
    return percent;
}

void
MSet::Internal::unshard_docids(Xapian::doccount shard,
			       Xapian::doccount n_shards)
{
    for (auto& result : items) {
	result.unshard_docid(shard, n_shards);
    }
}

void
MSet::Internal::merge_stats(const Internal* o, bool collapsing)
{
    if (snippet_bg_relevance.empty()) {
	snippet_bg_relevance = o->snippet_bg_relevance;
    } else {
	Assert(snippet_bg_relevance == o->snippet_bg_relevance);
    }
    if (collapsing) {
	matches_lower_bound = max(matches_lower_bound, o->matches_lower_bound);
	// matches_estimated will get adjusted later in this case.
    } else {
	matches_lower_bound += o->matches_lower_bound;
    }
    matches_estimated += o->matches_estimated;
    matches_upper_bound += o->matches_upper_bound;
    uncollapsed_lower_bound += o->uncollapsed_lower_bound;
    uncollapsed_estimated += o->uncollapsed_estimated;
    uncollapsed_upper_bound += o->uncollapsed_upper_bound;
    max_possible = max(max_possible, o->max_possible);
    if (o->max_attained > max_attained) {
	max_attained = o->max_attained;
	percent_scale_factor = o->percent_scale_factor;
    }
}

string
MSet::Internal::serialise() const
{
    string result;

    result += serialise_double(max_possible);
    result += serialise_double(max_attained);

    result += serialise_double(percent_scale_factor);

    pack_uint(result, first);
    // Send back the raw matches_* values.  MSet::get_matches_estimated()
    // rounds the estimate lazily, but when we merge MSet objects we really
    // want to merge based on the raw estimates.
    //
    // It is also cleaner that a round-trip through serialisation gives you an
    // object which is as close to the original as possible.
    pack_uint(result, matches_lower_bound);
    pack_uint(result, matches_estimated);
    pack_uint(result, matches_upper_bound);
    pack_uint(result, uncollapsed_lower_bound);
    pack_uint(result, uncollapsed_estimated);
    pack_uint(result, uncollapsed_upper_bound);

    pack_uint(result, items.size());
    for (auto&& item : items) {
	result += serialise_double(item.get_weight());
	pack_uint(result, item.get_docid());
	pack_string(result, item.get_sort_key());
	pack_string(result, item.get_collapse_key());
	pack_uint(result, item.get_collapse_count());
    }

    if (stats)
	result += serialise_stats(*stats);

    return result;
}

void
MSet::Internal::unserialise(const char * p, const char * p_end)
{
    items.clear();

    max_possible = unserialise_double(&p, p_end);
    max_attained = unserialise_double(&p, p_end);

    percent_scale_factor = unserialise_double(&p, p_end);

    size_t msize;
    if (!unpack_uint(&p, p_end, &first) ||
	!unpack_uint(&p, p_end, &matches_lower_bound) ||
	!unpack_uint(&p, p_end, &matches_estimated) ||
	!unpack_uint(&p, p_end, &matches_upper_bound) ||
	!unpack_uint(&p, p_end, &uncollapsed_lower_bound) ||
	!unpack_uint(&p, p_end, &uncollapsed_estimated) ||
	!unpack_uint(&p, p_end, &uncollapsed_upper_bound) ||
	!unpack_uint(&p, p_end, &msize)) {
	unpack_throw_serialisation_error(p);
    }
    while (msize-- > 0) {
	double wt = unserialise_double(&p, p_end);
	Xapian::docid did;
	string sort_key, key;
	Xapian::doccount collapse_cnt;
	if (!unpack_uint(&p, p_end, &did) ||
	    !unpack_string(&p, p_end, sort_key) ||
	    !unpack_string(&p, p_end, key) ||
	    !unpack_uint(&p, p_end, &collapse_cnt)) {
	    unpack_throw_serialisation_error(p);
	}
	items.emplace_back(wt, did, std::move(key), collapse_cnt,
			   std::move(sort_key));
    }

    if (p != p_end) {
	stats.reset(new Xapian::Weight::Internal());
	unserialise_stats(string(p, p_end - p), *stats);
    }
}

string
MSet::Internal::get_description() const
{
    string desc = "MSet(matches_lower_bound=";
    desc += str(matches_lower_bound);
    desc += ", matches_estimated=";
    desc += str(matches_estimated);
    desc += ", matches_upper_bound=";
    desc += str(matches_upper_bound);
    if (uncollapsed_lower_bound != matches_lower_bound) {
	desc += ", uncollapsed_lower_bound=";
	desc += str(uncollapsed_lower_bound);
    }
    if (uncollapsed_estimated != matches_estimated) {
	desc += ", uncollapsed_estimated=";
	desc += str(uncollapsed_estimated);
    }
    if (uncollapsed_upper_bound != matches_upper_bound) {
	desc += ", uncollapsed_upper_bound=";
	desc += str(uncollapsed_upper_bound);
    }
    if (first != 0) {
	desc += ", first=";
	desc += str(first);
    }
    if (max_possible > 0) {
	desc += ", max_possible=";
	desc += str(max_possible);
    }
    if (max_attained > 0) {
	desc += ", max_attained=";
	desc += str(max_attained);
    }
    desc += ", [";
    bool comma = false;
    for (auto&& item : items) {
	if (comma) {
	    desc += ", ";
	} else {
	    comma = true;
	}
	desc += item.get_description();
    }
    desc += "])";
    return desc;
}

}
