/** @file ndcg_score.cc
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

#include "xapian-letor/scorer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
using namespace Xapian;

NDCGScore::NDCGScore() {
    cout << "NDCGScore initialised!" << endl;
}

NDCGScore::~NDCGScore() {
}

static double
get_dcg(const std::vector<double> labels) {

    double dcg = 0;

    for (int i = 0; i < int(labels.size()); i++){
        dcg += (pow(2,labels[i]) - 1)/(log(i+2)/log(2));
    }

    return dcg;

}

double
NDCGScore::score(const std::vector<FeatureVector> & fvv) {

    std::vector<double> labels;

    for (int i = 0; i < int(fvv.size()); i++) {
        labels.push_back(fvv[i].get_label());
    }

    //DCG score of original ranking
    double DCG = get_dcg(labels);

    //DCG score of ideal ranking
    sort(labels.begin(), labels.begin() + labels.size(), std::greater<double>());
    double iDCG = get_dcg(labels);

    if (iDCG == 0) // Don't divide by 0
        return 0;

    return DCG/iDCG;
}
