/** @file api_letor.cc
 * @brief test common features of API classes
 */
/* Copyright (C) 2007,2009,2012,2014,2015,2016 Olly Betts
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

#include <xapian.h>
#include <xapian-letor.h>
#include <fstream>

#include "apitest.h"
#include "testutils.h"

using namespace std;

DEFINE_TESTCASE(check_db_path, !backend) {
   Xapian::ListNETRanker ranker1;
   ranker1.set_database_path("db_path");
   TEST_EQUAL("db_path", ranker1.get_database_path());
   Xapian::SVMRanker ranker2;
   ranker2.set_database_path("db_path");
   TEST_EQUAL("db_path", ranker2.get_database_path());

   return true;
}

DEFINE_TESTCASE(check_train_model, backend) {
    Xapian::ListNETRanker ranker1;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker1.train_model("", ""));
    Xapian::SVMRanker ranker2;
    TEST_EXCEPTION(Xapian::FileNotFoundError, ranker2.train_model("", ""));

    return true;
}

static void
populate_db(Xapian::WritableDatabase& db, const string&)
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

DEFINE_TESTCASE(listnet_ranker, generated) {
    Xapian::ListNETRanker ranker;
    string a = get_database_path("apitest_ranker1",
				       populate_db);
    Xapian::Enquire enquire((Xapian::Database(a)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    Xapian::prepare_training_file(a, "testdata/query.txt",
				  "testdata/qrel.txt", 10,
				  "training-data.txt");
    ranker.set_database_path(a);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model("training-data.txt", "");
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset, "");
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);

    return true;
}

DEFINE_TESTCASE(svm_ranker, generated) {
    Xapian::SVMRanker ranker;
    string a = get_database_path("apitest_ranker1",
				       populate_db);
    Xapian::Enquire enquire((Xapian::Database(a)));
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    Xapian::prepare_training_file(a, "testdata/query.txt",
				  "testdata/qrel.txt", 10,
				  "training-data.txt");
    ranker.set_database_path(a);
    ranker.set_query(Xapian::Query("lions"));
    ranker.train_model("training-data.txt", "");
    Xapian::docid doc1 = *mymset[0];
    Xapian::docid doc2 = *mymset[1];
    ranker.rank(mymset, "");
    TEST_EQUAL(doc2, *mymset[0]);
    TEST_EQUAL(doc1, *mymset[1]);

    return true;
}

DEFINE_TESTCASE(createfeaturevector, generated)
{
    Xapian::FeatureList fl;
    Xapian::Database db = get_database("apitest_ranker1",
				       populate_db);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("lions"));
    Xapian::MSet mset;
    TEST(mset.empty());
    vector<Xapian::FeatureVector> fv =
	    fl.create_feature_vectors(mset, Xapian::Query("lions"), db);
    TEST(fv.empty());
    mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    fv = fl.create_feature_vectors(mset, Xapian::Query("lions"), db);
    TEST_EQUAL(fv.size(), 2);
    TEST_EQUAL(fv[0].get_fcount(), 19);
    TEST_EQUAL(fv[1].get_fcount(), 19);

    return true;
}
