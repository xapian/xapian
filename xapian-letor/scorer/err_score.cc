/** @file err_score.cc
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
	throw Xapian::InvalidArgumentError("Empty argument supplied");
    }
    int length = fvv.size();

    /* used to store values which change from labels to
     * satisfaction probability
     */
    double intermediate_values[length];
    double max_label = fvv[0].get_label();

    // store the labels set	by the user in intermediate_values.
    for (int i = 0; i < length; ++i) {
	double label = fvv[i].get_label();
	intermediate_values[i] = label;
	max_label = max(max_label, label);
    }

    // Accumulated probability, which is updated for each document.
    double p = 1;
    double err_score = 0;
    int max_value = exp2(max_label);
    for (int rank = 1; rank <= length; ++rank) {

	/* Compute the probability of relevance for the document.
	 * Probability of relevance is calculated in accordance with the gain
	 * function for the Discounted Cumulative Gain in the paper:
	 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.74.9057&rep=rep1&type=pdf
	 */
	intermediate_values[rank - 1] = (exp2(intermediate_values[rank - 1]) -
					 1) / max_value;

       /* err_score = summation over all the documents
	* ((satisfaction probability * p) / rank).
	* Expected Reciprocal Rank(err_score) is calculated in accordance with
	* algorithm 2 in the paper http://olivier.chapelle.cc/pub/err.pdf
	* The paper assumes discrete relevances but continous relevances can
	* be scored using this scorer.
	*/
	err_score = err_score + (p * intermediate_values[rank - 1] / rank);
	p = p * (1 - intermediate_values[rank - 1]);
    }

    return err_score;
}
