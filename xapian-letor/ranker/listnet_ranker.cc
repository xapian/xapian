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

#include "xapian-letor/ranker.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;
using namespace Xapian;

/// A vector holding ground truth and prediction score probability distribution vectors.
typedef vector <vector <double> > prob_distrib_vector;

ListNETRanker::~ListNETRanker() {
}

static double
calculateInnerProduct(vector<double> parameters, vector<double> fvals) {

    double inner_product = 0.0;

    for (int i = 0; i < int(fvals.size()); ++i){
	inner_product += parameters[i] * fvals[i];
    }

    return inner_product;
}

// From Theorem (8) in Cao et al. "Learning to rank: from pairwise approach to listwise approach."
static prob_distrib_vector
initializeProbability(vector<FeatureVector> feature_vectors, vector<double> & new_parameters) {

    int list_length = feature_vectors.size();

    // probability distribution, y is ground truth, while z is predict score
    vector<double> prob_y;
    vector<double> prob_z;
    double expsum_y = 0.0;
    double expsum_z = 0.0;

    for (int i = 0; i < list_length; i++) {
	expsum_y += exp(feature_vectors[i].get_label());
	expsum_z += exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()));
    }

    for (int i = 0; i < list_length; i++) {
	prob_y.push_back(exp(feature_vectors[i].get_label())/expsum_y);
	prob_z.push_back(exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()))/expsum_z);

    }

    vector< vector<double> > prob;
    prob.push_back(prob_y);
    prob.push_back(prob_z);

    return prob;
}

// Equation (6) in paper Cao et al. "Learning to rank: from pairwise approach to listwise approach."
static vector<double>
calculateGradient(vector<FeatureVector> feature_vectors, prob_distrib_vector prob) {

    vector<double> gradient(feature_vectors[0].get_fcount(),0);
    int list_length = feature_vectors.size();

    // Hold ground truth probability distribution
    vector<double> prob_y(prob[0]);
    // Hold prediction score probability distribution
    vector<double> prob_z(prob[1]);

    for (int i = 0; i < list_length; i++) {
	vector<double> fvals = feature_vectors[i].get_fvals();

	for (int k = 0; k < int(fvals.size()); k++){

	    double first_term = - prob_y[i] * fvals[k];
	    gradient[k] += first_term;

	    double second_term = prob_z[i] * fvals[k];
	    gradient[k] += second_term;

	}
    }

    return gradient;
}

static void
updateParameters(vector<double> & new_parameters, vector<double> gradient, double learning_rate) {
    LOGCALL_STATIC_VOID(API, "updateParameters", new_parameters | gradient | learning_rate);
    int num = new_parameters.size();

    for (int i = 0; i < num; i++){
	new_parameters[i] -= gradient[i] * learning_rate;
    }

}

void
ListNETRanker::train_model() {

    std::cout << "ListNET model begin to train..." << endl;

    // get the training data
    std::vector<Xapian::FeatureVector> fvv = get_traindata();
    int fvv_len = fvv.size();

    int feature_cnt = -1;
    if (fvv_len != 0) {
	feature_cnt = fvv[0].get_fcount();
    }
    else {
	std::cout << "The training data is NULL!" << endl;
	exit(1); // TODO: Throw exception
    }

    // initialize the parameters for neural network
    vector<double> new_parameters(feature_cnt, 0.0);

    // iterations
    for (int iter_num = 1; iter_num < iterations; ++iter_num) {

	// initialize Probability distributions of y and z
	prob_distrib_vector prob = initializeProbability(fvv, new_parameters);
	// compute gradient
	vector<double> gradient = calculateGradient(fvv, prob);
	// update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);
    }

    parameters = new_parameters;

}

void
ListNETRanker::save_model_to_file(const char* output_filename) {

    ofstream parameters_file;
    parameters_file.open(output_filename);

    int parameters_size = parameters.size();

    std::ostringstream oss;
    oss.precision (std::numeric_limits<double>::digits10);

    for (int i = 0; i < parameters_size; ++i) {
	oss << parameters[i] << endl;
    }

    parameters_file << oss.str();
    parameters_file.close();

    cout<< "ListNET model saved to \"" << output_filename << "\"" << endl;
}

void
ListNETRanker::load_model_from_file(const char* model_filename) {

	vector<double> loaded_parameters;

    fstream train_file (model_filename, ios::in);
    if (!train_file.good()) {
	cout << "No ListNET parameters file found" << endl;
	exit(1); // TODO: should throw an exception
    }

    while (train_file.peek() != EOF) {

	double parameter;
	train_file >> parameter;
	loaded_parameters.push_back(parameter);
    }

    train_file.close();

    loaded_parameters.pop_back();
    parameters = loaded_parameters;

    cout<< "ListNET model loaded from: \"" << model_filename << "\""  << endl;
}

std::vector<FeatureVector>
ListNETRanker::rank(const std::vector<FeatureVector> & fvv) {

    std::vector<FeatureVector> testfvv = fvv;
    int testfvvsize = testfvv.size();

    cout << "Performing ranking using ListNET model..." << endl;

    int parameters_size = parameters.size();

    for (int i = 0; i <testfvvsize; ++i) {

	double listnet_score = 0;

	std::vector<double> fvals = testfvv[i].get_fvals();
	int fvalsize = fvals.size();

	if (fvalsize != parameters_size) {
	    std::cout << "Number of fvals don't match the number of ListNet parameters" << endl;
	    exit(1); // TODO: should throw an exception
	}

	for(int j = 0; j < fvalsize; ++j) {
	    listnet_score += fvals[j]* parameters[j];
	}

	testfvv[i].set_score(listnet_score);
    }

    std::sort(testfvv.begin(),testfvv.end(),&Ranker::scorecomparer);

    return testfvv;
}
