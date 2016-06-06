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
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <math.h>

#include <stdio.h>

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

    std::string s = "virtual_qid"; //TODO: Figure out virtual qid
    Xapian::RankList rlist = fm.create_rank_list(mset, s, 0);

    Xapian::RankList ranklist = ranker->rank(rlist);

    std::vector<Xapian::FeatureVector> rankedfvv = ranklist.get_fvv();
    int rankedsize = rankedfvv.size();
    std::vector<Xapian::docid> rankeddid;

    for (int i=0; i<rankedsize; ++i){
        rankeddid.push_back(rankedfvv[i].get_did());
    }
    return rankeddid;
}

vector<Xapian::RankList>
Letor::Internal::load_list_ranklist(const char *filename) { //directly use train.txt instead of train.bin
    vector<Xapian::RankList> ranklist;
    fstream train_file (filename, ios::in);
    if(!train_file.good()){
        cout << "No train file found"<<endl;
        throw FileNotFound();
    }

    std::vector<FeatureVector> fvv;
    string lastqid;
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

        if (lastqid==""){
            lastqid = qid;
        }
        //std::cout << fv.did << endl;

        std::map<int,double> fvals;//read features
        for(int i = 1; i < 20; ++i){
            train_file.ignore();
            int feature_index;
            double feature_value;
            train_file >> feature_index;
            train_file.ignore();
            train_file >> feature_value;
            //std::cout<<feature_index<<" "<<feature_value<<endl;
            fvals[feature_index] = feature_value;
        }
        fv.set_fvals(fvals);

        string did;
        train_file >> did;
        //cout << "original did: " <<did<<endl;
        did = did.substr(did.find("=")+1,did.length());
        Xapian::docid docid = (Xapian::docid) atoi(did.c_str());
        //cout << "did: " << did <<endl;
        //cout << "docid: " << docid << endl;


        fv.set_did(docid);
        train_file.ignore();
        //std::cout << fv.did << endl;

        fv.set_fcount(20);
        fv.set_score(0);

        fvv.push_back(fv);

        if (qid != lastqid) {
            RankList rlist;
            rlist.set_qid(lastqid);
            //cout << "show qid: "<< lastqid << endl;
            rlist.set_fvv(fvv);
            ranklist.push_back(rlist);
            fvv = std::vector<FeatureVector>();
            lastqid = qid;
        }

    }
    RankList rlist;
    rlist.set_qid(lastqid);
    //cout << "show qid: "<< lastqid << endl;
    rlist.set_fvv(fvv);
    ranklist.push_back(rlist);

    train_file.close();
    //cout << "ranklist loading OK" <<endl;
    cout << "the size of ranklist read from the training set: " << ranklist.size() <<endl;
    return ranklist;
}

void
Letor::Internal::letor_learn_model(){

    //printf("Learning the model..");
    string input_file_name;
    //string model_file_name;
    input_file_name = get_cwd().append("/train.txt");
    //model_file_name = get_cwd().append("/model.txt");

    vector<Xapian::RankList> samples = load_list_ranklist(input_file_name.c_str());

    ranker->set_training_data(samples);

    ranker->train_model();
}

static void
write_to_file(std::vector<Xapian::RankList> list_rlist) {

    /* This function will save the list<RankList> to the training file
     * so that this list<RankList> can be loaded again by train_model() and subsequent functions.
     */

    ofstream train_file;
    train_file.open("train.txt");
    // write it down with proper format
    int size_rlist = list_rlist.size();//return the size of list_rlist
    //for (list<Xapian::RankList>::iterator it = l.begin(); it != l.end(); it++);
    for(int i = 0; i < size_rlist; ++i) {
       Xapian::RankList rlist = list_rlist.begin()[i];
        /* now save this RankList...each RankList has a vectorr<FeatureVector>
         * each FeatureVector has the following data: double score, int fcount, string did, map<int, double> fvals
         * each line: double int string 1:double 2:double 3:double....
         */
        int size_rl = rlist.fvv.size();
        // print the size of the rlist so that later we know how many featureVector to scan for this particular rlist.
        // train_file << size_rl << " " << rlist.qid << endl;
        string qid =rlist.get_qid();
        for(int j=0; j < size_rl; ++j) {
            FeatureVector fv = rlist.fvv[j];

            double label = fv.get_label();
            std::map<int,double> fvals = fv.get_fvals();
            Xapian::docid did = fv.get_did();

            // now save this feature vector fv to the file
            // cout <<"label"<<fv.label<< "fv.score " << fv.score << "fv.fcount " << fv.fcount << "fv.did " << fv.did << endl;// << " ";
            train_file << label << " qid:" <<qid;// << " ";
            // if (fv.fcount==0){cout << "fcount is empty";}
            for(int k=1; k < 20; ++k) {//just start from 1 //TODO: create a get_fval method in featurevector class
            train_file << " " << k << ":" << fvals.find(k)->second;
            // cout << "write fcount" << endl;
            }
            train_file <<" #docid=" << did<<endl;
    	}
    }
    train_file.close();
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
    fm.load_relevance(qrel_file);

    vector<Xapian::RankList> l;

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
            //cout<< "str1 empty";
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

    	//cout << "Processing Query: " << qq << "\n";

    	Xapian::Query query = parser.parse_query(qq,
    						 parser.FLAG_DEFAULT|
    						 parser.FLAG_SPELLING_CORRECTION);

    	Xapian::Enquire enquire(letor_db);
    	enquire.set_query(query);

    	Xapian::MSet mset = enquire.get_mset(0, msetsize);

    	fm.set_query(query);

    	Xapian::RankList rl = fm.create_rank_list(mset, qid, 1);

        l.push_back(rl);
    }
    myfile1.close();
    write_to_file(l);
}
