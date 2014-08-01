/* feature_manager.h: FeatureManager is responsible for providing
 * feature-related information when generating feature values.
 *
 * Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2014 Jiarong Wei
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

#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature.h"
#include "feature_vector.h"
#include "rank_list.h"
#include "normalizer.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace Xapian {

class Feature;

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

public:
    // Map for docid and relevance judgement 0/1
    typedef map<string, int> did_rel_map;


    // Map for qid and docid_relevance_map
    typedef map<string, did_relevance_map> qid_did_rel_map;


    Xapian::Database & database;
    Xapian::Feature & feature;
    Xapian::Query & query;
    Xapian::MSet & mset;

    Xapian::Normlizer * normalizer;


    int query_term_length;
    vector<long int> query_term_frequency_database;
    vector<double> query_inverse_doc_frequency_database;
    vector<long int> database_details(3);


    // Called by Letor::Internal when the database is updated
    void update_database_details();


    // Called by Letor::Internal when the query is updated
    void update_query_term_frequency_database();


    // Called by Letor::Internal when the query is updated
    void update_query_inverse_doc_frequency_database();


    // ========================== General setters ===========================


    // Set Normalizer.
    void set_normalizer(const Xapian::Normalizer & normalizer_);


    // Set Database.
    void set_database(const Xapian::Database & database_);


    // Set Feature.
    void set_feature(const Xapian::Feature & feature_);


    // Set Query.
    void set_query(const Xapian::Query & query_);


    // Set MSet.
    void set_mset(const Xapian::MSet & mset_);


    // ====================== General getters ===============================


    // Get Database reference.
    Xapian::Database & get_database();


    // Get Query reference.
    Xapian::Query & get_query();


    // Get MSet reference.
    Xapian::MSet & get_mset();


    // Get Feature reference.
    Xapian::Feature & get_feature();


    // Get the number of features used.
    int get_features_num();


    // ========== Used for generating feature-related information ===========


    // Get three kinds of information of database: total length of title,
    // total length of body, total length of content of documents (including
    // title and body).
    vector<long int> & get_database_details();


    // Get term frequency of query in database.
    vector<long int> & get_q_term_freq_db();


    // Get inverse document frequency of query in database.
    vector<long int> & get_q_inv_doc_freq_db();


    // Get three kinds of information of the document: length of title,
    // length of body, length of total content of the document (including
    // title and body).
    vector<long int> get_doc_details(const Xapian::Docuement & doc_);


    // Get query term frequency in the document
    vector<long int> get_q_term_freq_doc(const Xapain::Document & doc_);


    // ===================== Used for generating RankList ===================


    // Create RankList from MSet with query id.
    Xapian::RankList create_ranklist(const string qid_);


    // Create RankList from MSet without query id.
    Xapian::RankList create_ranklist();


    // Create normalized RankList from MSet with query id.
    Xapian::RankList create_normalized_ranklist(const string qid_);


    // Create normalized RankList from MSet without query id.
    Xapian::RankList create_normalized_ranklist();

    
    // Use normalizer to normalize RankList.
    Xapian::RankList normalize(const Xapian::RankList * rlist_);


    // ============= Used for generating RankList for training ==============


    // Load query relevance information from file
    void train_load_qrel(const string qrel_file_);

    
    // Get the label for the document corresponding to query id in qrel.
    void train_get_label_qrel(const Document & doc_, const string qid_);


    // Create FeatureVector from MSetIterator for training.
    Xapian::FeatureVector train_create_feature_vector(const Xapian::MSetIterator & mset_it_);


    // Create RankList from MSet for training.
    Xapian::RankList train_create_ranklist(const string qid_);


    // Create normalized RankList from MSet for training.
    Xapian::RankList train_create_normalized_ranklist(const string qid_);


    // ============================= Update MSet ============================


    // Attach letor information to MSet.
    void update_mset(const vector<Xapian::MSet::letor_item> & letor_items_);
};

}

#endif // FEATURE_MANAGER_H
