/** @file letor.cc
 * @brief Letor Class definition
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2012 Olly Betts
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

#include "xapian-letor/letor.h"
#include "xapian-letor/ranker.h"
#include "letor_internal.h"

#include "debuglog.h"

#include <map>
#include <string>
#include <vector>

using namespace std;

namespace Xapian {

Letor::Letor() : internal(new Letor::Internal)
{
    LOGCALL_CTOR(API, "Letor", NO_ARGS);
}

Letor::Letor(const Xapian::Database & db, Xapian::Ranker * ranker)
{
    LOGCALL_CTOR(API, "Letor", db | ranker);
    internal = new Letor::Internal();
    internal->letor_db = db;
    if (ranker == 0) {
	internal->ranker = new ListNETRanker();
    } else {
	internal->ranker = ranker;
    }
}

Letor::Letor(const Xapian::Database & db, const Xapian::Query & query, Xapian::Ranker * ranker)
{
    LOGCALL_CTOR(API, "Letor", db | query | ranker);
    if (query.empty()) {
	throw Xapian::InvalidArgumentError("Can't initialise with an empty Query object");
    }
    internal = new Letor::Internal();
    internal->letor_db = db;
    internal->letor_query = query;
    if (ranker == 0) {
	internal->ranker = new ListNETRanker();
    } else {
	internal->ranker = ranker;
    }
}

Letor::~Letor()
{
    LOGCALL_DTOR(API, "Letor");
}

void
Letor::set_database(const Xapian::Database & db)
{
    LOGCALL_VOID(API, "Letor::set_database", db);
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query)
{
    LOGCALL_VOID(API, "Letor::set_query", query);
    if (query.empty()) {
	throw Xapian::InvalidArgumentError("Can't initialise with an empty Query object");
    }
    internal->letor_query = query;
}

std::vector<Xapian::docid>
Letor::letor_rank(const Xapian::MSet & mset, const char* model_filename, const Xapian::FeatureList & flist) const
{
    LOGCALL(API, std::vector<Xapian::docid>, "Letor::letor_rank", mset | model_filename | flist);
    return internal->letor_rank(mset, model_filename, flist);
}

void
Letor::letor_learn_model(const char* input_filename, const char* output_filename)
{
    LOGCALL_VOID(API, "Letor::letor_learn_model", input_filename | output_filename);
    internal->letor_learn_model(input_filename, output_filename);
}

void
Letor::prepare_training_file(const string & query_file, const string & qrel_file,
			     Xapian::doccount msetsize, const char* filename,
			     const Xapian::FeatureList & flist)
{
    LOGCALL_VOID(API, "Letor::prepare_training_file", query_file | qrel_file | msetsize | filename | flist);
    internal->prepare_training_file(query_file, qrel_file, msetsize, filename, flist);
}

void
Letor::set_ranker(Xapian::Ranker * ranker)
{
    LOGCALL_VOID(API, "Letor::set_ranker", ranker);
    internal->ranker = ranker;
}

void
Letor::set_scorer(Xapian::Scorer * scorer)
{
    LOGCALL_VOID(API, "Letor::set_scorer", scorer);
    internal->scorer = scorer;
}

void
Letor::letor_score(const std::string & query_file, const std::string & qrel_file,
		   const std::string & model_file, const std::string & output_file,
		   Xapian::doccount msetsize, const Xapian::FeatureList & flist) {
    LOGCALL_VOID(API, "Letor::letor_score", query_file | qrel_file | model_file | output_file | msetsize | flist);
    internal->letor_score(query_file, qrel_file, model_file, output_file, msetsize, flist);
}

}
