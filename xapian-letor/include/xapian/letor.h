/* letor.h: Letor provides weighting scheme based on Learning to Rank. Note: letor.h is
 * not a part of official stable Xapian API.
 *
 * Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2014 Jiarong Wei
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_LETOR_H
#define XAPIAN_INCLUDED_LETOR_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature.h"
#include "feature_vector.h"
#include "feature_manager.h"
#include "ranker.h"
#include "svmranker.h"
#include "normalizer.h"
#include "default_normalizer.h"
#include "feature_selector.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {

public:
    // @private @internal Class representing the Letor internals.
    class Internal;
    // @private @internal Reference counted internals.

    Xapian::Internal::intrusive_ptr<Internal> internal;


    // Copy constructor.
    Letor(const Letor & o);


    // Assignment.
    Letor & operator=(const Letor & o);


    // Default constructor.
    Letor();


    // Destructor.
    ~Letor();


    /// Specify the database to use for retrieval. This database will be used directly by the methods of Xapian::Letor::Internal
    void set_database(Xapian::Database & database_);


    // Set features used.
    void set_features(const vector<Xapian::Feature::feature_t> & features_);


    // Set ranker based on ranker flag.
    void set_ranker(const Ranker::ranker_t ranker_flag);


    // Set normalizer based on normalizer flag.
    void set_normalizer(const Normalizer::normalizer_t normalizer_flag);


    // Load training data from file, including query file and qrel file, and create typical Letor training data.
    // Call internal->prepare_training_file.
    //
    // The format for query file:
    //      <qid> <query>
    //
    // For example,
    //      2010001 'landslide malaysia'
    //      2010002 'search engine'
    //      2010003 'Monuments of India'
    //      2010004 'Indian food'
    //
    // The format for qrel file:
    //      <qid> <iter> <did> <label>
    //
    // For example,
    //      2010003 Q0 19243417 1
    //      2010003 Q0 3256433 1
    //      2010003 Q0 275014 1
    //      2010003 Q0 298021 0
    //      2010003 Q0 1456811 0
    //
    // The format for Letor training data:
    //      <label> qid:<qid> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>
    //
    // For example,
    //      0 qid:10032 1:0.130742 2:0.000000 3:0.333333 4:0.000000 ... 18:0.750000 19:1.000000
    //      1 qid:10032 1:0.593640 2:1.000000 3:0.000000 4:0.000000 ... 18:0.500000 19:0.023400
    //
    void prepare_training_file(const string query_file, const string qrel_file, const string output_file, Xapian::doccount mset_size);


    // Use training data to train the model. Call internal->train.
    void train(const string training_data_file_, const string model_file_);


    // Load model from file. Call internal->load_model_file.
    void load_model_file(const string model_file_);


    // Attach letor information to MSet. Call internal->update_mset.
    void update_mset(Xapian::Query & query_, Xapian::MSet & mset_);
};

}

#endif /* XAPIAN_INCLUDED_LETOR_H */
