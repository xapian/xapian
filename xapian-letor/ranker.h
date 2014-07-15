/* ranker.h: The abstract ranker file.
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

#ifndef RANKER_H
#define RANKER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"
#include "feature_vector.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Ranker {

    vector<RankList> training_data;

public:
    int MAXPATHLENTH_SVM = 200;


    Ranker() {};

    virtual ~Ranker() {};

    // Override all the 6 methods below in the ranker sub-classes files

    //------------------------- For training -----------------------------------

    // Set training data
    virtual void set_training_data(vector<RankList> training_data_) = 0;

    // The training process
    virtual void learn_model() = 0;

    // Save model to file
    virtual void save_model(const string model_file_) = 0;

    //------------------------- For prediction ----------------------------------

    // Load model from file
    virtual void load_model(const string model_file_) = 0;

    // Calculate score for a FeatureVector
    virtual double score_doc(const FeatureVector & fv) = 0;

    // returns a ranklist (scores added)
    virtual RankList calc(const RankList & rlist) = 0;

    // returns a SORTED ranklist (sorted by the score of document)
    virtual RankList rank(const RankList & rlist) = 0;

    //------------------------- Helper functions ---------------------------------

    // Get current working directory
    static string get_cwd() {
        char temp[MAXPATHLENTH_SVM];
        return ( getcwd(temp, MAXPATHLENTH_SVM) ? string( temp ) : string() );
    }
};

}

#endif /* RANKER_H */
