/** @file
 * @brief Xapian::MSet class
 */
/* Copyright (C) 2017,2024,2025 Olly Betts
 * Copyright (C) 2018 Uppinder Chugh
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

#include "xapian/cluster.h"

#include "backends/documentinternal.h"
#include "net/serialise.h"
#include "matcher/msetcmp.h"
#include "omassert.h"
#include "pack.h"
#include "roundestimate.h"
#include "serialise-double.h"
#include "str.h"
#include "unicode/description_append.h"

#include <algorithm>
#include <cfloat>
#include <string>
#include <string_view>
#include <unordered_set>

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

/** Evaluate a diversified mset
 *
 *  Evaluate a diversified mset using MPT algorithm
 *
 *  @param dmset	Set of points representing candidate diversifed set of
 *			documents.
 *  @param cset		Set of clusters of given MSet.
 */
static double
evaluate_dmset(const vector<Xapian::docid>& dmset,
	       const Xapian::ClusterSet& cset,
	       double factor1,
	       double factor2,
	       const Xapian::MSet& mset,
	       const vector<double>& dissimilarity)
{
    double score_1 = 0, score_2 = 0;

    // FIXME: We could compute score_1 once then adjust for each candidate
    // change.
    // Seems hard to do similar for score_2 though.
    for (auto mset_index : dmset)
	score_1 += mset[mset_index].get_weight();

    auto cset_size = cset.size();
    for (Xapian::doccount c = 0; c < cset_size; ++c) {
	double min_dist = numeric_limits<double>::max();
	unsigned int pos = 1;
	for (auto mset_index : dmset) {
	    // FIXME: Pre-compute 1.0 / log(2.0 + i) for i = [0, dmset.size()) ?
	    double weight = dissimilarity[mset_index * cset_size + c];
	    weight /= log(1.0 + pos);
	    min_dist = min(min_dist, weight);
	    ++pos;
	}
	score_2 += min_dist;
    }

    return factor2 * score_2 - factor1 * score_1;
}

void
MSet::diversify_(Xapian::doccount k,
		 Xapian::doccount r,
		 double factor1,
		 double factor2)
{
    // Ensured by inlined caller.
    AssertRel(k, >=, 2);

    auto mset_size = size();
    if (mset_size <= k) {
	// Picking k documents would pick the whole MSet so nothing to do.
	//
	// Since k >= 2, this means we don't try to diversify an MSet with
	// 2 documents (for which reordering can't usefully improve diversity
	// since the only possible change is to swap the order of the 2
	// documents).
	return;
    }

    /// Store MSet indices of top k diversified documents
    std::vector<Xapian::doccount> main_dmset;
    main_dmset.reserve(k);

    Xapian::doccount count = 0;
    TermListGroup tlg(*this);
    std::vector<Xapian::Point> points;
    points.reserve(mset_size);
    for (MSetIterator it = begin(); it != end(); ++it) {
	Xapian::Document doc = it.get_document();
	doc.internal->set_index(count);
	points.push_back(Xapian::Point(tlg, doc));
	// Initial top-k diversified documents
	if (count < k) {
	    // The initial diversified document set is the top-k documents from
	    // the MSet.
	    main_dmset.push_back(count);
	}
	++count;
    }

    // Cluster the MSet into k clusters.
    Xapian::ClusterSet cset = Xapian::LCDClusterer(k).cluster(*this);

    /** Precompute dissimilarity scores between each document and cluster
     *  centroid.
     *
     *  These scores are:
     *
     *  1.0 - cosine_similarity(docid, cluster_index)
     *
     *  The index into dissimilarity is:
     *
     *    mset_index * number_of_clusters + cluster_index
     */

    // Pre-compute all the dissimilarity values.
    auto cset_size = cset.size();
    std::vector<double> dissimilarity;
    dissimilarity.reserve(cset_size * points.size());
    {
	Xapian::CosineDistance d;
	for (const auto& point : points) {
	    for (unsigned int c = 0; c < cset_size; ++c) {
		double dist = d.similarity(point, cset[c].get_centroid());
		dissimilarity.push_back(1.0 - dist);
	    }
	}
    }

    // Build topc, which contains the union of the top-r relevant documents of
    // each cluster.
    vector<Xapian::docid> topc;
    for (Xapian::doccount c = 0; c < cset_size; ++c) {
	// FIXME: This is supposed to pick the `r` most relevant documents, but
	// actually seems to pick those with the lowest docids.
	auto documents = cset[c].get_documents();
	auto limit = std::min(r, documents.size());
	for (Xapian::doccount d = 0; d < limit; ++d) {
	    auto mset_index = documents[d].internal->get_index();
	    topc.push_back(mset_index);
	}
    }

    vector<Xapian::doccount> curr_dmset = main_dmset;

    while (true) {
	bool found_better_dmset = false;
	for (unsigned int i = 0; i < main_dmset.size(); ++i) {
	    auto curr_doc = main_dmset[i];
	    double best_score = evaluate_dmset(curr_dmset, cset,
					       factor1, factor2,
					       *this, dissimilarity);
	    bool found_better_doc = false;

	    for (unsigned int j = 0; j < topc.size(); ++j) {
		// Continue if candidate document from topc already
		// exists in curr_dmset.  FIXME: Linear search!
		auto candidate_doc = find(curr_dmset.begin(), curr_dmset.end(),
					  topc[j]);
		if (candidate_doc != curr_dmset.end()) {
		    continue;
		}

		auto temp_doc = curr_dmset[i];
		curr_dmset[i] = topc[j];
		double score = evaluate_dmset(curr_dmset, cset,
					      factor1, factor2,
					      *this, dissimilarity);

		if (score < best_score) {
		    curr_doc = curr_dmset[i];
		    best_score = score;
		    found_better_doc = true;
		}

		curr_dmset[i] = temp_doc;
	    }
	    if (found_better_doc) {
		curr_dmset[i] = curr_doc;
		found_better_dmset = true;
	    }
	}

	// Terminate algorithm when there's no change in current
	// document matchset
	if (!found_better_dmset)
	    break;

	main_dmset = curr_dmset;
    }

    // Reorder the results to reflect the diversification.  To do this we need
    // to partition the MSet so the promoted documents come first (in original
    // MSet order), followed by the non-promoted documents (also in original
    // MSet order).
    unordered_set<Xapian::docid> promoted{k};
    for (auto mset_index : main_dmset) {
	promoted.insert(internal->items[mset_index].get_docid());
    }

    stable_partition(internal->items.begin(), internal->items.end(),
		     [&](const Result& result) {
			 return promoted.count(result.get_docid());
		     });
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
MSet::get_termfreq(std::string_view term) const
{
    // Check the cached data for query terms first.
    Xapian::doccount termfreq;
    if (usual(internal->stats && internal->stats->get_stats(term, termfreq))) {
	return termfreq;
    }

    if (rare(!internal->enquire)) {
	// Consistent with get_termfreq() on an empty database which always
	// returns 0.
	return 0;
    }

    // Fall back to asking the database via enquire.
    return internal->enquire->get_termfreq(term);
}

double
MSet::get_termweight(std::string_view term) const
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
    return internal->items.size();
}

std::string
MSet::snippet(std::string_view text,
	      size_t length,
	      const Xapian::Stem& stemmer,
	      unsigned flags,
	      std::string_view hi_start,
	      std::string_view hi_end,
	      std::string_view omit) const
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
    Assert(enquire);
    return enquire->get_document(items[index].get_docid());
}

void
MSet::Internal::fetch(Xapian::doccount first_, Xapian::doccount last) const
{
    if (items.empty() || !enquire) {
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
    for ( ; msize; --msize) {
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
	unserialise_stats(p, p_end, *stats);
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
