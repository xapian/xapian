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
using std::endl;


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
Letor::Internal::write_to_txt(std::vector<Xapian::RankList> list_rlist) {
    ofstream train_file("train.txt");
    vector<Xapian::RankList>::iterator list_rlist_it = list_rlist.begin();
    for (; list_rlist_it != list_rlist.end(); ++list_rlist_it) {
        train_file << list_rlist_it->get_label_feature_values_text();
    }
    train_file.close();
}


static void
Letor::Internal::write_to_bin(std::vector<Xapian::RankList> list_rlist) {
    fstream train_file("train.bin", ios::in | ios::out | ios::binary);
    long int size = sizeof(list_rlist);
    train_file.write ((char*) &size, sizeof(size));
    train_file.write ((char*) &list_rlist, sizeof(list_rlist));
    train_file.close();
}


static vector<Xapian::RankList>
Letor::Internal::read_from_bin(const string training_data_file_) {
    fstream training_data(training_data_file_, ios::in | ios::out | ios::binary);

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
Letor::Internal::load_model_file(string model_file_) {
    ranker->load_model(model_file_);
}


void
Letor::Internal::update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_) {
    feature_manager.update_state(query_, mset_);

    // Create RankList and normalize it
    Xapian::RankList   orig_rlist = feature_manager.create_ranklist(mset_);
    Xapian::RankList        rlist = feature_manager.normalize(orig_rlist);

    // Ranker ranks the RankList
    Xapian::RankList ranked_rlist = ranker->rank(rlist);

    // Create letor_item for each doc in MSet
    vector<Xapian::MSet::letor_item> letor_items = ranked_rlist.create_letor_items();

    // Update MSet
    feature_manager.update_mset(letor_items);
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
Letor::Internal::prepare_training_file(const string query_file_, const string qrel_file_, Xapian::doccount mset_size) {

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


    FeatureManager::qid_docid_relevance_map qrel = feature_manager.train_load_qrel(qrel_file_);

    vector<Xapian::RankList> list_rlist;

    string line;
    ifstream queries;
    queries.open(query_file_.c_str(), ios::in);

    while (!queries.eof()) {        //reading all the queries line by line from the query file

        getline(queries, line);
        if (line.empty())
            break;

        string qid = line.substr(0, (int)line.find(" "));
        string query_txt = line.substr((int)line.find("'")+1, (line.length() - ((int)line.find("'") + 2)));

        string qq = query_txt;
        istringstream iss(query_txt);
        string title = "title:";
        while (iss)
        {
            string t;
            iss >> t;
            if (t.empty())
            break;
            string temp;
            temp.append(title);
            temp.append(t);
            temp.append(" ");
            temp.append(qq);
            qq = temp;
        }

        cout << "Processing Query: " << qq << endl;

        Xapian::Query query = parser.parse_query(qq,
                                        parser.FLAG_DEFAULT|
                                        parser.FLAG_SPELLING_CORRECTION);

        Xapian::Enquire enquire(letor_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, msetsize);

        feature_manager.update_state(query, mset);

        Xapian::RankList rl = feature_manager.train_create_ranklist(mset, qid);
        list_rlist.push_back(rl);
    }

    queries.close();

    // Output the training data
    write_to_txt(list_rlist);
    write_to_bin(list_rlist);
}