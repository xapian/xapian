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

//Map3 normalise(Map3 norm,map< int, list<double> >::iterator norm_outer,list<double>::iterator norm_inner) {
void
RankList::normalise() {
    
/*    if (!norm.empty()) {
	norm_outer=norm.begin();
	norm_outer++;
	int k=0;
	for (;norm_outer!=norm.end();++norm_outer) {
	    k=0;
	    double max= norm_outer->second.front();
	    for (norm_inner = norm_outer->second.begin();norm_inner != norm_outer->second.end(); ++norm_inner) {
		if (*norm_inner > max)
		    max = *norm_inner;
	    }
	    for (norm_inner = norm_outer->second.begin();norm_inner!=norm_outer->second.end();++norm_inner) {
		if (max!=0)      // sometimes value for whole feature is 0 and hence it may cause 'divide-by-zero'
		    *norm_inner /= max;
		k++;
	    }
	}

    }
    return norm;
    */
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
