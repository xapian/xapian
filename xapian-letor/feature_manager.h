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

namespace Xapian {

class Feature;

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

    Feature feature;

    Xapian::Database & database;
    Xapian::Query & query;
    Xapian::MSet & mset;
    Xapian::Normlizer & normalizer;

    int query_term_length;
    vector<long int> query_term_frequency_database;
    vector<double> query_inverse_doc_frequency_database;
    vector<long int> database_details(3);

    vector<Xapian::MSet::letor_item> generate_letor_info();

    void update_database_details();
    void update_query_term_frequency_database();
    void update_query_inverse_doc_frequency_database();


    qid_did_relevance_map qrel;

public:

    typedef map<string, int> did_rel_map;                    // docid and relevance judjement 0/1
    typedef map<string, did_relevance_map> qid_did_rel_map;  // qid and docid_relevance_map


    // ========================== General setters ========================================
    
    void set_normalizer(const Xapian::Normalizer & normalizer_);

    void update_context(const Xapian::Database database_, const vector<Feature::feature_t> features_);

    void update_state(const Xapian::Query query_, const Xapian::MSet mset_);

    void update_mset(const vector<Xapian::MSet::letor_item> & letor_items_);

    // ============================= General getters ===================================

    Xapian::Database & get_database();

    Xapian::Query & get_query();

    Xapian::MSet & get_mset();

    Feature & get_feature();

    int get_features_num();


    // =================== Used for generating feature-related information ======================

    // This method return three kinds of information: total length of title,
    // total length of body, total length of content of documents in database.
    vector<long int> & get_database_details();

    vector<long int> & get_q_term_freq_db();

    vector<long int> & get_q_inv_doc_freq_db();

    vector<long int> get_doc_details(const Xapian::Docuement doc_);

    vector<long int> get_q_term_freq_doc(const Xapain::Document doc_);


    // Create FeatureVector from MSetIterator.
    Xapian::FeatureVector create_feature_vector(const Xapian::MSetIterator & mset_it_);


    // Create RankList from MSet with query id.
    Xapian::RankList create_ranklist(const string qid_, const Xapian::MSet & mset_);


    // Create RankList from MSet without query id.
    Xapian::RankList create_ranklist(const Xapian::MSet & mset_);


    // Create RankList from MSet based on current state of FeatureManager.
    Xapian::RankList create_ranklist();

    
    // Normalize RankList.
    Xapian::RankList normalize(const Xapian::RankList * rlist_);


    // Load query relevance information from file
    void train_load_qrel(const std::string qrel_file_);

    
    // Get the label for the document corresponding to query id in qrel.
    void train_get_label_qrel(const Document &doc, std::string qid);


    // Creatue FeatureVector for training.
    Xapian::FeatureVector train_create_feature_vector(const Xapian::MSetIterator & mset_it_);


    // Create RankList for training.
    Xapian::RankList train_create_ranklist(const Xapian::MSet & mset, std::string qid);
};

}

#endif // FEATURE_MANAGER_H
