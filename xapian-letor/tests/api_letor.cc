/** @file
 * @brief test common features of API classes
 */
/* Copyright (C) 2007,2009,2012,2014,2015,2016 Olly Betts
 * Copyright (C) 2019 Vaibhav Kansagara
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_letor.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include <xapian.h>
#include <xapian-letor.h>

#include "apitest.h"
#include "filetests.h"
#include "safeunistd.h"
#include "testutils.h"

using namespace std;

// To check for one document edge
static void
db_index_one_document(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("Tigers are solitary animals", 1, "S");
    termgenerator.index_text("Might be that only one Tiger is good enough to "
			     "Take out a ranker, a Tiger is a good way to "
			     "check if a test is working or Tiger not. Tiger."
			     "What if the next line contains no Tigers? Would "
			     "it make a difference to your ranker ?  Tigers  "
			     "for the win.", 1, "XD");
    termgenerator.index_text("The will.");
    termgenerator.increase_termpos();
    termgenerator.index_text("Tigers would not be caught if one calls out the "
			     "Tiger from the den. This document is to check if "
			     "in the massive dataset, you forget the sense of "
			     "something you would not like to stop.");
    db.add_document(doc);
}

static void
db_index_two_documents(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("Lions, Tigers, Bears and Giraffes", 1, "S");
    termgenerator.index_text("This paragraph talks about lions and tigers and "
			     "bears (oh, my!). It mentions giraffes, "
			     "but that's not really very important. Lions "
			     "and tigers are big cats, so they must be really "
			     "cuddly. Bears are famous for being cuddly, at "
			     "least when they're teddy bears.", 1, "XD");
    termgenerator.index_text("Lions, Tigers, Bears and Giraffes");
    termgenerator.increase_termpos();
    termgenerator.index_text("This paragraph talks about lions and tigers and "
			     "bears (oh, my!). It mentions giraffes, "
			     "but that's not really very important. Lions "
			     "and tigers are big cats, so they must be really "
			     "cuddly. Bears are famous for being cuddly, at "
			     "least when they're teddy bears.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Lions, Tigers and Bears", 1, "S");
    termgenerator.index_text("This is the paragraph of interest. Tigers are "
			     "massive beasts - I wouldn't want to meet a "
			     "hungry one anywhere. Lions are scary even when "
			     "lyin' down. Bears are scary even when bare. "
			     "Together I suspect they'd be less scary, as the "
			     "tigers, lions, and bears would all keep each "
			     "other busy. On the other hand, bears don't live "
			     "in the same continent as far as I know.", 1,
			     "XD");
    termgenerator.index_text("Lions, Tigers and Bears");
    termgenerator.increase_termpos();
    termgenerator.index_text("This is the paragraph of interest. Tigers are "
			     "massive beasts - I wouldn't want to meet a "
			     "hungry one anywhere. Lions are scary even when "
			     "lyin' down. Bears are scary even when bare. "
			     "Together I suspect they'd be less scary, as the "
			     "tigers, lions, and bears would all keep each "
			     "other busy. On the other hand, bears don't live "
			     "in the same continent as far as I know.");
    db.add_document(doc);
}

// To check for three documents. out of which one is irrelevant
static void
db_index_three_documents(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("The will", 1, "S");
    termgenerator.index_text("The will are considered stop words in xapian and "
			     "would be thrown off, so the query I want to say "
			     "is score, yes, score. The Score of a game is "
			     "the determining factor of a game, the score is "
			     "what matters at the end of the day. so my advise "
			     "to everyone is to Score it!.", 1, "XD");
    termgenerator.index_text("Score might be something else too, but this para "
			     "refers to score only at an abstract. Scores are "
			     "in general scoring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Score score score score score score", 1, "S");
    termgenerator.index_text("it might have an absurdly high rank in the qrel "
			     "file or might have no rank at all in another. "
			     "Look out for this as a testcase, might be edgy "
			     "good luck and may this be with you.", 1, "XD");
    termgenerator.index_text("Another irrelevant paragraph to make sure the tf "
			     "values are down, but this increases idf values "
			     "but let's see how this works out.");
    termgenerator.increase_termpos();
    termgenerator.index_text("Nothing to do with the query.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Document has nothing to do with score", 1, "S");
    termgenerator.index_text("This is just to check if score is given a higher "
			     "score if it is in the subject or not. Nothing "
			     "special, just judging scores by the look of it. "
			     "Some more scores but a bad qrel should be enough "
			     "to make sure it is ranked down.", 1, "XD");
    termgenerator.index_text("Score might be something else too, but this para "
			     "refers to score only at an abstract. Scores are "
			     "in general scoring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important.");
    db.add_document(doc);
}

// To check for three documents in which one has no common terms with other two.
static void
db_index_three_documents_no_common(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    Xapian::TermGenerator termgenerator;
    termgenerator.set_document(doc);
    termgenerator.set_stemmer(Xapian::Stem("en"));
    termgenerator.index_text("The will", 1, "S");
    termgenerator.index_text("The will are considered stop words in xapian and "
			     "would be thrown off, so the query I want to say "
			     "is score, yes, score. The Score of a game is "
			     "the determining factor of a game, the score is "
			     "what matters at the end of the day. so my advise "
			     "to everyone is to Score it!.", 1, "XD");
    termgenerator.index_text("Score might be something else too, but this para "
			     "refers to score only at an abstract. Scores are "
			     "in general scoring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Document has nothing to do with score", 1, "S");
    termgenerator.index_text("This is just to check if score is given a higher "
			     "score if it is in the subject or not. Nothing "
			     "special, just judging scores by the look of it. "
			     "Some more scores but a bad qrel should be enough "
			     "to make sure it is ranked down.", 1, "XD");
    termgenerator.index_text("Score might be something else too, but this para "
			     "refers to score only at an abstract. Scores are "
			     "in general scoring. Score it!");
    termgenerator.increase_termpos();
    termgenerator.index_text("Score score score is important.");
    db.add_document(doc);
    doc.clear_terms();
    termgenerator.index_text("Tigers are solitary animals", 1, "S");
    termgenerator.index_text("Might be that only one Tiger is good enough to "
			     "Take out a ranker, a Tiger is a good way to "
			     "check if a test is working or Tiger not. Tiger."
			     "What if the next line contains no Tigers? Would "
			     "it make a difference to your ranker ?  Tigers  "
			     "for the win.", 1, "XD");
    termgenerator.index_text("The will.");
    termgenerator.increase_termpos();
    termgenerator.index_text("Tigers would not be caught if one calls out the "
			     "Tiger from the den. This document is to check if "
			     "in the massive dataset, you forget the sense of "
			     "something you would not like to stop.");
    db.add_document(doc);
}

DEFINE_TESTCASE(createfeaturevector, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("db_index_two_documents",
				       db_index_two_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    TEST_EQUAL(mset.size(), 2);
    auto fv = fl.create_feature_vectors(mset, Xapian::Query("lions"), db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    TEST_EQUAL(fv[1].get_fcount(), 19);
}

DEFINE_TESTCASE(createfeaturevectoronevector, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker2",
				       db_index_one_document);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    auto fv = fl.create_feature_vectors(mset, Xapian::Query("tigers"), db);
    TEST_EQUAL(fv.size(), 1);
    TEST_EQUAL(fv[0].get_fcount(), 19);
}

DEFINE_TESTCASE(createfeaturevectoronevector_wrongquery, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker3",
				       db_index_one_document);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("llamas"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);
    TEST(mset.empty());
    auto fv = fl.create_feature_vectors(mset, Xapian::Query("llamas"), db);
    TEST_EQUAL(fv.size(), 0);
}

DEFINE_TESTCASE(createfeaturevectorthree, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    auto fv = fl.create_feature_vectors(mset, Xapian::Query("score"), db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    TEST_EQUAL(fv[1].get_fcount(), 19);
}

DEFINE_TESTCASE(emptyfeaturelist, !backend)
{
    vector<Xapian::Feature*> f;
    TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::FeatureList fl(f));
}

DEFINE_TESTCASE(bigfeaturelist, generated)
{
    vector<Xapian::Feature*> f;
    f.push_back(new Xapian::TfFeature());
    f.push_back(new Xapian::TfDoclenFeature());
    f.push_back(new Xapian::IdfFeature());
    f.push_back(new Xapian::CollTfCollLenFeature());
    f.push_back(new Xapian::TfIdfDoclenFeature());
    f.push_back(new Xapian::TfDoclenCollTfCollLenFeature());
    f.push_back(new Xapian::TfFeature());
    f.push_back(new Xapian::TfDoclenFeature());

    // pass big feature list.
    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_two_documents",
				       db_index_two_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    TEST(!mset.empty());

    auto fv = fl.create_feature_vectors(mset, Xapian::Query("tigers"), db);
    TEST_EQUAL(fv.size(), 2);
    // Each feature contributes three values and weight as the default one
    // making total as 25.
    TEST_EQUAL(fv[0].get_fcount(), 25);
    TEST_EQUAL(fv[1].get_fcount(), 25);
}

DEFINE_TESTCASE(preparetrainingfileonedb, generated && path && writable)
{
    string db_path = get_database_path("apitest_listnet_ranker1",
				       db_index_one_document);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    unlink("training_output_data_one_doc.txt");
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_data_one_doc.txt");
    TEST(file_exists("training_output_data_one_doc.txt"));
    ifstream if1(training_data);
    ifstream if2("training_output_data_one_doc.txt");
    string line1;
    string line2;
    while (getline(if1, line1)) {
	TEST(getline(if2, line2));
	istringstream iss1(line1);
	istringstream iss2(line2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    // The 0th, 1st and 21st literals taken as input, are strings,
	    // and can be compared directly, They are: For example(test):
	    // ("1", "qid:20001" and "#docid=1") at 0th, 1st, and 21st pos
	    // respectively. Whereas the other values are doubles which
	    // would have to tested under TEST_DOUBLE() against precision.
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST_EQUAL_DOUBLE(stod(temp1.substr(t1 + 1)),
				  stod(temp2.substr(t2 + 1)));
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, line2));
    unlink("training_output_data_one_doc.txt");
}

#define TEST_PARSE_EXCEPTION(TESTFILE) TEST_EXCEPTION(Xapian::LetorParseError,\
	Xapian::prepare_training_file(db_path,\
				      data_directory + TESTFILE, qrel, 10,\
				      "training_output.txt"))

// test whether query ids are unique in queryfile.
DEFINE_TESTCASE(unique_queryid, generated && path)
{
    string db_path = get_database_path("db_index_one_document",
				       db_index_one_document);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string qrel = data_directory + "qrelone.txt";
    TEST_PARSE_EXCEPTION("unique_query_id.txt");
}

DEFINE_TESTCASE(parse_querystring, generated && path)
{
    // All those cases which are not valid.
    string db_path = get_database_path("db_index_one_document",
				       db_index_one_document);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string qrel = data_directory + "qrelone.txt";
    TEST_PARSE_EXCEPTION("parse_query_noopenquote.txt");
    TEST_PARSE_EXCEPTION("parse_query_noclosingquote.txt");
    TEST_PARSE_EXCEPTION("parse_query_empty_string.txt");
    TEST_PARSE_EXCEPTION("nospace.txt");
    TEST_PARSE_EXCEPTION("nosinglequotes.txt");
    TEST_PARSE_EXCEPTION("blank_space_before_query.txt");

    // All those cases which are valid.
    Xapian::prepare_training_file(db_path, data_directory +
				  "parse_query_valid.txt", qrel, 10,
				  "training_output.txt");
}

// Check stability for an empty qrel file
DEFINE_TESTCASE(preparetrainingfileonedb_empty_qrel, generated && path)
{
    string db_path = get_database_path("ranker_empty",
				       db_index_one_document);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "empty_file.txt";
    string training_data = data_directory + "empty_file.txt";
    unlink("training_output_empty.txt");
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_empty.txt");
    TEST(file_exists("training_output_empty.txt"));
    ifstream if1(training_data);
    ifstream if2("training_output_empty.txt");
    string line1;
    string line2;
    while (getline(if1, line1)) {
	TEST(getline(if2, line2));
	istringstream iss1(line1);
	istringstream iss2(line2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST_EQUAL_DOUBLE(stod(temp1.substr(t1 + 1)),
				  stod(temp2.substr(t2 + 1)));
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, line2));
    unlink("training_output_empty.txt");
}

DEFINE_TESTCASE(preparetrainingfile_two_docs, generated && path)
{
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    string db_path = get_database_path("db_index_two_documents",
				       db_index_two_documents);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    unlink("training_output1.txt");
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output1.txt");
    TEST(file_exists("training_output1.txt"));
    ifstream if1(training_data);
    ifstream if2("training_output1.txt");
    string line1;
    string line2;
    while (getline(if1, line1)) {
	TEST(getline(if2, line2));
	istringstream iss1(line1);
	istringstream iss2(line2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST_EQUAL_DOUBLE(stod(temp1.substr(t1 + 1)),
				  stod(temp2.substr(t2 + 1)));
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, line2));
    unlink("training_output1.txt");
}

DEFINE_TESTCASE(preparetrainingfilethree, generated && path)
{
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    unlink("training_output_three_correct.txt");
    Xapian::prepare_training_file(db_path, query, qrel, 10,
				  "training_output_three_correct.txt");
    TEST(file_exists("training_output_three_correct.txt"));
    ifstream if1(training_data);
    ifstream if2("training_output_three_correct.txt");
    string line1;
    string line2;
    while (getline(if1, line1)) {
	TEST(getline(if2, line2));
	istringstream iss1(line1);
	istringstream iss2(line2);
	string temp1;
	string temp2;
	int i = 0;
	while ((iss1 >> temp1) && (iss2 >> temp2)) {
	    if (i == 0 || i == 1 || i == 21) {
		TEST_EQUAL(temp1, temp2);
	    } else {
		size_t t1 = temp1.find_first_of(':');
		size_t t2 = temp2.find_first_of(':');
		TEST_EQUAL_DOUBLE(stod(temp1.substr(t1 + 1)),
				  stod(temp2.substr(t2 + 1)));
	    }
	    i++;
	}
	TEST_REL(i, ==, 22);
	TEST(!(iss2 >> temp2));
    }
    TEST(!getline(if2, line2));
    unlink("training_output_three_correct.txt");
}

// ListNet_Ranker check
DEFINE_TESTCASE(listnet_ranker, generated && path && writable)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("db_index_two_documents",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listnet_2.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output_listnet_2.txt",
		 10);
    TEST(file_exists("ndcg_output_listnet_2.txt"));
    unlink("ndcg_output_listnet_2.txt");
    unlink("err_output_listnet_2.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "err_output_listnet_2.txt",
		 10, "ERRScore");
    TEST(file_exists("err_output_listnet_2.txt"));
    unlink("err_output_listnet_2.txt");
}

DEFINE_TESTCASE(listnet_ranker_one_file, generated && path && writable)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listnet_ranker5",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listnet_1.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output_listnet_1.txt",
		 10);
    TEST(file_exists("ndcg_output_listnet_1.txt"));
    unlink("ndcg_output_listnet_1.txt");
    unlink("err_output_listnet_1.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "err_output_listnet_1.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_listnet_1.txt"));
    unlink("err_output_listnet_1.txt");
}

DEFINE_TESTCASE(listnet_ranker_three_correct, generated && path && writable)
{
    Xapian::ListNETRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListNet_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListNet_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListNet_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listnet_3.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_output_listnet_3.txt",
		 10);
    TEST(file_exists("ndcg_output_listnet_3.txt"));
    unlink("ndcg_output_listnet_3.txt");
    unlink("err_output_listnet_3.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "err_output_listnet_3.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_listnet_3.txt"));
    unlink("err_output_listnet_3.txt");
}

DEFINE_TESTCASE(scorer, generated && path && writable)
{
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    Xapian::ListNETRanker ranker;
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    unlink("ndcg_score_output.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_score_output.txt",
		 10);
    TEST(file_exists("ndcg_score_output.txt"));
    ifstream ndcg_score_file;
    ndcg_score_file.open("ndcg_score_output.txt", ios::in);
    string line;
    getline(ndcg_score_file, line);
    size_t pos = 1 + line.find_first_of("=");
    double ndcg_score = stod(line.substr(pos));
    // It should have the perfect ndcg score(1.0)
    TEST_EQUAL(ndcg_score, 1.0);

    unlink("ndcg_score_output.txt");
}

/// SVM_ranker check
DEFINE_TESTCASE(svm_ranker, generated && path && writable)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("db_index_two_documents",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_svm_2.txt");
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output_svm_2.txt", 10);
    TEST(file_exists("ndcg_output_svm_2.txt"));
    unlink("ndcg_output_svm_2.txt");
    unlink("err_output_svm_2.txt");
    ranker.score(query, qrel, "SVM_Ranker", "err_output_svm_2.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_svm_2.txt"));
    unlink("err_output_svm_2.txt");
}

DEFINE_TESTCASE(svm_ranker_one_file, generated && path && writable)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_svm_ranker1",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_svm_1.txt");
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output_svm_1.txt", 10);
    TEST(file_exists("ndcg_output_svm_1.txt"));
    unlink("ndcg_output_svm_1.txt");
    unlink("err_output_svm_1.txt");
    ranker.score(query, qrel, "SVM_Ranker", "err_output_svm_1.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_svm_1.txt"));
    unlink("err_output_svm_1.txt");
}

DEFINE_TESTCASE(svm_ranker_three_correct, generated && path && writable)
{
    Xapian::SVMRanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "SVM_Ranker");
    ranker.rank(mymset, "SVM_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "SVM_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "SVM_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "SVM_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_svm_3.txt");
    ranker.score(query, qrel, "SVM_Ranker", "ndcg_output_svm_3.txt", 10);
    TEST(file_exists("ndcg_output_svm_3.txt"));
    unlink("ndcg_output_svm_3.txt");
    unlink("err_output_svm_3.txt");
    ranker.score(query, qrel, "SVM_Ranker", "err_output_svm_3.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_svm_3.txt"));
    unlink("err_output_svm_3.txt");
}

// ListMLE_Ranker check
DEFINE_TESTCASE(listmle_ranker, generated && path && writable)
{
    Xapian::ListMLERanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("db_index_two_documents",
				       db_index_two_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "query.txt";
    string qrel = data_directory + "qrel.txt";
    string training_data = data_directory + "training_data.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listmle_2.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output_listmle_2.txt",
		 10);
    TEST(file_exists("ndcg_output_listmle_2.txt"));
    unlink("ndcg_output_listmle_2.txt");
    unlink("err_output_listmle_2.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output_listmle_2.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_listmle_2.txt"));
    unlink("err_output_listmle_2.txt");
}

DEFINE_TESTCASE(listmle_ranker_one_file, generated && path && writable)
{
    Xapian::ListMLERanker ranker;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker.train_model(""));
    string db_path = get_database_path("apitest_listmle_ranker1",
				       db_index_one_document);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "queryone.txt";
    string qrel = data_directory + "qrelone.txt";
    string training_data = data_directory + "training_data_one_document.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("tigers"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    ranker.rank(mymset);
    TEST_EQUAL(doc1, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc1, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listmle_1.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output_listmle_1.txt",
		 10);
    TEST(file_exists("ndcg_output_listmle_1.txt"));
    unlink("ndcg_output_listmle_1.txt");
    unlink("err_output_listmle_1.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output_listmle_1.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_listmle_1.txt"));
    unlink("err_output_listmle_1.txt");
}

DEFINE_TESTCASE(listmle_ranker_three_correct, generated && path && writable)
{
    Xapian::ListMLERanker ranker;
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "qrelthree_correct.txt";
    string training_data = data_directory + "training_data_three_correct.txt";
    ranker.set_database_path(db_path);
    TEST_EQUAL(ranker.get_database_path(), db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data);
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset);
    TEST_EQUAL(mymset.size(), 2);
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    mymset = enquire.get_mset(0, 10);
    ranker.train_model(training_data, "ListMLE_Ranker");
    ranker.rank(mymset, "ListMLE_Ranker");
    TEST_EQUAL(doc1, *mymset[1]);
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EXCEPTION(Xapian::LetorInternalError,
		   ranker.score(query, qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10, ""));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score("", qrel, "ListMLE_Ranker",
				"scorer_output.txt", 10));
    TEST_EXCEPTION(Xapian::FileNotFoundError,
		   ranker.score(qrel, "", "ListMLE_Ranker",
				"scorer_output.txt", 10));
    unlink("ndcg_output_listmle_3.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "ndcg_output_listmle_3.txt",
		 10);
    TEST(file_exists("ndcg_output_listmle_3.txt"));
    unlink("err_output_listmle_3.txt");
    unlink("ndcg_output_listmle_3.txt");
    ranker.score(query, qrel, "ListMLE_Ranker", "err_output_listmle_3.txt", 10,
		 "ERRScore");
    TEST(file_exists("err_output_listmle_3.txt"));
    unlink("err_output_listmle_3.txt");
}

// Featurename check
DEFINE_TESTCASE(featurename, !backend)
{
    Xapian::TfDoclenCollTfCollLenFeature feature1;
    Xapian::TfDoclenFeature feature2;
    Xapian::IdfFeature feature3;
    Xapian::TfFeature feature4;
    Xapian::TfIdfDoclenFeature feature5;
    Xapian::CollTfCollLenFeature feature6;
    TEST_EQUAL(feature1.name(), "TfDoclenCollTfCollLenFeature");
    TEST_EQUAL(feature2.name(), "TfDoclenFeature");
    TEST_EQUAL(feature3.name(), "IdfFeature");
    TEST_EQUAL(feature4.name(), "TfFeature");
    TEST_EQUAL(feature5.name(), "TfIdfDoclenFeature");
    TEST_EQUAL(feature6.name(), "CollTfCollLenFeature");
}

DEFINE_TESTCASE(err_scorer, !backend)
{
    /* Derived from the example mentioned in the blogpost
     * https://lingpipe-blog.com/2010/03/09/chapelle-metzler-zhang-grinspan-2009-expected-reciprocal-rank-for-graded-relevance/
     */
    vector<Xapian::FeatureVector> fvv;
    Xapian::FeatureVector temp1;
    Xapian::FeatureVector temp2;
    Xapian::FeatureVector temp3;
    temp1.set_label(3);
    fvv.push_back(temp1);
    temp2.set_label(2);
    fvv.push_back(temp2);
    temp3.set_label(4);
    fvv.push_back(temp3);
    Xapian::ERRScore err;
    double err_score = err.score(fvv);

    TEST(abs(err_score - 0.63) < 0.01);
}

DEFINE_TESTCASE(ndcg_score_test, generated && path && writable)
{
    Xapian::ListNETRanker ranker;
    string db_path = get_database_path("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire((Xapian::Database(db_path)));
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    string data_directory = test_driver::get_srcdir() + "/testdata/";
    string query = data_directory + "querythree.txt";
    string qrel = data_directory + "score_qrel.txt";
    string training_data = data_directory + "training_data_ndcg.txt";
    ranker.set_database_path(db_path);
    ranker.set_query(Xapian::Query("score"));
    ranker.train_model(training_data, "ListNet_Ranker");
    ranker.rank(mymset, "ListNet_Ranker");
    unlink("ndcg_score_test.txt");
    ranker.score(query, qrel, "ListNet_Ranker", "ndcg_score_test.txt", 10);
    TEST(file_exists("ndcg_score_test.txt"));
    unlink("ndcg_score_test.txt");
}

// Test createfeaturevector method for TfFeature
DEFINE_TESTCASE(createfeaturevector_tffeature, generated)
{
    vector<Xapian::Feature*> f;
    Xapian::TfFeature* f1 = new Xapian::TfFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate TfFeature values for the first document.
    test_vals_doc1[0] = 0.301029995663981;
    test_vals_doc1[1] = 1.653212513775344;
    test_vals_doc1[2] = 1.954242509439325;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // These are the appropriate TfFeature values for the second document.
    test_vals_doc2[0] = 0;
    test_vals_doc2[1] = 1.732393759822969;
    test_vals_doc2[2] = 1.732393759822969;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_val[4];
    for (int i = 0; i < 4; ++i) {
	max_val[i] = max(test_vals_doc1[i], test_vals_doc2[i]);
    }

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], test_vals_doc1[0] / max_val[0]);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], test_vals_doc2[0] / max_val[0]);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], test_vals_doc1[1] / max_val[1]);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], test_vals_doc2[1] / max_val[1]);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], test_vals_doc1[2] / max_val[2]);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], test_vals_doc2[2] / max_val[2]);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_val[3]);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_val[3]);
}

// Test createfeaturevector method for IdfFeature
DEFINE_TESTCASE(createfeaturevector_idffeature, generated)
{
    vector<Xapian::Feature*> f;
    Xapian::IdfFeature* f1 = new Xapian::IdfFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:tigers description:tigers tigers"
			 " title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate IdfFeature values for the first document.
    test_vals_doc1[0] = 0.0;
    test_vals_doc1[1] = 0.0;
    test_vals_doc1[2] = 0.0;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // These are the appropriate IdfFeature values for the second document.
    test_vals_doc2[0] = 0.0;
    test_vals_doc2[1] = 0.0;
    test_vals_doc2[2] = 0.0;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_weight = max(test_vals_doc1[3], test_vals_doc2[3]);

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], 0.0);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], 0.0);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], 0.0);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_weight);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_weight);
}

// Test createfeaturevector method for TfDoclenFeature
DEFINE_TESTCASE(createfeaturevector_tfdoclenfeature, generated)
{
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    vector<Xapian::Feature*> f;
    Xapian::TfDoclenFeature* f1 = new Xapian::TfDoclenFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents_no_common",
				       db_index_three_documents_no_common);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate TfDoclenFeature values for the first
    // document.
    test_vals_doc1[0] = 0.0511525224473813;
    test_vals_doc1[1] = 0.0323089286738408;
    test_vals_doc1[2] = 0.0335890631408052;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // These are the appropriate TfDoclenFeature values for the second
    // document.
    test_vals_doc2[0] = 0.0;
    test_vals_doc2[1] = 0.03237347800973529;
    test_vals_doc2[2] = 0.03200637092048766;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_val[4];
    for (int i = 0; i < 4; ++i) {
	max_val[i] = max(test_vals_doc1[i], test_vals_doc2[i]);
    }

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], test_vals_doc1[0] / max_val[0]);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], test_vals_doc2[0] / max_val[0]);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], test_vals_doc1[1] / max_val[1]);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], test_vals_doc2[1] / max_val[1]);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], test_vals_doc1[2] / max_val[2]);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], test_vals_doc2[2] / max_val[2]);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_val[3]);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_val[3]);
}

// Test createfeaturevector method for CollTfCollLenFeature
DEFINE_TESTCASE(createfeaturevector_colltfcolllenfeature, generated)
{
    vector<Xapian::Feature*> f;
    Xapian::CollTfCollLenFeature* f1 = new Xapian::CollTfCollLenFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents_no_common",
				       db_index_three_documents_no_common);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate CollTfCollLenFeature values for the first
    // document.
    test_vals_doc1[0] = 0.45863784902564930;
    test_vals_doc1[1] = 3.13291481930625260;
    test_vals_doc1[2] = 4.94672282004904673;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // values will be same as that of the first document
    test_vals_doc2[0] = 0.45863784902564930;
    test_vals_doc2[1] = 3.13291481930625260;
    test_vals_doc2[2] = 4.94672282004904673;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_val[4];
    for (int i = 0; i < 4; ++i) {
	max_val[i] = max(test_vals_doc1[i], test_vals_doc2[i]);
    }

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], test_vals_doc1[0] / max_val[0]);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], test_vals_doc2[0] / max_val[0]);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], test_vals_doc1[1] / max_val[1]);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], test_vals_doc2[1] / max_val[1]);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], test_vals_doc1[2] / max_val[2]);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], test_vals_doc2[2] / max_val[2]);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_val[3]);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_val[3]);
}

// Test createfeaturevector method for TfIdfDoclenFeature
DEFINE_TESTCASE(createfeaturevector_tfidfdoclenfeature, generated)
{
    vector<Xapian::Feature*> f;
    Xapian::TfIdfDoclenFeature* f1 = new Xapian::TfIdfDoclenFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate TfIdfDoclenFeature values for the first
    // document.
    test_vals_doc1[0] = 0.0;
    test_vals_doc1[1] = 0.0;
    test_vals_doc1[2] = 0.0;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // These are the appropriate TfIdfDoclenFeature values for the second
    // document.
    test_vals_doc2[0] = 0.0;
    test_vals_doc2[1] = 0.0;
    test_vals_doc2[2] = 0.0;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_weight = max(test_vals_doc1[3], test_vals_doc2[3]);

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], 0.0);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], 0.0);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], 0.0);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], 0.0);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_weight);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_weight);
}

// Test createfeaturevector method for TfDoclenCollTfCollLenFeature
DEFINE_TESTCASE(createfeaturevector_tfdoclencolllfcolllen, generated)
{
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    vector<Xapian::Feature*> f;
    Xapian::TfDoclenCollTfCollLenFeature* f1 =
				new Xapian::TfDoclenCollTfCollLenFeature();
    f.push_back(f1);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents",
				       db_index_three_documents);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("score"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    // As the feature values depend on different prefixed terms in the query
    // like "title","body" and "whole", so we need to separate it out instead
    // of just writing Query("score").
    string querystring = "title:score description:score score";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 4);
    TEST_EQUAL(fv[1].get_fcount(), 4);

    vector<double> fvals_doc1 = fv[0].get_fvals();
    vector<double> fvals_doc2 = fv[1].get_fvals();
    TEST_EQUAL(fvals_doc1.size(), 4);
    TEST_EQUAL(fvals_doc2.size(), 4);

    vector<double> test_vals_doc1(4);
    // These are the appropriate TfDoclenCollTfCollLenFeature values for
    // the first document.
    test_vals_doc1[0] = 0.11394335230683678;
    test_vals_doc1[1] = 0.76130720333102619;
    test_vals_doc1[2] = 0.90738326700002048;

    Xapian::MSetIterator it = mset.begin();
    test_vals_doc1[3] = it.get_weight();

    vector<double> test_vals_doc2(4);
    // These are the appropriate TfDoclenCollTfCollLenFeature values for
    // the second document.
    test_vals_doc2[0] = 0.0;
    test_vals_doc2[1] = 0.77758890362035493;
    test_vals_doc2[2] = 0.78786362447009839;

    ++it;
    test_vals_doc2[3] = it.get_weight();

    double max_val[4];
    for (int i = 0; i < 4; ++i) {
	max_val[i] = max(test_vals_doc1[i], test_vals_doc2[i]);
    }

    // test for title in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[0], test_vals_doc1[0] / max_val[0]);
    TEST_EQUAL_DOUBLE(fvals_doc2[0], test_vals_doc2[0] / max_val[0]);

    // test for body in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[1], test_vals_doc1[1] / max_val[1]);
    TEST_EQUAL_DOUBLE(fvals_doc2[1], test_vals_doc2[1] / max_val[1]);

    // test for whole in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[2], test_vals_doc1[2] / max_val[2]);
    TEST_EQUAL_DOUBLE(fvals_doc2[2], test_vals_doc2[2] / max_val[2]);

    // test for weight in normalized form
    TEST_EQUAL_DOUBLE(fvals_doc1[3], test_vals_doc1[3] / max_val[3]);
    TEST_EQUAL_DOUBLE(fvals_doc2[3], test_vals_doc2[3] / max_val[3]);
}

class CustomFeature : public Xapian::Feature {
  public:
    CustomFeature() {
	need_stat(Xapian::Feature::TERM_FREQUENCY);
	need_stat(Xapian::Feature::DOCUMENT_LENGTH);
	need_stat(Xapian::Feature::COLLECTION_TERM_FREQ);
	need_stat(Xapian::Feature::COLLECTION_LENGTH);
	need_stat(Xapian::Feature::INVERSE_DOCUMENT_FREQUENCY);
    }
    std::vector<double> get_values() const {
	return vector<double>();
    }
    std::string name() const {
	return "CustomFeature";
    }
    void test_stats() {
	// test for term frequency
	TEST_EQUAL(get_termfreq("ZStiger"), 1);
	TEST_EQUAL(get_termfreq("ZXDtiger"), 6);
	TEST_EQUAL(get_termfreq("Ztiger"), 2);

	// test for inverse document frequency
	TEST_EQUAL_DOUBLE(get_inverse_doc_freq("ZStiger"), 0.176091259055681);
	TEST_EQUAL_DOUBLE(get_inverse_doc_freq("ZXDtiger"), 0.176091259055681);
	TEST_EQUAL_DOUBLE(get_inverse_doc_freq("Ztiger"), 0.176091259055681);

	// test for document length
	TEST_EQUAL(get_doc_length("title"), 4);
	TEST_EQUAL(get_doc_length("body"), 182);
	TEST_EQUAL(get_doc_length("whole"), 186);

	// test for collection length
	TEST_EQUAL(get_collection_length("title"), 13);
	TEST_EQUAL(get_collection_length("body"), 509);
	TEST_EQUAL(get_collection_length("whole"), 522);

	// test for collection term frequency
	TEST_EQUAL(get_collection_termfreq("ZStiger"), 1);
	TEST_EQUAL(get_collection_termfreq("ZXDtiger"), 6);
	TEST_EQUAL(get_collection_termfreq("Ztiger"), 2);
    }
};

DEFINE_TESTCASE(populatefeature, generated) {
    XFAIL_FOR_BACKEND("multi", "Testcase fails with multidatabase");
    vector<Xapian::Feature*> f;
    CustomFeature* custom_feature = new CustomFeature();
    f.push_back(custom_feature);

    Xapian::FeatureList fl(f);
    Xapian::Database db = get_database("db_index_three_documents_no_common",
				       db_index_three_documents_no_common);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("tigers"));
    Xapian::MSet mset;
    mset = enquire.get_mset(0, 10);

    TEST(!mset.empty());

    Xapian::QueryParser queryparser;
    queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_ALL_Z);
    queryparser.add_prefix("title", "S");
    queryparser.add_prefix("description", "XD");

    string querystring = "title:tigers description:tigers tigers";
    Xapian::Query query = queryparser.parse_query(querystring);

    auto fv = fl.create_feature_vectors(mset, query, db);
    TEST_EQUAL(fv.size(), 1);

    custom_feature->test_stats();
}
