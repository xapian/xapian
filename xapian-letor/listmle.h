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

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT ListMLE: public Ranker {

    string model = null;
    double[] weight;
    vector<double> params;
    double tolerance_rate;
    double learning_rate;
    
  public:
    ListMLE() {};

    /* Override all the four methods below in the ranker sub-classes files
     * wiz listmle.cc , listnet.cc, listmle.cc and so on
     */
    Xapian::RankList rank(const Xapian::RankList & rl);

    void learn_model();

    void load_model(const std::string & model_file);

    void save_model();

    double score(const Xapian::FeatureVector & fv);

};

}
#endif /* LISTMLE_H */
