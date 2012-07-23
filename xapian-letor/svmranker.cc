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

#ifndef SVMRANKER_H
#define SVMRANKER_H


#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"

#include <list>
#include <map>

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT SVMRanker: public Ranker {

    string model;
    double[] weight;
  public:
    SVMRanker() {};

    /* Override all the four methods below in the ranker sub-classes files
     * wiz svmranker.cc , listnet.cc, listmle.cc and so on
     */
    Xapian::RankList rank(const Xapian::RankList & rl);

    void learn_model();
    void
/*Letor::Internal::letor_learn_model(int s_type, int k_type) {
    // default values
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
    string input_file_name;
    string model_file_name;
    const char *error_msg;

    input_file_name = get_cwd().append("/train.txt");
    model_file_name = get_cwd().append("/model.txt");

    read_problem(input_file_name.c_str());
    error_msg = svm_check_parameter(&prob, &param);
    if (error_msg) {
	fprintf(stderr, "svm_check_parameter failed: %s\n", error_msg);
	exit(1);
    }

    model = svm_train(&prob, &param);
    if (svm_save_model(model_file_name.c_str(), model)) {
	fprintf(stderr, "can't save model to file %s\n", model_file_name.c_str());
	exit(1);
    }
}*/


    void load_model(const std::string & model_file);

    void save_model();

    double score(const Xapian::FeatureVector & fv);

};

}
#endif /* SVMRANKER_H */
