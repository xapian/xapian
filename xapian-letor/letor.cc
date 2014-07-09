/** @file letor.cc
 * @brief Letor Class
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2012 Olly Betts
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
#include <xapian/letor.h>

#include "letor_internal.h"
#include "ranker.h"
#include "svmranker.h"
#include "listmle.h"
#include "listnet.h"

#include <iostream>
#include <cstdlib>

#include <string>
#include <vcetor>

using namespace Xapian;
using std::string;
using std::vector;

namespace Xapian {

Letor &
Letor::operator=(const Letor & o)
{
    internal = o.internal;
    return *this;
}


Letor::Letor(const Letor & o) : internal(o.internal) {}


Letor::Letor() : internal(new Letor::Internal) {}


Letor::~Letor() {
    if (internal != NULL) {
        delete(internal);
        internal = NULL;
    }
    else
        internal = NULL;
}


void
Letor::update_context(const Xapian::Database & database_, const vector<Xapian::Feature::FeatureBase & features_, const Ranker & ranker_, const Normalizer & normalizer_) {
    internal->feature_manager.update_context(database_, features_);
    internal->feature_manager.set_normalizer(normalizer_);
    internal->ranker = ranker_;
}


void
Letor::Internal::prepare_training_file(const string query_file_, const string qrel_file_, Xapian::doccount mset_size) {
    internal->prepare_training_file(query_file_, qrel_file_, mset_size);
}


void
Letor::load_model_file(string model_file_) {
    internal->load_model_file(model_file_);
}


void
Letor::update_mset(const Xapian::Query & query_, const Xapian::MSet mset_) {
    internal->update_mset(query_, mset_);
}


void
Letor::train(string training_data_file_, string model_file_) {
    internal->train(training_data_file_, model_file_);
}