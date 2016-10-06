/** @file listnet_ranker.cc
 *  @brief Implementation of ListNetRanker
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
Cao, Zhe, et al. "Learning to rank: from pairwise approach to listwise approach."
Proceedings of the 24th international conference on Machine learning. ACM, 2007.
*/
#include <config.h>

#include "xapian-letor/ranker.h"

#include "debuglog.h"
#include "serialise-double.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;
using namespace Xapian;

/// A vector holding ground truth and prediction score probability distribution vectors.
typedef vector <vector <double> > prob_distrib_vector;

ListNETRanker::~ListNETRanker() {
    LOGCALL_DTOR(API, "ListNETRanker");
}

static double
calculateInnerProduct(const vector<double> &parameters, const vector<double> &fvals) {
    LOGCALL_STATIC_VOID(API, "calculateInnerProduct", parameters | fvals);
    double inner_product = 0.0;
    for (size_t i = 0; i < fvals.size(); ++i)
	inner_product += parameters[i] * fvals[i];
    return inner_product;
}

// From Theorem (8) in Cao et al. "Learning to rank: from pairwise approach to listwise approach."
static prob_distrib_vector
initializeProbability(const vector<FeatureVector> &feature_vectors, const vector<double> &new_parameters) {
    LOGCALL_STATIC_VOID(API, "initializeProbability", feature_vectors | new_parameters);

    // probability distribution, y is ground truth, while z is predict score
    vector<double> prob_y;
    vector<double> prob_z;
    double expsum_y = 0.0;
    double expsum_z = 0.0;

    for (size_t i = 0; i < feature_vectors.size(); i++) {
	expsum_y += exp(feature_vectors[i].get_label());
	expsum_z += exp(calculateInnerProduct(new_parameters, feature_vectors[i].get_fvals()));
    }
    for (size_t i = 0; i < feature_vectors.size(); i++) {
	prob_y.push_back(exp(feature_vectors[i].get_label()) / expsum_y);
	prob_z.push_back(exp(calculateInnerProduct(new_parameters, feature_vectors[i].get_fvals())) / expsum_z);
    }
    vector< vector<double> > prob;
    prob.push_back(prob_y);
    prob.push_back(prob_z);
    return prob;
}

// Equation (6) in paper Cao et al. "Learning to rank: from pairwise approach to listwise approach."
static vector<double>
calculateGradient(const vector<FeatureVector> &feature_vectors, const prob_distrib_vector &prob) {
    LOGCALL_STATIC_VOID(API, "calculateGradient", feature_vectors | prob);

    vector<double> gradient(feature_vectors[0].get_fcount(), 0);

    // Hold ground truth probability distribution
    const vector<double> prob_y(prob[0]);
    // Hold prediction score probability distribution
    const vector<double> prob_z(prob[1]);

    for (size_t i = 0; i < feature_vectors.size(); i++) {
	vector<double> fvals = feature_vectors[i].get_fvals();
	for (size_t k = 0; k < fvals.size(); k++) {
	    double first_term = - prob_y[i] * fvals[k];
	    gradient[k] += first_term;

	    double second_term = prob_z[i] * fvals[k];
	    gradient[k] += second_term;
	}
    }
    return gradient;
}

static void
updateParameters(vector<double> &new_parameters, const vector<double> &gradient, double learning_rate) {
    LOGCALL_STATIC_VOID(API, "updateParameters", new_parameters | gradient | learning_rate);
    for (size_t i = 0; i < new_parameters.size(); i++) {
	new_parameters[i] -= gradient[i] * learning_rate;
    }
}

void
ListNETRanker::train_model(const std::vector<Xapian::FeatureVector> & training_data) {
    LOGCALL_VOID(API, "ListNETRanker::train_model", training_data);
    size_t fvv_len = training_data.size();
    int feature_cnt = -1;
    if (fvv_len != 0) {
	feature_cnt = training_data[0].get_fcount();
    } else {
	throw LetorInternalError("Training data is empty. Check training file.");
    }
    // initialize the parameters for neural network
    vector<double> new_parameters(feature_cnt, 0.0);

    // iterations
    for (int iter_num = 1; iter_num < iterations; ++iter_num) {
	// initialize Probability distributions of y and z
	prob_distrib_vector prob = initializeProbability(training_data, new_parameters);
	// compute gradient
	vector<double> gradient = calculateGradient(training_data, prob);
	// update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);
    }
    swap(parameters, new_parameters);
}

void
ListNETRanker::save_model_to_metadata(const string & model_key) {
    LOGCALL_VOID(API, "ListNETRanker::save_model_to_file", model_key);
    Xapian::WritableDatabase letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "ListNET.model.default";
    }
    ostringstream oss;
    for (size_t i = 0; i <  parameters.size(); ++i)
	oss << serialise_double(parameters[i]) << endl;
    string model_data = oss.str();
    letor_db.set_metadata(key, model_data);
}

void
ListNETRanker::load_model_from_metadata(const string & model_key) {
    LOGCALL_VOID(API, "ListNETRanker::load_model_from_file", model_key);
    Xapian::Database letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "LisntNET.model.default";
    }
    string model_data = letor_db.get_metadata(key);
    // Throw exception if no model data associated with key
    if (model_data.empty()) {
	throw Xapian::LetorInternalError("No model found. Check key.");
    }
    vector<double> loaded_parameters;
    istringstream model_str(model_data);
    string line;
    while (getline(model_str, line)) {
	const char *ptr = line.data();
	const char *end = ptr + line.size();
	double parameter = unserialise_double(&ptr, end);
	loaded_parameters.push_back(parameter);
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
	for(size_t j = 0; j < fvals.size(); ++j)
	    listnet_score += fvals[j] * parameters[j];
	testfvv[i].set_score(listnet_score);
    }
    std::sort(testfvv.begin(), testfvv.end(), &Ranker::scorecomparer);
    return testfvv;
}
