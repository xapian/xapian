/** @file
 * @brief Implementation of Ranker class
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
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

#include "xapian-letor/ranker.h"

#include "xapian-letor/featurelist.h"
#include "xapian-letor/featurevector.h"
#include "xapian-letor/letor_error.h"
#include "xapian-letor/scorer.h"

#include "debuglog.h"
#include "omassert.h"
#include "str.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace Xapian;

// Map to store qrels
map<string, map<string, int>> qrel;

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

static vector<FeatureVector>
load_list_fvecs(const string & filename)
{
    fstream train_file(filename, ios::in);
    if (!train_file.good())
	throw Xapian::FileNotFoundError("No training file found. Check path.");

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
	qid = qid.substr(qid.find(":") + 1, qid.length()); // <label> qid:*<xxx>* n:<fval> #docid=<xxx>
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
	did = did.substr(did.find("=") + 1, did.length()); // #docid=*<xxx>*
	Xapian::docid docid = (Xapian::docid) atoi(did.c_str());
	fv.set_did(docid);
	train_file.ignore();

	fv.set_score(0);
	fvv.push_back(fv);
    }
    return fvv;
}

static int
getlabel(const Document & doc, const std::string & qid)
{
    int label = -1;
    string id = str(doc.get_docid());
    map<string, map<string, int>>::const_iterator outerit;
    map<string, int>::const_iterator innerit;

    outerit = qrel.find(qid);
    if (outerit != qrel.end()) {
	innerit = outerit->second.find(id);
	if (innerit != outerit->second.end()) {
	    label = innerit->second;
	}
    }
    return label;
}

static map<string, map<string, int>>
load_relevance(const std::string & qrel_file)
{
    map<string, map<string, int>> qrel1;     // <qid, <docid, relevance_judgement>>

    string line;
    ifstream myfile(qrel_file.c_str(), ifstream::in);
    if (!myfile.good()) {
	throw Xapian::FileNotFoundError("No Qrel file found. Check path.");
    }
    int qrel_count = 0;
    if (myfile.is_open()) {
	while (myfile.good()) {
	    getline(myfile, line);        // read a file line by line
	    if (line.empty()) {
		break;
	    }
	    ++qrel_count;
	    vector<string> token;
	    size_t j = 0;
	    while (j < line.size()) {
		size_t i = line.find_first_not_of(' ', j);
		if (i == string::npos) break;
		j = line.find_first_of(' ', i);
		token.push_back(line.substr(i, j - i));
	    }
	    // Exceptions for parse errors
	    if (token.size() != 4 || token[1] != "Q0") {
		throw LetorParseError("Could not parse Qrel file at line:" + str(qrel_count));
	    }
	    // Exception if relevance label is not a number
	    char * end;
	    int label = int(strtol(token[3].c_str(), &end, 10));
	    if (*end) {
		throw LetorParseError("Could not parse relevance label in Qrel file at line:" + str(qrel_count));
	    }
	    qrel1[token[0]].insert(make_pair(token[2], label));
	}
	myfile.close();
    }
    return qrel1;
}

static void
write_to_file(const std::vector<Xapian::FeatureVector> & list_fvecs, const string & qid, ofstream & train_file)
{
    /* This function will save the vector<FeatureVector> to the training file
     * so that this vector<FeatureVector> can be loaded again by train_model() and subsequent functions.
     */
    // write it down with proper format
    for (size_t i = 0; i < list_fvecs.size(); ++i) {
	Xapian::FeatureVector fv = list_fvecs[i];
	double label = fv.get_label();
	std::vector<double> fvals = fv.get_fvals();
	Xapian::docid did = fv.get_did();

	train_file << label << " qid:" << qid;
	for (int k = 0; k < fv.get_fcount(); ++k) {
	    train_file << " " << (k + 1) << ":" << fvals[k];
	}
	train_file << " #docid=" << did << endl;
    }
}

// Query file is in the format: <qid> '<query_string>'
// Although it will accept any number of arbitrary characters between
// the first space and the first single quote.
static std::pair<string, string>
parse_query_string(const string & query_line, int line_number)
{
    Assert(!query_line.empty());
    string::size_type j = query_line.find_first_of(' ');
    if (j == 0) {
	throw LetorParseError("Empty query id found in "
			      "file at line:" + str(line_number));
    }
    if (j == string::npos) {
	throw LetorParseError("Missing space between fields in Query "
			      "file at line:" + str(line_number));
    }
    string qid = query_line.substr(0, j);
    string::size_type i = query_line.find_first_of("'", j);
    j = query_line.length() - 1;
    // check if the last character is '
    if (query_line[j] != '\'' || j == i) {
	throw LetorParseError("Could not parse Query file at line:" +
			       str(line_number));
    }
    string querystr = query_line.substr(i + 1, j - i - 1);

    if (querystr.empty()) {
	throw LetorParseError("Empty query string in query file at line:" +
			       str(line_number));
    }
    return std::pair<string, string> (querystr, qid);
}

static Xapian::QueryParser
initialise_queryparser(const Xapian::Database & db)
{
    Xapian::SimpleStopper* mystopper;
    mystopper = new Xapian::SimpleStopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");
    Xapian::QueryParser parser;
    parser.add_prefix("title", "S");
    parser.add_prefix("subject", "S");
    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(mystopper->release());
    return parser;
}

void
Xapian::prepare_training_file(const string & db_path, const string & queryfile,
		      const string & qrel_file, Xapian::doccount msetsize,
		      const string & filename, const FeatureList & flist)
{
    // Set db
    Xapian::Database letor_db(db_path);

    Xapian::QueryParser parser = initialise_queryparser(letor_db);

    qrel = load_relevance(qrel_file);

    ofstream train_file;
    train_file.open(filename);

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(), ios::in);
    if (!myfile1.good()) {
	throw Xapian::FileNotFoundError("No Query file found. Check path.");
    }

    int query_count = 0;
    set<string> queries;
    while (!myfile1.eof()) { // reading all the queries line by line from the query file
	getline(myfile1, str1);
	if (str1.empty()) {
	    break;
	}
	++query_count;

	std::pair<string, string> parsed_query = parse_query_string(str1, query_count);
	string querystr = parsed_query.first;
	string qid = parsed_query.second;
	if (!queries.insert(qid).second) {
	    throw Xapian::LetorParseError("Query id should be unique");
	}

	Xapian::Query query_no_prefix = parser.parse_query(querystr,
					parser.FLAG_DEFAULT|
					parser.FLAG_SPELLING_CORRECTION);
	// query with 'title' field as default prefix "S"
	Xapian::Query query_default_prefix = parser.parse_query(querystr,
					     parser.FLAG_DEFAULT|
					     parser.FLAG_SPELLING_CORRECTION,
					     "S");
	// Combine queries
	Xapian::Query query = Xapian::Query(Xapian::Query::OP_OR, query_no_prefix, query_default_prefix);

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
	    ++k;
	}
	write_to_file(fvv_qrel, qid, train_file);
    }
    myfile1.close();
    train_file.close();
}

Ranker::Ranker()
{
    LOGCALL_CTOR(API, "Ranker", NO_ARGS);
}

Ranker::~Ranker()
{
    LOGCALL_DTOR(API, "Ranker");
}

void
Ranker::set_database_path(const string & dbpath)
{
    LOGCALL_VOID(API, "Ranker::set_database_path", dbpath);
    db_path = dbpath;
}

std::string
Ranker::get_database_path()
{
    LOGCALL(API, std::string, "Ranker::get_database_path", NO_ARGS);
    return db_path;
}

void
Ranker::set_query(const Query & query)
{
    LOGCALL_VOID(API, "Ranker::set_query", query);
    letor_query = query;
}

bool
Ranker::scorecomparer(const FeatureVector & firstfv, const FeatureVector& secondfv)
{
    LOGCALL(API, bool, "Ranker::scorecomparer", firstfv | secondfv);
    return firstfv.get_score() > secondfv.get_score();
}

bool
Ranker::labelcomparer(const FeatureVector & firstfv, const FeatureVector& secondfv)
{
    LOGCALL(API, bool, "Ranker::labelcomparer", firstfv | secondfv);
    return firstfv.get_label() > secondfv.get_label();
}

class ScoreIterator {
    std::vector<FeatureVector>::const_iterator it;

  public:
    explicit ScoreIterator(const std::vector<FeatureVector>::const_iterator it_) : it(it_) { }

    double operator*() const { return it->get_score(); }

    void operator++() { ++it; }

    bool operator!=(const ScoreIterator& o) { return it != o.it; }

    bool operator==(const ScoreIterator& o) { return it == o.it; }

    Xapian::doccount operator-(const ScoreIterator& o) { return Xapian::doccount(it - o.it); }
};

void
Ranker::rank(Xapian::MSet & mset, const string & model_key, const Xapian::FeatureList & flist)
{
    LOGCALL_VOID(API, "Ranker::rank", mset | model_key | flist);
    if (mset.empty()) {
	return;
    }
    std::vector<FeatureVector> fvv = flist.create_feature_vectors(mset, letor_query, Xapian::Database(db_path));
    load_model_from_metadata(model_key);
    std::vector<FeatureVector> rankedfvv = rank_fvv(fvv);
    mset.replace_weights(ScoreIterator(rankedfvv.begin()), ScoreIterator(rankedfvv.end()));
    mset.sort_by_relevance();
}

void
Ranker::train_model(const std::string & input_filename, const std::string & model_key)
{
    LOGCALL_VOID(API, "Ranker::train_model", input_filename | model_key);
    vector<FeatureVector> list_fvecs = load_list_fvecs(input_filename);
    train(list_fvecs);
    save_model_to_metadata(model_key);
}

void
Ranker::score(const string & query_file, const string & qrel_file,
      const string & model_key, const string & output_file,
      Xapian::doccount msetsize, const string & scorer_type, const Xapian::FeatureList & flist)
{
    // Set db
    Xapian::Database letor_db(db_path);
    // Load ranker model
    load_model_from_metadata(model_key);
    // Set scorer
    Xapian::Internal::intrusive_ptr<Scorer> scorer;
    if (scorer_type == "NDCGScore") {
	scorer = new NDCGScore();
    } else if (scorer_type == "ERRScore") {
	scorer = new ERRScore();
    } else {
	throw LetorInternalError("Invalid Scorer type.");
    }

    Xapian::QueryParser parser = initialise_queryparser(letor_db);

    qrel = load_relevance(qrel_file);

    string str1;
    ifstream queryfile;
    queryfile.open(query_file.c_str(), ios::in);
    if (!queryfile.good()) {
	throw Xapian::FileNotFoundError("No Query file found. Check path.");
    }

    ofstream out_file;
    out_file.open(output_file);

    double total_score = 0;
    int num_queries = 0;
    while (!queryfile.eof()) {
	getline(queryfile, str1);
	if (str1.empty()) {
	    break;
	}
	++num_queries;

	std::pair<string, string> parsed_query = parse_query_string(str1, num_queries);
	string querystr = parsed_query.first;
	string qid = parsed_query.second;

	Xapian::Query query_no_prefix = parser.parse_query(querystr,
					parser.FLAG_DEFAULT|
					parser.FLAG_SPELLING_CORRECTION);
	// query with 'title' field as default prefix "S"
	Xapian::Query query_default_prefix = parser.parse_query(querystr,
					     parser.FLAG_DEFAULT|
					     parser.FLAG_SPELLING_CORRECTION,
					     "S");
	// Combine queries
	Xapian::Query query = Xapian::Query(Xapian::Query::OP_OR, query_no_prefix, query_default_prefix);

	Xapian::Enquire enquire(letor_db);
	enquire.set_query(query);
	Xapian::MSet mset = enquire.get_mset(0, msetsize);

	rank(mset, model_key, flist);
	std::vector<FeatureVector> fvv_mset = flist.create_feature_vectors(mset, query, letor_db);
	std::vector<FeatureVector> rankedfvv_qrel;

	int k = 0;
	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	    Xapian::Document doc = i.get_document();
	    int label = getlabel(doc, qid);
	    if (label != -1) { // only add FeatureVector which is found in the qrel file
		fvv_mset[k].set_label(label);
		rankedfvv_qrel.push_back(fvv_mset[k]);
	    }
	    ++k;
	}
	double iter_score = scorer->score(rankedfvv_qrel);
	out_file << "Ranking score for qid:" << qid << " = " << iter_score << endl;
	total_score += iter_score;
    }
    queryfile.close();
    total_score = total_score / num_queries;
    out_file << "Average ranking score = " << total_score << endl;
    out_file.close();
}
