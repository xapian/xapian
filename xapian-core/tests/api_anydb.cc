/** @file
 * @brief tests which work with any backend
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2015,2016,2017,2020 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
 * Copyright 2011 Action Without Borders
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

#include "api_anydb.h"

#include <algorithm>
#include <string>

#define XAPIAN_DEPRECATED(X) X
#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

#include <list>

using namespace std;

static void
print_mset_weights(const Xapian::MSet &mset)
{
    Xapian::MSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
	tout << " " << i.get_weight();
    }
}

static void
print_mset_percentages(const Xapian::MSet &mset)
{
    Xapian::MSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
	tout << " " << mset.convert_to_percent(i);
    }
}

static Xapian::Query
query(Xapian::Query::op op,
      const string & t1 = string(), const string & t2 = string(),
      const string & t3 = string(), const string & t4 = string(),
      const string & t5 = string(), const string & t6 = string(),
      const string & t7 = string(), const string & t8 = string(),
      const string & t9 = string(), const string & t10 = string())
{
    vector<string> v;
    Xapian::Stem stemmer("english");
    if (!t1.empty()) v.push_back(stemmer(t1));
    if (!t2.empty()) v.push_back(stemmer(t2));
    if (!t3.empty()) v.push_back(stemmer(t3));
    if (!t4.empty()) v.push_back(stemmer(t4));
    if (!t5.empty()) v.push_back(stemmer(t5));
    if (!t6.empty()) v.push_back(stemmer(t6));
    if (!t7.empty()) v.push_back(stemmer(t7));
    if (!t8.empty()) v.push_back(stemmer(t8));
    if (!t9.empty()) v.push_back(stemmer(t9));
    if (!t10.empty()) v.push_back(stemmer(t10));
    return Xapian::Query(op, v.begin(), v.end());
}

static Xapian::Query
query(Xapian::Query::op op, Xapian::termcount parameter,
      const string & t1 = string(), const string & t2 = string(),
      const string & t3 = string(), const string & t4 = string(),
      const string & t5 = string(), const string & t6 = string(),
      const string & t7 = string(), const string & t8 = string(),
      const string & t9 = string(), const string & t10 = string())
{
    vector<string> v;
    Xapian::Stem stemmer("english");
    if (!t1.empty()) v.push_back(stemmer(t1));
    if (!t2.empty()) v.push_back(stemmer(t2));
    if (!t3.empty()) v.push_back(stemmer(t3));
    if (!t4.empty()) v.push_back(stemmer(t4));
    if (!t5.empty()) v.push_back(stemmer(t5));
    if (!t6.empty()) v.push_back(stemmer(t6));
    if (!t7.empty()) v.push_back(stemmer(t7));
    if (!t8.empty()) v.push_back(stemmer(t8));
    if (!t9.empty()) v.push_back(stemmer(t9));
    if (!t10.empty()) v.push_back(stemmer(t10));
    return Xapian::Query(op, v.begin(), v.end(), parameter);
}

static Xapian::Query
query(const string &t)
{
    return Xapian::Query(Xapian::Stem("english")(t));
}

// #######################################################################
// # Tests start here

// tests that the backend doesn't return zero docids
DEFINE_TESTCASE(zerodocid1, backend) {
    // open the database (in this case a simple text file
    // we prepared earlier)

    Xapian::Database mydb(get_database("apitest_onedoc"));

    Xapian::Enquire enquire(mydb);

    // make a simple query, with one word in it - "word".
    enquire.set_query(Xapian::Query("word"));

    // retrieve the top ten results (we only expect one)
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // We've done the query, now check that the result is what
    // we expect (1 document, with non-zero docid)
    TEST_MSET_SIZE(mymset, 1);

    TEST_AND_EXPLAIN(*(mymset.begin()) != 0,
		     "A query on a database returned a zero docid");
}

// tests that an empty query returns no matches
DEFINE_TESTCASE(emptyquery1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));

    enquire.set_query(Xapian::Query());
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 0);

    vector<Xapian::Query> v;
    enquire.set_query(Xapian::Query(Xapian::Query::OP_AND, v.begin(), v.end()));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 0);
}

// tests the document count for a simple query
DEFINE_TESTCASE(simplequery1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 2);
}

// tests for the right documents and weights returned with simple query
DEFINE_TESTCASE(simplequery2, backend) {
    // open the database (in this case a simple text file
    // we prepared earlier)
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));

    // retrieve the top results
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // We've done the query, now check that the result is what
    // we expect (documents 2 and 4)
    mset_expect_order(mymset, 2, 4);

    // Check the weights
    Xapian::MSetIterator i = mymset.begin();
    // These weights are for BM25Weight(1,0,1,0.5,0.5)
    TEST_EQUAL_DOUBLE(i.get_weight(), 1.04648168717725);
    i++;
    TEST_EQUAL_DOUBLE(i.get_weight(), 0.640987686595914);
}

// tests for the right document count for another simple query
DEFINE_TESTCASE(simplequery3, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // Check that 6 documents were returned.
    TEST_MSET_SIZE(mymset, 6);
}

// test that a multidb with 3 dbs query returns correct docids
DEFINE_TESTCASE(multidb2, backend && !multi) {
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
}

// tests that when specifying maxitems to get_mset, no more than
// that are returned.
DEFINE_TESTCASE(msetmaxitems1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 1);
    TEST_MSET_SIZE(mymset, 1);

    mymset = enquire.get_mset(0, 5);
    TEST_MSET_SIZE(mymset, 5);
}

// tests the returned weights are as expected (regression test for remote
// backend which was using the average weight rather than the actual document
// weight for computing weights - fixed in 1.0.0).
DEFINE_TESTCASE(expandweights1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet eset = enquire.get_eset(3, myrset, enquire.USE_EXACT_TERMFREQ);
    TEST_EQUAL(eset.size(), 3);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 4.73383620844021);

    // Test non-default k too.
    eset = enquire.get_eset(3, myrset, enquire.USE_EXACT_TERMFREQ, 2.0);
    TEST_EQUAL(eset.size(), 3);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 5.88109547674955);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 5.88109547674955);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 5.44473599216144);
}

// Just like test_expandweights1 but without USE_EXACT_TERMFREQ.
DEFINE_TESTCASE(expandweights2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet eset = enquire.get_eset(3, myrset);
    TEST_EQUAL(eset.size(), 3);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    // With a multi backend, the top three terms all happen to occur in both
    // shard so their termfreq is exactly known even without
    // USE_EXACT_TERMFREQ and so the weights should be the same for all
    // test harness backends.
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 4.73383620844021);
}

DEFINE_TESTCASE(expandweights3, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    // Set min_wt to 6.0
    Xapian::ESet eset = enquire.get_eset(50, myrset, 0, 0, 6.0);
    TEST_EQUAL(eset.size(), 2);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    // With a multi backend, the top two terms all happen to occur in both
    // shard so their termfreq is exactly known even without
    // USE_EXACT_TERMFREQ and so the weights should be the same for all
    // test harness backends.
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.08904001099445);
}

// tests that negative weights are returned
DEFINE_TESTCASE(expandweights4, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("paragraph"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet eset = enquire.get_eset(37, myrset, 0, 0, -100);
    // Now include negative weights
    TEST_EQUAL(eset.size(), 37);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    TEST_REL(eset[36].get_weight(), <, 0);
    TEST_REL(eset[36].get_weight(), >=, -100);
}

// test for Bo1EWeight
DEFINE_TESTCASE(expandweights5, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    enquire.set_expansion_scheme("bo1");
    Xapian::ESet eset = enquire.get_eset(3, myrset);

    TEST_EQUAL(eset.size(), 3);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 7.21765284821702);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.661623193760022);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 5.58090119783738);
}

// test that "trad" can be set as an expansion scheme.
DEFINE_TESTCASE(expandweights6, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    enquire.set_expansion_scheme("trad");
    Xapian::ESet eset = enquire.get_eset(3, myrset, enquire.USE_EXACT_TERMFREQ);

    TEST_EQUAL(eset.size(), 3);
    TEST_REL(eset.get_ebound(), >=, eset.size());
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 4.73383620844021);
}

// test that invalid scheme names are not accepted
DEFINE_TESTCASE(expandweights7, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   enquire.set_expansion_scheme("no_such_scheme"));
}

// test that "expand_k" can be passed as a parameter to get_eset
DEFINE_TESTCASE(expandweights8, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    // Set expand_k to 1.0 and min_wt to 0
    Xapian::ESet eset = enquire.get_eset(50, myrset, 0, 1.0, 0, 0);
    // With a multi backend, the top three terms all happen to occur in both
    // shard so their termfreq is exactly known even without
    // USE_EXACT_TERMFREQ and so the weights should be the same for all
    // test harness backends.
    TEST_EQUAL_DOUBLE(eset[0].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[1].get_weight(), 6.08904001099445);
    TEST_EQUAL_DOUBLE(eset[2].get_weight(), 4.73383620844021);
    TEST_REL(eset.back().get_weight(),>=,0);
}

// tests that when specifying maxitems to get_eset, no more than
// that are returned.
DEFINE_TESTCASE(expandmaxitems1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    tout << "mymset.size() = " << mymset.size() << '\n';
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(1, myrset);
    TEST_EQUAL(myeset.size(), 1);
    TEST_REL(myeset.get_ebound(), >=, myeset.size());
}

// tests that a pure boolean query has all weights set to 0
DEFINE_TESTCASE(boolquery1, backend) {
    Xapian::Query myboolquery(query("this"));

    // open the database (in this case a simple text file
    // we prepared earlier)
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(myboolquery);
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    // retrieve the top results
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    TEST_NOT_EQUAL(mymset.size(), 0);
    TEST_EQUAL(mymset.get_max_possible(), 0);
    for (Xapian::MSetIterator i = mymset.begin(); i != mymset.end(); ++i) {
	TEST_EQUAL(i.get_weight(), 0);
    }
}

// tests that get_mset() specifying "this" works as expected
DEFINE_TESTCASE(msetfirst1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 6);
    Xapian::MSet mymset2 = enquire.get_mset(3, 3);
    TEST(mset_range_is_same(mymset1, 3, mymset2, 0, 3));

    // Regression test - we weren't adjusting the index into items[] by
    // firstitem in api/omenquire.cc.
    TEST_EQUAL(mymset1[5].get_document().get_data(),
	       mymset2[2].get_document().get_data());
}

// tests the converting-to-percent functions
DEFINE_TESTCASE(topercent1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 20);

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
}

// tests the percentage values returned
DEFINE_TESTCASE(topercent2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));

    int pct;

    // First, test a search in which the top document scores 100%.
    enquire.set_query(query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 20);

    Xapian::MSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    pct = mymset.convert_to_percent(i);
    TEST_EQUAL(pct, 100);

    TEST_EQUAL(mymset.get_matches_lower_bound(), 6);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 6);
    TEST_EQUAL(mymset.get_matches_estimated(), 6);
    TEST_EQUAL_DOUBLE(mymset.get_max_attained(), 0.0553904060041786);
    TEST_EQUAL(mymset.size(), 6);
    mset_expect_order(mymset, 2, 1, 3, 5, 6, 4);

    // A search in which the top document doesn't have 100%
    Xapian::Query q = query(Xapian::Query::OP_OR,
			    "this", "line", "paragraph", "rubbish");
    enquire.set_query(q);
    mymset = enquire.get_mset(0, 20);

    i = mymset.begin();
    TEST(i != mymset.end());
    pct = mymset.convert_to_percent(i);
    TEST_REL(pct,>,60);
    TEST_REL(pct,<,76);

    ++i;

    TEST(i != mymset.end());
    pct = mymset.convert_to_percent(i);
    TEST_REL(pct,>,40);
    TEST_REL(pct,<,50);

    TEST_EQUAL(mymset.get_matches_lower_bound(), 6);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 6);
    TEST_EQUAL(mymset.get_matches_estimated(), 6);
    TEST_EQUAL_DOUBLE(mymset.get_max_attained(), 1.67412192414056);
    TEST_EQUAL(mymset.size(), 6);
    mset_expect_order(mymset, 3, 1, 4, 2, 5, 6);
}

class EvenParityExpandFunctor : public Xapian::ExpandDecider {
  public:
    bool operator()(const string & tname) const {
	unsigned long sum = 0;
	for (unsigned ch : tname) {
	    sum += ch;
	}
//	if (verbose) {
//	    tout << tname << "==> " << sum << "\n";
//	}
	return (sum % 2) == 0;
    }
};

// tests the expand decision functor
DEFINE_TESTCASE(expandfunctor1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    EvenParityExpandFunctor myfunctor;

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
}

DEFINE_TESTCASE(expanddeciderfilterprefix2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset_orig = enquire.get_eset(1000, myrset);
    unsigned int neweset_size = 0;

    // Choose the first char in the first term as prefix.
    Xapian::ESetIterator j = myeset_orig.begin();
    TEST(myeset_orig.size() >= 1);
    string prefix(*j, 0, 1);
    Xapian::ExpandDeciderFilterPrefix myfunctor(prefix);

    for ( ; j != myeset_orig.end(); ++j) {
	if (myfunctor(*j)) neweset_size++;
    }
    Xapian::ESet myeset = enquire.get_eset(neweset_size, myrset, &myfunctor);

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
}

// tests the percent cutoff option
DEFINE_TESTCASE(pctcutoff1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query(Xapian::Query::OP_OR,
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
}

// Tests the percent cutoff option combined with collapsing
DEFINE_TESTCASE(pctcutoff2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_AND_NOT, Xapian::Query("this"), Xapian::Query("banana")));
    Xapian::MSet mset = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset pcts:";
	print_mset_percentages(mset);
	tout << "\n";
    }

    TEST(mset.size() >= 2);
    TEST(mset[0].get_percent() - mset[1].get_percent() >= 2);

    int cutoff = mset[0].get_percent() + mset[1].get_percent();
    cutoff /= 2;

    enquire.set_cutoff(cutoff);
    enquire.set_collapse_key(1234); // Value which is always empty.

    Xapian::MSet mset2 = enquire.get_mset(0, 1);
    TEST_EQUAL(mset2.size(), 1);
    TEST_REL(mset2.get_matches_lower_bound(),>=,1);
    TEST_REL(mset2.get_uncollapsed_matches_lower_bound(),>=,
	     mset2.get_matches_lower_bound());
    TEST_REL(mset2.get_uncollapsed_matches_lower_bound(),<=,mset.size());
    TEST_REL(mset2.get_uncollapsed_matches_upper_bound(),>=,mset.size());
    TEST_REL(mset2.get_uncollapsed_matches_lower_bound(),<=,mset2.get_uncollapsed_matches_estimated());
    TEST_REL(mset2.get_uncollapsed_matches_upper_bound(),>=,mset2.get_uncollapsed_matches_estimated());
}

// Test that the percent cutoff option returns all the answers it should.
DEFINE_TESTCASE(pctcutoff3, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mset1 = enquire.get_mset(0, 10);

    if (verbose) {
	tout << "Original mset pcts:";
	print_mset_percentages(mset1);
	tout << "\n";
    }

    int percent = 100;
    for (Xapian::MSetIterator i = mset1.begin(); i != mset1.end(); ++i) {
	int new_percent = mset1.convert_to_percent(i);
	if (new_percent != percent) {
	    tout.str(string());
	    tout << "Testing " << percent << "% cutoff\n";
	    enquire.set_cutoff(percent);
	    Xapian::MSet mset2 = enquire.get_mset(0, 10);
	    TEST_EQUAL(mset2.back().get_percent(), percent);
	    TEST_EQUAL(mset2.size(), i.get_rank());
	    percent = new_percent;
	}
    }
}

// tests the cutoff option
DEFINE_TESTCASE(cutoff1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query(Xapian::Query::OP_OR,
			    "this", "line", "paragraph", "rubbish"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset weights:";
	print_mset_weights(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    double my_wt = -100;
    int changes = 0;
    Xapian::MSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
	double new_wt = i.get_weight();
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
}

// tests the allow query terms expand option
DEFINE_TESTCASE(allowqterms1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    string term = "paragraph";
    enquire.set_query(Xapian::Query(term));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(1000, myrset);
    Xapian::ESetIterator j = myeset.begin();
    for ( ; j != myeset.end(); ++j) {
	TEST_NOT_EQUAL(*j, term);
    }

    Xapian::ESet myeset2 = enquire.get_eset(1000, myrset, Xapian::Enquire::INCLUDE_QUERY_TERMS);
    j = myeset2.begin();
    for ( ; j != myeset2.end(); ++j) {
	if (*j == term) break;
    }
    TEST(j != myeset2.end());
}

// tests that the MSet max_attained works
DEFINE_TESTCASE(maxattain1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 100);

    double mymax = 0;
    Xapian::MSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
	if (i.get_weight() > mymax) mymax = i.get_weight();
    }
    TEST_EQUAL(mymax, mymset.get_max_attained());
}

// tests a reversed boolean query
DEFINE_TESTCASE(reversebool1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    Xapian::MSet mymset3 = enquire.get_mset(0, 100);

    // mymset1 and mymset2 should be identical
    TEST_EQUAL(mymset1.size(), mymset2.size());

    {
	Xapian::MSetIterator i = mymset1.begin();
	Xapian::MSetIterator j = mymset2.begin();
	for ( ; i != mymset1.end(); ++i, j++) {
	    TEST(j != mymset2.end());
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
	TEST(j == mymset2.end());
    }

    // mymset1 and mymset3 should be same but reversed
    TEST_EQUAL(mymset1.size(), mymset3.size());

    {
	Xapian::MSetIterator i = mymset1.begin();
	Xapian::MSetIterator j = mymset3.end();
	for ( ; i != mymset1.end(); ++i) {
	    --j;
	    // if this fails, then setting match_sort_forward=false didn't
	    // reverse the results.
	    TEST_EQUAL(*i, *j);
	}
    }
}

// tests a reversed boolean query, where the full mset isn't returned
DEFINE_TESTCASE(reversebool2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);

    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    Xapian::doccount msize = mymset1.size() / 2;
    Xapian::MSet mymset2 = enquire.get_mset(0, msize);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    Xapian::MSet mymset3 = enquire.get_mset(0, msize);

    // mymset2 should be first msize items of mymset1
    TEST_EQUAL(msize, mymset2.size());
    {
	Xapian::MSetIterator i = mymset1.begin();
	Xapian::MSetIterator j = mymset2.begin();
	for ( ; j != mymset2.end(); ++i, ++j) {
	    TEST(i != mymset1.end());
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
	// mymset1 should be larger.
	TEST(i != mymset1.end());
    }

    // mymset3 should be last msize items of mymset1, in reverse order
    TEST_EQUAL(msize, mymset3.size());
    {
	Xapian::MSetIterator i = mymset1.end();
	Xapian::MSetIterator j;
	for (j = mymset3.begin(); j != mymset3.end(); ++j) {
	    // if this fails, then setting match_sort_forward=false didn't
	    // reverse the results.
	    --i;
	    TEST_EQUAL(*i, *j);
	}
    }
}

// tests that get_matching_terms() returns the terms in the right order
DEFINE_TESTCASE(getmterms1, backend) {
    list<string> answers_list;
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
    list<string> list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
    TEST(list == answers_list);
}

// tests that get_matching_terms() returns the terms only once
DEFINE_TESTCASE(getmterms2, backend) {
    list<string> answers_list;
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
    list<string> list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
    TEST(list == answers_list);
}

// test that running a query twice returns the same results
DEFINE_TESTCASE(repeatquery1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    enquire.set_query(query(Xapian::Query::OP_OR, "this", "word"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset1, mymset2);
}

// test that prefetching documents works (at least, gives same results)
DEFINE_TESTCASE(fetchdocs1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

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
}

// test that searching for a term not in the database fails nicely
DEFINE_TESTCASE(absentterm1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(Xapian::Query("frink"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);
}

// as absentterm1, but setting query from a vector of terms
DEFINE_TESTCASE(absentterm2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    vector<string> terms;
    terms.push_back("frink");

    Xapian::Query query(Xapian::Query::OP_OR, terms.begin(), terms.end());
    enquire.set_query(query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);
}

// test that rsets do sensible things
DEFINE_TESTCASE(rset1, backend) {
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
}

// test that rsets do more sensible things
DEFINE_TESTCASE(rset2, backend) {
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
}

// test that rsets behave correctly with multiDBs
DEFINE_TESTCASE(rsetmultidb1, backend && !multi) {
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

    TEST(mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2));
    TEST(mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2));
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);
}

// regression tests - used to cause assertion in stats.h to fail
// Doesn't actually fail for multi but it doesn't make sense to run there.
DEFINE_TESTCASE(rsetmultidb3, backend && !multi) {
    Xapian::Enquire enquire(get_database("apitest_simpledata2"));
    enquire.set_query(query(Xapian::Query::OP_OR, "cuddly", "people"));
    Xapian::MSet mset = enquire.get_mset(0, 10); // used to fail assertion
}

/// Simple test of the elite set operator.
DEFINE_TESTCASE(eliteset1, backend && !multi) {
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery1 = query(Xapian::Query::OP_OR, "word");

    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, 1,
				   "simple", "word");

    enquire.set_query(myquery1, 2); // So the query lengths are the same.
    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);
}

/// Multi-backend variant of eliteset1.
DEFINE_TESTCASE(elitesetmulti1, multi) {
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, 1,
				   "simple", "word");

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    // For a sharded database, the elite set is resolved per shard and can
    // select different terms because the max term weights vary with the
    // per-shard term statistics.  I can't see a feasible way to create
    // an equivalent MSet to compare with so for now at least we hard-code
    // the expected values.
    TEST_EQUAL(mymset2.size(), 3);
    TEST_EQUAL(mymset2.get_matches_lower_bound(), 3);
    TEST_EQUAL(mymset2.get_matches_estimated(), 3);
    TEST_EQUAL(mymset2.get_matches_upper_bound(), 3);
    TEST_EQUAL_DOUBLE(mymset2.get_max_possible(), 1.1736756775723788948);
    TEST_EQUAL_DOUBLE(mymset2.get_max_attained(), 1.0464816871772451012);
    mset_expect_order(mymset2, 2, 4, 5);
    TEST_EQUAL_DOUBLE(mymset2[0].get_weight(), 1.0464816871772451012);
    TEST_EQUAL_DOUBLE(mymset2[1].get_weight(), 0.64098768659591376373);
    TEST_EQUAL_DOUBLE(mymset2[2].get_weight(), 0.46338869498075929698);
}

/// Test that the elite set operator works if the set contains
/// sub-expressions (regression test)
DEFINE_TESTCASE(eliteset2, backend && !multi) {
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery1 = query(Xapian::Query::OP_AND, "word", "search");

    vector<Xapian::Query> qs;
    qs.push_back(query("this"));
    qs.push_back(query(Xapian::Query::OP_AND, "word", "search"));
    Xapian::Query myquery2(Xapian::Query::OP_ELITE_SET,
			   qs.begin(), qs.end(), 1);

    enquire.set_query(myquery1);
    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);
}

/// Multi-backend variant of eliteset2.
DEFINE_TESTCASE(elitesetmulti2, multi) {
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);

    Xapian::Query myquery1 = query(Xapian::Query::OP_AND, "word", "search");

    vector<Xapian::Query> qs;
    qs.push_back(query("this"));
    qs.push_back(query(Xapian::Query::OP_AND, "word", "search"));
    Xapian::Query myquery2(Xapian::Query::OP_ELITE_SET,
			   qs.begin(), qs.end(), 1);

    enquire.set_query(myquery2);
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    // For a sharded database, the elite set is resolved per shard and can
    // select different terms because the max term weights vary with the
    // per-shard term statistics.  I can't see a feasible way to create
    // an equivalent MSet to compare with so for now at least we hard-code
    // the expected values.
    TEST_EQUAL(mymset2.size(), 4);
    TEST_EQUAL(mymset2.get_matches_lower_bound(), 4);
    TEST_EQUAL(mymset2.get_matches_estimated(), 4);
    TEST_EQUAL(mymset2.get_matches_upper_bound(), 4);
    TEST_EQUAL_DOUBLE(mymset2.get_max_possible(), 2.6585705165783908299);
    TEST_EQUAL_DOUBLE(mymset2.get_max_attained(), 1.9700834242150864206);
    mset_expect_order(mymset2, 2, 1, 3, 5);
    TEST_EQUAL_DOUBLE(mymset2[0].get_weight(), 1.9700834242150864206);
    TEST_EQUAL_DOUBLE(mymset2[1].get_weight(), 0.051103097360122341775);
    TEST_EQUAL_DOUBLE(mymset2[2].get_weight(), 0.043131803408968119595);
    TEST_EQUAL_DOUBLE(mymset2[3].get_weight(), 0.043131803408968119595);
}


/// Test that elite set doesn't affect query results if we have fewer
/// terms than the threshold
DEFINE_TESTCASE(eliteset3, backend) {
    Xapian::Database mydb1(get_database("apitest_simpledata"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Enquire enquire2(mydb2);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer("word");
    string term2 = stemmer("rubbish");
    string term3 = stemmer("banana");

    vector<string> terms;
    terms.push_back(term1);
    terms.push_back(term2);
    terms.push_back(term3);

    Xapian::Query myquery1(Xapian::Query::OP_OR, terms.begin(), terms.end());
    enquire1.set_query(myquery1);

    Xapian::Query myquery2(Xapian::Query::OP_ELITE_SET, terms.begin(), terms.end(), 3);
    enquire2.set_query(myquery2);

    // retrieve the results
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);

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
}

/// Test that elite set doesn't pick terms with 0 frequency
DEFINE_TESTCASE(eliteset4, backend && !multi) {
    Xapian::Database mydb1(get_database("apitest_simpledata"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery1 = query("rubbish");
    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, 1,
				   "word", "rubbish", "fibble");
    enquire1.set_query(myquery1);
    enquire2.set_query(myquery2);

    // retrieve the results
    Xapian::MSet mymset1 = enquire1.get_mset(0, 10);
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    TEST_NOT_EQUAL(mymset2.size(), 0);
    TEST_EQUAL(mymset1, mymset2);
}

/// Multi-backend variant of eliteset4.
DEFINE_TESTCASE(elitesetmulti4, multi) {
    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery2 = query(Xapian::Query::OP_ELITE_SET, 1,
				   "word", "rubbish", "fibble");
    enquire2.set_query(myquery2);

    // retrieve the results
    Xapian::MSet mymset2 = enquire2.get_mset(0, 10);

    // For a sharded database, the elite set is resolved per shard and can
    // select different terms because the max term weights vary with the
    // per-shard term statistics.  I can't see a feasible way to create
    // an equivalent MSet to compare with so for now at least we hard-code
    // the expected values.
    TEST_EQUAL(mymset2.size(), 3);
    TEST_EQUAL(mymset2.get_matches_lower_bound(), 3);
    TEST_EQUAL(mymset2.get_matches_estimated(), 3);
    TEST_EQUAL(mymset2.get_matches_upper_bound(), 3);
    TEST_EQUAL_DOUBLE(mymset2.get_max_possible(), 1.4848948390060121572);
    TEST_EQUAL_DOUBLE(mymset2.get_max_attained(), 1.4848948390060121572);
    mset_expect_order(mymset2, 3, 2, 4);
    TEST_EQUAL_DOUBLE(mymset2[0].get_weight(), 1.4848948390060121572);
    TEST_EQUAL_DOUBLE(mymset2[1].get_weight(), 1.0464816871772451012);
    TEST_EQUAL_DOUBLE(mymset2[2].get_weight(), 0.64098768659591376373);
}

/// Regression test for problem with excess precision.
DEFINE_TESTCASE(eliteset5, backend) {
    Xapian::Database mydb1(get_database("apitest_simpledata"));
    Xapian::Enquire enquire1(mydb1);

    vector<string> v;
    for (int i = 0; i != 3; ++i) {
	v.push_back("simpl");
	v.push_back("queri");

	v.push_back("rubbish");
	v.push_back("rubbish");
	v.push_back("rubbish");
	v.push_back("word");
	v.push_back("word");
	v.push_back("word");
    }

    for (Xapian::termcount n = 1; n != v.size(); ++n) {
	Xapian::Query myquery1 = Xapian::Query(Xapian::Query::OP_ELITE_SET,
					       v.begin(), v.end(), n);
	myquery1 = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
				 myquery1,
				 0.004);

	enquire1.set_query(myquery1);
	// On architectures with excess precision (or, at least, on x86), the
	// following call used to result in a segfault (at least when n=1).
	enquire1.get_mset(0, 10);
    }
}

/// Test that the termfreq returned by termlists is correct.
DEFINE_TESTCASE(termlisttermfreq1, backend) {
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
    string theterm = stemmer("another");

    double wt1 = 0;
    double wt2 = 0;
    {
	Xapian::ESetIterator i = eset1.begin();
	for ( ; i != eset1.end(); ++i) {
	    if (*i == theterm) {
		wt1 = i.get_weight();
		break;
	    }
	}
    }
    {
	Xapian::ESetIterator i = eset2.begin();
	for ( ; i != eset2.end(); ++i) {
	    if (*i == theterm) {
		wt2 = i.get_weight();
		break;
	    }
	}
    }

    TEST_NOT_EQUAL(wt1, 0);
    TEST_NOT_EQUAL(wt2, 0);
    TEST_EQUAL(wt1, wt2);
}

/// Test the termfreq and termweight info returned for query terms
DEFINE_TESTCASE(qterminfo1, backend) {
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer("word");
    string term2 = stemmer("inmemory");
    string term3 = stemmer("flibble");

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
    TEST_EQUAL(mymset1a.get_termfreq(term2),
	       mymset2a.get_termfreq(term2));
    TEST_EQUAL(mymset1a.get_termfreq(term3),
	       mymset2a.get_termfreq(term3));

    TEST_EQUAL(mymset1a.get_termfreq(term1), 3);
    TEST_EQUAL(mymset1a.get_termfreq(term2), 1);
    TEST_EQUAL(mymset1a.get_termfreq(term3), 0);

    TEST_NOT_EQUAL(mymset1a.get_termweight(term1), 0);
    TEST_NOT_EQUAL(mymset1a.get_termweight(term2), 0);
    // non-existent terms should have 0 weight.
    TEST_EQUAL(mymset1a.get_termweight(term3), 0);

    TEST_EQUAL(mymset1a.get_termfreq(stemmer("banana")), 1);
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   mymset1a.get_termweight(stemmer("banana")));

    TEST_EQUAL(mymset1a.get_termfreq("sponge"), 0);
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   mymset1a.get_termweight("sponge"));
}

/// Regression test for bug #37.
DEFINE_TESTCASE(qterminfo2, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);

    // make a query
    Xapian::Stem stemmer("english");

    string term1 = stemmer("paragraph");
    string term2 = stemmer("another");

    enquire.set_query(Xapian::Query(term1));
    Xapian::MSet mset0 = enquire.get_mset(0, 10);

    TEST_NOT_EQUAL(mset0.get_termweight("paragraph"), 0);

    Xapian::Query query(Xapian::Query::OP_AND_NOT, term1,
	    Xapian::Query(Xapian::Query::OP_AND, term1, term2));
    enquire.set_query(query);

    // retrieve the results
    // Note: get_mset() used to throw "AssertionError" in debug builds
    Xapian::MSet mset = enquire.get_mset(0, 10);

    TEST_NOT_EQUAL(mset.get_termweight("paragraph"), 0);
}

// tests that when specifying that no items are to be returned, those
// statistics which should be the same are.
DEFINE_TESTCASE(msetzeroitems1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(query("this"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 0);

    Xapian::MSet mymset2 = enquire.get_mset(0, 1);

    TEST_EQUAL(mymset1.get_max_possible(), mymset2.get_max_possible());
}

// test that the matches_* of a simple query are as expected
DEFINE_TESTCASE(matches1, backend) {
    bool remote = get_dbtype().find("remote") != string::npos;

    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query myquery;
    Xapian::MSet mymset;

    myquery = query("word");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "inmemory", "word");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 0);

    myquery = query(Xapian::Query::OP_AND, "simple", "word");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "simple", "word");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 0);
    if (db.size() == 1) {
	// This isn't true for sharded DBs since there one sub-database has 3
	// documents and simple and word both have termfreq of 2, so the
	// matcher can tell at least one document must match!)
	TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    }
    TEST_REL(mymset.get_matches_lower_bound(),<=,mymset.get_matches_estimated());
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),<=,mymset.get_uncollapsed_matches_estimated());
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    mymset = enquire.get_mset(0, 1);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    mymset = enquire.get_mset(0, 2);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    myquery = query(Xapian::Query::OP_AND, "paragraph", "another");
    enquire.set_query(myquery);
    mymset = enquire.get_mset(0, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);

    mymset = enquire.get_mset(0, 1);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 1);
    if (db.size() > 1 && remote) {
	// The matcher can tell there's only one match in this case.
	TEST_EQUAL(mymset.get_matches_estimated(), 1);
	TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 1);
	TEST_EQUAL(mymset.get_matches_upper_bound(), 1);
	TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 1);
    } else {
	TEST_EQUAL(mymset.get_matches_estimated(), 2);
	TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 2);
	TEST_EQUAL(mymset.get_matches_upper_bound(), 2);
	TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 2);
    }

    mymset = enquire.get_mset(0, 2);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 1);

    mymset = enquire.get_mset(1, 20);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 1);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 1);
}

// tests that wqf affects the document weights
DEFINE_TESTCASE(wqf1, backend) {
    // Both queries have length 2; in q1 word has wqf=2, in q2 word has wqf=1
    Xapian::Query q1("word", 2);
    Xapian::Query q2("word");
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(q1);
    Xapian::MSet mset1 = enquire.get_mset(0, 10);
    enquire.set_query(q2);
    Xapian::MSet mset2 = enquire.get_mset(0, 2);
    // Check the weights
    TEST(mset1.begin().get_weight() > mset2.begin().get_weight());
}

// tests that query length affects the document weights
DEFINE_TESTCASE(qlen1, backend) {
    Xapian::Query q1("word");
    Xapian::Query q2("word");
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(q1);
    Xapian::MSet mset1 = enquire.get_mset(0, 10);
    enquire.set_query(q2);
    Xapian::MSet mset2 = enquire.get_mset(0, 2);
    // Check the weights
    // TEST(mset1.begin().get_weight() < mset2.begin().get_weight());
    TEST(mset1.begin().get_weight() == mset2.begin().get_weight());
}

// tests that opening a non-existent termlist throws the correct exception
DEFINE_TESTCASE(termlist1, backend) {
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
}

// tests that a Xapian::TermIterator works as an STL iterator
DEFINE_TESTCASE(termlist2, backend) {
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator t = db.termlist_begin(1);
    Xapian::TermIterator tend = db.termlist_end(1);

    // test operator= creates a copy which compares equal
    Xapian::TermIterator t_copy = t;
    TEST_EQUAL(t, t_copy);

    // test copy constructor creates a copy which compares equal
    Xapian::TermIterator t_clone(t);
    TEST_EQUAL(t, t_clone);

    vector<string> v(t, tend);

    t = db.termlist_begin(1);
    tend = db.termlist_end(1);
    vector<string>::const_iterator i;
    for (i = v.begin(); i != v.end(); ++i) {
	TEST_NOT_EQUAL(t, tend);
	TEST_EQUAL(*i, *t);
	t++;
    }
    TEST_EQUAL(t, tend);
}

static Xapian::TermIterator
test_termlist3_helper()
{
    Xapian::Database db(get_database("apitest_onedoc"));
    return db.termlist_begin(1);
}

// tests that a Xapian::TermIterator still works when the DB is deleted
DEFINE_TESTCASE(termlist3, backend) {
    Xapian::TermIterator u = test_termlist3_helper();
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator t = db.termlist_begin(1);
    Xapian::TermIterator tend = db.termlist_end(1);

    while (t != tend) {
	TEST_EQUAL(*t, *u);
	t++;
	u++;
    }
}

// tests skip_to
DEFINE_TESTCASE(termlist4, backend) {
    Xapian::Database db(get_database("apitest_onedoc"));
    Xapian::TermIterator i = db.termlist_begin(1);
    i.skip_to("");
    i.skip_to("\xff");
}

// tests punctuation is OK in terms (particularly in remote queries)
DEFINE_TESTCASE(puncterms1, backend) {
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
}

// test that searching for a term with a space or backslash in it works
DEFINE_TESTCASE(spaceterms1, backend) {
    Xapian::Enquire enquire(get_database("apitest_space"));
    Xapian::MSet mymset;
    Xapian::doccount count;
    Xapian::MSetIterator m;
    Xapian::Stem stemmer("english");

    enquire.set_query(stemmer("space man"));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	TEST_NOT_EQUAL(mymset.begin().get_document().get_data(), "");
	TEST_NOT_EQUAL(mymset.begin().get_document().get_value(value_no), "");
    }

    enquire.set_query(stemmer("tab\tby"));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 0; value_no < 7; ++value_no) {
	string value = mymset.begin().get_document().get_value(value_no);
	TEST_NOT_EQUAL(value, "");
	if (value_no == 0) {
	    TEST(value.size() > 262);
	    TEST_EQUAL(static_cast<unsigned char>(value[262]), 255);
	}
    }

    enquire.set_query(stemmer("back\\slash"));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);
}

// test that XOR queries work
DEFINE_TESTCASE(xor1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Stem stemmer("english");

    vector<string> terms;
    terms.push_back(stemmer("this"));
    terms.push_back(stemmer("word"));
    terms.push_back(stemmer("of"));

    Xapian::Query query(Xapian::Query::OP_XOR, terms.begin(), terms.end());
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    //	Docid	this	word	of	Match?
    //	1	*			*
    //	2	*	*	*	*
    //	3	*		*
    //	4	*	*
    //	5	*			*
    //	6	*			*
    mset_expect_order(mymset, 1, 2, 5, 6);
}

/// Test that weighted XOR queries work (bug fixed in 1.2.1 and 1.0.21).
DEFINE_TESTCASE(xor2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Stem stemmer("english");

    vector<string> terms;
    terms.push_back(stemmer("this"));
    terms.push_back(stemmer("word"));
    terms.push_back(stemmer("of"));

    Xapian::Query query(Xapian::Query::OP_XOR, terms.begin(), terms.end());
    enquire.set_query(query);

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    //	Docid	LEN	this	word	of	Match?
    //	1	28	2			*
    //	2	81	5	8	1	*
    //	3	15	1		2
    //	4	31	1	1
    //	5	15	1			*
    //	6	15	1			*
    mset_expect_order(mymset, 2, 1, 5, 6);
}

// test Xapian::Database::get_document()
DEFINE_TESTCASE(getdoc1, backend) {
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
}

// test whether operators with no elements work as a null query
DEFINE_TESTCASE(emptyop1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    vector<Xapian::Query> nullvec;

    Xapian::Query query1(Xapian::Query::OP_XOR, nullvec.begin(), nullvec.end());

    enquire.set_query(query1);
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 0);
    // In Xapian < 1.3.0, this gave InvalidArgumentError (because
    // query1.empty()) but elsewhere we treat an empty query as just not
    // matching any documents, so we now do the same here too.
    TEST_EQUAL(enquire.get_matching_terms_begin(1),
	       enquire.get_matching_terms_end(1));
}

// Regression test for check_at_least SEGV when there are no matches.
DEFINE_TESTCASE(checkatleast1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("thom"));
    Xapian::MSet mymset = enquire.get_mset(0, 10, 11);
    TEST_EQUAL(0, mymset.size());
}

// Regression test - if check_at_least was set we returned (check_at_least - 1)
// results, rather than the requested msize.  Fixed in 1.0.2.
DEFINE_TESTCASE(checkatleast2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("paragraph"));

    Xapian::MSet mymset = enquire.get_mset(0, 3, 10);
    TEST_MSET_SIZE(mymset, 3);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 5);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 5);

    mymset = enquire.get_mset(0, 2, 4);
    TEST_MSET_SIZE(mymset, 2);
    TEST_REL(mymset.get_matches_lower_bound(),>=,4);
    TEST_REL(mymset.get_matches_lower_bound(),>=,4);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,4);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,4);
}

// Feature tests - check_at_least with various sorting options.
DEFINE_TESTCASE(checkatleast3, backend) {
    Xapian::Enquire enquire(get_database("etext"));
    enquire.set_query(Xapian::Query("prussian")); // 60 matches.

    for (int order = 0; order < 3; ++order) {
	switch (order) {
	    case 0:
		enquire.set_docid_order(Xapian::Enquire::ASCENDING);
		break;
	    case 1:
		enquire.set_docid_order(Xapian::Enquire::DESCENDING);
		break;
	    case 2:
		enquire.set_docid_order(Xapian::Enquire::DONT_CARE);
		break;
	}

	for (int sort = 0; sort < 7; ++sort) {
	    bool reverse = (sort & 1);
	    switch (sort) {
		case 0:
		    enquire.set_sort_by_relevance();
		    break;
		case 1: case 2:
		    enquire.set_sort_by_value(0, reverse);
		    break;
		case 3: case 4:
		    enquire.set_sort_by_value_then_relevance(0, reverse);
		    break;
		case 5: case 6:
		    enquire.set_sort_by_relevance_then_value(0, reverse);
		    break;
	    }

	    Xapian::MSet mset = enquire.get_mset(0, 100, 500);
	    TEST_MSET_SIZE(mset, 60);
	    TEST_EQUAL(mset.get_matches_lower_bound(), 60);
	    TEST_EQUAL(mset.get_matches_estimated(), 60);
	    TEST_EQUAL(mset.get_matches_upper_bound(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_lower_bound(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_estimated(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_upper_bound(), 60);

	    mset = enquire.get_mset(0, 50, 100);
	    TEST_MSET_SIZE(mset, 50);
	    TEST_EQUAL(mset.get_matches_lower_bound(), 60);
	    TEST_EQUAL(mset.get_matches_estimated(), 60);
	    TEST_EQUAL(mset.get_matches_upper_bound(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_lower_bound(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_estimated(), 60);
	    TEST_EQUAL(mset.get_uncollapsed_matches_upper_bound(), 60);

	    mset = enquire.get_mset(0, 10, 50);
	    TEST_MSET_SIZE(mset, 10);
	    TEST_REL(mset.get_matches_lower_bound(),>=,50);
	    TEST_REL(mset.get_uncollapsed_matches_lower_bound(),>=,50);
	}
    }
}

// tests all document postlists
DEFINE_TESTCASE(allpostlist1, backend) {
    Xapian::Database db(get_database("apitest_manydocs"));
    Xapian::PostingIterator i = db.postlist_begin("");
    unsigned int j = 1;
    while (i != db.postlist_end("")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
    }
    TEST_EQUAL(j, 513);

    i = db.postlist_begin("");
    j = 1;
    while (i != db.postlist_end("")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
	if (j == 50) {
	    j += 10;
	    i.skip_to(j);
	}
    }
    TEST_EQUAL(j, 513);
}

static void test_emptyterm1_helper(Xapian::Database & db)
{
    // Don't bother with postlist_begin() because allpostlist tests cover that.
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.positionlist_begin(1, ""));
    TEST_EQUAL(db.get_doccount(), db.get_termfreq(""));
    TEST_EQUAL(db.get_doccount() != 0, db.term_exists(""));
    TEST_EQUAL(db.get_doccount(), db.get_collection_freq(""));
}

// tests results of passing an empty term to various methods
DEFINE_TESTCASE(emptyterm1, backend) {
    Xapian::Database db(get_database("apitest_manydocs"));
    TEST_EQUAL(db.get_doccount(), 512);
    test_emptyterm1_helper(db);

    db = get_database("apitest_onedoc");
    TEST_EQUAL(db.get_doccount(), 1);
    test_emptyterm1_helper(db);

    db = get_database("");
    TEST_EQUAL(db.get_doccount(), 0);
    test_emptyterm1_helper(db);
}

// Test for alldocs postlist with a sparse database.
DEFINE_TESTCASE(alldocspl1, backend) {
    Xapian::Database db = get_database("alldocspl1",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   Xapian::Document doc;
					   doc.set_data("5");
					   doc.add_value(0, "5");
					   wdb.replace_document(5, doc);
				       });
    Xapian::PostingIterator i = db.postlist_begin("");
    TEST(i != db.postlist_end(""));
    TEST_EQUAL(*i, 5);
    TEST_EQUAL(i.get_doclength(), 0);
    TEST_EQUAL(i.get_unique_terms(), 0);
    TEST_EQUAL(i.get_wdf(), 1);
    ++i;
    TEST(i == db.postlist_end(""));
}

// Test reading and writing a modified alldocspostlist.
DEFINE_TESTCASE(alldocspl2, writable) {
    Xapian::PostingIterator i, end;
    {
	Xapian::WritableDatabase db = get_writable_database();
	Xapian::Document doc;
	doc.set_data("5");
	doc.add_value(0, "5");
	db.replace_document(5, doc);

	// Test iterating before committing the changes.
	i = db.postlist_begin("");
	end = db.postlist_end("");
	TEST(i != end);
	TEST_EQUAL(*i, 5);
	TEST_EQUAL(i.get_doclength(), 0);
	TEST_EQUAL(i.get_unique_terms(), 0);
	TEST_EQUAL(i.get_wdf(), 1);
	++i;
	TEST(i == end);

	db.commit();

	// Test iterating after committing the changes.
	i = db.postlist_begin("");
	end = db.postlist_end("");
	TEST(i != end);
	TEST_EQUAL(*i, 5);
	TEST_EQUAL(i.get_doclength(), 0);
	TEST_EQUAL(i.get_unique_terms(), 0);
	TEST_EQUAL(i.get_wdf(), 1);
	++i;
	TEST(i == end);

	// Add another document.
	doc = Xapian::Document();
	doc.set_data("5");
	doc.add_value(0, "7");
	db.replace_document(7, doc);

	// Test iterating through before committing the changes.
	i = db.postlist_begin("");
	end = db.postlist_end("");
	TEST(i != end);
	TEST_EQUAL(*i, 5);
	TEST_EQUAL(i.get_doclength(), 0);
	TEST_EQUAL(i.get_unique_terms(), 0);
	TEST_EQUAL(i.get_wdf(), 1);
	++i;
	TEST(i != end);
	TEST_EQUAL(*i, 7);
	TEST_EQUAL(i.get_doclength(), 0);
	TEST_EQUAL(i.get_unique_terms(), 0);
	TEST_EQUAL(i.get_wdf(), 1);
	++i;
	TEST(i == end);

	// Delete the first document.
	db.delete_document(5);

	// Test iterating through before committing the changes.
	i = db.postlist_begin("");
	end = db.postlist_end("");
	TEST(i != end);
	TEST_EQUAL(*i, 7);
	TEST_EQUAL(i.get_doclength(), 0);
	TEST_EQUAL(i.get_unique_terms(), 0);
	TEST_EQUAL(i.get_wdf(), 1);
	++i;
	TEST(i == end);

	// Test iterating through after committing the changes, and dropping the
	// reference to the main DB.
	db.commit();
	i = db.postlist_begin("");
	end = db.postlist_end("");
    }

    TEST(i != end);
    TEST_EQUAL(*i, 7);
    TEST_EQUAL(i.get_doclength(), 0);
    TEST_EQUAL(i.get_unique_terms(), 0);
    TEST_EQUAL(i.get_wdf(), 1);
    ++i;
    TEST(i == end);
}

// Feature test for Query::OP_SCALE_WEIGHT.
DEFINE_TESTCASE(scaleweight1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::QueryParser qp;

    static const char * const queries[] = {
	"pad",
	"milk fridge",
	"leave milk on fridge",
	"ordered milk operator",
	"ordered phrase operator",
	"leave \"milk on fridge\"",
	"notpresent",
	"leave \"milk notpresent\"",
    };
    static const double multipliers[] = {
	-1000000, -2.5, -1, -0.5, 0, 0.5, 1, 2.5, 1000000,
	0, 0
    };

    for (auto qstr : queries) {
	tout.str(string());
	Xapian::Query query1 = qp.parse_query(qstr);
	tout << "query1: " << query1.get_description() << '\n';
	for (const double *multp = multipliers; multp[0] != multp[1]; ++multp) {
	    double mult = *multp;
	    if (mult < 0) {
		TEST_EXCEPTION(Xapian::InvalidArgumentError,
			       Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
					     query1, mult));
		continue;
	    }
	    Xapian::Query query2(Xapian::Query::OP_SCALE_WEIGHT, query1, mult);
	    tout << "query2: " << query2.get_description() << '\n';

	    enq.set_query(query1);
	    Xapian::MSet mset1 = enq.get_mset(0, 20);
	    enq.set_query(query2);
	    Xapian::MSet mset2 = enq.get_mset(0, 20);

	    TEST_EQUAL(mset1.size(), mset2.size());

	    Xapian::MSetIterator i1, i2;
	    if (mult > 0) {
		for (i1 = mset1.begin(), i2 = mset2.begin();
		     i1 != mset1.end() && i2 != mset2.end(); ++i1, ++i2) {
		    TEST_EQUAL_DOUBLE(i1.get_weight() * mult, i2.get_weight());
		    TEST_EQUAL(*i1, *i2);
		}
	    } else {
		// Weights in mset2 are 0; so it should be sorted by docid.
		vector<Xapian::docid> ids1;
		vector<Xapian::docid> ids2;
		for (i1 = mset1.begin(), i2 = mset2.begin();
		     i1 != mset1.end() && i2 != mset2.end(); ++i1, ++i2) {
		    TEST_NOT_EQUAL_DOUBLE(i1.get_weight(), 0);
		    TEST_EQUAL_DOUBLE(i2.get_weight(), 0);
		    ids1.push_back(*i1);
		    ids2.push_back(*i2);
		}
		sort(ids1.begin(), ids1.end());
		TEST_EQUAL(ids1, ids2);
	    }
	}
    }
}

// Test Query::OP_SCALE_WEIGHT being used to multiply some of the weights of a
// search by zero.
DEFINE_TESTCASE(scaleweight2, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::MSetIterator i;

    Xapian::Query query1("fridg");
    Xapian::Query query2(Xapian::Query::OP_SCALE_WEIGHT, query1, 2.5);
    Xapian::Query query3("milk");
    Xapian::Query query4(Xapian::Query::OP_SCALE_WEIGHT, query3, 0);
    Xapian::Query query5(Xapian::Query::OP_OR, query2, query4);

    // query5 should first return the same results as query1, in the same
    // order, and then return the results of query3 which aren't also results
    // of query1, in ascending docid order.  We test that this happens.

    // First, build a vector of docids matching the first part of the query,
    // and append the non-duplicate docids matching the second part of the
    // query.
    vector<Xapian::docid> ids1;
    set<Xapian::docid> idsin1;
    vector<Xapian::docid> ids3;

    enq.set_query(query1);
    Xapian::MSet mset1 = enq.get_mset(0, 20);
    enq.set_query(query3);
    Xapian::MSet mset3 = enq.get_mset(0, 20);
    TEST_NOT_EQUAL(mset1.size(), 0);
    for (i = mset1.begin(); i != mset1.end(); ++i) {
	ids1.push_back(*i);
	idsin1.insert(*i);
    }
    TEST_NOT_EQUAL(mset3.size(), 0);
    for (i = mset3.begin(); i != mset3.end(); ++i) {
	if (idsin1.find(*i) != idsin1.end())
	    continue;
	ids3.push_back(*i);
    }
    sort(ids3.begin(), ids3.end());
    ids1.insert(ids1.end(), ids3.begin(), ids3.end());

    // Now, run the combined query and build a vector of the matching docids.
    vector<Xapian::docid> ids5;
    enq.set_query(query5);
    Xapian::MSet mset5 = enq.get_mset(0, 20);
    for (i = mset5.begin(); i != mset5.end(); ++i) {
	ids5.push_back(*i);
    }

    TEST_EQUAL(ids1, ids5);
}

// Regression test for bug fixed in 1.0.5 - this test would failed under
// valgrind because it used an uninitialised value.
DEFINE_TESTCASE(bm25weight1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::BM25Weight(1, 25, 1, 0.01, 0.5));
    enquire.set_query(Xapian::Query("word"));

    Xapian::MSet mset = enquire.get_mset(0, 25);
}

// Feature test for TradWeight.
DEFINE_TESTCASE(tradweight1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::TradWeight());
    enquire.set_query(Xapian::Query("word"));

    Xapian::MSet mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 2);

    enquire.set_weighting_scheme(Xapian::TradWeight(0));
    enquire.set_query(Xapian::Query("this"));

    mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 6);

    // Check that TradWeight(0) means wdf and doc length really don't affect
    // the weights as stated in the documentation.
    TEST_EQUAL(mset[0].get_weight(), mset[5].get_weight());
}

// Test TradWeight when weighting documents using an RSet.
// Simply changed the weighting scheme used by rset2 testcase.
DEFINE_TESTCASE(tradweight4, backend) {
    Xapian::Database mydb(get_database("apitest_rset"));
    Xapian::Enquire enquire(mydb);
    Xapian::Query myquery = query(Xapian::Query::OP_OR, "cuddly", "people");

    enquire.set_query(myquery);
    enquire.set_weighting_scheme(Xapian::TradWeight());

    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    Xapian::RSet myrset;
    myrset.add_document(2);

    Xapian::MSet mymset2 = enquire.get_mset(0, 10, &myrset);

    mset_expect_order(mymset1, 1, 2);
    // Document 2 should have higher weight than document 1 despite the wdf of
    // "people" being 1 because "people" indexes a document in the RSet whereas
    // "cuddly" (wdf=2) does not.
    mset_expect_order(mymset2, 2, 1);
}

// Feature test for Database::get_uuid().
DEFINE_TESTCASE(uuid1, backend && !multi) {
    SKIP_TEST_FOR_BACKEND("inmemory");
    Xapian::Database db = get_database("apitest_simpledata");
    string uuid1 = db.get_uuid();
    TEST_EQUAL(uuid1.size(), 36);

    // A database with no sub-databases has an empty UUID.
    Xapian::Database db2;
    TEST(db2.get_uuid().empty());

    db2.add_database(db);
    TEST_EQUAL(uuid1, db2.get_uuid());

    // Multi-database has multiple UUIDs (we don't define the format exactly
    // so this assumes something about the implementation).
    db2.add_database(db);
    TEST_EQUAL(uuid1 + ":" + uuid1, db2.get_uuid());

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
    // This relies on InMemory databases not supporting uuids.
    // A multi-database containing a database with no uuid has no uuid.
    db2.add_database(Xapian::Database(string(), Xapian::DB_BACKEND_INMEMORY));
    TEST(db2.get_uuid().empty());
#endif
}
