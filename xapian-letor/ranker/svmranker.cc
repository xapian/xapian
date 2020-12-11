/** @file
 *  @brief Implementation of Ranking-SVM
 */
/* Copyright (C) 2012 Parth Gupta
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
Ranking-SVM is adapted from the paper:
Joachims, Thorsten. "Optimizing search engines using clickthrough data."
Proceedings of the eighth ACM SIGKDD international conference on Knowledge discovery and data mining. ACM, 2002.
*/
#include <config.h>

#include "xapian-letor/ranker.h"

#include "debuglog.h"
#include "serialise-double.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <libsvm/svm.h>

using namespace std;
using namespace Xapian;

static void
empty_function(const char*) {}

static void
clear_svm_problem(svm_problem *problem)
{
    delete [] problem->y;
    problem->y = NULL;
    for (int i = 0; i < problem->l; ++i) {
	delete [] problem->x[i];
    }
    delete [] problem->x;
    problem->x = NULL;
}

SVMRanker::SVMRanker()
{
    LOGCALL_CTOR(API, "SVMRanker", NO_ARGS);
}

SVMRanker::~SVMRanker()
{
    LOGCALL_DTOR(API, "SVMRanker");
}

static int
get_non_zero_num(const Xapian::FeatureVector & fv_non_zero)
{
    int non_zero = 0;
    for (int i = 0; i < fv_non_zero.get_fcount(); ++i)
	if (fv_non_zero.get_feature_value(i) != 0.0)
	    ++non_zero;
    return non_zero;
}

void
SVMRanker::train(const std::vector<Xapian::FeatureVector> & training_data)
{
    LOGCALL_VOID(API, "SVMRanker::train", training_data);
    svm_set_print_string_function(&empty_function);
    struct svm_parameter param;
    param.svm_type = NU_SVR;
    param.kernel_type = LINEAR;
    param.degree = 3;           // parameter for poly Kernel, default 3
    param.gamma = 0;            // parameter for poly/rbf/sigmoid Kernel, default 1/num_features
    param.coef0 = 0;            // parameter for poly/sigmoid Kernel, default 0
    param.nu = 0.5;             // parameter for nu-SVC/one-class SVM/nu-SVR, default 0.5
    param.cache_size = 100;     // default 40MB
    param.C = 1;                // penalty parameter, default 1
    param.eps = 1e-3;           // stopping criteria, default 1e-3
    param.p = 0.1;              // parameter for e -SVR, default 0.1
    param.shrinking = 1;        // use the shrinking heuristics
    param.probability = 0;      // probability estimates
    param.nr_weight = 0;        // parameter for C-SVCl
    param.weight_label = NULL;  // parameter for C-SVC
    param.weight = NULL;        // parameter for c-SVC

    int fvv_len = training_data.size();
    if (fvv_len == 0) {
	throw LetorInternalError("Training data is empty. Check training file.");
    }
    int feature_cnt = training_data[0].get_fcount();

    struct svm_problem prob;
    // set the parameters for svm_problem
    prob.l = fvv_len;
    // feature vector
    prob.x = new svm_node* [prob.l];
    prob.y = new double [prob.l];

    // Sparse storage; libsvm needs only non-zero features
    for (int i = 0; i < fvv_len; ++i) {
	prob.x[i] = new svm_node [get_non_zero_num(training_data[i]) + 1]; // one extra for default -1 at the end
	int non_zero_flag = 0;
	for (int k = 0; k < feature_cnt; ++k) {
	    double fval = training_data[i].get_feature_value(k);
	    if (fval != 0.0) {
		prob.x[i][non_zero_flag].index = k;
		prob.x[i][non_zero_flag].value = fval;
		++non_zero_flag;
	    }
	}
	prob.x[i][non_zero_flag].index = -1;
	prob.x[i][non_zero_flag].value = -1;

	prob.y[i] = training_data[i].get_label();
    }

    const char * error_msg;
    error_msg = svm_check_parameter(&prob, &param);
    if (error_msg)
	throw LetorInternalError("svm_check_parameter failed: %s", error_msg);

    struct svm_model * trainmodel = svm_train(&prob, &param);
    // Generate temporary file to extract model data
    char templ[] = "/tmp/svmtemp.XXXXXX";
    int fd = mkstemp(templ);
    if (fd == -1) {
	throw LetorInternalError("Training failed: ", errno);
    }
    try {
	svm_save_model(templ, trainmodel);
	// Read content of model to string
	std::ifstream f(templ);
	std::ostringstream ss;
	ss << f.rdbuf();
	// Save model string
	this->model_data = ss.str();
	close(fd);
	std::remove(templ);
    } catch (...) {
	close(fd);
	std::remove(templ);
    }
    svm_free_and_destroy_model(&trainmodel);
    clear_svm_problem(&prob);
    svm_destroy_param(&param);
    if (this->model_data.empty()) {
	throw LetorInternalError("SVM model empty. Training failed.");
    }
}

void
SVMRanker::save_model_to_metadata(const string & model_key)
{
    LOGCALL_VOID(API, "SVMRanker::save_model_to_metadata", model_key);
    Xapian::WritableDatabase letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "SVMRanker.model.default";
    }
    letor_db.set_metadata(key, this->model_data);
}

void
SVMRanker::load_model_from_metadata(const string & model_key)
{
    LOGCALL_VOID(API, "SVMRanker::load_model_from_metadata", model_key);
    Xapian::Database letor_db(get_database_path());
    string key = model_key;
    if (key.empty()) {
	key = "SVMRanker.model.default";
    }
    string model_data_load = letor_db.get_metadata(key);
    // Throw exception if no model data associated with key
    if (model_data_load.empty()) {
	throw Xapian::LetorInternalError("No model found. Check key.");
    }
    this->model_data = model_data_load;
}

std::vector<FeatureVector>
SVMRanker::rank_fvv(const std::vector<FeatureVector> & fvv) const
{
    LOGCALL(API, std::vector<FeatureVector>, "SVMRanker::rank_fvv", fvv);
    if (this->model_data.empty()) {
	throw LetorInternalError("SVM model empty. Load correct model.");
    }
    // Generate temporary file containing model data for svm_load_model() method
    struct svm_model * model = NULL;
    char templ[] = "/tmp/svmtemp.XXXXXX";
    int fd = mkstemp(templ);
    if (fd == -1) {
	throw LetorInternalError("Ranking failed: ", errno);
    }
    try {
	std::ofstream f(templ);
	f << this->model_data.c_str();
	f.close();
	model = svm_load_model(templ);
	close(fd);
	std::remove(templ);
    } catch (...) {
	close(fd);
	std::remove(templ);
    }
    if (!model) {
	throw LetorInternalError("Ranking failed. SVM model not usable.");
    }

    std::vector<FeatureVector> testfvv = fvv;

    struct svm_node * test = NULL;
    for (size_t i = 0; i < testfvv.size(); ++i) {
	test = new svm_node [get_non_zero_num(testfvv[i]) + 1];
	int feature_cnt = testfvv[i].get_fcount();
	int non_zero_flag = 0;
	for (int k = 0; k < feature_cnt; ++k) {
	    double fval = testfvv[i].get_feature_value(k);
	    if (fval != 0.0) {
		test[non_zero_flag].index = k;
		test[non_zero_flag].value = fval;
		++non_zero_flag;
	    }
	}
	test[non_zero_flag].index = -1;
	test[non_zero_flag].value = -1;
	testfvv[i].set_score(svm_predict(model, test));
	delete [] test;
    }
    svm_free_and_destroy_model(&model);
    return testfvv;
}
