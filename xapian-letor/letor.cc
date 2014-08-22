/* letor.cc: Letor provides weighting scheme based on Learning to Rank. Note: letor.h is
 * not a part of official stable Xapian API.
 *
 * Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2012 Olly Betts
 * Copyright (C) 2014 Jiarong Wei
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include <xapian/letor.h>

#include "letor_internal.h"
#include "ranker.h"
#include "svmranker.h"
#include "normalizer.h"
#include "default_normalizer.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

Letor &
Letor::operator=(const Letor & o) {
    internal = o.internal;
    return *this;
}


Letor::Letor(const Letor & o) : internal(o.internal) {}


Letor::Letor() : internal(new Letor::Internal) {
    internal->init();
}


Letor::~Letor() {}


void
Letor::set_database(Xapian::Database & database_) {
    internal->set_database(database_);
}


void
Letor::set_features(const vector<Feature::feature_t> & features_) {
    internal->set_features(features_);
}


void
Letor::set_ranker(const Ranker::ranker_t ranker_flag) {
    switch (ranker_flag) {
        case Ranker::SVM_RANKER:
            internal->ranker = new Xapian::SVMRanker();
            break;
        default:
            printf("Ranker flag error!\n");
            exit(1);
    }
}


void
Letor::set_normalizer(const Normalizer::normalizer_t normalizer_flag) {
    switch (normalizer_flag) {
        case Normalizer::DEFAULT_NORMALIZER:
            internal->set_normalizer(new Xapian::DefaultNormalizer());
            break;
        default:
            printf("Normalizer flag error!\n");
            exit(1);
    }
}


void
Letor::prepare_training_file(const string query_file, const string qrel_file, const string output_file, Xapian::doccount mset_size) {
    internal->prepare_training_file(query_file, qrel_file, output_file, mset_size);
}


void
Letor::load_model_file(const string model_file_) {
    internal->load_model_file(model_file_);
}


void
Letor::update_mset(Xapian::Query & query_, Xapian::MSet & mset_) {
    internal->update_mset(query_, mset_);
}


void
Letor::train(const string training_data_file_, const string model_file_) {
    internal->train(training_data_file_, model_file_);
}

}