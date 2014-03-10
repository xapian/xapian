/* ranker.h: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
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

#include "ranklist.h"
//#include "evalmetric.h"
#include "svmranker.h"

#include "safeunistd.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include <cstring>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace std;
using namespace Xapian;


int MAXPATHLENTH_SVM = 200;

static string get_cwd() {
    char temp[MAXPATHLENTH_SVM];
    return ( getcwd(temp, MAXPATHLENTH_SVM) ? std::string( temp ) : std::string() );
}

inline int max_position( vector<double> & v){
    int max = 0;
    for (unsigned int i = 1; i < v.size(); i++){
        if( v[i] > v[max])
            max = i;
    }

    return max;
}

Xapian::RankList // returns a SORTED ranklist (sorted by the score of each document)
SVMRanker::rank(Xapian::RankList rlist)
{
    Xapian::RankList rl_out;
    std::vector<FeatureVector> local_rl = rlist.get_data();
    int num_fv = local_rl.size();
    double temp = 0.0;

    for(int i=0; i<num_fv; ++i) {
        FeatureVector fv_temp = local_rl[i];
        temp = score_doc(fv_temp);
        fv_temp.set_score(temp);
        rl_out.add_feature_vector(fv_temp);
    }
    local_rl = rl_out.sort_by_score();
    rl_out.set_rl(local_rl);
    
    return rl_out;
}

void
SVMRanker::set_training_data(vector<Xapian::RankList> training_data1)
{
    this->training_data = training_data1;
}

void
SVMRanker::read_problem() {
    vector<RankList>::iterator training_data_it;

    // problem length
    prob.l = 0;
    for (training_data_it = training_data.begin();
        training_data_it != training_data.end(); ++training_data_it)
    {
        prob.l += training_data_it->get_data().size();
    }

    // elements
    int elements = 0;
    for (training_data_it = training_data.begin();
        training_data_it != training_data.end(); ++training_data_it)
    {
        vector<FeatureVector> feature_vectors = training_data_it->get_data();
        vector<FeatureVector>::iterator feature_vectors_it;
        for (feature_vectors_it = feature_vectors.begin();
            feature_vectors_it != feature_vectors.end(); ++feature_vectors_it)
        {
            elements += feature_vectors_it->get_fvals().size();
        }
    }

    prob.y = Malloc(double, prob.l);
    prob.x = Malloc(struct svm_node *, prob.l);
    x_space = Malloc(struct svm_node, elements);


    int max_index = 0;
    int i = 0;
    int j = 0;

    for (training_data_it = training_data.begin();
        training_data_it != training_data.end(); ++training_data_it)
    {
        prob.x[i] = &x_space[j];

        vector<FeatureVector> feature_vectors = training_data_it->get_data();
        vector<FeatureVector>::iterator feature_vectors_it;

        for (feature_vectors_it = feature_vectors.begin();
            feature_vectors_it != feature_vectors.end(); ++feature_vectors_it)
        {
            map<int, double>feature_vector = feature_vectors_it->get_fvals();
            map<int, double>::iterator fv_it;

            for (fv_it = feature_vector.begin();
                fv_it != feature_vector.end(); ++fv_it)
            {
                x_space[j].index = fv_it->first;
                x_space[j].value = fv_it->second;

                if (max_index < x_space[j].index)
                    max_index = x_space[j].index;

                ++j;
            }
        }

        ++i;
    }

    if (param.gamma == 0 && max_index > 0)
        param.gamma = 1.0 / max_index;

    if (param.kernel_type == PRECOMPUTED)
    {
        for (i = 0; i < prob.l; ++i)
        {
            if (prob.x[i][0].index != 0)
            {
                fprintf(stderr, "Wrong input format: first column must be 0:sample_serial_number\n");
                exit(1);
            }
            if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
            {
                fprintf(stderr, "Wrong input format: sample_serial_number out of range\n");
                exit(1);
            }
        }   
    }
}

void
SVMRanker::learn_model() {
    // default values
    int s_type = 4;
    int k_type = 0;

    param.svm_type = s_type;
    param.kernel_type = k_type;
    param.degree = 3;
    param.gamma = 0;	// 1/num_features
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
    cross_validation = 0;

    printf("Learning the model..");

    string model_file_name;
    const char *error_msg;
    model_file_name = get_cwd().append("/model.txt");

    read_problem();
    error_msg = svm_check_parameter(&prob, &param);
    if (error_msg) {
	   fprintf(stderr, "svm_check_parameter failed: %s\n", error_msg);
	   exit(1);
    }

    model = svm_train(&prob, &param);
    SVMRanker::save_model(model_file_name);
}

void 
SVMRanker::load_model(const std::string & model_file_name) {
    model = svm_load_model(model_file_name.c_str());
}

void
SVMRanker::save_model(const string & model_file_name) {
    if (svm_save_model(model_file_name.c_str(), model)) {
       fprintf(stderr, "can't save model to file %s\n", model_file_name.c_str());
       exit(1);
    }
}

double
SVMRanker::score_doc(FeatureVector fv) {
    predict_probability = 0;

    string model_file_name;
    model_file_name = get_cwd().append("/model.txt");

    model = svm_load_model(model_file_name.c_str());

    int svm_type = svm_get_svm_type(model);
    int nr_class = svm_get_nr_class(model);

    if (predict_probability) {
        if (svm_type == NU_SVR || svm_type == EPSILON_SVR) {
            printf("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n" , svm_get_svr_probability(model));
        } else {
            // int *labels = (int *) malloc(nr_class * sizeof(int));
            int *labels = (int *)Malloc(int, nr_class);
            svm_get_labels(model, labels);
            free(labels);
        }
    }

    map<int, double>feature_vector = fv.get_fvals();
    map<int, double>::iterator fv_it;
    int i = 0;

    // x = (struct svm_node *)malloc(feature_vector.size() * sizeof(struct svm_node));
    x = Malloc(struct svm_node, feature_vector.size());

    for (fv_it = feature_vector.begin();
        fv_it != feature_vector.end(); ++fv_it)
    {
        x[i].index = fv_it->first;
        x[i].value = fv_it->second;
        ++i;
    }

    return svm_predict(model, x);  //this is the score for a particular document
}

