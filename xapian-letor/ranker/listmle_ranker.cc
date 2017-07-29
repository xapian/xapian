/** @file listmle_ranker.cc
 *  @brief Implementation of ListMLERanker
 */
/* Copyright (C) 2014 Hanxiao Sun
 * Copyright (C) 2016 Ayush Tomar
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

/*
ListNET is adapted from the paper:
Fen Xia, Tie-Yan Liu, Jue Wang, Wensheng Zhang, Hang Li
et al. "Listwise Approach to Learning to Rank - Theory and Algorithm."
*/
#include <config.h>

#include "xapian-letor/ranker.h"

#include "debuglog.h"
#include "serialise-double.h"

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <algorithm>
#include <cmath>
#include <stdlib.h>

using namespace std;
using namespace Xapian;

ListMLERanker::~ListMLERanker() {
    LOGCALL_DTOR(API, "ListMLERanker");
}

static bool labelComparer(const FeatureVector & firstfv,
			  const FeatureVector& secondfv) {
    return firstfv.get_label() > secondfv.get_label();
}

static double
calculateInnerProduct(vector<double> & parameters,
		      vector<double> feature_sets) {

    double inner_product = 0.0;
    for (size_t i = 0; i < parameters.size(); ++i) {
	inner_product += parameters[i] * feature_sets[i];
    }
    return inner_product;
}

static vector<double>
calculateGradient(vector<FeatureVector> & sorted_feature_vectors,
		  vector<double> & new_parameters) {

    //need to be optimized,how to get the length of the features
    vector<double> gradient(sorted_feature_vectors[0].get_fcount(), 0);

    size_t list_length = sorted_feature_vectors.size();

    vector<double> exponents;
    double expsum = 0.0;

    for (size_t i = 0; i < list_length; ++i) {
	double temp = exp(calculateInnerProduct(
			      new_parameters,
			      sorted_feature_vectors[i].get_fvals()));
	exponents.push_back(temp);
	expsum += temp;
    }

    for (size_t i = 0; i < list_length; ++i) {
	vector<double> feature_sets = sorted_feature_vectors[i].get_fvals();
	for (size_t j = 0; j < feature_sets.size() - 1; ++j) {
	    gradient[j] += feature_sets[j] * exponents[i] / expsum;
	}
    }

    vector<double> first_place_in_ground_truth_feature_sets =
	    sorted_feature_vectors[0].get_fvals();

    for (size_t i = 0; i < first_place_in_ground_truth_feature_sets.size() - 1;
	 ++i) {
	gradient[i] -= first_place_in_ground_truth_feature_sets[i];

    }

    return gradient;
}

static void
updateParameters(vector<double> & new_parameters, vector<double> & gradient,
		 double & learning_rate) {
    size_t num = new_parameters.size();
    for (size_t i = 0; i < num; ++i) {
	new_parameters[i] -= gradient[i] * learning_rate;
    }
}

/* Updates new_parameters after calculating the gradient.
 * new_parameters(w) is updated as: w = w - gradient * learningRate
 */
static void
batchLearning(vector<FeatureVector> feature_vectors,
	      vector<double> & new_parameters, double learning_rate) {

    sort(feature_vectors.begin(), feature_vectors.end(), labelComparer);
    vector<double> gradient = calculateGradient(feature_vectors,
						new_parameters);
    updateParameters(new_parameters, gradient, learning_rate);

}

void
ListMLERanker::train(const vector<FeatureVector> & training_data) {

    int feature_cnt = training_data[0].get_fcount();

    //initialize the parameters for neural network
    vector<double> new_parameters;
    for (int feature_num = 0; feature_num < feature_cnt; ++feature_num) {
	new_parameters.push_back(0.0);
    }

    for (int iter_num = 1; iter_num < iterations; ++iter_num) {
	batchLearning(training_data, new_parameters, learning_rate);
    }

    swap(parameters, new_parameters);
}

void
ListMLERanker::save_model_to_metadata(const string & model_key) {
    LOGCALL_VOID(API, "ListMLERanker::save_model_to_metadata", model_key);
    WritableDatabase letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "ListMLE.model.default";
    }
    string model_data;
    for (size_t i = 0; i < parameters.size(); ++i)
	model_data += serialise_double(parameters[i]);
    letor_db.set_metadata(key, model_data);
}

void
ListMLERanker::load_model_from_metadata(const string & model_key) {
    LOGCALL_VOID(API, "ListMLERanker::load_model_from_metadata", model_key);
    Database letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "ListMLE.model.default";
    }
    string model_data = letor_db.get_metadata(key);
    // Throw exception if no model data associated with key
    if (model_data.empty()) {
	throw LetorInternalError("No model found. Check key.");
    }
    vector<double> loaded_parameters;
    const char *ptr = model_data.data();
    const char *end = ptr + model_data.size();
    while (ptr != end) {
	loaded_parameters.push_back(unserialise_double(&ptr, end));
    }
    swap(parameters, loaded_parameters);
}

std::vector<FeatureVector>
ListNETRanker::rank_fvv(const std::vector<FeatureVector> & fvv) const {
    LOGCALL(API, std::vector<FeatureVector>, "ListNETRanker::rank_fvv", fvv);
    std::vector<FeatureVector> testfvv = fvv;
    for (size_t i = 0; i < testfvv.size(); ++i) {
	double listnet_score = 0;
	std::vector<double> fvals = testfvv[i].get_fvals();
	if (fvals.size() != parameters.size())
	    throw LetorInternalError("Model incompatible. Make sure that you are using "
				     "the same set of Features using which the model was created.");
	for (size_t j = 0; j < fvals.size(); ++j)
	    listnet_score += fvals[j] * parameters[j];
	testfvv[i].set_score(listnet_score);
    }
    return testfvv;
}
