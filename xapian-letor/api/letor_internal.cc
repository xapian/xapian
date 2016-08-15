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

#include "xapian-letor/feature.h"
#include "xapian-letor/featurelist.h"
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

std::vector<Xapian::docid>
Letor::Internal::letor_rank(const Xapian::MSet & mset, const char* model_filename,
                            Xapian::FeatureList & flist) {

    std::vector<FeatureVector> fvv = flist.create_feature_vectors(mset, letor_query, letor_db);

    ranker->load_model_from_file(model_filename);
    std::vector<FeatureVector> rankedfvv = ranker->rank(fvv);

    int rankedsize = rankedfvv.size();
    std::vector<Xapian::docid> rankeddid;

    for (int i=0; i<rankedsize; ++i){
	rankeddid.push_back(rankedfvv[i].get_did());
    }
    return rankeddid;
}

vector<FeatureVector>
Letor::Internal::load_list_fvecs(const char *filename) { // directly use train.txt instead of train.bin
    fstream train_file (filename, ios::in);
    if(!train_file.good()){
	cout << "No train file found"<<endl;
	throw FileNotFound();
    }

    std::vector<FeatureVector> fvv;
    while (train_file.peek() != EOF) {

	// A training file looks like this:
	// <label> qid:<xxx> n:<fval> #docid:<xxx>
	// e.g. 1 qid:002 1:0 2:0.792481 3:0.792481 ... #docid=8

	FeatureVector fv;

	double label;
	train_file >> label; // *<label>* qid:<xxx> n:<fval> #docid=<xxx>
	fv.set_label(label);
	train_file.ignore(); // <label>* *qid:<xxx> n:<fval> #docid=<xxx>

	string qid;
	train_file >> qid;   // <label> *qid:<xxx>* n:<fval> #docid=<xxx>

	qid = qid.substr(qid.find(":")+1,qid.length()); // <label> qid:*<xxx>* n:<fval> #docid=<xxx>
	train_file.ignore(); // <label> qid:<xxx>* *n:<fval> #docid=<xxx>

	std::vector<double> fvals;

	while (train_file.peek() != '#') { // read till '#docid' is found
	    int feature_index;
	    double feature_value;
	    train_file >> feature_index; // <label> qid:<xxx> *n*:<fval> #docid=<xxx>
	    train_file.ignore();         // <label> qid:<xxx> n*:*<fval> #docid=<xxx>
	    train_file >> feature_value; // <label> qid:<xxx> n:*<fval>* #docid=<xxx>
	    fvals.push_back(feature_value);
	    train_file.ignore();         // <label> qid:<xxx> n:<fval>* *#docid=<xxx>
	}
	fv.set_fvals(fvals);

	string did;
	train_file >> did;               // <label> qid:<xxx> n:<fval> *#docid=<xxx>*
	did = did.substr(did.find("=")+1,did.length()); // #docid=*<xxx>*
	Xapian::docid docid = (Xapian::docid) atoi(did.c_str());

	fv.set_did(docid);
	train_file.ignore();

	fv.set_score(0);

	fvv.push_back(fv);

    }

    return fvv;
}

void
Letor::Internal::letor_learn_model(const char* input_filename, const char* output_filename){

    std::cout << "Learning the model from \"" << input_filename <<"\"" << endl;

    vector<FeatureVector> list_fvecs = load_list_fvecs(input_filename);

    ranker->set_training_data(list_fvecs);

    ranker->train_model();

    ranker->save_model_to_file(output_filename);
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
	std::vector<double> fvals = fv.get_fvals();
	Xapian::docid did = fv.get_did();

	// now save this feature vector fv to the file
	train_file << label << " qid:" <<qid;// << " ";
	for(int k=0; k < fv.get_fcount(); ++k) {
	    train_file << " " << (k+1) << ":" << fvals[k];
	}
	train_file <<" #docid=" << did<<endl;
    }
}

void
Letor::Internal::prepare_training_file(const string & queryfile, const string & qrel_file,
				       Xapian::doccount msetsize, const char* filename,
				       FeatureList & flist) {

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

    this->qrel = load_relevance(qrel_file);

    ofstream train_file;
    train_file.open(filename);

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(), ios::in);

    if(!myfile1.good()){
	cout << "No Query file found"<<endl;
	throw FileNotFound();
    }


    while (!myfile1.eof()) {           // reading all the queries line by line from the query file

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

    	std::vector<FeatureVector> fvv_mset = flist.create_feature_vectors(mset, query, letor_db);
        std::vector<FeatureVector> fvv_qrel;

        int k = 0;
        for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {

            Xapian::Document doc = i.get_document();
            int label = getlabel(doc, qid);
            if (label != -1) { // only add FeatureVector which is found in the qrel file
                fvv_mset[k].set_label(label);
                fvv_qrel.push_back(fvv_mset[k]);
            }
            k += 1;
        }

        write_to_file(fvv_qrel, qid, train_file);

    }
    myfile1.close();
    train_file.close();
    cout << "Training file stored as: \"" << filename << "\"" << endl;
}

void
Letor::Internal::letor_score(const std::string & query_file,
                             const std::string & qrel_file,
                             const std::string & model_file,
                             Xapian::doccount msetsize,
                             Xapian::FeatureList & flist) {

    ranker->load_model_from_file(model_file.c_str());

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

    this->qrel = load_relevance(qrel_file);

    string str1;
    ifstream queryfile;
    queryfile.open(query_file.c_str(), ios::in);

    if(!queryfile.good()){
        cout << "No Query file found"<<endl;
        throw FileNotFound();
    }

    double total_score = 0;
    int num_queries = 0;

    while (!queryfile.eof()) {           //reading all the queries line by line from the query file

        getline(queryfile, str1);

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

        std::vector<FeatureVector> fvv_mset = flist.create_feature_vectors(mset, query, letor_db);
        std::vector<FeatureVector> rankedfvv = ranker->rank(fvv_mset);
        std::vector<FeatureVector> rankedfvv_qrel;

        int k = 0;
        for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {

            Xapian::Document doc = i.get_document();
            int label = getlabel(doc, qid);
            if (label != -1) { // only add FeatureVector which is found in the qrel file
                rankedfvv[k].set_label(label);
                rankedfvv_qrel.push_back(rankedfvv[k]);
            }
            k += 1;
        }

        double score = scorer->score(rankedfvv);
        cout << "Ranking score for qid:" << qid << " = " << score << endl;

        total_score += score;
        num_queries += 1;

    }
    queryfile.close();
    total_score = total_score/num_queries;
    cout << "Average ranking score = " << total_score << endl;

}
