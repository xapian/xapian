/* listmle.h: The abstract ranker file.
 *
 * Copyright (C) 2012 Rishabh Mehrotra
 *
 * Implementation likely based on code without a suitable
 * license.
 */

#ifndef LISTMLE_H
#define LISTMLE_H


#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"

#include <list>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
//typedef map<int,double> tuple;
//typedef vector<tuple> instance;
//typedef vector<double> scores;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT ListMLE: public Ranker {

  public:
    string models;
    vector<double> parameters;
    double tolerance_rate;
    double learning_rate;
    //vector<scores> all_tuple_scores;
    
  public:
    ListMLE() {};


    /* Override all the four methods below in the ranker sub-classes files
     * wiz listmle.cc , listnet.cc, listmle.cc and so on
     */
    Xapian::RankList rank(const Xapian::RankList rlist);

    void learn_model();

    void load_model(const std::string & model_file_name);

    void save_model(const std::string & model_file_name);

    double score_doc(Xapian::FeatureVector fv);
    
    //vector<double> listmle_train(vector<instance> & instances, double tolerance_rate, double learning_rate);
    
    vector<double> listmle_train(vector<RankList> & samples);
    
    double absolute (double a);

};

}
#endif /* LISTMLE_H */
