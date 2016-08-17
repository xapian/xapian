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

Letor::Letor(const Xapian::Database & db, Xapian::Ranker * ranker) {
    internal = new Letor::Internal();
    internal->letor_db = db;
    if (ranker == 0) {
	internal->ranker = new ListNETRanker();
    }
    else {
	internal->ranker = ranker;
    }
}

Letor::Letor(const Xapian::Database & db, const Xapian::Query & query, Xapian::Ranker * ranker) {
    internal = new Letor::Internal();
    internal->letor_db = db;
    internal->letor_query = query;
    if (ranker == 0) {
	internal->ranker = new ListNETRanker();
    }
    else {
	internal->ranker = ranker;
    }
}

Letor::~Letor() {
    delete internal->ranker;
}

void
Letor::set_database(const Xapian::Database & db) {
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query) {
    internal->letor_query = query;
}

std::vector<Xapian::docid>
Letor::letor_rank(const Xapian::MSet & mset, const char* model_filename, Xapian::FeatureList & flist) {
    return internal->letor_rank(mset, model_filename, flist);
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
Letor::set_ranker(Xapian::Ranker * ranker) {
    internal->ranker = ranker;
}

void
Letor::set_scorer(Xapian::Scorer * scorer) {
    internal->scorer = scorer;
}

void
Letor::letor_score(const std::string & query_file,
		   const std::string & qrel_file,
		   const std::string & model_file,
		   Xapian::doccount msetsize,
		   Xapian::FeatureList & flist) {

    internal->letor_score(query_file, qrel_file, model_file, msetsize, flist);
}

}
