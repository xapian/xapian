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

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/ranker.h>
#include <xapian-letor/ranklist.h>
#include "listnet_ranker.h"

#include <cmath>
#include <stdlib.h>

#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace Xapian;

ListNETRanker::ListNETRanker(){
    // default values
    learning_rate = 0.0001;
    iterations = 15;
}

ListNETRanker::ListNETRanker(int metric_type, double learn_rate, int num_iterations)
                                :Ranker(metric_type, learn_rate, num_iterations) {
    this->learning_rate = learn_rate;
    this->iterations = num_iterations;
}

static double
calculateInnerProduct(vector<double> parameters, map<int,double> feature_sets){

	double inner_product = 0.0;

	for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){

        // the feature start from 1, while the parameters start from 0
		inner_product += parameters[iter->first-1] * iter->second;
	}

	return inner_product;
}

static vector< vector<double> >
initializeProbability(vector<FeatureVector> feature_vectors, vector<double> &new_parameters) {

    int list_length = feature_vectors.size();

    // probability distribution, y is ground truth, while z is predict score
    vector<double> prob_y;
    vector<double> prob_z;
    double expsum_y = 0.0;
    double expsum_z = 0.0;

    for(int i = 0; i < list_length; i++){
        expsum_y += exp(feature_vectors[i].get_label());
        expsum_z += exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()));
    }

    for(int i = 0; i < list_length; i++){
        prob_y.push_back(exp(feature_vectors[i].get_label())/expsum_y);
        prob_z.push_back(exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()))/expsum_z);

    }

    vector< vector<double> > prob;
    prob.push_back(prob_y);
    prob.push_back(prob_z);

	return prob;
}

static vector<double>
calculateGradient(vector<FeatureVector> feature_vectors, vector< vector<double> > prob) {

	vector<double> gradient(feature_vectors[0].get_fcount()-1,0);
	int list_length = feature_vectors.size();

	vector<double> prob_y(prob[0]);
	vector<double> prob_z(prob[1]);

	for(int i = 0; i < list_length; i++){
		map<int,double> feature_sets = feature_vectors[i].get_fvals();

		for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){

			double first_term = - prob_y[i] * iter->second;
			gradient[iter->first-1] += first_term;

			double second_term = prob_z[i] * iter->second;
			gradient[iter->first-1] += second_term;

		}
	}

	return gradient;
}

static void
updateParameters(vector<double> &new_parameters, vector<double> gradient, double learning_rate){
	int num = new_parameters.size();

	for (int i = 0; i < num; i++){
		new_parameters[i] -= gradient[i] * learning_rate;
	}

}

static void
batchLearning(RankList ranklists, vector<double> &new_parameters, double learning_rate){

	// get feature vectors
	vector<FeatureVector> feature_vectors = ranklists.get_fvv();
	// initialize Probability distributions of y and z
	vector< vector<double> > prob = initializeProbability(feature_vectors, new_parameters);
	// compute gradient
	vector<double> gradient = calculateGradient(feature_vectors, prob);
	// update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);

}

void
ListNETRanker::train_model(){

	std::cout << "ListNet model begin to train..." << endl;

	// get the training data
	vector<Xapian::RankList> ranklists = get_traindata();
	int ranklist_len = ranklists.size();

	int feature_cnt = -1;
	if (ranklist_len != 0){
		feature_cnt = ranklists[0].fvv[0].get_fcount();
	}
	else{
		std::cout << "The training data in ListNet is NULL!" << endl;
		exit(1); // TODO: Throw exception
	}

	// initialize the parameters for neural network
	vector<double> new_parameters(feature_cnt-1, 0.0);

	// iterations
	for(int iter_num = 1; iter_num < iterations; ++iter_num){

		for(int sample_num = 0; sample_num < ranklist_len; ++sample_num){

			batchLearning(ranklists[sample_num], new_parameters, learning_rate);
        }

	}

    this->parameters = new_parameters;

    save_model_to_file();

}

void
ListNETRanker::save_model_to_file(){

    ofstream parameters_file;
    parameters_file.open("ListNet_parameters.txt");

    int parameters_size = parameters.size();

    std::ostringstream oss;
    oss.precision (std::numeric_limits<double>::digits10);

    for(int i = 0; i < parameters_size; ++i) {
        oss << parameters[i] << endl;
    }

    parameters_file << oss.str();
    parameters_file.close();

    cout<< "ListNET parameters saved to \"ListNet_parameters.txt\"" << endl;
}

void
ListNETRanker::load_model_from_file(const std::string & parameters_file){

	vector<double> loaded_parameters;

    fstream train_file (parameters_file, ios::in);
    if(!train_file.good()){
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
    this->parameters = loaded_parameters;

    cout<< "ListNET parameters loaded!" << endl;
}

Xapian::RankList
ListNETRanker::rank(Xapian::RankList & ranklist){

    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    // Loads the parameters file to be used for ranking
    load_model_from_file("ListNet_parameters.txt");
    cout << "Performing ranking using model loaded from \"ListNet_parameters.txt\"" << endl;

    std::vector<double> new_parameters = this->parameters;
    int parameters_size = new_parameters.size();

    for (int i = 0; i <testfvvsize; ++i){

    	double listnet_score = 0;

        map <int,double> fvals = testfvv[i].get_fvals();
        int fvalsize = fvals.size();

        if (fvalsize != parameters_size+1){ // fval start from 1, while the parameters start from 1
        	std::cout << "Number of fvals don't match the number of ListNet parameters" << endl;
            exit(1); // TODO: should throw an exception
        }

        for(int j = 1; j < fvalsize; ++j){ // fvals starts from 1, parameters from 0
        	listnet_score += fvals[j]* new_parameters[j-1];
        }

        testfvv[i].set_score(listnet_score);
    }

    ranklist.set_fvv(testfvv);
    ranklist.sort_by_score();

    return ranklist;
}
