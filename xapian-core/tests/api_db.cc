/* api_db.cc: tests which need a backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include "utils.h"

#include "apitest.h"
#include "api_db.h"

#include <list>

using namespace std;

typedef list<string> om_termname_list;
extern BackendManager backendmanager;

// #######################################################################
// # Tests start here

// tests that the backend doesn't return zero docids
static bool test_zerodocid1()
{
    // open the database (in this case a simple text file
    // we prepared earlier)

    Xapian::Database mydb(get_database("apitest_onedoc"));

    Xapian::Enquire enquire(mydb);

    // make a simple query, with one word in it - "word".
    Xapian::Query myquery("word");
    enquire.set_query(myquery);

    // retrieve the top ten results (we only expect one)
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // We've done the query, now check that the result is what
    // we expect (1 document, with non-zero docid)
    TEST_MSET_SIZE(mymset, 1);

    TEST_AND_EXPLAIN(*(mymset.begin()) != 0,
		     "A query on a database returned a zero docid");

    return true;
}

Xapian::Database get_simple_database()
{
    return Xapian::Database(get_database("apitest_simpledata"));
}

// tests Xapian::Database::get_termfreq() and Xapian::Database::term_exists()
static bool test_termstats()
{
    // open the database (in this case a simple text file
    // we prepared earlier)

    Xapian::Database db(get_simple_database());

    TEST(!db.term_exists("corn"));
    TEST(db.term_exists("paragraph"));
    TEST_EQUAL(db.get_termfreq("banana"), 1);
    TEST_EQUAL(db.get_termfreq("paragraph"), 5);

    return true;
}

void init_simple_enquire(Xapian::Enquire &enq, const Xapian::Query &query = Xapian::Query("this"))
{
    enq.set_query(query);
}

Xapian::Query
query(Xapian::Query::op op, string t1 = "", string t2 = "",
      string t3 = "", string t4 = "", string t5 = "",
      string t6 = "", string t7 = "", string t8 = "",
      string t9 = "", string t10 = "")
{
    vector<string> v;
    Xapian::Stem stemmer("english");    
    if (!t1.empty()) v.push_back(stemmer.stem_word(t1));
    if (!t2.empty()) v.push_back(stemmer.stem_word(t2));
    if (!t3.empty()) v.push_back(stemmer.stem_word(t3));
    if (!t4.empty()) v.push_back(stemmer.stem_word(t4));
    if (!t5.empty()) v.push_back(stemmer.stem_word(t5));
    if (!t6.empty()) v.push_back(stemmer.stem_word(t6));
    if (!t7.empty()) v.push_back(stemmer.stem_word(t7));
    if (!t8.empty()) v.push_back(stemmer.stem_word(t8));
    if (!t9.empty()) v.push_back(stemmer.stem_word(t9));
    if (!t10.empty()) v.push_back(stemmer.stem_word(t10));
    return Xapian::Query(op, v.begin(), v.end());
}

Xapian::Query
query(string t)
{
    return Xapian::Query(Xapian::Stem("english").stem_word(t));
}

Xapian::MSet do_get_simple_query_mset(Xapian::Query query, int maxitems = 10, int first = 0)
{
    // open the database (in this case a simple text file
    // we prepared earlier)
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire, query);

    // retrieve the top results
    return enquire.get_mset(first, maxitems);
}

// tests the document count for a simple query
static bool test_simplequery1()
{
    Xapian::MSet mymset = do_get_simple_query_mset(Xapian::Query("word"));
    TEST_MSET_SIZE(mymset, 2);
    return true;
}

// tests for the right documents and weights returned with simple query
static bool test_simplequery2()
{
    Xapian::MSet mymset = do_get_simple_query_mset(Xapian::Query("word"));

    // We've done the query, now check that the result is what
    // we expect (documents 2 and 4)
    mset_expect_order(mymset, 2, 4);

    // Check the weights
    //these weights are for C=.5 in bm25weight
    Xapian::MSetIterator i = mymset.begin();
    //weights_are_equal_enough(i.get_weight(), 0.661095);
    weights_are_equal_enough(i.get_weight(), 1.046482);
    i++;
    //weights_are_equal_enough(i.get_weight(), 0.56982);
    weights_are_equal_enough(i.get_weight(), 0.640988);

    return true;
}

// tests for the right document count for another simple query
static bool test_simplequery3()
{
    Xapian::MSet mymset = do_get_simple_query_mset(query("this"));

    // Check that 6 documents were returned.
    TEST_MSET_SIZE(mymset, 6);

    return true;
}

// tests punctuation is OK in terms in remote queries
static bool test_puncterms1()
{
    Xapian::Database db(get_database("apitest_punc"));
    Xapian::Enquire enquire(db);

    Xapian::Query q1("semi;colon");
    enquire.set_query(q1);
    Xapian::MSet m1 = enquire.get_mset(0, 10);

    Xapian::Query q2("col:on");
    enquire.set_query(q2);
    Xapian::MSet m2 = enquire.get_mset(0, 10);

    Xapian::Query q3("com,ma");
    enquire.set_query(q3);
    Xapian::MSet m3 = enquire.get_mset(0, 10);

    return true;
}


// tests a query across multiple databases
static bool test_multidb1()
{
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make a simple query, with one word in it - "word".
    Xapian::Query myquery("word");
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1.size(), mymset2.size());
    TEST(mset_range_is_same_weights(mymset1, 0, mymset2, 0, mymset1.size()));
    return true;
}

// tests a query across multiple databases with terms only
// in one of the two databases
static bool test_multidb2()
{
    Xapian::Database mydb1(get_database("apitest_simpledata",
				  "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make a simple query
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1.size(), mymset2.size());
    TEST(mset_range_is_same_weights(mymset1, 0, mymset2, 0, mymset1.size()));
    return true;
}

// test that a multidb with 2 dbs query returns correct docids
static bool test_multidb3()
{
    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire(mydb2);

    // make a query
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(myquery);

    // retrieve the top ten results
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2, 3, 7);

    return true;
}

// test that a multidb with 3 dbs query returns correct docids
static bool test_multidb4()
{
    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    mydb2.add_database(get_database("apitest_termorder"));
    Xapian::Enquire enquire(mydb2);

    // make a query
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(myquery);

    // retrieve the top ten results
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2, 3, 4, 10);

    return true;
}

// tests MultiPostList::skip_to().
static bool test_multidb5()
{
    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire(mydb2);

    // make a query
    Xapian::Query myquery = query(Xapian::Query::OP_AND, "inmemory", "word");
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(myquery);

    // retrieve the top ten results
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    return true;
}

// check that stubdbs work
static bool test_stubdb1()
{
    ofstream out("stubdb1");
    TEST(out.is_open());
    out << "remote :../bin/omprogsrv " << backendmanager.get_datadir() << " apitest_simpledata\n";
    out.close();

    {
	Xapian::Database db = Xapian::Auto::open_stub("stubdb1");
	Xapian::Enquire enquire(db);
	Xapian::Query myquery("word");
	enquire.set_query(myquery);
	enquire.get_mset(0, 10);
    }
    {
	Xapian::Database db = Xapian::Auto::open("stubdb1");
	Xapian::Enquire enquire(db);
	Xapian::Query myquery("word");
	enquire.set_query(myquery);
	enquire.get_mset(0, 10);
    }

    unlink("stubdb1");

    return true;
}

#if 0 // the "force error" mechanism is no longer in place...
class MyErrorHandler : public Xapian::ErrorHandler {
    public:
	int count;

	bool handle_error(Xapian::Error & error) {
	    ++count;
	    tout << "Error handling caught: " << error.get_type() << ": " <<
		    error.get_msg() << ", with context `" <<
		    error.get_context() << "': count is now " << count << "\n";
	    return true;
	}

	MyErrorHandler() : count (0) {}
};

// tests error handler in multimatch().
static bool test_multierrhandler1()
{
    MyErrorHandler myhandler;

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Database mydb3(get_database("apitest_simpledata2"));
    int errcount = 1;
    for (int testcount = 0; testcount < 14; testcount ++) {
	tout << "testcount=" << testcount << "\n";
	Xapian::Database mydb4(get_database("-e", "apitest_termorder"));
	Xapian::Database mydb5(get_network_database("apitest_termorder", 1));
	Xapian::Database mydb6(get_database("-e2", "apitest_termorder"));
	Xapian::Database mydb7(get_database("-e3", "apitest_simpledata"));

	Xapian::Database dbs;
	switch (testcount) {
	    case 0:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		break;
	    case 1:
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 2:
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		break;
	    case 3:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		sleep(1);
		break;
	    case 4:
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		sleep(1);
		break;
	    case 5:
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		sleep(1);
		break;
	    case 6:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		break;
	    case 7:
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 8:
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		break;
	    case 9:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		break;
	    case 10:
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 11:
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		break;
	    case 12:
		dbs.add_database(mydb2);
		dbs.add_database(mydb6);
		dbs.add_database(mydb7);
		break;
	    case 13:
		dbs.add_database(mydb2);
		dbs.add_database(mydb7);
		dbs.add_database(mydb6);
		break;
	}
	tout << "db=" << dbs << "\n";
	Xapian::Enquire enquire(dbs, &myhandler);

	// make a query
	Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
	enquire.set_weighting_scheme(Xapian::BoolWeight());
	enquire.set_query(myquery);

	tout << "query=" << myquery << "\n";
	// retrieve the top ten results
	Xapian::MSet mymset = enquire.get_mset(0, 10);

	switch (testcount) {
	    case 0: case 3: case 6: case 9:
		mset_expect_order(mymset, 2, 4, 10);
		break;
	    case 1: case 4: case 7: case 10:
		mset_expect_order(mymset, 3, 5, 11);
		break;
	    case 2: case 5: case 8: case 11:
		mset_expect_order(mymset, 1, 6, 12);
		break;
	    case 12:
	    case 13:
		mset_expect_order(mymset, 4, 10);
		errcount += 1;
		break;
	}
	TEST_EQUAL(myhandler.count, errcount);
	errcount += 1;
    }

    return true;
}
#endif

// tests that changing a query object after calling set_query()
// doesn't make any difference to get_mset().
static bool test_changequery1()
{
    // Open the database (in this case a simple text file
    // we prepared earlier)
    Xapian::Enquire enquire(get_simple_database());

    Xapian::Query myquery("this");
    // make a simple query
    enquire.set_query(myquery);

    // retrieve the top ten results
    Xapian::MSet mset1 = enquire.get_mset(0, 10);

    myquery = Xapian::Query("foo");
    Xapian::MSet mset2 = enquire.get_mset(0, 10);

    // verify that both msets are identical
    TEST_EQUAL(mset1, mset2);
    return true;
}

// tests that an empty query returns no matches
static bool test_emptyquery1()
{
    Xapian::MSet mymset = do_get_simple_query_mset(Xapian::Query());
    TEST_MSET_SIZE(mymset, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    vector<Xapian::Query> v;
    mymset = do_get_simple_query_mset(Xapian::Query(Xapian::Query::OP_AND,
					      v.begin(), v.end()));
    TEST_MSET_SIZE(mymset, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    return true;
}

// tests that when specifiying maxitems to get_mset, no more than
// that are returned.
static bool test_msetmaxitems1()
{
    Xapian::MSet mymset = do_get_simple_query_mset(query("this"), 1);
    TEST_MSET_SIZE(mymset, 1);
    return true;
}

// tests that when specifying maxitems to get_eset, no more than
// that are returned.
static bool test_expandmaxitems1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    tout << "mymset.size() = " << mymset.size() << endl;
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(1, myrset);
    TEST_EQUAL(myeset.size(), 1);

    return true;
}

// tests that a pure boolean query has all weights set to 0
static bool test_boolquery1()
{
    Xapian::Query myboolquery(query("this"));

    // open the database (in this case a simple text file
    // we prepared earlier)
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire, myboolquery);
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    // retrieve the top results
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    TEST_NOT_EQUAL(mymset.size(), 0);
    TEST_EQUAL(mymset.get_max_possible(), 0);
    for (Xapian::MSetIterator i = mymset.begin(); i != mymset.end(); ++i) {
	TEST_EQUAL(i.get_weight(), 0);
    }
    return true;
}

// tests that get_mset() specifying "this" works as expected
static bool test_msetfirst1()
{
    Xapian::MSet mymset1 = do_get_simple_query_mset(query("this"), 6, 0);
    Xapian::MSet mymset2 = do_get_simple_query_mset(query("this"), 3, 3);
    TEST(mset_range_is_same(mymset1, 3, mymset2, 0, 3));
    return true;
}

// Regression test - we weren't adjusting the index into items[] by firstitem
// in api/omenquire.cc.
static bool test_msetfirst2()
{
    Xapian::MSet mymset1 = do_get_simple_query_mset(query("this"), 6, 0);
    Xapian::MSet mymset2 = do_get_simple_query_mset(query("this"), 3, 3);
    TEST_EQUAL(mymset1[5].get_document().get_data(),
	       mymset2[2].get_document().get_data());
    return true;
}

// tests the converting-to-percent functions
static bool test_topercent1()
{
    Xapian::MSet mymset = do_get_simple_query_mset(query("this"), 20, 0);

    int last_pct = 100;
    Xapian::MSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
	int pct = mymset.convert_to_percent(i);
	TEST_AND_EXPLAIN(pct == i.get_percent(),
			 "convert_to_%(msetitor) != convert_to_%(wt)");
	TEST_AND_EXPLAIN(pct == mymset.convert_to_percent(i.get_weight()),
			 "convert_to_%(msetitor) != convert_to_%(wt)");
        TEST_AND_EXPLAIN(pct >= 0 && pct <= 100,
			 "percentage out of range: " << pct);
        TEST_AND_EXPLAIN(pct <= last_pct, "percentage increased down mset");
	last_pct = pct;
    }
    return true;
}

class myExpandFunctor : public Xapian::ExpandDecider {
    public:
	int operator()(const string & tname) const {
	    unsigned long sum = 0;
	    for (string::const_iterator i=tname.begin(); i!=tname.end(); ++i) {
		sum += *i;
	    }
//	    if (verbose) {
//		tout << tname << "==> " << sum << "\n";
//	    }
	    return (sum % 2) == 0;
	}
};

// tests the expand decision functor
static bool test_expandfunctor1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    myExpandFunctor myfunctor;

    Xapian::ESet myeset_orig = enquire.get_eset(1000, myrset);
    unsigned int neweset_size = 0;
    Xapian::ESetIterator j = myeset_orig.begin();
    for ( ; j != myeset_orig.end(); ++j) {
        if (myfunctor(*j)) neweset_size++;
    }
    Xapian::ESet myeset = enquire.get_eset(neweset_size, myrset, &myfunctor);

#if 0
    // Compare myeset with the hand-filtered version of myeset_orig.
    if (verbose) {
	tout << "orig_eset: ";
	copy(myeset_orig.begin(), myeset_orig.end(),
	     ostream_iterator<Xapian::ESetItem>(tout, " "));
	tout << "\n";

	tout << "new_eset: ";
	copy(myeset.begin(), myeset.end(),
	     ostream_iterator<Xapian::ESetItem>(tout, " "));
	tout << "\n";
    }
#endif
    Xapian::ESetIterator orig = myeset_orig.begin();
    Xapian::ESetIterator filt = myeset.begin();
    for (; orig != myeset_orig.end() && filt != myeset.end(); ++orig, ++filt) {
	// skip over items that shouldn't be in myeset
	while (orig != myeset_orig.end() && !myfunctor(*orig)) {
	    ++orig;
	}

	TEST_AND_EXPLAIN(*orig == *filt &&
			 orig.get_weight() == filt.get_weight(),
			 "Mismatch in items " << *orig << " vs. " << *filt
			 << " after filtering");
    }

    while (orig != myeset_orig.end() && !myfunctor(*orig)) {
	++orig;
    }

    TEST_EQUAL(orig, myeset_orig.end());
    TEST_AND_EXPLAIN(filt == myeset.end(),
		     "Extra items in the filtered eset.");
    return true;
}

class myMatchDecider : public Xapian::MatchDecider {
    public:
        int operator()(const Xapian::Document &doc) const {
	    // Note that this is not recommended usage of get_data()
	    return doc.get_data().find("This is") != string::npos;
	}
};

// tests the match decision functor
static bool test_matchfunctor1()
{
    // FIXME: check that the functor works both ways.
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    myMatchDecider myfunctor;

    Xapian::MSet mymset = enquire.get_mset(0, 100, 0, &myfunctor);

    Xapian::MSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    TEST_EQUAL(mymset.size(), 3);
    for ( ; i != mymset.end(); ++i) {
	const Xapian::Document doc(i.get_document());
        TEST(myfunctor(doc));
    }

    return true;
}

// tests that mset iterators on msets compare correctly.
static bool test_msetiterator1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    Xapian::MSet mymset = enquire.get_mset(0, 2);

    Xapian::MSetIterator j;
    j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    Xapian::MSetIterator n = mymset.begin();
    Xapian::MSetIterator o = mymset.begin();
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    TEST_EQUAL(j, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    TEST_NOT_EQUAL(k, o);
    o++;
    TEST_EQUAL(k, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, mymset.begin());
    TEST_EQUAL(n, mymset.end());

    return true;
}

// tests that mset iterators on empty msets compare equal.
static bool test_msetiterator2()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    Xapian::MSet mymset = enquire.get_mset(0, 0);

    Xapian::MSetIterator j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests that begin().get_document() works when first != 0
static bool test_msetiterator3()
{
    Xapian::Database mydb(get_database("apitest_simpledata"));

    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery("this");
    enquire.set_query(myquery);

    Xapian::MSet mymset = enquire.get_mset(2, 10);

    TEST(!mymset.empty());
    Xapian::Document doc(mymset.begin().get_document());
    TEST(!doc.get_data().empty());

    return true;
}

// tests that eset iterators on empty esets compare equal.
static bool test_esetiterator1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(2, myrset);
    Xapian::ESetIterator j;
    j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    Xapian::ESetIterator n = myeset.begin();

    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, myeset.begin());
    TEST_EQUAL(n, myeset.end());

    return true;
}

// tests that eset iterators on empty esets compare equal.
static bool test_esetiterator2()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(0, myrset);
    Xapian::ESetIterator j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

static void
print_mset_weights(const Xapian::MSet &mset)
{
    Xapian::MSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
        tout << " " << i.get_weight();
    }
}

// tests the cutoff option
static bool test_cutoff1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire, query(Xapian::Query::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset weights:";
	print_mset_weights(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    Xapian::weight my_wt = -100;
    int changes = 0;
    Xapian::MSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
        Xapian::weight new_wt = i.get_weight();
        if (new_wt != my_wt) {
	    changes++;
	    if (changes > 3) break;
	    num_items = c;
	    my_wt = new_wt;
	}
    }

    TEST_AND_EXPLAIN(changes > 3, "MSet not varied enough to test");
    if (verbose) {
        tout << "Cutoff weight: " << my_wt << "\n";
    }

    enquire.set_cutoff(0, my_wt);
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);

    if (verbose) {
        tout << "Weights after cutoff:";
	print_mset_weights(mymset2);
        tout << "\n";
    }

    TEST_AND_EXPLAIN(mymset2.size() >= num_items,
		     "Match with cutoff lost too many items");

    TEST_AND_EXPLAIN(mymset2.size() == num_items ||
		     (mymset2[num_items].get_weight() == my_wt &&
		      mymset2.back().get_weight() == my_wt),
		     "Match with cutoff returned too many items");

    return true;
}

// tests the cutoff option
static bool test_cutoff2()
{
    Xapian::Enquire enquire(get_simple_database());
    Xapian::Query q = query(Xapian::Query::OP_OR, "this", "line", "paragraph", "rubbish");
    init_simple_enquire(enquire, query(Xapian::Query::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset weights:";
	print_mset_weights(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    Xapian::weight my_wt = -100;
    int changes = 0;
    Xapian::MSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
        Xapian::weight new_wt = i.get_weight();
        if (new_wt != my_wt) {
	    changes++;
	    if (changes > 3) break;
	    num_items = c;
	    my_wt = new_wt;
	}
    }

    TEST_AND_EXPLAIN(changes > 3, "MSet not varied enough to test");
    if (verbose) {
        tout << "Cutoff weight: " << my_wt << "\n";
    }

    Xapian::Query cutoffq(Xapian::Query::OP_WEIGHT_CUTOFF, q);
    cutoffq.set_cutoff(my_wt);
    enquire.set_query(cutoffq);
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);

    if (verbose) {
        tout << "Weights after cutoff:";
	print_mset_weights(mymset2);
        tout << "\n";
    }

    TEST_AND_EXPLAIN(mymset2.size() >= num_items,
		     "Match with cutoff lost too many items");

    TEST_AND_EXPLAIN(mymset2.size() == num_items ||
		     (mymset2[num_items].get_weight() == my_wt &&
		      mymset2.back().get_weight() == my_wt),
		     "Match with cutoff returned too many items");

    return true;
}

static void
print_mset_percentages(const Xapian::MSet &mset)
{
    Xapian::MSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
        tout << " " << mset.convert_to_percent(i);
    }
}

// tests the percent cutoff option
static bool test_pctcutoff1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire, query(Xapian::Query::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset pcts:";
	print_mset_percentages(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    int my_pct = 100;
    int changes = 0;
    Xapian::MSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
        int new_pct = mymset1.convert_to_percent(i);
        if (new_pct != my_pct) {
	    changes++;
	    if (changes > 3) break;
	    num_items = c;
	    my_pct = new_pct;
	}
    }

    TEST_AND_EXPLAIN(changes > 3, "MSet not varied enough to test");
    if (verbose) {
        tout << "Cutoff percent: " << my_pct << "\n";
    }

    enquire.set_cutoff(my_pct);
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);

    if (verbose) {
        tout << "Percentages after cutoff:";
	print_mset_percentages(mymset2);
        tout << "\n";
    }

    TEST_AND_EXPLAIN(mymset2.size() >= num_items,
		     "Match with % cutoff lost too many items");

    TEST_AND_EXPLAIN(mymset2.size() == num_items ||
		     (mymset2.convert_to_percent(mymset2[num_items]) == my_pct &&
		      mymset2.convert_to_percent(mymset2.back()) == my_pct),
		     "Match with % cutoff returned too many items");

    return true;
}

// tests the allow query terms expand option
static bool test_allowqterms1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(1000, myrset);
    Xapian::ESetIterator j = myeset.begin();
    for ( ; j != myeset.end(); ++j) {
        TEST_NOT_EQUAL(*j, "this");
    }

    Xapian::ESet myeset2 = enquire.get_eset(1000, myrset, Xapian::Enquire::include_query_terms);
    j = myeset2.begin();
    for ( ; j != myeset2.end(); ++j) {
        if (*j == "this") break;
    }
    TEST(j != myeset2.end());
    return true;
}

// tests that the MSet max_attained works
static bool test_maxattain1()
{
    Xapian::MSet mymset = do_get_simple_query_mset(query("this"), 100, 0);

    Xapian::weight mymax = 0;
    Xapian::MSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
        if (i.get_weight() > mymax) mymax = i.get_weight();
    }
    TEST_EQUAL(mymax, mymset.get_max_attained());

    return true;
}

// tests the collapse-on-key
static bool test_collapsekey1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    Xapian::doccount mymsize1 = mymset1.size();

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 100);

	TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
			 "Had no fewer items when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// tests the collapse-on-key for DA databases
static bool test_collapsekey2()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    Xapian::doccount mymsize1 = mymset1.size();

    const Xapian::valueno value_no = 0;
    enquire.set_collapse_key(value_no);
    Xapian::MSet mymset = enquire.get_mset(0, 100);

    TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
		     "Had no fewer items when performing collapse: don't know whether it worked.");

    map<string, Xapian::docid> values;
    Xapian::MSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
	string value = i.get_document().get_value(value_no);
	TEST(values[value] == 0 || value == "");
	values[value] = *i;
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately.
static bool test_collapsekey3()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset1 = enquire.get_mset(0, 3);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST_AND_EXPLAIN(mymset1.get_matches_lower_bound() > mymset.get_matches_lower_bound(),
			 "Lower bound was not lower when performing collapse: don't know whether it worked.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() > mymset.get_matches_upper_bound(),
			 "Upper bound was not lower when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    // Test that, if no duplicates are found (eg, by collapsing on key 1000,
    // which has no entries), the upper bound stays the same, but the lower
    // bound drops.
    {
        Xapian::valueno value_no = 1000;
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST_AND_EXPLAIN(mymset1.get_matches_lower_bound() > mymset.get_matches_lower_bound(),
			 "Lower bound was not lower when performing collapse: don't know whether it worked.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() == mymset.get_matches_upper_bound(),
			 "Upper bound was not equal when collapse turned on, but no duplicates found.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately even when no results are requested.
static bool test_collapsekey4()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    Xapian::MSet mymset1 = enquire.get_mset(0, 0);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 0);

	TEST_AND_EXPLAIN(mymset.get_matches_lower_bound() == 1,
			 "Lower bound was not 1 when performing collapse but not asking for any results.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() == mymset.get_matches_upper_bound(),
			 "Upper bound was changed when performing collapse but not asking for any results.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// tests a reversed boolean query
static bool test_reversebool1()
{
    Xapian::Enquire enquire(get_simple_database());
    Xapian::Query query("this");
    init_simple_enquire(enquire, query);
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    enquire.set_sort_forward(true);
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);
    enquire.set_sort_forward(false);
    Xapian::MSet mymset3 = enquire.get_mset(0, 100);

    // mymset1 and mymset2 should be identical
    TEST_EQUAL(mymset1.size(), mymset2.size());

    {
	Xapian::MSetIterator i = mymset1.begin();
	Xapian::MSetIterator j = mymset2.begin();
	for ( ; i != mymset1.end(), j != mymset2.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
    }

    // mymset1 and mymset3 should be same but reversed
    TEST_EQUAL(mymset1.size(), mymset3.size());

    {
	Xapian::MSetIterator i = mymset1.begin();
#ifdef __SUNPRO_CC
	vector<Xapian::docid> rev;
	for (Xapian::MSetIterator t = mymset3.begin(); t != mymset3.end(); ++t) 
	    rev.push_back(*t);
#else
	vector<Xapian::docid> rev(mymset3.begin(), mymset3.end());
#endif
	// Next iterator not const because of compiler brokenness (egcs 1.1.2)
	vector<Xapian::docid>::reverse_iterator j = rev.rbegin();
	for ( ; i != mymset1.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=false didn't
	    // reverse the results.
	    TEST_EQUAL(*i, *j);
	}
    }

    return true;
}

// tests a reversed boolean query, where the full mset isn't returned
static bool test_reversebool2()
{
    Xapian::Enquire enquire(get_simple_database());
    Xapian::Query query("this");
    init_simple_enquire(enquire, query);
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    enquire.set_sort_forward(true);
    Xapian::doccount msize = mymset1.size() / 2;
    Xapian::MSet mymset2 = enquire.get_mset(0, msize);
    enquire.set_sort_forward(false);
    Xapian::MSet mymset3 = enquire.get_mset(0, msize);

    // mymset2 should be first msize items of mymset1
    TEST_EQUAL(msize, mymset2.size());

    {
	Xapian::MSetIterator i = mymset1.begin();
	Xapian::MSetIterator j = mymset2.begin();
	for ( ; i != mymset1.end(), j != mymset2.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
    }

    // mymset3 should be last msize items of mymset1, in reverse order
    TEST_EQUAL(msize, mymset3.size());
    {
#ifdef __SUNPRO_CC
	vector<Xapian::docid> rev;
	for (Xapian::MSetIterator t = mymset1.begin(); t != mymset1.end(); ++t) 
	    rev.push_back(*t);
#else
	vector<Xapian::docid> rev(mymset1.begin(), mymset1.end());
#endif
	// Next iterator not const because of compiler brokenness (egcs 1.1.2)
	vector<Xapian::docid>::reverse_iterator i = rev.rbegin();
	Xapian::MSetIterator j = mymset3.begin();
	for ( ; j != mymset3.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=false didn't
	    // reverse the results.
	    TEST_EQUAL(*i, *j);
	}
    }

    return true;
}

// tests that get_matching_terms() returns the terms in the right order
static bool test_getmterms1()
{
    om_termname_list answers_list;
    answers_list.push_back("one");
    answers_list.push_back("two");
    answers_list.push_back("three");
    answers_list.push_back("four");

    Xapian::Database mydb(get_database("apitest_termorder"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery(Xapian::Query::OP_OR,
	    Xapian::Query(Xapian::Query::OP_AND,
		    Xapian::Query("one", 1, 1),
		    Xapian::Query("three", 1, 3)),
	    Xapian::Query(Xapian::Query::OP_OR,
		    Xapian::Query("four", 1, 4),
		    Xapian::Query("two", 1, 2)));

    enquire.set_query(myquery);

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    TEST_MSET_SIZE(mymset, 1);
#ifdef __SUNPRO_CC
    om_termname_list list;
    {
        Xapian::TermIterator t;
        for (t = enquire.get_matching_terms_begin(mymset.begin());
	     t != enquire.get_matching_terms_end(mymset.begin()); ++t) {
            list.push_back(*t);
	}
    }
#else
    om_termname_list list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
#endif
    TEST(list == answers_list);

    return true;
}

// tests that get_matching_terms() returns the terms only once
static bool test_getmterms2()
{
    om_termname_list answers_list;
    answers_list.push_back("one");
    answers_list.push_back("two");
    answers_list.push_back("three");

    Xapian::Database mydb(get_database("apitest_termorder"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery(Xapian::Query::OP_OR,
	    Xapian::Query(Xapian::Query::OP_AND,
		    Xapian::Query("one", 1, 1),
		    Xapian::Query("three", 1, 3)),
	    Xapian::Query(Xapian::Query::OP_OR,
		    Xapian::Query("one", 1, 4),
		    Xapian::Query("two", 1, 2)));

    enquire.set_query(myquery);

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    TEST_MSET_SIZE(mymset, 1);
#ifdef __SUNPRO_CC
    om_termname_list list;
    {
        Xapian::TermIterator t;
        for (t = enquire.get_matching_terms_begin(mymset.begin());
	     t != enquire.get_matching_terms_end(mymset.begin()); ++t) {
            list.push_back(*t);
	}
    }
#else
    om_termname_list list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
#endif
    TEST(list == answers_list);

    return true;
}

// tests that specifying a nonexistent input file throws an exception.
static bool test_absentfile1()
{
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Database mydb(get_database("/this_does_not_exist"));
		   Xapian::Enquire enquire(mydb);
		   
		   Xapian::Query myquery("cheese");
		   enquire.set_query(myquery);
		   
		   Xapian::MSet mymset = enquire.get_mset(0, 10););
    return true;
}

// tests that the collapsing on termpos optimisation works
static bool test_poscollapse1()
{
    Xapian::Query myquery1(Xapian::Query::OP_OR,
		     Xapian::Query("this", 1, 1),
		     Xapian::Query("this", 1, 1));
    Xapian::Query myquery2("this", 2, 1);

    if (verbose) {
	tout << myquery1.get_description() << "\n";
	tout << myquery2.get_description() << "\n";
    }

    Xapian::MSet mymset1 = do_get_simple_query_mset(myquery1);
    Xapian::MSet mymset2 = do_get_simple_query_mset(myquery2);

    TEST_EQUAL(mymset1, mymset2);

    return true;
}

// tests that the collapsing on termpos optimisation gives correct query length
static bool test_poscollapse2()
{
    Xapian::Query q(Xapian::Query::OP_OR, Xapian::Query("this", 1, 1), Xapian::Query("this", 1, 1));
    TEST_EQUAL(q.get_length(), 2);
    return true;
}

// test that running a query twice returns the same results
static bool test_repeatquery1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    enquire.set_query(query(Xapian::Query::OP_OR, "this", "word"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset1, mymset2);

    return true;
}

// test that prefetching documents works (at least, gives same results)
static bool test_fetchdocs1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    enquire.set_query(query(Xapian::Query::OP_OR, "this", "word"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset1, mymset2);
    mymset2.fetch(mymset2[0], mymset2[mymset2.size() - 1]);
    mymset2.fetch(mymset2.begin(), mymset2.end());
    mymset2.fetch(mymset2.begin());
    mymset2.fetch();

    Xapian::MSetIterator it1 = mymset1.begin();
    Xapian::MSetIterator it2 = mymset2.begin();

    while (it1 != mymset1.end() && it2 != mymset2.end()) {
	TEST_EQUAL(it1.get_document().get_data(),
		   it2.get_document().get_data());
	TEST_NOT_EQUAL(it1.get_document().get_data(), "");
	TEST_NOT_EQUAL(it2.get_document().get_data(), "");
	it1++;
	it2++;
    }
    TEST_EQUAL(it1, mymset1.end());
    TEST_EQUAL(it1, mymset2.end());

    return true;
}

// test that searching for a term with a space or backslash in it works
static bool test_spaceterms1()
{
    Xapian::Enquire enquire(get_database("apitest_space"));
    Xapian::MSet mymset;
    Xapian::doccount count;
    Xapian::MSetIterator m;
    Xapian::Stem stemmer("english");

    init_simple_enquire(enquire, Xapian::Query(stemmer.stem_word("space man")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	TEST_NOT_EQUAL(mymset.begin().get_document().get_data(), "");
	TEST_NOT_EQUAL(mymset.begin().get_document().get_value(value_no), "");
    }

    init_simple_enquire(enquire, Xapian::Query(stemmer.stem_word("tab\tby")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	string value = mymset.begin().get_document().get_value(value_no);
	TEST_NOT_EQUAL(value, "");
	if (value_no == 0) {
	    TEST(value.size() > 262);
	    TEST_EQUAL((unsigned char)(value[261]), 255);
	}
    }
    
    init_simple_enquire(enquire, Xapian::Query(stemmer.stem_word("back\\slash")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    return true;
}

// test that XOR queries work
static bool test_xor1()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    Xapian::Stem stemmer("english");

    vector<string> terms;
    terms.push_back(stemmer.stem_word("this"));
    terms.push_back(stemmer.stem_word("word"));
    terms.push_back(stemmer.stem_word("of"));

    Xapian::Query query(Xapian::Query::OP_XOR, terms.begin(), terms.end());
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2, 5, 6);

    return true;
}

// test Xapian::Database::get_document()
static bool test_getdoc1()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::Document doc(db.get_document(1));
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.get_document(0));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(999999999));    
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(123456789));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(3));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));
    // Check that Document works as a handle on modification
    // (this was broken for the first try at Xapian::Document prior to 0.7).
    Xapian::Document doc2 = doc;
    doc.set_data("modified!");
    TEST_EQUAL(doc.get_data(), "modified!");
    TEST_EQUAL(doc.get_data(), doc2.get_data());
    return true;
}

// test whether operators with no elements work as a null query
static bool test_emptyop1()
{
    Xapian::Enquire enquire(get_simple_database());
    vector<Xapian::Query> nullvec;
    
    Xapian::Query query1(Xapian::Query::OP_XOR, nullvec.begin(), nullvec.end());

    Xapian::MSet mymset = do_get_simple_query_mset(query1);
    TEST_MSET_SIZE(mymset, 0);
    TEST_EXCEPTION(Xapian::InvalidArgumentError, enquire.get_matching_terms_begin(1));

    return true;
}

// test for keepalives
static bool test_keepalive1()
{
    Xapian::Database db(get_network_database("apitest_simpledata", 5000));

    /* Test that keep-alives work */
    for (int i=0; i<10; ++i) {
	sleep(2);
	db.keep_alive();
    }
    Xapian::Enquire enquire(db);
    Xapian::Query myquery("word");
    enquire.set_query(myquery);
    enquire.get_mset(0, 10);

    /* Test that things break without keepalives */
    sleep(10);
    enquire.set_query(myquery);
    TEST_EXCEPTION(Xapian::NetworkError,
		   enquire.get_mset(0, 10));

    return true;
}

// test that iterating through all terms in a database works.
static bool test_allterms1()
{
    Xapian::Database db(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    Xapian::TermIterator ati2 = ati;

    ati++;
    TEST(ati != db.allterms_end());
    if (verbose) {
	tout << "*ati = `" << *ati << "'\n";
	tout << "*ati.length = `" << (*ati).length() << "'\n";
	tout << "*ati == \"one\" = " << (*ati == "one") << "\n";
	tout << "*ati[3] = " << ((*ati)[3]) << "\n";
	tout << "*ati = `" << *ati << "'\n";
    }
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "one");
    TEST(ati2.get_termfreq() == 1);
#endif

    ++ati;
#if 0
    ++ati2;
#endif
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "three");
    TEST(ati2.get_termfreq() == 3);
#endif

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}
    
// test that iterating through all terms in two databases works.
static bool test_allterms2()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));
    Xapian::TermIterator ati = db.allterms_begin();

    TEST(ati != db.allterms_end());
    TEST(*ati == "five");
    TEST(ati.get_termfreq() == 2);
    ati++;

    TEST(ati != db.allterms_end());
    TEST(*ati == "four");
    TEST(ati.get_termfreq() == 1);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    ++ati;
    TEST(ati != db.allterms_end());
    TEST(*ati == "six");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}

// test that skip_to sets at_end (regression test)
static bool test_allterms3()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();

    ati.skip_to(string("zzzzzz"));
    TEST(ati == db.allterms_end());

    return true;
}

// test that next ignores extra entries due to long posting lists being
// chunked (regression test for quartz)
static bool test_allterms4()
{
    Xapian::WritableDatabase db = get_writable_database("");
    // 682 was the magic number which started to cause Quartz problems
    for (int c = 0; c < 682; ++c) {
	Xapian::Document doc;
	doc.add_term("foo");
	db.add_document(doc); 
    }
    db.flush();

    Xapian::TermIterator i = db.allterms_begin();
    TEST(i != db.allterms_end());
    TEST(*i == "foo");
    TEST(i.get_termfreq() == 682);
    ++i;
    TEST(i == db.allterms_end());

    return true;
}

// test that searching for a term with a special characters in it works
static bool test_specialterms1()
{
    Xapian::Enquire enquire(get_database("apitest_space"));
    Xapian::MSet mymset;
    Xapian::doccount count;
    Xapian::MSetIterator m;
    Xapian::Stem stemmer("english");

    init_simple_enquire(enquire, Xapian::Query(stemmer.stem_word("new\nline")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 0; value_no < 7; ++value_no) {
	string value = mymset.begin().get_document().get_value(value_no);
	TEST_NOT_EQUAL(value, "");
	if (value_no == 0) {
	    TEST(value.size() > 263);
	    TEST_EQUAL((unsigned char)(value[262]), 255);
	    for (int k = 0; k < 256; k++) {
		TEST_EQUAL((unsigned char)(value[k+7]), k);
	    }
	}
    }
    
    init_simple_enquire(enquire,
			Xapian::Query(stemmer.stem_word(string("big\0zero", 8))));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    return true;
}

// test that searching for a term not in the database fails nicely
static bool test_absentterm1()
{
    Xapian::Enquire enquire(get_simple_database());
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::Query query("frink");
    init_simple_enquire(enquire, query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    return true;
}

// as absentterm1, but setting query from a vector of terms
static bool test_absentterm2()
{
    Xapian::Enquire enquire(get_simple_database());
    vector<string> terms;
    terms.push_back("frink");

    Xapian::Query query(Xapian::Query::OP_OR, terms.begin(), terms.end());
    enquire.set_query(query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    return true;
}

// test that rsets do sensible things
static bool test_rset1()
{
    Xapian::Database mydb(get_database("apitest_rset"));
    Xapian::Enquire enquire(mydb);
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "giraffe", "tiger");
    enquire.set_query(myquery);

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    myrset.add_document(1);

    Xapian::MSet mymset2 = enquire.get_mset(0, 10, &myrset);

    // We should have the same documents turn up, but 1 and 3 should
    // have higher weights with the RSet.
    TEST_MSET_SIZE(mymset1, 3);
    TEST_MSET_SIZE(mymset2, 3);

    return true;
}

// test that rsets do more sensible things
static bool test_rset2()
{
    Xapian::Database mydb(get_database("apitest_rset"));
    Xapian::Enquire enquire(mydb);
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "cuddly", "people");
    enquire.set_query(myquery);

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    myrset.add_document(2);

    Xapian::MSet mymset2 = enquire.get_mset(0, 10, &myrset);

    mset_expect_order(mymset1, 1, 2);
    mset_expect_order(mymset2, 2, 1);

    return true;
}

// test that rsets behave correctly with multiDBs
static bool test_rsetmultidb1()
{
    Xapian::Database mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    Xapian::Database mydb2(get_database("apitest_rset"));
    mydb2.add_database(get_database("apitest_simpledata2"));

    Xapian::Enquire enquire1(mydb1);
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery = query(Xapian::Query::OP_OR, "cuddly", "multiple");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    Xapian::RSet myrset1;
    Xapian::RSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    Xapian::MSet mymset1a = enquire1.get_mset(0, 10);
    Xapian::MSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    Xapian::MSet mymset2a = enquire2.get_mset(0, 10);
    Xapian::MSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    mset_expect_order(mymset1a, 1, 4);
    mset_expect_order(mymset1b, 4, 1);
    mset_expect_order(mymset2a, 1, 2);
    mset_expect_order(mymset2b, 2, 1);

    mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2);
    mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2);
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

// test that rsets behave correctly with multiDBs
static bool test_rsetmultidb2()
{
    Xapian::Database mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    Xapian::Database mydb2(get_database("apitest_rset"));
    mydb2.add_database(get_database("apitest_simpledata2"));

    Xapian::Enquire enquire1(mydb1);
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery = query("is");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    Xapian::RSet myrset1;
    Xapian::RSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    Xapian::MSet mymset1a = enquire1.get_mset(0, 10);
    Xapian::MSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    Xapian::MSet mymset2a = enquire2.get_mset(0, 10);
    Xapian::MSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    mset_expect_order(mymset1a, 4, 3);
    mset_expect_order(mymset1b, 4, 3);
    mset_expect_order(mymset2a, 2, 5);
    mset_expect_order(mymset2b, 2, 5);

    mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2);
    mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2);
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

// regression tests - used to cause assertion in stats.h to fail
static bool test_rsetmultidb3()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata2"));
    enquire.set_query(query(Xapian::Query::OP_OR, "cuddly", "people"));
    Xapian::MSet mset = enquire.get_mset(0, 10); // used to fail assertion
    return true;
}

/// Simple test of the elite set operator.
static bool test_eliteset1()
{
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery1 = query(Xapian::Query::OP_OR, "word");
    myquery1.set_length(2); // so the query lengths are the same

    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, "simple", "word");
    myquery2.set_elite_set_size(1);

    enquire.set_query(myquery1);
    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);
    return true;
}

/// Test that the elite set operator works if the set contains
/// sub-expressions (regression test)
static bool test_eliteset2()
{
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery1 = query(Xapian::Query::OP_AND, "word", "search");

    Xapian::Query myquery2(Xapian::Query::OP_ELITE_SET,
		     query("this"),
		     query(Xapian::Query::OP_AND, "word", "search"));
    myquery2.set_elite_set_size(1);

    enquire.set_query(myquery1);
    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);
    // query lengths differ so mset weights not the same (with some weighting
    // parameters)
    //test_mset_order_equal(mymset1, mymset2);

    return true;
}

/// Test that elite set doesn't affect query results if we have fewer
/// terms than the threshold
static bool test_eliteset3()
{
    Xapian::Database mydb1(get_database("apitest_simpledata"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Enquire enquire2(mydb2);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer.stem_word("word");
    string term2 = stemmer.stem_word("rubbish");
    string term3 = stemmer.stem_word("banana");

    vector<string> terms;
    terms.push_back(term1);
    terms.push_back(term2);
    terms.push_back(term3);

    Xapian::Query myquery1(Xapian::Query::OP_OR, terms.begin(), terms.end());
    enquire1.set_query(myquery1);

    Xapian::Query myquery2(Xapian::Query::OP_ELITE_SET, terms.begin(), terms.end());
    myquery2.set_elite_set_size(3);
    enquire2.set_query(myquery2);

    // retrieve the results
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1.get_termfreq(term1),
	       mymset2.get_termfreq(term1));
    TEST_EQUAL(mymset1.get_termweight(term1),
	       mymset2.get_termweight(term1));
    TEST_EQUAL(mymset1.get_termfreq(term2),
	       mymset2.get_termfreq(term2));
    TEST_EQUAL(mymset1.get_termweight(term2),
	       mymset2.get_termweight(term2));
    TEST_EQUAL(mymset1.get_termfreq(term3),
	       mymset2.get_termfreq(term3));
    TEST_EQUAL(mymset1.get_termweight(term3),
	       mymset2.get_termweight(term3));
//    TEST_EQUAL(mymset1, mymset2);

    return true;
}

/// Test that elite set doesn't pick terms with 0 frequency
static bool test_eliteset4()
{
    Xapian::Database mydb1(get_database("apitest_simpledata"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery1 = query("rubbish");
    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, "word", "rubbish", "fibble");
    myquery2.set_elite_set_size(1);
    enquire1.set_query(myquery1);
    enquire2.set_query(myquery2);

    // retrieve the results
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_NOT_EQUAL(mymset2.size(), 0);
    TEST_EQUAL(mymset1, mymset2);
//    TEST_EQUAL(mymset1, mymset2);

    return true;
}

/// Test that the termfreq returned by termlists is correct.
static bool test_termlisttermfreq1()
{
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);
    Xapian::Stem stemmer("english");
    Xapian::RSet rset1;
    Xapian::RSet rset2;
    rset1.add_document(5);
    rset2.add_document(6);

    Xapian::ESet eset1 = enquire.get_eset(1000, rset1);
    Xapian::ESet eset2 = enquire.get_eset(1000, rset2);

    // search for weight of term 'another'
    string theterm = stemmer.stem_word("another");

    Xapian::weight wt1 = 0;
    Xapian::weight wt2 = 0;
    {
	Xapian::ESetIterator i = eset1.begin();
	for ( ; i != eset1.end(); i++) {
	    if (*i == theterm) {
		wt1 = i.get_weight();
		break;
	    }
	}
    }
    {
	Xapian::ESetIterator i = eset2.begin();
	for ( ; i != eset2.end(); i++) {
	    if (*i == theterm) {
		wt2 = i.get_weight();
		break;
	    }
	}
    }

    TEST_NOT_EQUAL(wt1, 0);
    TEST_NOT_EQUAL(wt2, 0);
    TEST_EQUAL(wt1, wt2);

    return true;
}

// tests an expand across multiple databases
static bool test_multiexpand1()
{
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make simple equivalent rsets, with a document from each database in each.
    Xapian::RSet rset1;
    Xapian::RSet rset2;
    rset1.add_document(1);
    rset1.add_document(7);
    rset2.add_document(1);
    rset2.add_document(2);

    // retrieve the top ten results from each method of accessing
    // multiple text files

    // This is the single database one.
    Xapian::ESet eset1 = enquire1.get_eset(1000, rset1);

    // This is the multi database with approximation
    Xapian::ESet eset2 = enquire2.get_eset(1000, rset2);

    // This is the multi database without approximation
    Xapian::ESet eset3 = enquire2.get_eset(1000, rset2, Xapian::Enquire::use_exact_termfreq);

    TEST_EQUAL(eset1.size(), eset2.size());
    TEST_EQUAL(eset1.size(), eset3.size());

    Xapian::ESetIterator i = eset1.begin();
    Xapian::ESetIterator j = eset2.begin();
    Xapian::ESetIterator k = eset3.begin();
    bool all_iwts_equal_jwts = true;
    while (i != eset1.end() && j != eset2.end() && k != eset3.end()) {
	if (i.get_weight() != j.get_weight()) all_iwts_equal_jwts = false;
	TEST_EQUAL(i.get_weight(), k.get_weight());
	TEST_EQUAL(*i, *k);
	++i;
	++j;
	++k;
    }
    TEST(i == eset1.end());
    TEST(j == eset2.end());
    TEST(k == eset3.end());
    TEST(!all_iwts_equal_jwts);
    return true;
}

/// Test the termfrequency and termweight info returned for query terms
static bool test_qterminfo1()
{
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer.stem_word("word");
    string term2 = stemmer.stem_word("inmemory");
    string term3 = stemmer.stem_word("flibble");

    Xapian::Query myquery(Xapian::Query::OP_OR,
		    Xapian::Query(term1),
		    Xapian::Query(Xapian::Query::OP_OR,
			    Xapian::Query(term2),
			    Xapian::Query(term3)));
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the results
    Xapian::MSet mymset1a = enquire1.get_mset(0, 0);
    Xapian::MSet mymset2a = enquire2.get_mset(0, 0);

    TEST_EQUAL(mymset1a.get_termfreq(term1),
	       mymset2a.get_termfreq(term1));
    TEST_EQUAL(mymset1a.get_termweight(term1),
	       mymset2a.get_termweight(term1));
    TEST_EQUAL(mymset1a.get_termfreq(term2),
	       mymset2a.get_termfreq(term2));
    TEST_EQUAL(mymset1a.get_termweight(term2),
	       mymset2a.get_termweight(term2));
    TEST_EQUAL(mymset1a.get_termfreq(term3),
	       mymset2a.get_termfreq(term3));
    TEST_EQUAL(mymset1a.get_termweight(term3),
	       mymset2a.get_termweight(term3));

    TEST_EQUAL(mymset1a.get_termfreq(term1), 3);
    TEST_EQUAL(mymset1a.get_termfreq(term2), 1);
    TEST_EQUAL(mymset1a.get_termfreq(term3), 0);

    TEST_NOT_EQUAL(mymset1a.get_termweight(term1), 0);
    TEST_NOT_EQUAL(mymset1a.get_termweight(term2), 0);
    // non-existant terms still have weight
    TEST_NOT_EQUAL(mymset1a.get_termweight(term3), 0);

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   mymset1a.get_termfreq("sponge"));

    return true;
}

/// Regression test for bug #37.
static bool test_qterminfo2()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer.stem_word("paragraph");
    string term2 = stemmer.stem_word("another");

    Xapian::Query query(Xapian::Query::OP_AND_NOT, term1,
	    Xapian::Query(Xapian::Query::OP_AND, term1, term2));
    enquire.set_query(query);

    // retrieve the results
    // Note: get_mset() used to throw "AssertionError" in debug builds
    Xapian::MSet mset = enquire.get_mset(0, 10);

    TEST_NOT_EQUAL(mset.get_termweight("paragraph"), 0);

    return true;
}

// tests that when specifiying that no items are to be returned, those
// statistics which should be the same are.
static bool test_msetzeroitems1()
{
    Xapian::MSet mymset1 = do_get_simple_query_mset(query("this"), 0);
    Xapian::MSet mymset2 = do_get_simple_query_mset(query("this"), 1);

    TEST_EQUAL(mymset1.get_max_possible(), mymset2.get_max_possible());

    return true;
}

// test that the matches_* of a simple query are as expected
static bool test_matches1()
{
    Xapian::Query myquery;
    Xapian::MSet mymset;

    myquery = query("word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "inmemory", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);

    myquery = query(Xapian::Query::OP_AND, "simple", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "simple", "word");
    mymset = do_get_simple_query_mset(myquery, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    mymset = do_get_simple_query_mset(myquery, 1);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    mymset = do_get_simple_query_mset(myquery, 2);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "paragraph", "another");
    mymset = do_get_simple_query_mset(myquery, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    mymset = do_get_simple_query_mset(myquery, 1);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    mymset = do_get_simple_query_mset(myquery, 2);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 1);

    mymset = do_get_simple_query_mset(myquery, 1, 20);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 1);

    return true;
}

// test that indexing a term more than once at the same position increases
// the wdf
static bool test_adddoc1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1, doc2, doc3;

    // doc1 should come top, but if term "foo" gets wdf of 1, doc2 will beat it
    // doc3 should beat both
    // Note: all docs have same length
    doc1.set_data(string("tom"));
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 3);
    doc1.add_posting("bar", 4);
    db.add_document(doc1);
    
    doc2.set_data(string("dick"));
    doc2.add_posting("foo", 1);
    doc2.add_posting("foo", 2);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    db.add_document(doc2);

    doc3.set_data(string("harry"));
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 2);
    doc3.add_posting("foo", 2);
    doc3.add_posting("bar", 3);
    db.add_document(doc3);

    Xapian::Query query("foo");

    Xapian::Enquire enq(db);
    enq.set_query(query);

    Xapian::MSet mset = enq.get_mset(0, 10);

    mset_expect_order(mset, 3, 1, 2);

    return true;    
}

// test that removing a posting and removing a term works
static bool test_adddoc2()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);
    // Quartz had a bug handling a term >= 128 characters longer than the
    // preceding term in the sort order - this is "foo" + 130 "X"s
    doc1.add_posting("fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 1);
    Xapian::docid did;

    Xapian::Document doc2 = db.get_document(did = db.add_document(doc1));
    TEST_EQUAL(did, 1);

    Xapian::TermIterator iter1 = doc1.termlist_begin();
    Xapian::TermIterator iter2 = doc2.termlist_begin();
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bar");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    //TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 4);
    TEST_EQUAL(iter2.get_wdf(), 4);
    //TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    // assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "gone");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    // assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 == doc1.termlist_end());
    TEST(iter2 == doc2.termlist_end());

    doc2.remove_posting("foo", 1, 5);
    doc2.add_term("bat", 0);
    doc2.add_term("bar", 8);
    doc2.add_term("bag", 0);
    doc2.remove_term("gone");
    doc2.remove_term("fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

    // Should have (doc,wdf) pairs: (bag,0)(bar,9)(bat,0)(foo,0)
    // positionlists (bag,none)(bar,3)(bat,none)(foo,2)

    iter2 = doc2.termlist_begin();
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bag");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bar");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bat");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "foo");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 == doc2.termlist_end());

    doc1 = db.get_document(did = db.add_document(doc2));
    TEST_EQUAL(did, 2);

    iter1 = doc1.termlist_begin();
    iter2 = doc2.termlist_begin();
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bag");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    TEST_EQUAL(iter1.get_termfreq(), 1);
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    TEST(iter1.positionlist_begin() == iter1.positionlist_end());
    TEST(iter2.positionlist_begin() == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bar");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 9);
    TEST_EQUAL(iter2.get_wdf(), 9);
    TEST_EQUAL(iter1.get_termfreq(), 2);
    //TEST_EQUAL(iter2.get_termfreq(), 0);

    Xapian::PositionIterator pi1;
    pi1 = iter1.positionlist_begin();
    Xapian::PositionIterator pi2 = iter2.positionlist_begin();
    TEST_EQUAL(*pi1, 3); pi1++;
    TEST_EQUAL(*pi2, 3); pi2++;
    TEST(pi1 == iter1.positionlist_end());
    TEST(pi2 == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bat");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    TEST_EQUAL(iter1.get_termfreq(), 1);
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    TEST(iter1.positionlist_begin() == iter1.positionlist_end());
    TEST(iter2.positionlist_begin() == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    TEST_EQUAL(iter1.get_termfreq(), 2);
    //TEST_EQUAL(iter2.get_termfreq(), 0);

    Xapian::PositionIterator temp1 = iter1.positionlist_begin();
    pi1 = temp1;
    Xapian::PositionIterator temp2 = iter2.positionlist_begin();
    pi2 = temp2;
    TEST_EQUAL(*pi1, 2); pi1++;
    TEST_EQUAL(*pi2, 2); pi2++;
    TEST(pi1 == iter1.positionlist_end());
    TEST(pi2 == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 == doc1.termlist_end());
    TEST(iter2 == doc2.termlist_end());

    return true;    
}

// test that adding lots of documents works, and doesn't leak memory
// REGRESSION FIXED:2003-09-07
static bool test_adddoc3()
{
    Xapian::WritableDatabase db = get_writable_database("");

    for (Xapian::doccount i = 0; i < 2100; ++i) {
	Xapian::Document doc;
	for (Xapian::termcount t = 0; t < 100; ++t) {
	    string term("foo");
	    term += char(t ^ 70 ^ i);
	    doc.add_posting(term, t);
	}
	db.add_document(doc);
    }
    return true;    
}

// tests that database destructors flush if it isn't done explicitly
static bool test_implicitendsession1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc;

    doc.set_data(string("top secret"));
    doc.add_posting("cia", 1);
    doc.add_posting("nsa", 2);
    doc.add_posting("fbi", 3);
    db.add_document(doc);

    return true;
}

// tests that assignment of Xapian::Database and Xapian::WritableDatabase work as expected
static bool test_databaseassign1()
{
    Xapian::WritableDatabase wdb = get_writable_database("");
    Xapian::Database db = get_database("");
    Xapian::Database actually_wdb = wdb;
    Xapian::WritableDatabase w1(wdb);
    w1 = wdb;
    Xapian::Database d1(wdb);
    Xapian::Database d2(actually_wdb);
    d2 = wdb;
    d2 = actually_wdb;
    wdb = wdb; // check assign to itself works
    db = db; // check assign to itself works
    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("gone");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_term("new", 1);
    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.delete_document(1);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));

    doc1 = db.get_document(2);
    doc1.remove_term("foo");
    doc1.add_term("fwing");
    db.replace_document(2, doc1);

    Xapian::Document doc2 = db.get_document(2);
    Xapian::TermIterator tit = doc2.termlist_begin();
    TEST_NOT_EQUAL(tit, doc2.termlist_end());
    TEST_EQUAL(*tit, "bar");
    tit++;
    TEST_NOT_EQUAL(tit, doc2.termlist_end());
    TEST_EQUAL(*tit, "fwing");
    tit++;
    TEST_EQUAL(tit, doc2.termlist_end());

    return true;
}

static bool test_replacedoc()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone",3);
    doc1.add_posting("bar", 4);
    doc1.add_posting("foo", 5);
    Xapian::docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    Xapian::Document doc2;

    doc2.add_posting("foo", 1);
    doc2.add_posting("pipco", 2);
    doc2.add_posting("bar", 4);
    doc2.add_posting("foo", 5);

    db.replace_document(did, doc2);

    Xapian::Document doc3 = db.get_document(did);
    Xapian::TermIterator tIter = doc3.termlist_begin();
    TEST_EQUAL(*tIter, "bar");
    Xapian::PositionIterator pIter = tIter.positionlist_begin();
    TEST_EQUAL(*pIter, 4);
    ++tIter;
    TEST_EQUAL(*tIter, "foo");
    Xapian::PositionIterator qIter = tIter.positionlist_begin();
    TEST_EQUAL(*qIter, 1);
    ++qIter;
    TEST_EQUAL(*qIter, 5);
    ++tIter;
    TEST_EQUAL(*tIter, "pipco");
    Xapian::PositionIterator rIter = tIter.positionlist_begin();
    TEST_EQUAL(*rIter, 2);
    ++tIter;
    TEST_EQUAL(tIter, doc3.termlist_end());
    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc2()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);
    Xapian::docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("one");
    doc1.add_posting("three", 4);

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_posting("one", 7);
    doc1.remove_term("two");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.flush();

    db.reopen();

    db.delete_document(1);
    db.delete_document(2);
    db.delete_document(3);

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(3));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(4));
    
    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);
    TEST_EQUAL(db.get_termfreq("two"), 0);
    TEST_EQUAL(db.get_termfreq("three"), 0);

    TEST_EQUAL(db.term_exists("one"), false);
    TEST_EQUAL(db.term_exists("two"), false);
    TEST_EQUAL(db.term_exists("three"), false);

    TEST_EQUAL(db.get_collection_freq("one"), 0);
    TEST_EQUAL(db.get_collection_freq("two"), 0);
    TEST_EQUAL(db.get_collection_freq("three"), 0);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(3));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(3));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// another test of deletion of documents, a cut-down version of deldoc2
static bool test_deldoc3()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("one", 1);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    db.flush();

    db.reopen();

    db.delete_document(1);

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(1));
    (void)&db; // gcc 2.95 seems to miscompile without this!!! - Olly
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(2));
    
    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);

    TEST_EQUAL(db.term_exists("one"), false);

    TEST_EQUAL(db.get_collection_freq("one"), 0);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(2));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// tests that deletion and updating of (lots of) documents works as expected
static bool test_deldoc4()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);

    Xapian::Document doc2 = doc1;
    doc2.remove_term("one");
    doc2.add_posting("three", 4);

    Xapian::Document doc3 = doc2;
    doc3.add_posting("one", 7);
    doc3.remove_term("two");

    const Xapian::docid maxdoc = 1000 * 3;
    Xapian::docid did;
    for (Xapian::docid i = 0; i < maxdoc / 3; ++i) {
	did = db.add_document(doc1);
	TEST_EQUAL(did, i * 3 + 1);
	did = db.add_document(doc2);
	TEST_EQUAL(did, i * 3 + 2);
	did = db.add_document(doc3);
	TEST_EQUAL(did, i * 3 + 3);

	bool is_power_of_two = ((i & (i - 1)) == 0);
	if (is_power_of_two) {
	    db.flush();
	    db.reopen();
	}
    }
    db.flush();
    db.reopen();

    /* delete the documents in a peculiar order */
    for (Xapian::docid i = 0; i < maxdoc / 3; ++i) {
	db.delete_document(maxdoc - i);
	db.delete_document(maxdoc / 3 + i + 1);
	db.delete_document(i + 1);
    }

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    for (Xapian::docid i = 1; i <= maxdoc; ++i) {
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(i));
    }
    
    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);
    TEST_EQUAL(db.get_termfreq("two"), 0);
    TEST_EQUAL(db.get_termfreq("three"), 0);

    TEST_EQUAL(db.term_exists("one"), false);
    TEST_EQUAL(db.term_exists("two"), false);
    TEST_EQUAL(db.term_exists("three"), false);

    TEST_EQUAL(db.get_collection_freq("one"), 0);
    TEST_EQUAL(db.get_collection_freq("two"), 0);
    TEST_EQUAL(db.get_collection_freq("three"), 0);

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// tests that wqf affects the document weights
static bool test_wqf1()
{
    // both queries have length 2; in q1 word has wqf=2, in q2 word has wqf=1
    Xapian::Query q1("word", 2);
    Xapian::Query q2("word");
    q2.set_length(2);
    Xapian::MSet mset1 = do_get_simple_query_mset(q1);
    Xapian::MSet mset2 = do_get_simple_query_mset(q2);
    // Check the weights
    TEST(mset1.begin().get_weight() > mset2.begin().get_weight());
    return true;
}

// tests that query length affects the document weights
static bool test_qlen1()
{
    Xapian::Query q1("word");
    Xapian::Query q2("word");
    q2.set_length(2);
    Xapian::MSet mset1 = do_get_simple_query_mset(q1);
    Xapian::MSet mset2 = do_get_simple_query_mset(q2);
    // Check the weights
    //TEST(mset1.begin().get_weight() < mset2.begin().get_weight());
    TEST(mset1.begin().get_weight() == mset2.begin().get_weight());
    return true;
}

// tests that opening a non-existant termlist throws the correct exception
static bool test_termlist1()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   Xapian::TermIterator t = db.termlist_begin(0));
    TEST_EXCEPTION(Xapian::DocNotFoundError,
		   Xapian::TermIterator t = db.termlist_begin(2));
    /* Cause the database to be used properly, showing up problems
     * with the link being in a bad state.  CME */
    Xapian::TermIterator temp = db.termlist_begin(1);
    TEST_EXCEPTION(Xapian::DocNotFoundError,
		   Xapian::TermIterator t = db.termlist_begin(999999999));
    return true;
}

// tests that a Xapian::TermIterator works as an STL iterator
static bool test_termlist2()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator t = db.termlist_begin(1);
    Xapian::TermIterator tend = db.termlist_end(1);

    // test operator= creates a copy which compares equal
    Xapian::TermIterator t_copy = t;
    TEST_EQUAL(t, t_copy);

    // test copy constructor creates a copy which compares equal
    Xapian::TermIterator t_clone(t);
    TEST_EQUAL(t, t_clone);

#ifdef __SUNPRO_CC
    vector<string> v;
    while (t != tend) {
	v.push_back(*t);
	++t;
    }
#else
    vector<string> v(t, tend);
#endif

    t = db.termlist_begin(1);    
    tend = db.termlist_end(1);
    vector<string>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	TEST_NOT_EQUAL(t, tend);
	TEST_EQUAL(*i, *t);
	t++;
    }
    TEST_EQUAL(t, tend);
    return true;
}

static Xapian::TermIterator
test_termlist3_helper()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    return db.termlist_begin(1);
}

// tests that a Xapian::TermIterator still works when the DB is deleted
static bool test_termlist3()
{
    Xapian::TermIterator u = test_termlist3_helper();
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator t = db.termlist_begin(1);
    Xapian::TermIterator tend = db.termlist_end(1);

    while (t != tend) {
	TEST_EQUAL(*t, *u);
	t++;
	u++;
    }
    return true;
}

// tests skip_to
static bool test_termlist4()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator i = db.termlist_begin(1);
    i.skip_to("");
    i.skip_to("\xff");
    return true;
}

// tests that opening a non-existant postlist return an empty list
static bool test_postlist1()
{
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.postlist_begin(""));

    TEST_EQUAL(db.postlist_begin("rosebud"), db.postlist_end("rosebud"));

    string s = "let_us_see_if_we_can_break_it_with_a_really_really_long_term.";
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    TEST_EQUAL(db.postlist_begin(s), db.postlist_end(s));

    // a regression test (no, really)
    TEST_NOT_EQUAL(db.postlist_begin("a"), db.postlist_end("a"));

    return true;
}

// tests that a Xapian::PostingIterator works as an STL iterator
static bool test_postlist2()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p;
    p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    // test operator= creates a copy which compares equal
    Xapian::PostingIterator p_copy = p;
    TEST_EQUAL(p, p_copy);

    // test copy constructor creates a copy which compares equal
    Xapian::PostingIterator p_clone(p);
    TEST_EQUAL(p, p_clone);

#ifdef __SUNPRO_CC
    vector<Xapian::docid> v;
    while (p != pend) {
	v.push_back(*p);
	++p;
    }
#else
    vector<Xapian::docid> v(p, pend);
#endif

    p = db.postlist_begin("this");
    pend = db.postlist_end("this");
    vector<Xapian::docid>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	TEST_NOT_EQUAL(p, pend);
	TEST_EQUAL(*i, *p);
	p++;
    }
    TEST_EQUAL(p, pend);
    return true;
}

static Xapian::PostingIterator
test_postlist3_helper()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    return db.postlist_begin("this");
}

// tests that a Xapian::PostingIterator still works when the DB is deleted
static bool test_postlist3()
{
    Xapian::PostingIterator u = test_postlist3_helper();
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    while (p != pend) {
	TEST_EQUAL(*p, *u);
	p++;
	u++;
    }
    return true;
}

// tests skip_to
static bool test_postlist4()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    i.skip_to(1);
    i.skip_to(999999999);
    TEST(i == db.postlist_end("this"));
    return true;
}

// tests long postlists
static bool test_postlist5()
{
    Xapian::Database db(get_database("apitest_manydocs"));
    // Allow for databases which don't support length
    if (db.get_avlength() != 1)
	TEST_EQUAL_DOUBLE(db.get_avlength(), 4);
    Xapian::PostingIterator i = db.postlist_begin("this");
    unsigned int j = 1;
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
    }
    TEST_EQUAL(j, 513);
    return true;
}

// tests document length in postlists
static bool test_postlist6()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    TEST(i != db.postlist_end("this"));
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(i.get_doclength(), db.get_doclength(*i));
	i++;
    }
    return true;
}

// tests collection frequency
static bool test_collfreq1()
{
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.get_collection_freq("this"), 11);
    TEST_EQUAL(db.get_collection_freq("first"), 1);
    TEST_EQUAL(db.get_collection_freq("last"), 0);
    TEST_EQUAL(db.get_collection_freq("word"), 9);

    Xapian::Database db1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Database db2(get_database("apitest_simpledata"));
    db2.add_database(get_database("apitest_simpledata2"));

    TEST_EQUAL(db1.get_collection_freq("this"), 15);
    TEST_EQUAL(db1.get_collection_freq("first"), 1);
    TEST_EQUAL(db1.get_collection_freq("last"), 0);
    TEST_EQUAL(db1.get_collection_freq("word"), 11);
    TEST_EQUAL(db2.get_collection_freq("this"), 15);
    TEST_EQUAL(db2.get_collection_freq("first"), 1);
    TEST_EQUAL(db2.get_collection_freq("last"), 0);
    TEST_EQUAL(db2.get_collection_freq("word"), 11);

    return true;
}

// Regression test for the "more than 100%" sort_bands bug
static bool test_sortbands1()
{
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    const char * terms[] = {"better", "place", "reader", "without", "would"};
    for (size_t j = 0; j < sizeof(terms) / sizeof(const char *); ++j) {
	enquire.set_query(Xapian::Query(terms[j]));
	enquire.set_sorting(Xapian::valueno(-1), 10);
	Xapian::MSet mset = enquire.get_mset(0, 20);
	Xapian::docid prev = 0;
	int band = 9;
	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	    int this_band = (i.get_percent() - 1) / 10;
	    TEST(this_band <= band);
	    if (this_band == band) {
		TEST(prev < *i);
	    } else {
		this_band = band;
	    }
	    prev = *i;
	}
    }
    return true;
}

// Regression test for split msets being incorrect when sorting
static bool test_sortbands2()
{
    Xapian::Enquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    for (int pass = 1; pass <= 2; ++pass) { 
	for (int bands = 1; bands <= 10; bands += 9) {
	    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
		tout << "Sorting on value " << value_no << endl;
		enquire.set_sorting(value_no, bands);
		Xapian::MSet allbset = enquire.get_mset(0, 100);
		Xapian::MSet partbset1 = enquire.get_mset(0, 3);
		Xapian::MSet partbset2 = enquire.get_mset(3, 97);
		TEST_EQUAL(allbset.size(), partbset1.size() + partbset2.size());

		bool ok = true;
		int n = 0;
		Xapian::MSetIterator i, j;
		j = allbset.begin();
		for (i = partbset1.begin(); i != partbset1.end(); ++i) {
		    tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		    TEST(j != allbset.end()); 	
		    if (*i != *j) ok = false;
		    ++j;
		    ++n;
		}
		tout << "===\n";
		for (i = partbset2.begin(); i != partbset2.end(); ++i) {
		    tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		    TEST(j != allbset.end()); 	
		    if (*i != *j) ok = false;
		    ++j;
		    ++n;
		}
		TEST(j == allbset.end()); 	
		if (!ok)
		    FAIL_TEST("Split msets aren't consistent with unsplit");
	    }
	}
        enquire.set_sort_forward(false);
    }

    return true;
}

// consistency check match - vary mset size and check results agree
static bool test_consistency1()
{
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, Xapian::Query("the"), Xapian::Query("sky")));
    Xapian::doccount lots = 214;
    Xapian::MSet bigmset = enquire.get_mset(0, lots);
    try {
	for (Xapian::doccount start = 0; start < lots; ++start) {
	    for (Xapian::doccount size = 0; size < lots - start; ++size) {
		Xapian::MSet mset = enquire.get_mset(start, size);
		if (mset.size()) {
		    TEST_EQUAL(start + mset.size(),
			       min(start + size, bigmset.size()));
		} else if (size) {
//		tout << start << mset.size() << bigmset.size() << endl;
		    TEST(start >= bigmset.size());
		}
		for (Xapian::doccount i = 0; i < mset.size(); ++i) {
		    TEST_EQUAL(*mset[i], *bigmset[start + i]);
		    TEST_EQUAL(mset[i].get_weight(),
			       bigmset[start + i].get_weight());
		}
	    }
	}
    }
    catch (const Xapian::NetworkTimeoutError &) {
	// consistency1 is a long test - may timeout with the remote backend...
	SKIP_TEST("Test taking too long");
    }
    return true;
}

// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which use a backend
test_desc db_tests[] = {
    {"zerodocid1", 	   test_zerodocid1},
    {"emptyquery1",	   test_emptyquery1},
    {"simplequery1",       test_simplequery1},
    {"simplequery3",       test_simplequery3},
    {"multidb1",           test_multidb1},
    {"multidb2",           test_multidb2},
    {"multidb3",           test_multidb3},
    {"multidb4",           test_multidb4},
    {"multidb5",           test_multidb5},
    {"changequery1",	   test_changequery1},
    {"msetmaxitems1",      test_msetmaxitems1},
    {"expandmaxitems1",    test_expandmaxitems1},
    {"boolquery1",         test_boolquery1},
    {"msetfirst1",         test_msetfirst1},
    {"msetfirst2",         test_msetfirst2},
    {"topercent1",	   test_topercent1},
    {"expandfunctor1",	   test_expandfunctor1},
    {"pctcutoff1",	   test_pctcutoff1},
    {"cutoff1",		   test_cutoff1},
    {"cutoff2",		   test_cutoff2},
    {"allowqterms1",       test_allowqterms1},
    {"maxattain1",         test_maxattain1},
    {"reversebool1",	   test_reversebool1},
    {"reversebool2",	   test_reversebool2},
    {"getmterms1",	   test_getmterms1},
    {"getmterms2",	   test_getmterms2},
    {"absentfile1",	   test_absentfile1},
    {"poscollapse1",	   test_poscollapse1},
    {"poscollapse2",	   test_poscollapse2},
    {"repeatquery1",	   test_repeatquery1},
    {"fetchdocs1",	   test_fetchdocs1},
    {"absentterm1",	   test_absentterm1},
    {"absentterm2",	   test_absentterm2},
    {"rset1",              test_rset1},
    {"rset2",              test_rset2},
    {"rsetmultidb1",       test_rsetmultidb1},
    {"rsetmultidb3",       test_rsetmultidb3},
    {"eliteset1",          test_eliteset1},
    {"eliteset2",          test_eliteset2},
    {"eliteset3",          test_eliteset3},
    {"eliteset4",          test_eliteset4},
    {"termlisttermfreq1",  test_termlisttermfreq1},
    {"qterminfo1",	   test_qterminfo1},
    {"qterminfo2",	   test_qterminfo2},
    {"msetzeroitems1",     test_msetzeroitems1},
    {"matches1",	   test_matches1},
    {"wqf1",		   test_wqf1},
    {"qlen1",		   test_qlen1},
    {"termlist1",	   test_termlist1},
    {"termlist2",	   test_termlist2},
    {"termlist3",	   test_termlist3},
    {"termlist4",	   test_termlist4},
    {"puncterms1",	   test_puncterms1},
    {"spaceterms1",	   test_spaceterms1},
    {"xor1",		   test_xor1},
    {"getdoc1",		   test_getdoc1},
    {"emptyop1",	   test_emptyop1},
    {0, 0}
};

/// The tests which require a database which supports values > 0 sensibly
test_desc multivalue_tests[] = {
    {"collapsekey1",	   test_collapsekey1},
    {"collapsekey3",	   test_collapsekey3},
    {"collapsekey4",	   test_collapsekey4},
    {0, 0}
};

test_desc mus36_tests[] = {
    {"collapsekey2",       test_collapsekey2},
    {0, 0}
};

/// The tests which need a backend which supports iterating over all terms
test_desc allterms_tests[] = {
    {"allterms1",	   test_allterms1},
    {"allterms2",	   test_allterms2},
    {"allterms3",	   test_allterms3},
    {"allterms4",	   test_allterms4},
    {0, 0}
};

/// The tests which need a backend which supports terms with newlines / zeros
test_desc specchar_tests[] = {
    {"specialterms1", 	   test_specialterms1},
    {0, 0}
};

/// The tests which need a backend which supports document length information
test_desc doclendb_tests[] = {
// get wrong weight back - probably because no document length in calcs
    {"simplequery2",       test_simplequery2},
// Mset comes out in wrong order - no document length?
    {"rsetmultidb2",       test_rsetmultidb2},
    {0, 0}
};

/// Tests which need getting collection frequencies to be supported.
test_desc collfreq_tests[] = {
    {"collfreq1",          test_collfreq1},
    {0, 0}
};

/// The tests which use a writable backend
test_desc writabledb_tests[] = {
    {"adddoc1",		   test_adddoc1},
    {"adddoc2",		   test_adddoc2},
    {"adddoc3",		   test_adddoc3},
    {"implicitendsession1",test_implicitendsession1},
    {"databaseassign1",	   test_databaseassign1},
    {"deldoc1",		   test_deldoc1},
    {"deldoc2",		   test_deldoc2},
    {"deldoc3",		   test_deldoc3},
    {"deldoc4",		   test_deldoc4},
    {"replacedoc",	   test_replacedoc},
    {0, 0}
};

test_desc localdb_tests[] = {
    {"matchfunctor1",	   test_matchfunctor1},
    {"msetiterator1",	   test_msetiterator1},
    {"msetiterator2",	   test_msetiterator2},
    {"msetiterator3",	   test_msetiterator3},
    {"esetiterator1",	   test_esetiterator1},
    {"esetiterator2",	   test_esetiterator2},
    {"multiexpand1",       test_multiexpand1},
    {"postlist1",	   test_postlist1},
    {"postlist2",	   test_postlist2},
    {"postlist3",	   test_postlist3},
    {"postlist4",	   test_postlist4},
    {"postlist5",	   test_postlist5},
    {"postlist6",	   test_postlist6},
    {"termstats",	   test_termstats},
    {"sortbands1",	   test_sortbands1},
    {"sortbands2",	   test_sortbands2},
    // consistency1 will run on the remote backend, but it's particularly slow
    // with that, and testing it there doesn't actually improve the test
    // coverage really.
    {"consistency1",	   test_consistency1},
    {0, 0}
};

test_desc remotedb_tests[] = {
// FIXME:    {"multierrhandler1",   test_multierrhandler1},
    {"stubdb1",		   test_stubdb1},
    {"keepalive1",	   test_keepalive1},
    {"termstats",	   test_termstats},
    {0, 0}
};
