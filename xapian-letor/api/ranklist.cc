#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/ranklist.h>
#include <xapian-letor/featurevector.h>

#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Xapian;

// TODO: Missing method definitions. Add when integrating rankers and scorers.

RankList::RankList()
{
}

std::vector<FeatureVector>
RankList::normalise() {

    std::vector<FeatureVector> local_fvv = this->fvv;

    // find the max value for each feature gpr all the FeatureVectors in the RankList rl.
    int num_features = 19;
    double temp = 0.0;
    double max[num_features];

    for(int i=0; i<19; ++i)
    max[i] = 0.0;

    int num_fv = local_fvv.size();
    for(int i=0; i < num_fv; ++i) {
        for(int j=0; j<19; ++j) {
            if(max[j] < local_fvv[i].get_fvals().find(j)->second)
            max[j] = local_fvv[i].get_fvals().find(j)->second;
        }
    }

    /* We have the maximum value of each feature overall.
       Now we need to normalize each feature value of a featureVector by dividing it by the corresponding max of the feature value
    */

    for(int i=0; i < num_fv; ++i) {
        for(int j=0; j<19; ++j) {
            temp = local_fvv[i].get_fvals().find(j)->second;
            temp /= max[j];
            local_fvv[i].get_fvals().insert(pair<int,double>(j,temp));
            temp = 0.9;
        }
    }
    return local_fvv;
}

void
RankList::add_feature_vector(const Xapian::FeatureVector fv1) {
    this->fvv.push_back(fv1);
}

void
RankList::set_qid(std::string qid1) {
    this->qid=qid1;
}

std::string
RankList::get_qid(){
    return this->qid;
}

void
RankList::set_fvv(std::vector<FeatureVector> & local_fvv) {
    this->fvv=local_fvv;
}

std::vector<FeatureVector>
RankList::get_fvv() {
    return this->fvv;
}
