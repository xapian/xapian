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

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using namespace Xapian;
using namespace std;

Diversify::Diversify(unsigned int k_,
		     double lambda_,
		     double b_,
		     double sigma_sqr_)
    : k(k_), lambda(lambda_), b(b_), sigma_sqr(sigma_sqr_)
{
    LOGCALL_CTOR(API, "Diversify", k_ | lambda_ | b_ | sigma_sqr_);
}

string
Diversify::get_description() const
{
    return "Diversify()";
}

void
Diversify::initialise_points(const MSet& source)
{
    LOGCALL_VOID(API, "Diversify::initialise_points", source);
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

pair<Xapian::docid, Xapian::docid>
Diversify::get_key(const Xapian::docid& docid_a, const Xapian::docid& docid_b)
{
    pair<Xapian::docid, Xapian::docid> key;
    if (docid_a <= docid_b) {
	key = make_pair(docid_a, docid_b);
    } else {
	key = make_pair(docid_b, docid_a);
    }

    return key;
}

void
Diversify::compute_similarities()
{
    LOGCALL_VOID(API, "Diversify::compute_similarities", NO_ARGS);
    Xapian::CosineDistance d;
    for (auto p_a : points) {
	Xapian::docid pointid_a = p_a.first;
	const Xapian::Point& point_a = p_a.second;
	for (auto p_b : points) {
	    Xapian::docid pointid_b = p_b.first;
	    const Xapian::Point& point_b = p_b.second;

	    if (pointid_a > pointid_b) {
		continue;
	    }

	    double sim = d.similarity(point_a, point_b);
	    auto key = get_key(pointid_a, pointid_b);
	    pairwise_sim[key] = sim;
	}
    }
}

vector<Xapian::docid>
Diversify::compute_diff_dmset(const vector<Xapian::docid>& dmset)
{
    LOGCALL(API, vector<Xapian::docid>, "Diversify::compute_diff_dmset", dmset);
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
Diversify::evaluate_dmset(const vector<Xapian::docid>& dmset)
{
    LOGCALL(API, double, "Diversify::evaluate_dmset", dmset);
    double score_1 = 0, score_2 = 0;

    for (auto doc_id : dmset)
	score_1 += scores[doc_id];

    vector<Xapian::docid> diff_dmset = compute_diff_dmset(dmset);

    for (auto point_id : diff_dmset) {
	double min_dist = numeric_limits<double>::max();
	unsigned int pos = 1;
	for (auto doc_id : dmset) {
	    auto key = get_key(point_id, doc_id);
	    double sim = pairwise_sim[key];
	    double weight = 2 * b * sigma_sqr * (1 / log(1 + pos)) * (1 - sim);
	    min_dist = min(min_dist, weight);
	    ++pos;
	}
	score_2 += min_dist;
    }

    return -lambda * score_1 + (1 - lambda) * score_2;
}

DocumentSet
Diversify::get_dmset(const MSet& mset)
{
    LOGCALL(API, MSet, "Diversify::get_dmset", mset);
    // Return original mset if no need to diversify
    if (k == 0 || mset.size() <= 2) {
	DocumentSet dmset;
	for (MSetIterator it = mset.begin(); it != mset.end(); ++it)
	    dmset.add_document(it.get_document());
	return dmset;
    }

    if (k > mset.size())
	k = mset.size();

    initialise_points(mset);
    compute_similarities();

    vector<Xapian::docid> curr_dmset = main_dmset;

    while (true) {
	bool found_better_dmset = false;
	for (unsigned int i = 0; i < main_dmset.size(); ++i) {
	    auto curr_doc = main_dmset[i];
	    double best_score = evaluate_dmset(curr_dmset);

	    vector<Xapian::docid> diff_dmset = compute_diff_dmset(curr_dmset);

	    bool found_better_doc = false;
	    for (unsigned int j = 0; j < diff_dmset.size(); ++j) {
		vector<Xapian::docid> temp_dmset = curr_dmset;
		temp_dmset[i] = diff_dmset[j];
		double score = evaluate_dmset(temp_dmset);
		if (score < best_score) {
		    curr_doc = temp_dmset[i];
		    best_score = score;
		    found_better_doc = true;
		}
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
