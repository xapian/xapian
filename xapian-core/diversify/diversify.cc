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
Diversify::initialise_points(const MSet &source)
{
    LOGCALL_VOID(API, "Diversify::initialise_points", source);
    TermListGroup tlg(source);
    for (MSetIterator it = source.begin(); it != source.end(); ++it) {
	points.push_back(Point(tlg, it.get_document()));
	weights[*it] = it.get_weight();
    }
}

pair<Xapian::docid, Xapian::docid>
Diversify::get_key(const Xapian::Point &doc_a, const Xapian::Point &doc_b)
{
    Xapian::docid docid_a = doc_a.get_document().get_docid();
    Xapian::docid docid_b = doc_b.get_document().get_docid();

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
    for (unsigned int i = 0; i < points.size() - 1; ++i) {
	for (unsigned int j = i + 1; j < points.size(); ++j) {
	    double sim = d.similarity(points[i], points[j]);
	    auto key = get_key(points[i], points[j]);
	    pairwise_sim[key] = sim;
	}
    }
}

vector<Point>
Diversify::compute_diff_dmset(const std::vector<Point> &dmset)
{
    LOGCALL(API, vector<Point>, "Diversify::compute_diff_dmset", dmset);
    vector<Point> diff_dmset;
    for (auto point : points) {
	bool found_point = false;
	Xapian::docid point_docid = point.get_document().get_docid();
	for (auto doc : dmset) {
	    if (point_docid == doc.get_document().get_docid()) {
		found_point = true;
		break;
	    }
	}

	if (!found_point) {
	    diff_dmset.push_back(point);
	}
    }

    return diff_dmset;
}

double
Diversify::evaluate_dmset(const vector<Point> &dmset)
{
    LOGCALL(API, double, "Diversify::evaluate_dmset", dmset);
    double score_1 = 0, score_2 = 0;

    for (auto doc : dmset)
	score_1 += weights[doc.get_document().get_docid()];

    vector<Point> diff_dmset = compute_diff_dmset(dmset);

    for (auto point : diff_dmset) {
	double min_dist = numeric_limits<double>::max();
	unsigned int pos = 1;
	for (auto doc : dmset) {
	    auto key = get_key(point, doc);
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
Diversify::get_dmset(const MSet &mset)
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
    vector<Point> main_dmset(points.begin(), points.begin() + k);
    vector<Point> curr_dmset = main_dmset;

    while (true) {
	bool found_better_dmset = false;
	for (unsigned int i = 0; i < main_dmset.size(); ++i) {
	    auto curr_doc = main_dmset[i];
	    double best_score = evaluate_dmset(curr_dmset);

	    vector<Point> diff_dmset = compute_diff_dmset(curr_dmset);

	    bool found_better_doc = false;
	    for (unsigned int j = 0; j < diff_dmset.size(); ++j) {
		vector<Point> temp_dmset = curr_dmset;
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
    for (auto doc : main_dmset) {
	dmset.add_document(doc.get_document());
    }

    vector<Point> diff_dmset = compute_diff_dmset(main_dmset);
    for (auto doc : diff_dmset)
	dmset.add_document(doc.get_document());

    return dmset;
}
