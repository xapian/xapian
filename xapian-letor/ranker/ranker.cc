/** @file ranker.cc
 * @brief Implementation of Ranker class
 */
/* Copyright (C) 2012 Parth Gupta
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

#include "xapian-letor/ranker.h"
#include "debuglog.h"

using namespace std;
using namespace Xapian;

Ranker::Ranker() {
    LOGCALL_CTOR(API, "Ranker", NO_ARGS);
}

Ranker::~Ranker() {
    LOGCALL_DTOR(API, "Ranker");
}

bool
Ranker::scorecomparer(const FeatureVector & firstfv, const FeatureVector& secondfv) {
    LOGCALL(API, bool, "Ranker::scorecomparer", firstfv | secondfv);
    return firstfv.get_score() > secondfv.get_score();
}

bool
Ranker::labelcomparer(const FeatureVector & firstfv, const FeatureVector& secondfv) {
    LOGCALL(API, bool, "Ranker::labelcomparer", firstfv | secondfv);
    return firstfv.get_label() > secondfv.get_label();
}

std::vector<Xapian::FeatureVector>
Ranker::get_traindata(){
    LOGCALL(API, std::vector<FeatureVector>, "Ranker::get_traindata", NO_ARGS);
    return traindata;
}

void
Ranker::set_training_data(vector<Xapian::FeatureVector> training_data) {
    LOGCALL_VOID(API, "Ranker::set_training_data", training_data);
    traindata = training_data;
}
