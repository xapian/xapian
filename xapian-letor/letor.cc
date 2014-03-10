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

#include <map>
#include <string>

using namespace std;

namespace Xapian {

Letor &
Letor::operator=(const Letor & o)
{
    internal = o.internal;
    return *this;
}

Letor::Letor(const Letor & o) : internal(o.internal) {}

Letor::Letor() : internal(new Letor::Internal) {}

Letor::~Letor() {}

void
Letor::set_database(const Xapian::Database & db)
{
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query)
{
    internal->letor_query = query;
}

map<Xapian::docid, double>
Letor::letor_score(const Xapian::MSet & mset)
{
    return internal->letor_score(mset);
}

void
Letor::letor_learn_model()
{
    internal->letor_learn_model();
}

void
Letor::set_training_file_svm(const string & query_f, const string & qrel_f, Xapian::doccount mset_s)
{
    if (ranker_type == RANKER_SVM)
    {
        query_file = query_f;
        qrel_file  = qrel_f;
        mset_size  = mset_s;
    }
    else
    {
        cout << "ranker type is not svm" << endl;
        exit(1);
    }
}

void
Letor::prepare_training_file_svm(const string & query_f, const string & qrel_f, Xapian::doccount mset_s)
{
    internal->prepare_training_file_svm(query_f, qrel_f, mset_s);
}

void
Letor::set_training_file_listwise(const string & query_f, int num_f)
{
    if (ranker_type == RANKER_LISTMLE || ranker_type == RANKER_LISTNET)
    {
        query_file    = query_f;
        num_features  = num_f;
    }
    else
    {
        cout << "ranker type is not listmle or listnet" << endl;
        exit(1);
    }
}

void
Letor::prepare_training_file_listwise(const string & query_f, int num_f) 
{
    internal->prepare_training_file_listwise(query_f, num_f);
}

void
Letor::prepare_training_file()
{
    switch (ranker_type)
    {
        case RANKER_SVM:
            internal->prepare_training_file_svm(query_file, qrel_file, mset_size);
            break;
        case RANKER_LISTMLE:
        case RANKER_LISTNET:
            internal->prepare_training_file_listwise(query_file, num_features);
            break;
        default:
            cout << "ranker type error";
            break;
    }
}

void
Letor::create_ranker(int ranker_t)
{
    ranker_type = ranker_t;

    switch(ranker_type)
    {
        case RANKER_SVM:
            internal->ranker = new SVMRanker;
            break;
        case RANKER_LISTMLE:
            internal->ranker = new ListMLE;
            break;
        case RANKER_LISTNET:
            internal->ranker = new ListNET;
            break;
        default:
            break;
            // cout << "Please specify proper ranker.";
    }
}

}
