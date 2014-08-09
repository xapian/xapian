/* svmranker.cc: The ranker using SVM.
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

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature_vector.h"
#include "ranklist.h"
#include "svmranker.h"

#include "safeunistd.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include <cstring>
#include <cstdlib>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using std::string;
using std::vector;
using std::cout;
using std::cerr;

namespace Xapian {

SVMRanker::SVMRanker() {}


SVMRanker::~SVMRanker() {}


void
SVMRanker::read_problem() {
    vector<RankList>::iterator training_data_it;

    // problem length
    prob.l = 0;
    for (training_data_it = training_data.begin();
            training_data_it != training_data.end(); ++training_data_it) {
        prob.l += training_data_it->get_feature_vector_list().size();
    }

    // elements
    int elements = 0;
    for (training_data_it = training_data.begin();
            training_data_it != training_data.end(); ++training_data_it) {
        vector<FeatureVector> feature_vectors = training_data_it->get_feature_vector_list();

        for (vector<FeatureVector>::iterator feature_vectors_it = feature_vectors.begin();
                feature_vectors_it != feature_vectors.end(); ++feature_vectors_it) {
            elements += feature_vectors_it->get_feature_values().size();
        }
    }

    prob.y = Malloc(double, prob.l);
    prob.x = Malloc(struct svm_node *, prob.l);
    x_space = Malloc(struct svm_node, elements);


    int max_index = 0;
    int i = 0;
    int j = 0;

    for (training_data_it = training_data.begin();
            training_data_it != training_data.end(); ++training_data_it) {

        prob.x[i] = &x_space[j];

        vector<FeatureVector> feature_vectors = training_data_it->get_feature_vector_list();

        for (vector<FeatureVector>::iterator feature_vectors_it = feature_vectors.begin();
                feature_vectors_it != feature_vectors.end(); ++feature_vectors_it) {

            for (int idx = 1; idx <= feature_vectors_it->get_feature_num(); ++idx) {

                x_space[j].index = idx;
                x_space[j].value = feature_vectors_it->get_feature_value_of(idx);

                if (max_index < x_space[j].index)
                    max_index = x_space[j].index;

                ++j;
            }
        }

        ++i;
    }

    if (param.gamma == 0 && max_index > 0)
        param.gamma = 1.0 / max_index;

    if (param.kernel_type == PRECOMPUTED) {
        for (i = 0; i < prob.l; ++i) {

            if (prob.x[i][0].index != 0) {
                cerr << "Wrong input format: first column must be 0:sample_serial_number" << '\n';
                exit(1);
            }

            if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index) {
                cerr << "Wrong input format: sample_serial_number out of range" << '\n';
                exit(1);
            }
        }   
    }
}


void
SVMRanker::set_training_data(vector<RankList> training_data_) {
    this->training_data = training_data_;
}


void
SVMRanker::learn_model() {
    // default values
    int s_type = 4;
    int k_type = 0;

    param.svm_type = s_type;
    param.kernel_type = k_type;
    param.degree = 3;
    param.gamma = 0;
    param.coef0 = 0;
    param.nu = 0.5;
    param.cache_size = 100;
    param.C = 1;
    param.eps = 1e-3;
    param.p = 0.1;
    param.shrinking = 1;
    param.probability = 0;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    // cross_validation = 0;


    cout << "Learning the model..";

    read_problem();

    const char *error_msg = svm_check_parameter(&prob, &param);
    if (error_msg) {
	   cerr << "svm_check_parameter failed: " << error_msg << '\n';
	   exit(1);
    }

    model = svm_train(&prob, &param);
}


void
SVMRanker::load_model(const string model_file_) {
    model = svm_load_model(model_file_.c_str());
}


void
SVMRanker::save_model(const string model_file_) {
    if (svm_save_model(model_file_.c_str(), model)) {
       cerr << "Can't save model to file " << model_file_.c_str() << '\n';
       exit(1);
    }
}


double
SVMRanker::score_doc(FeatureVector & fvector) {
    predict_probability = 0;

    // string model_file_name;
    // model_file_name = get_cwd().append("/model.txt");
    // model = svm_load_model(model_file_name.c_str());

    if (model == NULL) {
        cerr << "Please load the model first!" << '\n';
        exit(1);
    }

    int svm_type = svm_get_svm_type(model);
    int nr_class = svm_get_nr_class(model);

    if (predict_probability) {
        if (svm_type == NU_SVR || svm_type == EPSILON_SVR) {
            cout << "Prob. model for test data: target value = predicted value + z,"
                << '\n' << "z: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma="
                << svm_get_svr_probability(model) << '\n';
        }
        else {
            // int *labels = (int *) malloc(nr_class * sizeof(int));
            int *labels = (int *)Malloc(int, nr_class);
            svm_get_labels(model, labels);
            free(labels);
        }
    }

    // x = (struct svm_node *)malloc(fvector.size() * sizeof(struct svm_node));
    int feature_num = fvector.get_feature_num();
    x = Malloc(struct svm_node, feature_num);

    int i = 0;
    for (int idx = 1; idx <= fvector.get_feature_num(); ++idx) {
        x[i].index = idx;
        x[i].value = fvector.get_feature_value_of(idx);
        ++i;
    }

    return svm_predict(model, x);  //this is the score for a particular document
}


Xapian::RankList
SVMRanker::calc(Xapian::RankList & rlist) {
    vector<FeatureVector> fvector_list = rlist.get_feature_vector_list();
    for (vector<FeatureVector>::iterator fvector_it = fvector_list.begin();
            fvector_it != fvector_list.end(); ++fvector_it) {
        fvector_it->set_score( score_doc((*fvector_it)) );
    }
    rlist.set_feature_vector_list(fvector_list);
    return rlist;
}


RankList 
SVMRanker::rank(RankList & rlist) {
    RankList rlist_out = calc(rlist);

    // TO DO

    return rlist_out;
}

}