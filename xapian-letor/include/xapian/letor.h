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

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {

    Xapian::Internal::intrusive_ptr<Internal> * internal;

public:
    /// @private @internal Class representing the Letor internals.
    class Internal;
    /// @private @internal Reference counted internals.

    /// Copy constructor.
    Letor(const Letor & o);

    /// Assignment.
    Letor & operator=(const Letor & o);

    /// Default constructor.
    Letor();

    /// Destructor.
    ~Letor();

    /// Specify the database to use for retrieval. This database will be used directly by the methods of Xapian::Letor::Internal
    void update_context(const Xapian::Database & database_, vector<Xapian::Feature::FeatureBase> features_, Ranker & ranker_, const Normalizer & normalizer_);


    // Load training data from file, including query file and qrel file, and create typical Letor training data.
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
    void prepare_training_file(const string query_file_, const string qrel_file_, Xapian::doccount mset_size);


    // Train the model.
    void train(string training_data_file_, string model_file_);


    // Load model file.
    void load_model_file(string model_file_);


    // Update Xapian::MSet.
    void update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_);
};

}

#endif /* XAPIAN_INCLUDED_LETOR_H */
