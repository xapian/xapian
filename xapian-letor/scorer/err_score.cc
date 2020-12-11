/** @file
 *  @brief Implementation of ERRScore
 *
 *  ERR Score is adapted from the paper: http://olivier.chapelle.cc/pub/err.pdf
 *  Chapelle, Metzler, Zhang, Grinspan (2009)
 *  Expected Reciprocal Rank for Graded Relevance
 */
/* Copyright (C) 2014 Hanxiao Sun
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

#include "xapian-letor/scorer.h"

#include "debuglog.h"

#include <algorithm>
#include <cmath>

using namespace std;

using namespace Xapian;

ERRScore::ERRScore()
{
    LOGCALL_CTOR(API, "ERRScore", NO_ARGS);
}

ERRScore::~ERRScore()
{
    LOGCALL_DTOR(API, "ERRScore");
}

double
ERRScore::score(const std::vector<FeatureVector> & fvv) const
{
    LOGCALL(API, double, "ERRScore::score", fvv);
    if (fvv.empty()) {
	return 0;
    }
    size_t length = fvv.size();
    FeatureVector max_label = *max_element(fvv.begin(), fvv.end(),
					   [](FeatureVector x,
					      FeatureVector y) {
						  return x.get_label() <
							 y.get_label();
					      });
    double max_value = exp2(max_label.get_label());

    // Accumulated probability, which is updated for each document.
    double p = 1;
    double err_score = 0;
    for (size_t rank = 1; rank <= length; ++rank) {
	/* Compute the probability of relevance for the document.
	 * Probability of relevance is calculated in accordance with the gain
	 * function for the Discounted Cumulative Gain in the paper:
	 * https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.74.9057&rep=rep1&type=pdf
	 */
	auto label = fvv[rank - 1].get_label();
	double relevance_probability = (exp2(label) - 1) / max_value;

	/* err_score = summation over all the documents
	* ((satisfaction probability * p) / rank).
	* Expected Reciprocal Rank(err_score) is calculated in accordance with
	* algorithm 2 in the paper http://olivier.chapelle.cc/pub/err.pdf
	* The paper assumes discrete relevances but continuous relevances can
	* be scored using this scorer.
	*/
	err_score = err_score + (p * relevance_probability / rank);
	p = p * (1 - relevance_probability);
    }

    return err_score;
}
