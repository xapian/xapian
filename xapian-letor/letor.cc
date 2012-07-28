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
#include "ranklist.h"

#include <map>
#include <string>

using namespace std;

namespace Xapian {

Letor::Letor(const Letor & o) : internal(o.internal) { }

Letor &
Letor::operator=(const Letor & o)
{
    internal = o.internal;
    return *this;
}

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

map<Xapian::docid, double>
Letor::letor_score(const Xapian::MSet & mset) {
    return internal->letor_score(mset);
}

void
Letor::letor_learn_model() {
    internal->letor_learn_model();
}

void
Letor::prepare_training_file(const string & query_file, const string & qrel_file, Xapian::doccount msetsize) {
    internal->prepare_training_file(query_file, qrel_file, msetsize);
}

void
Letor::prepare_training_file_listwise(const string & query_file, int num_features) {
    internal->prepare_training_file_listwise(query_file, num_features);
}

void
Letor::create_ranker(int ranker_type) {
    switch(ranker_type) {
        case 0: internal->ranker = * new SVMRanker;
                break;
        case 1: internal->ranker = * new ListMLE;
		break;
        default: ;//cout<<"Please specify proper ranker.";
    }
}

}
