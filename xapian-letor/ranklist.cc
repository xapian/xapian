#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <featurevector.h>

#include <list>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Xapian;

//Ranklist(const Xapian::MSet & mset,const Xapian::Database & db,const Xapian::Query & query)
RankList::RankList()
{
    /*map<Xapian::docid,double> letor_mset;

    map<string,long int> coll_len;
    coll_len=collection_length(letor_db);

    map<string,long int> coll_tf;
    coll_tf=collection_termfreq(letor_db,letor_query);

    map<string,double> idf;
    idf=inverse_doc_freq(letor_db,letor_query);

    int first=1;                //used as a flag in QueryLevelNorm module

  //the above list will be mapped to an integer with its feature id.

	


    map< int, list<double> >::iterator norm_outer;
    list<double>::iterator norm_inner;


    List2 doc_ids;

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
	Xapian::Document doc = i.get_document();
	
	FeatureVector fv;
	fv.set_database(letor_db);
	fv.set_query(letor_query);
	std::map<int,double> fvals=fv.transform(doc);
	
	add_feature_vector(fv);
	
	if (first==1) {
	    for (int j=1;j<20;j++) {
		List1 l;
		l.push_back(fvals[j]);
		norm.insert(pair <int , list<double> > (j,l));
	    }
	    first=0;
	} else {
	    norm_outer=norm.begin();
	    int k=1;
	    for (;norm_outer!=norm.end();norm_outer++) {
		norm_outer->second.push_back(fvals[k]);
		k++;
	    }
	}
	
	}
	norm = normalise(norm,norm_outer,norm_inner);
	*/
}

std::vector<FeatureVector>
RankList::normalise() {

    std::vector<FeatureVector> local_rl = this->rl;
    
    // find the max value for each feature gpr all the FeatureVectors in the RankList rl.
    int num_features = 19;
    double temp = 0.0;
    double max[num_features];
    
    for(int i=0; i<19; ++i)
	max[i] = 0.0;
    
    int num_fv = local_rl.size();
    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<19; ++j) {
	    if(max[j] < local_rl[i].fvals.find(j)->second)
		max[j] = local_rl[i].fvals.find(j)->second;
	}
    }
    
    /* We have the maximum value of each feature overall.
       Now we need to normalize each feature value of a featureVector by dividing it by the corresponding max of the feature value
    */
    
    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<19; ++j) {
	    temp = local_rl[i].fvals.find(j)->second;
	    temp /= max[j];
	    local_rl[i].fvals.insert(pair<int,double>(j,temp));
	    temp = 0.9;
	}
    }
    
    return local_rl;
}

void
RankList::add_feature_vector(const Xapian::FeatureVector fv) {
    this->rl.push_back(fv);
}

void
RankList::set_qid(std::string qid1) {
    this->qid=qid1;
}

void
RankList::set_rl(std::vector<FeatureVector> local_rl) {
    this->rl=local_rl;
}

std::vector<FeatureVector> 
RankList::get_data() {
    return this->rl;
}

std::vector<FeatureVector>
RankList::sort_by_score() {
    std::vector<FeatureVector> local_rl = this->rl;
    sort(local_rl.begin(), local_rl.end(), FeatureVector::before);
    return local_rl;
}
