/** @file err_score.cc
 *  @brief Implementation of ERRScore
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

static vector<double>
get_labels(vector<Xapian::FeatureVector> fvv)
{
    LOGCALL_STATIC(API, vector<double>, "get_labels", fvv);
    size_t fvvsize = fvv.size();
    vector<double> labels;

    for (size_t i = 0; i < fvvsize; ++i) {
	labels.push_back(fvv[i].get_label());
    }

    return labels;
}

/* test the err_Score() use the data from goo.gl/LxEmPZ
 * input:[3,2,4]
 * output:0.63
 */
double
ERRScore::score(const std::vector<FeatureVector> & fvv) const
{
    LOGCALL(API, double, "ERRScore::score", fvv);

    // hard code for the a five-point scale, the 16 means 2^(5-1)
    int MAX_LABEL = 16;

    std::vector<double> labels = get_labels(fvv);
    int length = labels.size();

    // compute the satisfaction probability for lable of each doc in the ranking
    for (int i = 0; i < length; ++i) {
	labels[i] = (pow(2, labels[i]) - 1) / MAX_LABEL;
    }

    double err_score = labels[0];

    // compute the accumulated probability for each doc which user will stop at
    for (int i = 1; i < length; ++i) {

	// single stop probability
	double temp_err = (1.0 / (i + 1.0)) * labels[i];

	// for users
	for (int j = i - 1; j >= 0; --j) {
	    temp_err *= (1 - labels[j]);
	}
	err_score += temp_err;
    }

    return err_score;
}
