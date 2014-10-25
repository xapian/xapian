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

#include <xapian/letor.h>

#include <xapian.h>

#include "letor_internal.h"
#include "featuremanager.h"
#include "str.h"
#include "stringutils.h"
#include "ranker.h"
#include "svmranker.h"
#include "letor_features.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safeerrno.h"
#include "safeunistd.h"

#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <math.h>

#include <libsvm/svm.h>
#define Malloc(type, n) (type *)malloc((n) * sizeof(type))

using namespace std;

using namespace Xapian;

typedef vector<Xapian::RankList> Samples;

struct svm_parameter param;
struct svm_problem prob;
struct svm_model *model;
struct svm_node *x_space;
int cross_validation;
int nr_fold;



struct svm_node *x;
int max_nr_attr = 64;

int predict_probability = 0;

//static char *line = NULL;
//static int max_line_len;

int MAXPATHLEN = 200;


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




/*static void exit_input_error(int line_num) {
    printf("Error at Line : %d", line_num);
    exit(1);
}*/
/*
static string convertDouble(double value) {
    std::ostringstream o;
    if (!(o << value))
	return string();
    return o.str();
}
*/
static string get_cwd() {
    char temp[MAXPATHLEN];
    return (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string());
}



/* This method will calculate the score assigned by the Letor function.
 * It will take MSet as input then convert the documents in feature vectors
 * then normalize them according to QueryLevelNorm
 * and after that use the machine learned model file
 * to assign a score to the document
 */
map<Xapian::docid, double>
Letor::Internal::letor_score(const Xapian::MSet & mset) {

    map<Xapian::docid, double> letor_mset;

    Xapian::FeatureManager fm;
    fm.set_database(letor_db);
    fm.set_query(letor_query);
    
    std::string s = "1";
    Xapian::RankList rlist = fm.create_rank_list(mset, s);
    
    std::vector<double> scores = ranker.rank(rlist);
    
    /*Converting list<double> scores to map<docid,double> letor_mset*/
    int num_fv = scores.size();
    for(int i=0; i<num_fv; ++i) {
	//Xapian::docid did = (Xapian::docid) rlist.rl[i].did;//need to convert did from string to Xapian::docid
	Xapian::docid did = (Xapian::docid) atoi(rlist.rl[i].did.c_str());//need to convert did from string to Xapian::docid
	letor_mset.insert(pair<Xapian::docid,double>(did, scores[i]));
    }
    
    return letor_mset;
}

/*
static char* readline(FILE *input) {
    int len;

    if (fgets(line, max_line_len, input) == NULL)
	return NULL;

    while (strrchr(line, '\n') == NULL) {
	max_line_len *= 2;
	line = (char *)realloc(line, max_line_len);
	len = (int)strlen(line);
	if (fgets(line + len, max_line_len - len, input) == NULL)
	    break;
    }
    return line;
}

static void
read_problem(const char *filename) {
    int elements, max_index, inst_max_index, i, j;
    FILE *fp = fopen(filename, "r");
    char *endptr;
    char *idx, *val, *label;

    if (fp == NULL) {
	fprintf(stderr, "can't open input file %s\n", filename);
	exit(1);
    }

    prob.l = 0;
    elements = 0;
    max_line_len = 1024;
    line = Malloc(char, max_line_len);

    while (readline(fp) != NULL) {
	char *p = strtok(line, " \t"); // label

	// features
	while (1) {
	    p = strtok(NULL, " \t");
	    if (p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
		break;
	    ++elements;
	}
	++elements;
	++prob.l;
    }
    rewind(fp);

    prob.y = Malloc(double, prob.l);
    prob.x = Malloc(struct svm_node *, prob.l);
    x_space = Malloc(struct svm_node, elements);

    max_index = 0;
    j = 0;

    for (i = 0; i < prob.l; ++i) {
	inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
	readline(fp);
	prob.x[i] = &x_space[j];
	label = strtok(line, " \t\n");
	if (label == NULL) // empty line
	    exit_input_error(i + 1);
	prob.y[i] = strtod(label, &endptr);
	if (endptr == label || *endptr != '\0')
	    exit_input_error(i + 1);

	while (1) {
	    idx = strtok(NULL, ":");
	    val = strtok(NULL, " \t");

	    if (val == NULL)
		break;

	    errno = 0;
	    x_space[j].index = (int)strtol(idx, &endptr, 10);

	    if (endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
		exit_input_error(i + 1);
	    else
		inst_max_index = x_space[j].index;

	    errno = 0;
	    x_space[j].value = strtod(val, &endptr);

	    if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
		exit_input_error(i + 1);

	    ++j;
	}

	if (inst_max_index > max_index)
	    max_index = inst_max_index;
	x_space[j++].index = -1;
    }

    if (param.gamma == 0 && max_index > 0)
	param.gamma = 1.0 / max_index;

    if (param.kernel_type == PRECOMPUTED)
	for (i = 0; i < prob.l; ++i) {
	    if (prob.x[i][0].index != 0) {
		fprintf(stderr, "Wrong input format: first column must be 0:sample_serial_number\n");
		exit(1);
	    }
	    if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index) {
		fprintf(stderr, "Wrong input format: sample_serial_number out of range\n");
		exit(1);
	    }
	}
	fclose(fp);
}
*/
vector<Xapian::RankList>
Letor::Internal::load_list_ranklist(const char *filename) { //train.bin
    fstream train_file (filename, ios::in | ios::out | ios::binary);
    int size = 0;
    train_file.read((char *)(&size), sizeof(size));
    vector<Xapian::RankList> samples;// = (Samples *) malloc(size);
    train_file.read((char*) &samples, size);
    train_file.close();
    return samples;
}

void
Letor::Internal::letor_learn_model() {

    printf("Learning the model..");
    string input_file_name;
    string model_file_name;
    input_file_name = get_cwd().append("/train.bin");
    model_file_name = get_cwd().append("/model.txt");
    
    //read_problem(input_file_name.c_str());
    
    vector<Xapian::RankList> samples = load_list_ranklist(input_file_name.c_str());
    
    ranker.set_training_data(samples);
    
    ranker.learn_model();
}


/* This is the method which prepares the train.txt file of the standard Letor Format.
 * param 1 = Xapian Database
 * param 2 = path to the query file
 * param 3 = path to the qrel file
 *
 * output : It produces the train.txt method in /core/examples/ which is taken as input by learn_model() method
 */

static void
write_to_file(std::vector<Xapian::RankList> list_rlist) {

    /* This function will save the list<RankList> to the training file
     * so that this list<RankList> can be loaded again by train_model() and subsequent functions.
     */
    ofstream train_file;
    train_file.open("train.txt");
    // write it down with proper format
    int size_rlist = list_rlist.size();
    //for (list<Xapian::RankList>::iterator it = l.begin(); it != l.end(); it++);
    for(int i = 0; i < size_rlist; ++i) {
	RankList rlist = list_rlist[i];
	/* now save this RankList...each RankList has a vectorr<FeatureVector>
	 * each FeatureVector has the following data: double score, int fcount, string did, map<int, double> fvals
	 * each line: double int string 1:double 2:double 3:double....
	 */
	int size_rl = rlist.rl.size();
	// print the size of the rlist so that later we know how many featureVector to scan for this particular rlist.
	train_file << size_rl << " " << rlist.qid << endl;
	for(int j=0; j < size_rl; ++j) {
	    FeatureVector fv = rlist.rl[j];
	    // now save this feature vector fv to the file
	    train_file << fv.score << " " << fv.fcount << " " << fv.did;// << " ";
	    for(int k=0; k < fv.fcount; ++k) {
		train_file << " " << k << ":" << fv.fvals.find(k)->second;
	    }
	    train_file << endl;
	}
    }
    train_file.close();
}

static void
write_ranklist(std::vector<Xapian::RankList> list_rlist) {
    fstream train_file ("train.bin", ios::in | ios::out | ios::binary);
    long int size = sizeof(list_rlist);
    train_file.write ((char*) &size, sizeof(size));
    train_file.write ((char*) &list_rlist, sizeof(list_rlist));
    train_file.close();
}

void
Letor::Internal::prepare_training_file_listwise(const string & /*queryfile*/, int /*num_features*/) {
}

void
Letor::Internal::prepare_training_file(const string & queryfile, const string & qrel_file, Xapian::doccount msetsize) {

//    ofstream train_file;
//    train_file.open("train.txt");

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

    /* ---------------------------- store whole qrel file in a Map<> ---------------------*/

//    typedef map<string, int> Map1;      //docid and relevance judjement 0/1
//    typedef map<string, Map1> Map2;     // qid and map1
//    Map2 qrel;

    map<string, map<string, int> > qrel; // 1

    Xapian::FeatureManager fm;
    fm.set_database(letor_db);
    fm.load_relevance(qrel_file);
    qrel = fm.load_relevance(qrel_file);

    vector<Xapian::RankList> list_rlist;

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(), ios::in);


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

	cout << "Processing Query: " << qq << "\n";
  
	Xapian::Query query = parser.parse_query(qq,
						 parser.FLAG_DEFAULT|
						 parser.FLAG_SPELLING_CORRECTION);

	Xapian::Enquire enquire(letor_db);
	enquire.set_query(query);

	Xapian::MSet mset = enquire.get_mset(0, msetsize);

	fm.set_query(query);

	Xapian::RankList rl = fm.create_rank_list(mset, qid);
	list_rlist.push_back(rl);
    }//while closed
    myfile1.close();
    /* Call either one of the following
     */
    write_to_file(list_rlist);
    write_ranklist(list_rlist);
//    train_file.close();
}
