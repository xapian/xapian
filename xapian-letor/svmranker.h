/* svmranker.h: The ranker using SVM.
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

#ifndef SVMRANKER_H
#define SVMRANKER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "feature_vector.h"

#include <libsvm/svm.h>

using std::string;
using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT SVMRanker: public Ranker {

    struct svm_parameter param;
    struct svm_problem prob;
    struct svm_model *model;
    struct svm_node *x_space;
    // int cross_validation;
    // int nr_fold;

    struct svm_node *x;

    int predict_probability;
    

    void read_problem();

public:

    SVMRanker();
    
    virtual ~SVMRanker();

    virtual void set_training_data(vector<RankList> training_data_);

    virtual void learn_model();

    virtual void save_model(const string model_file_);

    virtual void load_model(const string model_file_);

    virtual double score_doc(FeatureVector & fvector);

    virtual RankList calc(RankList & rlist);

    virtual RankList rank(RankList & rlist);

};

}

#endif /* SVMRANKER_H */
