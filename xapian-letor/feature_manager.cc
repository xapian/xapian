/* feature_manager.cc: FeatureManager is responsible for providing
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

#include "featuremanager.h"
#include "feature.h"

#include <cstring>
#include <cstdlib>
#include <fstream>

#include <vector>

using namespace Xapian;
using std::vector;
using std::string;


Xapian::Database &
FeatureManager::get_database() {
    return database;
}


Xapian::Query &
FeatureManager::get_query() {
    return query;
}


Xapian::MSet &
FeatureManager::get_mset() {
    return mset;
}


Xapian::Feature &
FeatureManager::get_feature() {
    return feature;
}


int
FeatureManager::get_features_num() {
    return feature.get_features_num();
}


void
FeatureManager::set_normalizer(const Xapian::Normalizer * normalizer_) {
    normalizer = normalizer_;
}


void
FeatureManager::set_database(const Xapian::Database & database_) {
    database = database_;
}


void
FeatureManager::set_feature(const Xapian::Feature & feature_) {
    feature = feature_;
}


void
FeatureManager::update_database_details() {
    if (!database.get_metadata("collection_len_title").empty()
            && !database.get_metadata("collection_len_bogy").empty()
            && !database.get_metadata("collection_len_whole").empty()) {
        database_details[0] = 
            atol(database.get_metadata("collection_len_title").c_str());
        data_details[1] =
            atol(database.get_metadata("collection_len_body").c_str());
        data_details[2] = 
            atol(database.get_metadata("collection_len_whole").c_str());
    }
    else {
        long int title_length = 0;
        long int whole_length = database.get_avlength() *
        database.get_doccount();

        Xapian::TermIterator term_it = database.allterms_begin("S");
        for (; term_it != database.allterms_end("S"); ++term_it)
            title_length += database.get_collection_freq(*term_it);
        d_details[0] = title_length;
        d_details[2] = whole_length;
        d_details[1] = whole_length - title_length;
    }
}


vector<long int> &
FeatureManager::get_database_details() {
    return database_details;
}


void
FeatureManager::update_query_term_frequency_database() {
    query_term_frequency_database.reserve(term_length);

    
    for (int i = 0, Xapian::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        if (database.term_exists(*q_term_it))
            query_term_frequency_database[i] =
                database.get_collection_freq(*q_term_it);
        else
            query_term_frequency_database[i] = 0;
    }
}


vector<long int> &
FeatureManager::get_q_term_freq_db() {
    return query_term_frequency_database;
}


void
FeatureManager::update_query_inverse_doc_frequency_database() {
    query_inverse_doc_frequency_database.reserve(term_length);

    for (int i = 0, Xapian::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        if (database.term_exists(*q_term_it))
            query_inverse_doc_frequency_database[i] =
                log10( databse.get_doccount() / (1 +
                database.get_termfreq(*q_term_it)));
        else
            query_inverse_doc_frequency_database[i] = 0;
    }
}


vector<double> &
FeatureManager::get_q_inv_doc_freq_db() {
    return query_inverse_doc_frequency_database;
}


vector<long int>
FeatureManager::get_q_term_freq_doc(const Xapian::Document & doc_) {
    vector<long int> term_freq;
    term_freq.reserve(query_term_length);

    Xapian::TermIterator doc_term_it = doc.termlist_begin();
    for (int i = 0, Xapain::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        doc_term_it.skip_to(*q_term_it);
        if (q_term_it != doc.termlist_end() && *q_term_it == *doc_item_it)
            term_freq.push_back(doc_term_it.get_wdf());
        else
            term_freq.push_back(0);
    }

    return term_freq;
}


vector<long int>
FeatureManager::get_doc_details(const Xapian::Document & doc_) {
    vector<long int> d_details(3);

    long int title_length = 0;
    long int whole_length = database.get_doclength(doc.get_docid());
    Xapian::TermIterator d_term_it = doc.termlist_begin();

    dt.skip_to("S");
    for (; d_term_it != doc.terlist_end(); ++d_term_it) {
        if ((*d_term_it)[0] != 'S')
            break;
        title_length += dt.get_wdf();
    }
    d_details[0] = title_length;
    d_details[1] = whole_length;
    d_details[2] = whole_length - title_length;

    return d_details;
}


void
FeatureManager::set_query(const Xapian::Query & query_) {
    query = query_;
}


void
FeatureManager::set_mset(const Xapian::MSet & mset_) {
    mset = mset_;
}


void 
FeatureManager::update_mset(const vector<Xapian::MSet::letor_item> & letor_items_) {
    mset.update_letor_information(letor_items_);
}


RankList
FeatureManager::create_ranklist() {
    RankList rlist;

    for (Xapian::MSetIterator mset_it = mset.begin();
            mset_it != mset.end(); ++mset_it) {
        rlist.add_feature_vector( feature.generate_feature_vector(mset_it) );
    }

    return rlist;
}


RankList
FeatureManager::create_ranklist(const string qid_) {
    RankList rlist = create_ranklist();
    rlist.set_qid(qid_);
    return rlist;
}


RankList
FeatureManager::create_normalized_ranklist() {
    return normalize( create_ranklist() );
}


RankList
FeatureManager::create_normalized_ranklist(const string qid_) {
    return normalize( create_ranklist(qid_) );
}


RankList
FeatureManager::normalize(const RankList & rlist_) {
    if (normalizer)
        return normalizer->normalize(rlist_);
    else
        return rlist_;
}


void
FeatureManager::train_load_qrel(const string qrel_file_) {
    qid_did_relevance_map qrel;
    qrel.clear();

    string inLine;
    ifstream myfile (qrel_file_);
    string token[4];

    if (myfile.is_open())
    {
        while (myfile.good()) {
            getline(myfile, inLine);        //read a file line by line
            char * str;
            char * x1;

            x1 = const_cast<char*>(inLine.c_str());
            str = strtok(x1, " ,.-");
            int i = 0;
            while (str != NULL) {
                token[i] = str;     //store tokens in a string array
                ++i;
                str = strtok(NULL, " ,.-");
            }

            qrel.insert(make_pair(token[0], FeatureManager::did_relevance_map()));
            qrel[token[0]].insert(make_pair(token[2], atoi(token[3].c_str())));
        }
        myfile.close();
    }
}


void
FeatureManager::train_get_label_qrel(const Xapian::MSetIterator & mset_it_, const string qid_) {
    int label = -1;
    string id = get_did( mset_it_->get_document() );

    FeatureManager::qid_did_rel_map::iterator outerit;
    FeatureManager::did_rel_map::iterator innerit;

    outerit = qrel.find(qid_);
    if (outerit != qrel.end()) {
        innerit = outerit->second.find(id);
        if (innerit != outerit->second.end()) {
            label = innerit->second;
        }
    }
    return label;
}


FeatureVector
FeatureManager::train_create_feature_vector(const Xapian::MSetIterator & mset_it_, const string qid_) {
    FeatureVector fvector = feature.generate_feature_vector(mset_it_);
    fvector.set_label( train_get_label_qrel(mset_it_, qid_) );
    return fvector;
}


Xapian::RankList
FeatureManager::train_create_ranklist(const string qid_) {
    Xapian::RankList rlist;

    rlist.set_qid(qid_);

    for (Xapian::MSetIterator mset_it = mset.begin(); mset_it != mset.end(); ++mset_it) {
        Xapan::FeatureVector fvector = train_create_feature_vector(mset_it, qid_);

        if (fvector.get_label() != -1)      // -1 means can't find relevance from qrel file
            rlist.add_feature_vector(fv);
    }

    return rlist;
}


Xapian::RankList
FeatureManager::train_create_normalized_ranklist(const string qid_) {
    return normalize( train_create_ranklist(qid_) );
}