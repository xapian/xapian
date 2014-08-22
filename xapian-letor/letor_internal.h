/* letor_internal.h: The internal class of Xapian::Letor class
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

#ifndef XAPIAN_INCLUDED_LETOR_INTERNAL_H
#define XAPIAN_INCLUDED_LETOR_INTERNAL_H

#include <xapian/letor.h>

#include "feature.h"
#include "feature_manager.h"
#include "ranker.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class Feature;
class FeatureManager;
class Ranker;

class Letor::Internal : public Xapian::Internal::intrusive_base {
    friend class Letor;

    Xapian::Database * database;                // Stored as reference
    Xapian::Ranker * ranker;

    Xapian::Feature feature;
    Xapian::FeatureManager feature_manager;

public:

    Internal();


    ~Internal();


    // ======================= Used for training =========================


    // Write the training data to file in text format.
    static void write_to_txt(vector<Xapian::RankList> list_rlist, const string output_file);


    // Write the training data to file in text format. The file's name is "train.txt".
    static void write_to_txt(vector<Xapian::RankList> list_rlist);


    // Wrtie the training data to file in binary format.
    static void write_to_bin(vector<Xapian::RankList> list_rlist, const string output_file);


    // Wrtie the training data to file in binary format. The file's name is "train.bin".
    static void write_to_bin(vector<Xapian::RankList> list_rlist);


    // Read the training data in text format.
    static vector<Xapian::RankList> read_from_txt(const string training_data_file_);


    // Read the training data in binary format.
    static vector<Xapian::RankList> read_from_bin(const string training_data_file_);


    // Connect all parts (since we use reference).
    void init();


    // Set the database.
    void set_database(Xapian::Database & database_);


    // Set the feature.
    void set_features(const vector<Xapian::Feature::feature_t> & features);


    // Set Normalizer.
    void set_normalizer(Xapian::Normalizer * normalizer_);


    // Generate training data from query file and qrel file and store into file.
    void prepare_training_file(const string query_file_, const string qrel_file_, const string output_file, Xapian::doccount mset_size);


    // Use training data to train the model.
    void train(const string training_data_file_, const string model_file_);


    // Load model from file. Call ranker's corresponding functions.
    void load_model_file(const string model_file_);


    // Attach letor information to MSet.
    void update_mset(Xapian::Query & query_, Xapian::MSet & mset_);
};

}

#endif // XAPIAN_INCLUDED_LETOR_INTERNAL_H
