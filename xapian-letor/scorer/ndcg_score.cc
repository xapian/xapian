/** @file
 *  @brief Implementation of NDCGScore
 */
/* Copyright (C) 2014 Hanxiao Sun
 * Copyright (C) 2016 Ayush Tomar
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
#include "common/log2.h"
#include "omassert.h"

#include <algorithm>
#include <cmath>

using namespace std;
using namespace Xapian;

NDCGScore::NDCGScore()
{
    LOGCALL_CTOR(API, "NDCGScore", NO_ARGS);
}

NDCGScore::~NDCGScore()
{
    LOGCALL_DTOR(API, "NDCGScore");
}

static double
get_dcg(const std::vector<double> &labels)
{
    LOGCALL_STATIC(API, double, "get_dcg", labels);
    double dcg = 0;
    for (int i = 0; i < int(labels.size()); ++i) {
	dcg += (exp2(labels[i]) - 1) / log2(i + 2);
    }
    return dcg;
}

double
NDCGScore::score(const std::vector<FeatureVector> & fvv) const {
    LOGCALL(API, double, "NDCGScore::score", fvv);
    std::vector<double> labels;
    for (auto&& v : fvv) {
	labels.push_back(v.get_label());
    }
    // DCG score of original ranking
    double dcg = get_dcg(labels);
    if (rare(dcg == 0.0)) {
	// Avoid dividing by 0.
	return dcg;
    }
    // DCG score of ideal ranking
    sort(labels.begin(), labels.end(), std::greater<double>());
    double idcg = get_dcg(labels);
    AssertRel(idcg, >=, dcg);
    return dcg / idcg;
}
