/** @file letor_internal.cc
 * @brief Internals of Xapian::Letor class
 */
/* Copyright (C) 2011 Parth Gupta
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

#include <config.h>

#include <xapian.h>

#include <xapian-letor/letor.h>

#include "xapian-letor/letor_features.h"
#include "xapian-letor/featuremanager.h"
#include "xapian-letor/featurevector.h"
#include "xapian-letor/ranker.h"
#include "letor_internal.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safeerrno.h"
#include "safeunistd.h"
#include "str.h"
#include "stringutils.h"

#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <math.h>

#include <cstring>
#include <cstdlib>

using namespace std;
using namespace Xapian;

int MAXPATHLEN = 200;

struct FileNotFound { };

//Stop-words
static const char * sw[] = {
    "a", "about", "an", "and", "are", "as", "at",
    "be", "by",
    "en",
    "for", "from",
    "how",
    "i", "in", "is", "it",
    "of", "on", "or",
    "that", "the", "this", "to",
    "was", "what", "when", "where", "which", "who", "why", "will", "with"
};

static string get_cwd() {
    char temp[MAXPATHLEN];
    return (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string());
}

std::vector<Xapian::docid>
Letor::Internal::letor_rank(const Xapian::MSet & mset) {

    map<Xapian::docid, double> letor_mset;

    Xapian::FeatureManager fm;
    fm.set_database(letor_db);
    fm.set_query(letor_query);

    std::vector<FeatureVector> fvv = fm.create_feature_vectors(mset);

    std::vector<FeatureVector> rankedfvv = ranker->rank(fvv);

    int rankedsize = rankedfvv.size();
    std::vector<Xapian::docid> rankeddid;

    for (int i=0; i<rankedsize; ++i){
        rankeddid.push_back(rankedfvv[i].get_did());
    }
    return rankeddid;
}

vector<FeatureVector>
Letor::Internal::load_list_fvecs(const char *filename) { //directly use train.txt instead of train.bin
    fstream train_file (filename, ios::in);
    if(!train_file.good()){
        cout << "No train file found"<<endl;
        throw FileNotFound();
    }

    std::vector<FeatureVector> fvv;
    int k =0;
    while (train_file.peek() != EOF) {
        k++;
        FeatureVector fv;//new a fv

        double label;//read label
        train_file >> label;
        fv.set_label(label);
        train_file.ignore();

        string qid;//read qid
        train_file >> qid;

        qid = qid.substr(qid.find(":")+1,qid.length());

        std::map<int,double> fvals;//read features
        for(int i = 1; i < 20; ++i){
            train_file.ignore();
            int feature_index;
            double feature_value;
            train_file >> feature_index;
            train_file.ignore();
            train_file >> feature_value;
            fvals[feature_index] = feature_value;
        }
        fv.set_fvals(fvals);

        string did;
        train_file >> did;
        did = did.substr(did.find("=")+1,did.length());
        Xapian::docid docid = (Xapian::docid) atoi(did.c_str());

        fv.set_did(docid);
        train_file.ignore();

        fv.set_fcount(20);
        fv.set_score(0);

        fvv.push_back(fv);

    }

    return fvv;
}

void
Letor::Internal::letor_learn_model(){

    //printf("Learning the model..");
    string input_file_name;
    //string model_file_name;
    input_file_name = get_cwd().append("/train.txt");
    //model_file_name = get_cwd().append("/model.txt");

    vector<FeatureVector> list_fvecs = load_list_fvecs(input_file_name.c_str());

    ranker->set_training_data(list_fvecs);

    ranker->train_model();
}

int
Letor::Internal::getlabel(const Document & doc, const std::string & qid)
{
    int label = -1;
    string id = std::to_string(doc.get_docid());

    map<string, map<string, int> >::iterator outerit;
    map<string, int>::iterator innerit;

    outerit = this->qrel.find(qid);
    if (outerit != this->qrel.end()) {
    innerit = outerit->second.find(id);
    if (innerit != outerit->second.end()) {
        label = innerit->second;
    }
    }
    return label;
}

static map<string, map<string,int> >
load_relevance(const std::string & qrel_file)
{
    map<string, map<string, int>> qrel1;     // < qid, <docid, relevance_judgement> >

    string inLine;
    ifstream myfile (qrel_file.c_str(), ifstream::in);
    if(!myfile.good()){
        cout << "No Qrel file found" << endl;
        throw FileNotFound();
    }
    string token[4];
    if (myfile.is_open()) {
    while (myfile.good()) {
        getline(myfile, inLine);        // read a file line by line
        char * str;
        char * x1;
        x1 = const_cast<char*>(inLine.c_str());
        str = strtok(x1, " ,.-");
        int i = 0;
        while (str != NULL) {
        token[i] = str;     // store tokens in a string array
        ++i;
        str = strtok(NULL, " ,.-");
        }

        qrel1[token[0]].insert(make_pair(token[2], atoi(token[3].c_str())));
    }
    myfile.close();
    }
    return qrel1;
}

static void
write_to_file(const std::vector<Xapian::FeatureVector> list_fvecs, const string qid, ofstream & train_file) {

    /* This function will save the vector<FeatureVector> to the training file
     * so that this vector<FeatureVector> can be loaded again by train_model() and subsequent functions.
     */

    // write it down with proper format
    int size_flist = list_fvecs.size();
    for(int i = 0; i < size_flist; ++i) {
       Xapian::FeatureVector fv = list_fvecs[i];
        /* each FeatureVector has the following data: double score, int fcount, string did, map<int, double> fvals
         * each line: double int string 1:double 2:double 3:double....
         */

        double label = fv.get_label();
        std::map<int,double> fvals = fv.get_fvals();
        Xapian::docid did = fv.get_did();

        // now save this feature vector fv to the file
        train_file << label << " qid:" <<qid;// << " ";
        for(int k=1; k < 20; ++k) {//just start from 1 //TODO: create a get_fval method in featurevector class
        train_file << " " << k << ":" << fvals.find(k)->second;
        }
        train_file <<" #docid=" << did<<endl;
    }
}

void
Letor::Internal::prepare_training_file(const string & queryfile, const string & qrel_file, Xapian::doccount msetsize) {

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");

    Xapian::QueryParser parser;
    parser.add_prefix("title", "S");
    parser.add_prefix("subject", "S");

    parser.set_database(letor_db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    Xapian::FeatureManager fm;
    fm.set_database(letor_db);

    this->qrel = load_relevance(qrel_file);

    ofstream train_file;
    train_file.open("train.txt");

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(), ios::in);

    if(!myfile1.good()){
        cout << "No Query file found"<<endl;
        throw FileNotFound();
    }


    while (!myfile1.eof()) {           //reading all the queries line by line from the query file

    	getline(myfile1, str1);

    	if (str1.empty()) {
    	    break;
        }

    	string qid = str1.substr(0, (int)str1.find(" "));
    	string querystr = str1.substr((int)str1.find("'")+1, (str1.length() - ((int)str1.find("'") + 2)));

    	string qq = querystr;
    	istringstream iss(querystr);
    	string title = "title:";
    	while (iss) {
    	    string t;
    	    iss >> t;
    	    if (t.empty())
    		break;
    	    string temp;
    	    temp.append(title);
    	    temp.append(t);
    	    temp.append(" ");
    	    temp.append(qq);
    	    qq = temp;
    	}

    	Xapian::Query query = parser.parse_query(qq,
    						 parser.FLAG_DEFAULT|
    						 parser.FLAG_SPELLING_CORRECTION);

    	Xapian::Enquire enquire(letor_db);
    	enquire.set_query(query);

    	Xapian::MSet mset = enquire.get_mset(0, msetsize);

    	fm.set_query(query);

    	std::vector<FeatureVector> fvv_mset = fm.create_feature_vectors(mset);

        // Set labels from qrel file to FeatureVectors
        int k = 0;
        for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {

            Xapian::Document doc = i.get_document();
            fvv_mset[k++].set_label(getlabel(doc, qid));
        }

        write_to_file(fvv_mset, qid, train_file);

    }
    myfile1.close();
    train_file.close();
}
