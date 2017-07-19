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

struct labelComparer {
    bool operator()(const FeatureVector & firstfv,
		    const FeatureVector& secondfv) const {
	return firstfv.get_label() > secondfv.get_label();
    }
};

static double
calculateInnerProduct(vector<double> & parameters,
		      vector<double> feature_sets) {

    double inner_product = 0.0;
    for (size_t i = 0; i < parameters.size(); ++i) {
	//the feature start from 1, while the parameters strat from 0
	inner_product += parameters[i] * feature_sets[i];
    }
    return inner_product;
}

/*
loss function: negative log Likelihood
the equation (9) in the paper

static double
negativeLogLikelihood(vector<FeatureVector> sorted_feature_vectors,
		      vector<double> new_parameters) {

    size_t list_length = sorted_feature_vectors.size();
    //top 1 probability
    double first_place_in_ground_truth;
    double expsum = 0.0;
    if (list_length != 0) {
	//Also use top 1 probability, so we need the first one in the ground
	//truth.
	//Should be put into init?
	first_place_in_ground_truth =
		exp(calculateInnerProduct(new_parameters,
					  sorted_feature_vectors[0].get_fvals()
					  ));
    }
    else{
	printf("the feature_vectors is null in ListMLERanker:logLikelihood\n");
	exit(1);
    }

    for (size_t i = 0; i < list_length; ++i) {
	expsum += exp(calculateInnerProduct(new_parameters,
		      sorted_feature_vectors[i].get_fvals()));
    }
    return log(expsum) - log(first_place_in_ground_truth);
}
*/

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
	//Not convenient, but in order to get the feature,
	//I have to run the ranklist loop first.
	//Need to see whether the fvals could be a vector
	vector<double> feature_sets = sorted_feature_vectors[i].get_fvals();

	//for each feature in special feature vector
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
    if (num != gradient.size()) {
	printf("the size between base new_parameters and gradient is not "
	       "match in listnet::updateParameters\n");
	printf("the size of new_parameters: %zu\n", num);
	printf("the size of gradient: %zu\n", gradient.size());
    }
    else {
	for (size_t i = 0; i < num; ++i) {
	    new_parameters[i] -= gradient[i] * learning_rate;
	}
    }
}

static void
batchLearning(vector<FeatureVector> feature_vectors,
	      vector<double> & new_parameters, double learning_rate) {

    //ListMLE need to use the ground truth, so sort before train
    sort(feature_vectors.begin(), feature_vectors.end(), labelComparer());

    //compute gradient
    vector<double> gradient = calculateGradient(feature_vectors,
						new_parameters);

    //update parameters: w = w - gradient * learningRate
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

    //iterations
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

vector<FeatureVector>
ListMLERanker::rank_fvv(const vector<FeatureVector> & fvv) const {

    vector<FeatureVector> testfvv = fvv;
    size_t testfvvsize = testfvv.size();

    vector<double> new_parameters = this->parameters;
    size_t parameters_size = new_parameters.size();

    for (size_t i = 0; i < testfvvsize; ++i) {

	double listnet_score = 0.0;

	vector<double> fvals = testfvv[i].get_fvals();
	size_t fvalsize = fvals.size();

	if (fvalsize != parameters_size) {
	    throw Xapian::InvalidArgumentError(
			"Number of fvals don't match the number of ListNet "
			"parameters\n");
	}

	for (size_t j = 0; j < fvalsize; ++j) {
	    listnet_score += fvals[j] * new_parameters[j];
	}

	testfvv[i].set_score(listnet_score);
    }
    return testfvv;
}
