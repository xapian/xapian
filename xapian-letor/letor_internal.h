/** @file letor_internal.h
 * @brief Internals of Xapian::Letor class
 */
/* Copyright (C) 2011 Parth Gupta
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

#ifndef XAPIAN_INCLUDED_LETOR_INTERNAL_H
#define XAPIAN_INCLUDED_LETOR_INTERNAL_H

#include <xapian/letor.h>
#include "ranker.h"

#include <map>

using namespace std;


namespace Xapian {

class Letor::Internal : public Xapian::Internal::intrusive_base {
    friend class Letor;
    Ranker ranker;
    Database letor_db;
    Query letor_query;
    //vector<Xapian::RankList> samples;

  public:

    std::map<Xapian::docid, double>  letor_score(const Xapian::MSet & mset);

    void letor_learn_model();

    void prepare_training_file(const std::string & query_file, const std::string & qrel_file, Xapian::doccount msetsize);
    
    void prepare_training_file_listwise(const std::string & query_file, int num_features);
    
    vector<Xapian::RankList> load_list_ranklist(const char *filename);


};

}

#endif // XAPIAN_INCLUDED_LETOR_INTERNAL_H
