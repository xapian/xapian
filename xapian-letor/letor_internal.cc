/* letor_internal.cc: The internal class of Xapian::Letor class
 *
 * Copyright (C) 2011 Parth Gupta
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

#include <config.h>

#include <xapian/letor.h>

#include <xapian.h>

#include "letor_internal.h"
#include "featuremanager.h"
#include "ranker.h"
#include "svmranker.h"
#include "letor_features.h"

#include "str.h"
#include "stringutils.h"
#include "safeerrno.h"
#include "safeunistd.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <libsvm/svm.h>


using std::cout;
using std::vector;

//Stop-words
static const char * sw[] = {
    "a", "about", "an", "and", "are", "as", "at",
    "be", "by",
    "en",
    "for", "from",
    "how",
    "i", "in", "is", "it",
    "of", "on", "or",
    "that", "the", "this", "to",
    "was", "what", "when", "where", "which", "who", "why", "will", "with"
};


static void
Letor::Internal::write_to_txt(vector<Xapian::RankList> list_rlist, const string output_file) {
    ofstream train_file(output_file);
    vector<Xapian::RankList>::iterator list_rlist_it = list_rlist.begin();
    for (; list_rlist_it != list_rlist.end(); ++list_rlist_it) {
        train_file << list_rlist_it->get_label_feature_values_text();
    }
    train_file.close();
}


static void
Letor::Internal::write_to_txt(vector<Xapian::RankList> list_rlist) {
    write_to_txt(list_rlist, "train.txt");
}


static void
Letor::Internal::write_to_bin(vector<Xapian::RankList> list_rlist, const string output_file) {
    fstream train_file(output_file, ios::out | ios::binary);
    long int size = sizeof(list_rlist);
    train_file.write((char*) &size, sizeof(size));
    train_file.write((char*) &list_rlist, sizeof(list_rlist));
    train_file.close();
}


static void
Letor::Internal::write_to_bin(vector<Xapian::RankList> list_rlist) {
    write_to_bin(list_rlist, "train.bin");
}


static vector<Xapian::RankList>
Letor::Internal::read_from_bin(const string training_data_file_) {
    fstream training_data(training_data_file_, ios::in | ios::binary);

    vector<Xapian::RankList> samples;
    training_data.read((char*) &samples, size);
    training_data.close();

    return samples;
}


static vector<Xapian::RankList>
Letor::Internal::read_from_txt(const string training_data_file_) {
    fstream training_data(training_data_file_, ios::in | ios::out);

    vector<Xapian::RankList> samples;

    // TO DO

    return samples;
}


void
Letor::Internal::init() {
    feature_manager.set_database(database);
    feature_manager.set_feature(feature);
    feature_manager.set_query(query);
    feature_manager.set_mset(mset);
}


void
Letor::Internal::set_database(const Xapian::Database & database_) {
    database = database_;
    feature_manager.update_database_details();
}


void
Letor::Internal::set_features(const vector<Xapian::Feature::feature_t> & features) {
    feature.set_features(features);
}


void
Letor::Internal::load_model_file(string model_file_) {
    ranker->load_model(model_file_);
}


Xapian::MSet
Letor::Internal::update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_) {
    query = query_;
    query_term_length = query.get_length();
    feature_manager.update_query_term_frequency_database();
    feature_manager.update_query_inverse_doc_frequency_database();

    mset = mset_;

    // Create RankList and normalize it
    Xapian::RankList rlist = feature_manager.create_normalized_ranklist();

    // Ranker calculates scores for docs in RankList
    Xapian::RankList scored_rlist = ranker->calc(rlist);

    // Create letor_item for each doc in MSet
    vector<Xapian::MSet::letor_item> letor_items = scored_rlist.create_letor_items();

    // Update MSet
    mset.update_letor_information(letor_items);

    return mset;
}


void
Letor::Internal::train(const string training_data_file_, const string model_file_) {
    // Set training data for ranker
    vector<Xapian::RankList> samples = read_from_bin(training_data_file_);
    ranker->set_training_data(samples);

    // Learn the model
    ranker->learn_model();

    // Save the model
    ranker->save_model(model_file_);
}


void
Letor::Internal::prepare_training_file(const string query_file, const string qrel_file, const string output_file, Xapian::doccount mset_size) {

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");

    Xapian::QueryParser parser;
    parser.add_prefix("title", "S");
    parser.add_prefix("subject", "S");

    parser.set_database(letor_db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    // Load from qrel file
    FeatureManager::qid_docid_relevance_map qrel = feature_manager.train_load_qrel(qrel_file_);

    vector<Xapian::RankList> list_rlist;

    string line;
    ifstream queries;
    queries.open(query_file);

    // Load all the queries line by line from query file
    while (!queries.eof()) {

        getline(queries, line);
        if (line.empty()) break;

        string qid = line.substr(0, (int)line.find(" "));
        string query_txt = line.substr((int)line.find("'")+1, (line.length() - ((int)line.find("'") + 2)));

        string qq = query_txt;
        istringstream iss(query_txt);
        string title = "title:";
        while (iss)
        {
            string t;
            iss >> t;
            if (t.empty()) break;

            string temp;
            temp.append(title);
            temp.append(t);
            temp.append(" ");
            temp.append(qq);
            qq = temp;
        }

        cout << "Processing Query: " << qq << endl;

        Xapian::Query query = parser.parse_query(qq,
                                        parser.FLAG_DEFAULT |
                                        parser.FLAG_SPELLING_CORRECTION);

        Xapian::Enquire enquire(letor_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, msetsize);

        feature_manager.set_query(query);
        feature_manager.set_mset(mset);

        Xapian::RankList rl = feature_manager.train_create_normalized_ranklist(qid);

        list_rlist.push_back(rl);
    }

    queries.close();

    // Output the training data
    // write_to_txt(list_rlist);
    write_to_bin(list_rlist, output_file);
}