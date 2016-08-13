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
#include <xapian-letor/letor.h>
#include "letor_internal.h"
#include "xapian-letor/ranker.h"

#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace Xapian {

Letor::Letor() : internal(new Letor::Internal) { }

Letor::~Letor() { }


void
Letor::set_database(const Xapian::Database & db) {
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query) {
    internal->letor_query = query;
}

std::vector<Xapian::docid>
Letor::letor_rank(const Xapian::MSet & mset, Xapian::FeatureList & flist, const char* model_filename) {
    return internal->letor_rank(mset, flist, model_filename);
}

void
Letor::letor_learn_model(const char* input_filename, const char* output_filename) {
    internal->letor_learn_model(input_filename, output_filename);
}

void
Letor::prepare_training_file(const string & query_file, const string & qrel_file,
			     Xapian::doccount msetsize, const char* filename,
			     Xapian::FeatureList & flist) {

    internal->prepare_training_file(query_file, qrel_file, msetsize, filename, flist);
}

void
Letor::create_ranker(int ranker_type, int metric_type) { //TODO: update when adding rankers
    // switch(ranker_type) {
    //     case 0: internal->ranker = new SVMRanker(metric_type);
    //             cout << "SVMRanker created!" <<endl;
    //             break;
    //     case 1: break;
    //     default:cout<<"Please specify proper ranker.";
    (void)ranker_type;
    (void)metric_type;
}

}
