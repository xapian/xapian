/** @file diversify.cc
 *  @brief Diversification API
 */
/* Copyright (C) 2018 Uppinder Chugh
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian/diversify.h"
#include "xapian/error.h"

#include "debuglog.h"
#include "diversify/diversifyinternal.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using namespace Xapian;
using namespace std;

Diversify::Diversify(const Diversify&) = default;

Diversify&
Diversify::operator=(const Diversify&) = default;

Diversify::Diversify(Diversify&&) = default;

Diversify&
Diversify::operator=(Diversify&&) = default;

Diversify::Diversify(Xapian::doccount k_,
		     Xapian::doccount r_,
		     double lambda_,
		     double b_,
		     double sigma_sqr_)
    : internal(new Xapian::Diversify::Internal(k_, r_, lambda_, b_, sigma_sqr_))
{
    LOGCALL_CTOR(API, "Diversify", k_ | r_ | lambda_ | b_ | sigma_sqr_);
    if (r_ == 0)
	throw InvalidArgumentError("Value of r should be greater than zero.");
}

Diversify::~Diversify()
{
    LOGCALL_DTOR(API, "Diversify");
}

string
Diversify::get_description() const
{
    return "Diversify()";
}

DocumentSet
Diversify::get_dmset(const MSet& mset)
{
    LOGCALL(API, MSet, "Diversify::get_dmset", mset);
    return internal->get_dmset(mset);
}

void
Diversify::Internal::initialise_points(const MSet& source)
{
    unsigned int count = 0;
    TermListGroup tlg(source);
    for (MSetIterator it = source.begin(); it != source.end(); ++it) {
	points.emplace(*it, Xapian::Point(tlg, it.get_document()));
	scores[*it] = it.get_weight();
	// Initial top-k diversified documents
	if (count < k) {
	    main_dmset.push_back(*it);
	    ++count;
	}
    }
}

pair<Xapian::docid, unsigned int>
Diversify::Internal::get_key(Xapian::docid doc_id, unsigned int centroid_id)
{
    return make_pair(doc_id, centroid_id);
}

void
Diversify::Internal::compute_similarities(const Xapian::ClusterSet& cset)
{
    Xapian::CosineDistance d;
    for (auto p : points) {
	Xapian::docid point_id = p.first;
	Xapian::Point point = p.second;
	for (unsigned int c = 0; c < cset.size(); ++c) {
	    double dist = d.similarity(point, cset[c].get_centroid());
	    auto key = get_key(point_id, c);
	    pairwise_sim[key] = dist;
	}
    }
}

vector<Xapian::docid>
Diversify::Internal::compute_diff_dmset(const vector<Xapian::docid>& dmset)
{
    vector<Xapian::docid> diff_dmset;
    for (auto point : points) {
	Xapian::docid point_id = point.first;
	bool found_point = false;
	for (auto doc_id : dmset) {
	    if (point_id == doc_id) {
		found_point = true;
		break;
	    }
	}

	if (!found_point) {
	    diff_dmset.push_back(point_id);
	}
    }

    return diff_dmset;
}

double
Diversify::Internal::evaluate_dmset(const vector<Xapian::docid>& dmset,
				    const Xapian::ClusterSet& cset)
{
    double score_1 = 0, score_2 = 0;

    for (auto doc_id : dmset)
	score_1 += scores[doc_id];

    for (unsigned int c = 0; c < cset.size(); ++c) {
	double min_dist = numeric_limits<double>::max();
	unsigned int pos = 1;
	for (auto doc_id : dmset) {
	    auto key = get_key(doc_id, c);
	    double sim = pairwise_sim[key];
	    double weight = 2 * b * sigma_sqr / log(1 + pos) * (1 - sim);
	    min_dist = min(min_dist, weight);
	    ++pos;
	}
	score_2 += min_dist;
    }

    return -lambda * score_1 + (1 - lambda) * score_2;
}

DocumentSet
Diversify::Internal::get_dmset(const MSet& mset)
{
    // Return original mset if no need to diversify
    if (k == 0 || mset.size() <= 2) {
	DocumentSet dmset;
	for (MSetIterator it = mset.begin(); it != mset.end(); ++it)
	    dmset.add_document(it.get_document());
	return dmset;
    }

    unsigned int k_ = k;
    if (k_ > mset.size())
	k_ = mset.size();

    initialise_points(mset);

    // Cluster the given mset into k clusters
    Xapian::LCDClusterer lc(k_);
    Xapian::ClusterSet cset = lc.cluster(mset);
    compute_similarities(cset);

    // topC contains union of top-r relevant documents of each cluster
    vector<Xapian::docid> topc;

    // Build topC
    for (unsigned int c = 0; c < cset.size(); ++c) {
	auto documents = cset[c].get_documents();
	for (unsigned int d = 0; d < r && d < documents.size(); ++d) {
	    auto doc_id = documents[d].get_docid();
	    topc.push_back(doc_id);
	}
    }

    vector<Xapian::docid> curr_dmset = main_dmset;

    while (true) {
	bool found_better_dmset = false;
	for (unsigned int i = 0; i < main_dmset.size(); ++i) {
	    auto curr_doc = main_dmset[i];
	    double best_score = evaluate_dmset(curr_dmset, cset);
	    bool found_better_doc = false;

	    for (unsigned int j = 0; j < topc.size(); ++j) {
		// Continue if candidate document from topC already
		// exists in curr_dmset
		auto candidate_doc = find(curr_dmset.begin(), curr_dmset.end(),
					  topc[j]);
		if (candidate_doc != curr_dmset.end()) {
		    continue;
		}

		auto temp_doc = curr_dmset[i];
		curr_dmset[i] = topc[j];
		double score = evaluate_dmset(curr_dmset, cset);

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

    // Merge main_dmset and diff_dmset into final dmset
    DocumentSet dmset;
    for (auto doc_id : main_dmset)
	dmset.add_document(points.at(doc_id).get_document());

    vector<Xapian::docid> diff_dmset = compute_diff_dmset(main_dmset);
    for (auto doc_id : diff_dmset)
	dmset.add_document(points.at(doc_id).get_document());

    return dmset;
}
