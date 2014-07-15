/* ranklist.h: RankList which stors list of feature vectors.
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

#ifndef RANKLIST_H
#define RANKLIST_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class FeatureVector;

class XAPIAN_VISIBILITY_DEFAULT RankList {

    vector<FeatureVector> feature_vector_list;
    string qid;
    int feature_num;

public:

    RankList();

    virtual ~RankList();
    
    // Set query id
    void set_qid(string qid_);
    
    // Set feature vectors for documents in MSet corresponding to this RankList
    void set_feature_vector_list(vector<FeatureVector> feature_vector_list_);
    
    // Add a new feature vector
    void add_feature_vector(FeatureVector fvector_);

    // Get query id
    string get_qid();

    // Get the number of documents
    int get_num();

    // Get the number of features
    int get_feature_num();

    // Get the feature vectors
    vector<FeatureVector> & get_feature_vector_list();

    // Get text output for this RankList
    string get_label_feature_values_text();

    // Create letor items
    vector<Xapian::MSet::letor_item> create_letor_items();
};

}
#endif /* RANKLIST_H */
