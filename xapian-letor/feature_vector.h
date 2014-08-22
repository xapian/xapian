/* feature_vector.h: FeatureVector is the feature-based representation of documents.
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

#ifndef FEATURE_VECTOR_H
#define FEATURE_VECTOR_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT FeatureVector {

    string did;                     // the document ID
    double label;                   // the tagged relavance label
    double score;                   // the calculated score
    vector<double> feature_values;  // feature values
    Xapian::doccount index;         // the original index in MSet

public:
    FeatureVector() {}
    
    virtual ~FeatureVector() {}

    // Set document id
    void set_did(string did_);

    // Set the score
    void set_score(double score_);

    // Set the label
    void set_label(double label_);

    // Set the original index in MSet
    void set_index(Xapian::doccount index_);
    
    // Set feature values
    void set_feature_values(vector<double> feature_values_);

    // Get the document id
    string get_did();
    
    // Get the score
    double get_score();
    
    // Get the label
    double get_label();

    // Get the original index in MSet
    Xapian::doccount get_index();
    
    // Get the number of features
    int get_feature_num();

    // Get feature values
    vector<double> get_feature_values();
    
    // Get the value of the ith feature (the index starts from 1)
    double get_feature_value_of(int idx);

    // Get the vector in which label and feature_values store in the same vecto
    vector<double> get_label_feature_values();
    
    // Get the vector in which score and feature_values store in the same vector
    vector<double> get_score_feature_values();

    // Get the text output for feature values
    // The format:
    //    1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>
    //
    string get_feature_values_text();

    // Get the text output for feature values with docid
    string get_feature_values_did_text();

    // Get the text output for label and feature values with query id
    // The format:
    //    <label> qid:<query id> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>
    //
    string get_label_feature_values_text(string qid);

    // Get the text output for label and feature values with query id and docuement id
    string get_label_feature_values_did_text(string qid);

    // Get the text output for score and feature values
    // The format:
    //    <score> 1:<1st feature value> 2:<2nd feature value> .. n:<nth feature value>
    //
    string get_score_feature_values_text();

    // Create letor item
    Xapian::MSet::letor_item create_letor_item();

    // Restore FeatureVector from file
    static vector<FeatureVector> read_from_file(string file);

    // Extract certain feature values from FeatureVectors based on relevance level and feature index
    static vector<double> extract(vector<FeatureVector> & fvectors, double relevance, int f_idx);
};

}

#endif /* FEATURE_VECTOR_H */
