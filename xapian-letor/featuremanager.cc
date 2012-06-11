#include "featuremanager.h"
#include "letor_features.h"
#include "featurevector.h"
#include "ranklist.h"
#include <cstring>
#include <cstdlib>
#include <fstream>

#include <map>



using namespace Xapian;

using namespace std;



FeatureManager::FeatureManager() {
}

std::string
FeatureManager::getdid(const Document &doc) {
    string id="";
    string data = doc.get_data();
    string temp_id = data.substr(data.find("url=", 0), (data.find("sample=", 0) - data.find("url=", 0)));
    id = temp_id.substr(temp_id.rfind('/') + 1, (temp_id.rfind('.') - temp_id.rfind('/') - 1));  //to parse the actual document name associated with the         documents if any

    return id;
}

int
FeatureManager::getlabel(map<string, map<string, int> > qrel2, const Document &doc, std::string & qid) {
    int label = -1;
    string id = getdid(doc);    

    map<string, map<string, int> >::iterator outerit;
    map<string, int>::iterator innerit;

    outerit = qrel2.find(qid);
    if (outerit != qrel2.end()) {
	innerit = outerit->second.find(id);
	if (innerit != outerit->second.end()) {
        label = innerit->second;
	}
    }
    return label;
}

Xapian::RankList
FeatureManager::create_rank_list(const Xapian::MSet & mset, std::string & qid) {
    Xapian::RankList rl;

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
        
        Xapian::Document doc = i.get_document();
        
        // Here a weight vector can be created in future for different weights of the document 
        // like BM25, LM etc. 
        double weight = i.get_weight();

        map<int,double> fVals = transform(doc, weight);
        string did = getdid(doc);
        int label = getlabel(qrel, doc, qid);

        if(label!=-1) {
            Xapian::FeatureVector fv = create_feature_vector(fVals, label, did);
            rl.set_qid(qid);
            rl.add_feature_vector(fv);
        }
    }
    rl.normalise();
    return rl;
}

Xapian::FeatureVector
FeatureManager::create_feature_vector(map<int,double> fvals, int &label, std::string & did) {
    Xapian::FeatureVector fv;
    fv.set_did(did);
    fv.set_label(label);
    fv.set_fvals(fvals);

    return fv;
}

map<string, map<string,int> >
FeatureManager::load_relevance(const std::string & qrel_file) {
    typedef map<string, int> Map1;      //docid and relevance judjement 0/1
    typedef map<string, Map1> Map2;     // qid and map1
    Map2 qrel1;

    string inLine;
    ifstream myfile (qrel_file.c_str(), ifstream::in);
    string token[4];
    if (myfile.is_open()) {
    while (myfile.good()) {
        getline(myfile, inLine);        //read a file line by line
        char * str;
        char * x1;
        x1 = const_cast<char*>(inLine.c_str());
        str = strtok(x1, " ,.-");
        int i = 0;
        while (str != NULL) {
        token[i] = str;     //store tokens in a string array
        ++i;
        str = strtok(NULL, " ,.-");
        }

        qrel1.insert(make_pair(token[0], Map1()));
        qrel1[token[0]].insert(make_pair(token[2], atoi(token[3].c_str())));
    }
    myfile.close();
    }
    return qrel1;
}




std::map<int,double>
FeatureManager::transform(const Document &doc, double &weight)
{
    map<int, double> fvals;
    map<string,long int> tf = f.termfreq(doc, letor_query);
    map<string, long int> doclen = f.doc_length(letor_db, doc);

    double val[20];// = new double[fCount+1];

    // storing the feature values from array index 1 to sync it with feature number.
    val[1]=f.calculate_f1(letor_query,tf,'t');
    val[2]=f.calculate_f1(letor_query,tf,'b');
    val[3]=f.calculate_f1(letor_query,tf,'w');

    val[4]=f.calculate_f2(letor_query,tf,doclen,'t');
    val[5]=f.calculate_f2(letor_query,tf,doclen,'b');
    val[6]=f.calculate_f2(letor_query,tf,doclen,'w');

    val[7]=f.calculate_f3(letor_query,idf,'t');
    val[8]=f.calculate_f3(letor_query,idf,'b');
    val[9]=f.calculate_f3(letor_query,idf,'w');

    val[10]=f.calculate_f4(letor_query,coll_tf,coll_len,'t');
    val[11]=f.calculate_f4(letor_query,coll_tf,coll_len,'b');
    val[12]=f.calculate_f4(letor_query,coll_tf,coll_len,'w');

    val[13]=f.calculate_f5(letor_query,tf,idf,doclen,'t');
    val[14]=f.calculate_f5(letor_query,tf,idf,doclen,'b');
    val[15]=f.calculate_f5(letor_query,tf,idf,doclen,'w');

    val[16]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'t');
    val[17]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'b');
    val[18]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'w');

// this weight can be either set on the outside how it is done right now
// or, better, extend Enquiry to support advanced ranking models
    val[19]=weight;

    for(int i=0; i<=fNum;i++)
        fvals.insert(pair<int,double>(i,val[i]));

    return fvals;
}


void
FeatureManager::update_collection_level() {
    coll_len = f.collection_length(letor_db);
}


void
FeatureManager::update_query_level() {
    coll_tf = f.collection_termfreq(letor_db, letor_query);
    idf = f.inverse_doc_freq(letor_db, letor_query);
}
