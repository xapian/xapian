/* api_db.cc: tests which need a backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
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

#include "config.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

using std::vector;
using std::string;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"
#include "api_db.h"

#include <list>
typedef std::list<om_termname> om_termname_list;

// #######################################################################
// # Tests start here

// tests that the backend doesn't return zero docids
static bool test_zerodocid1()
{
    // open the database (in this case a simple text file
    // we prepared earlier)

    OmDatabase mydb(get_database("apitest_onedoc"));

    OmEnquire enquire(make_dbgrp(&mydb));

    // make a simple query, with one word in it - "word".
    OmQuery myquery("word");
    enquire.set_query(myquery);

    // retrieve the top ten results (we only expect one)
    OmMSet mymset = enquire.get_mset(0, 10);

    // We've done the query, now check that the result is what
    // we expect (1 document, with non-zero docid)
    TEST_MSET_SIZE(mymset, 1);

    TEST_AND_EXPLAIN(*(mymset.begin()) != 0,
		     "A query on a database returned a zero docid");

    return true;
}

OmDatabase get_simple_database()
{
    return OmDatabase(get_database("apitest_simpledata"));
}

void init_simple_enquire(OmEnquire &enq, const OmQuery &query = OmQuery("thi"))
{
    enq.set_query(query);
}

OmQuery
query(OmQuery::op op, om_termname t1 = "", om_termname t2 = "",
      om_termname t3 = "", om_termname t4 = "", om_termname t5 = "",
      om_termname t6 = "", om_termname t7 = "", om_termname t8 = "",
      om_termname t9 = "", om_termname t10 = "")
{
    vector<om_termname> v;
    OmStem stemmer("english");    
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
    return OmQuery(op, v.begin(), v.end());
}

OmQuery
query(om_termname t)
{
    return OmQuery(OmStem("english").stem_word(t));
}

OmMSet do_get_simple_query_mset(OmQuery query, int maxitems = 10, int first = 0)
{
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire, query);

    // retrieve the top results
    return enquire.get_mset(first, maxitems);
}

// tests the document count for a simple query
static bool test_simplequery1()
{
    OmMSet mymset = do_get_simple_query_mset(OmQuery("word"));
    TEST_MSET_SIZE(mymset, 2);
    return true;
}

// tests for the right documents and weights returned with simple query
static bool test_simplequery2()
{
    OmMSet mymset = do_get_simple_query_mset(OmQuery("word"));

    // We've done the query, now check that the result is what
    // we expect (documents 2 and 4)
    mset_expect_order(mymset, 2, 4);

    // Check the weights
    //these weights are for C=.5 in bm25weight
    OmMSetIterator i = mymset.begin();
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
    // The search is for "thi" rather than "this" because
    // the index will have stemmed versions of the terms.
    OmMSet mymset = do_get_simple_query_mset(query("this"));

    // Check that 6 documents were returned.
    TEST_MSET_SIZE(mymset, 6);

    return true;
}

// tests punctuation is OK in terms in remote queries
static bool test_puncterms1()
{
    OmDatabase db(get_database("apitest_punc"));
    OmEnquire enquire(db);

    OmQuery q1("semi;colon");
    enquire.set_query(q1);
    OmMSet m1 = enquire.get_mset(0, 10);

    OmQuery q2("col:on");
    enquire.set_query(q2);
    OmMSet m2 = enquire.get_mset(0, 10);

    OmQuery q3("com,ma");
    enquire.set_query(q3);
    OmMSet m3 = enquire.get_mset(0, 10);

    return true;
}


// tests a query accross multiple databases
static bool test_multidb1()
{
    OmDatabase mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make a simple query, with one word in it - "word".
    OmQuery myquery("word");
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    OmMSet mymset1 = enquire1.get_mset(0, 10);
    OmMSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1.size(), mymset2.size());
    TEST(mset_range_is_same_weights(mymset1, 0, mymset2, 0, mymset1.size()));
    return true;
}

// tests a query accross multiple databases with terms only
// in one of the two databases
static bool test_multidb2()
{
    OmDatabase mydb1(get_database("apitest_simpledata",
				  "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make a simple query
    OmQuery myquery = query(OmQuery::OP_OR, "inmemory", "word");
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    OmMSet mymset1 = enquire1.get_mset(0, 10);
    OmMSet mymset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mymset1.size(), mymset2.size());
    TEST(mset_range_is_same_weights(mymset1, 0, mymset2, 0, mymset1.size()));
    return true;
}

// test that a multidb with 2 dbs query returns correct docids
static bool test_multidb3()
{
    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire(make_dbgrp(&mydb2, &mydb3));

    // make a query
    OmQuery myquery = query(OmQuery::OP_OR, "inmemory", "word");
    OmSettings mopts;
    mopts.set("match_weighting_scheme", "bool");
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10, 0, &mopts);
    mset_expect_order(mymset, 2, 3, 7);

    return true;
}

// test that a multidb with 3 dbs query returns correct docids
static bool test_multidb4()
{
    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmDatabase mydb4(get_database("apitest_termorder"));
    OmEnquire enquire(make_dbgrp(&mydb2, &mydb3, &mydb4));

    // make a query
    OmQuery myquery = query(OmQuery::OP_OR, "inmemory", "word");
    OmSettings mopts;
    mopts.set("match_weighting_scheme", "bool");
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10, 0, &mopts);
    mset_expect_order(mymset, 2, 3, 4, 10);

    return true;
}

// tests MultiPostList::skip_to().
static bool test_multidb5()
{
    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire(make_dbgrp(&mydb2, &mydb3));

    // make a query
    OmQuery myquery = query(OmQuery::OP_AND, "inmemory", "word");
    OmSettings mopts;
    mopts.set("match_weighting_scheme", "bool");
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10, 0, &mopts);
    mset_expect_order(mymset, 2);

    return true;
}

class MyErrorHandler : public OmErrorHandler {
    public:
	int count;

	bool handle_error(OmError & error) {
	    count += 1;
	    tout << "Error handling caught: " << error.get_type() << ": " <<
		    error.get_msg() << ", with context `" <<
		    error.get_context() << "': count is now " << count << "\n";
	    return true;
	};

	MyErrorHandler() : count (0) {}
};

// tests error handler in multimatch().
static bool test_multierrhandler1()
{
    MyErrorHandler myhandler;

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    int errcount = 1;
    for (int testcount = 0; testcount < 14; testcount ++) {
	tout << "testcount=" << testcount << "\n";
	OmDatabase mydb4(get_database("-e", "apitest_termorder"));
	OmDatabase mydb5(get_network_database("apitest_termorder", 1));
	OmDatabase mydb6(get_database("-e2", "apitest_termorder"));
	OmDatabase mydb7(get_database("-e3", "apitest_simpledata"));

	OmDatabase dbgrp;
	switch (testcount) {
	    case 0:
		dbgrp = make_dbgrp(&mydb2, &mydb3, &mydb4);
		break;
	    case 1:
		dbgrp = make_dbgrp(&mydb4, &mydb2, &mydb3);
		break;
	    case 2:
		dbgrp = make_dbgrp(&mydb3, &mydb4, &mydb2);
		break;
	    case 3:
		dbgrp = make_dbgrp(&mydb2, &mydb3, &mydb5);
		sleep(1);
		break;
	    case 4:
		dbgrp = make_dbgrp(&mydb5, &mydb2, &mydb3);
		sleep(1);
		break;
	    case 5:
		dbgrp = make_dbgrp(&mydb3, &mydb5, &mydb2);
		sleep(1);
		break;
	    case 6:
		dbgrp = make_dbgrp(&mydb2, &mydb3, &mydb6);
		break;
	    case 7:
		dbgrp = make_dbgrp(&mydb6, &mydb2, &mydb3);
		break;
	    case 8:
		dbgrp = make_dbgrp(&mydb3, &mydb6, &mydb2);
		break;
	    case 9:
		dbgrp = make_dbgrp(&mydb2, &mydb3, &mydb7);
		break;
	    case 10:
		dbgrp = make_dbgrp(&mydb7, &mydb2, &mydb3);
		break;
	    case 11:
		dbgrp = make_dbgrp(&mydb3, &mydb7, &mydb2);
		break;
	    case 12:
		dbgrp = make_dbgrp(&mydb2, &mydb6, &mydb7);
		break;
	    case 13:
		dbgrp = make_dbgrp(&mydb2, &mydb7, &mydb6);
		break;
	}
	tout << "db=" << dbgrp << "\n";
	OmEnquire enquire(dbgrp, &myhandler);

	// make a query
	OmQuery myquery = query(OmQuery::OP_OR, "inmemory", "word");
	OmSettings mopts;
	mopts.set("match_weighting_scheme", "bool");
	enquire.set_query(myquery);

	tout << "query=" << myquery << "\n";
	// retrieve the top ten results
	OmMSet mymset = enquire.get_mset(0, 10, 0, &mopts);

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

// tests that changing a query object after calling set_query()
// doesn't make any difference to get_mset().
static bool test_changequery1()
{
    // The search is for "thi" rather than "this" because
    // the index will have stemmed versions of the terms.
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire(get_simple_database());

    OmQuery myquery("thi");
    // make a simple query
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mset1 = enquire.get_mset(0, 10);

    myquery = OmQuery("foo");
    OmMSet mset2 = enquire.get_mset(0, 10);

    // verify that both msets are identical
    TEST_EQUAL(mset1, mset2);
    return true;
}

// tests that an empty query returns no matches
static bool test_emptyquery1()
{
    OmMSet mymset = do_get_simple_query_mset(OmQuery());
    TEST_MSET_SIZE(mymset, 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    std::vector<OmQuery> v;
    mymset = do_get_simple_query_mset(OmQuery(OmQuery::OP_AND,
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
    OmMSet mymset = do_get_simple_query_mset(query("this"), 1);
    TEST_MSET_SIZE(mymset, 1);
    return true;
}

// tests that when specifiying maxitems to get_eset, no more than
// that are returned.
static bool test_expandmaxitems1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    OmRSet myrset;
    OmMSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    OmESet myeset = enquire.get_eset(1, myrset);
    TEST_EQUAL(myeset.size(), 1);

    return true;
}

// tests that a pure boolean query has all weights set to 1
static bool test_boolquery1()
{
    OmQuery myboolquery(query("this"));

    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire, myboolquery);

    OmSettings mopts;
    mopts.set("match_weighting_scheme", "bool");

    // retrieve the top results
    OmMSet mymset = enquire.get_mset(0, 10, 0, &mopts);

    TEST_NOT_EQUAL(mymset.size(), 0);
    TEST_EQUAL(mymset.get_max_possible(), 0);
    for (OmMSetIterator i = mymset.begin(); i != mymset.end(); ++i) {
	TEST_EQUAL(i.get_weight(), 0);
    }
    return true;
}

// tests that get_mset() specifying "this" works as expected
static bool test_msetfirst1()
{
    OmMSet mymset1 = do_get_simple_query_mset(query("this"), 6, 0);
    OmMSet mymset2 = do_get_simple_query_mset(query("this"), 3, 3);
    TEST(mset_range_is_same(mymset1, 3, mymset2, 0, 3));
    return true;
}

// tests the converting-to-percent functions
static bool test_topercent1()
{
    OmMSet mymset = do_get_simple_query_mset(query("this"), 20, 0);

    int last_pct = 100;
    OmMSetIterator i = mymset.begin();
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

class myExpandFunctor : public OmExpandDecider {
    public:
	int operator()(const om_termname & tname) const {
	    unsigned long sum = 0;
	    for (om_termname::const_iterator i=tname.begin(); i!=tname.end(); ++i) {
		sum += *i;
	    }
//	    if (verbose) {
//		cout << tname << "==> " << sum << "\n";
//	    }
	    return (sum % 2) == 0;
	}
};

// tests the expand decision functor
static bool test_expandfunctor1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    OmRSet myrset;
    OmMSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    myExpandFunctor myfunctor;

    OmESet myeset_orig = enquire.get_eset(1000, myrset);
    unsigned int neweset_size = 0;
    OmESetIterator j = myeset_orig.begin();
    for ( ; j != myeset_orig.end(); ++j) {
        if (myfunctor(*j)) neweset_size++;
    }
    OmESet myeset = enquire.get_eset(neweset_size, myrset, 0, &myfunctor);

#if 0
    // Compare myeset with the hand-filtered version of myeset_orig.
    if (verbose) {
	tout << "orig_eset: ";
	copy(myeset_orig.begin(), myeset_orig.end(),
	     ostream_iterator<OmESetItem>(tout, " "));
	tout << "\n";

	tout << "new_eset: ";
	copy(myeset.begin(), myeset.end(),
	     ostream_iterator<OmESetItem>(tout, " "));
	tout << "\n";
    }
#endif
    OmESetIterator orig = myeset_orig.begin();
    OmESetIterator filt = myeset.begin();
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

class myMatchDecider : public OmMatchDecider {
    public:
        int operator()(const OmDocument &doc) const {
	    // Note that this is not recommended usage of get_data()
	    return doc.get_data().value.find("This is") != std::string::npos;
	}
};

// tests the match decision functor
static bool test_matchfunctor1()
{
    // FIXME: check that the functor works both ways.
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    myMatchDecider myfunctor;

    OmMSet mymset = enquire.get_mset(0, 100, 0, 0, &myfunctor);

    OmMSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    TEST_EQUAL(mymset.size(), 3);
    for ( ; i != mymset.end(); ++i) {
	const OmDocument doc(i.get_document());
        TEST(myfunctor(doc));
    }

    return true;
}

// tests that mset iterators on msets compare correctly.
static bool test_msetiterator1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    OmMSet mymset = enquire.get_mset(0, 2);

    OmMSetIterator j;
    j = mymset.begin();
    OmMSetIterator k = mymset.end();
    OmMSetIterator l(j);
    OmMSetIterator m(k);
    OmMSetIterator n = mymset.begin();
    OmMSetIterator o = mymset.begin();
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
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    OmMSet mymset = enquire.get_mset(0, 0);

    OmMSetIterator j = mymset.begin();
    OmMSetIterator k = mymset.end();
    OmMSetIterator l(j);
    OmMSetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests that eset iterators on empty esets compare equal.
static bool test_esetiterator1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    OmRSet myrset;
    OmMSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    OmSettings eopt;
    eopt.set("expand_use_query_terms", false);

    OmESet myeset = enquire.get_eset(2, myrset, &eopt);
    OmESetIterator j;
    j = myeset.begin();
    OmESetIterator k = myeset.end();
    OmESetIterator l(j);
    OmESetIterator m(k);
    OmESetIterator n = myeset.begin();

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
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    OmRSet myrset;
    OmMSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    OmSettings eopt;
    eopt.set("expand_use_query_terms", false);

    OmESet myeset = enquire.get_eset(0, myrset, &eopt);
    OmESetIterator j = myeset.begin();
    OmESetIterator k = myeset.end();
    OmESetIterator l(j);
    OmESetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

static void
print_mset_weights(const OmMSet &mset)
{
    OmMSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
        tout << " " << i.get_weight();
    }
}

// tests the cutoff option
static bool test_cutoff1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire, query(OmQuery::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    OmMSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset weights:";
	print_mset_weights(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    om_weight my_wt = -100;
    int changes = 0;
    OmMSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
        om_weight new_wt = i.get_weight();
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

    OmSettings mymopt;
    mymopt.set("match_cutoff", my_wt);
    OmMSet mymset2 = enquire.get_mset(0, 100, NULL, &mymopt);

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
    OmEnquire enquire(get_simple_database());
    OmQuery q = query(OmQuery::OP_OR, "this", "line", "paragraph", "rubbish");
    init_simple_enquire(enquire, query(OmQuery::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    OmMSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset weights:";
	print_mset_weights(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    om_weight my_wt = -100;
    int changes = 0;
    OmMSetIterator i = mymset1.begin();
    int c = 0;
    for ( ; i != mymset1.end(); ++i, ++c) {
        om_weight new_wt = i.get_weight();
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

    OmQuery cutoffq(OmQuery::OP_WEIGHT_CUTOFF, q);
    cutoffq.set_cutoff(my_wt);
    enquire.set_query(cutoffq);
    OmMSet mymset2 = enquire.get_mset(0, 100);

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
print_mset_percentages(const OmMSet &mset)
{
    OmMSetIterator i = mset.begin();
    for ( ; i != mset.end(); ++i) {
        tout << " " << mset.convert_to_percent(i);
    }
}

// tests the percent cutoff option
static bool test_pctcutoff1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire, query(OmQuery::OP_OR,
				       "this", "line", "paragraph", "rubbish"));
    OmMSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
	tout << "Original mset pcts:";
	print_mset_percentages(mymset1);
	tout << "\n";
    }

    unsigned int num_items = 0;
    int my_pct = 100;
    int changes = 0;
    OmMSetIterator i = mymset1.begin();
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

    OmSettings mymopt;
    mymopt.set("match_percent_cutoff", my_pct);
    OmMSet mymset2 = enquire.get_mset(0, 100, NULL, &mymopt);

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
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    OmRSet myrset;
    OmMSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    OmSettings eopt;
    eopt.set("expand_use_query_terms", false);

    OmESet myeset = enquire.get_eset(1000, myrset, &eopt);
    OmESetIterator j = myeset.begin();
    for ( ; j != myeset.end(); ++j) {
        TEST_NOT_EQUAL(*j, "thi");
    }

    return true;
}

// tests that the MSet max_attained works
static bool test_maxattain1()
{
    OmMSet mymset = do_get_simple_query_mset(query("this"), 100, 0);

    om_weight mymax = 0;
    OmMSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
        if (i.get_weight() > mymax) mymax = i.get_weight();
    }
    TEST_EQUAL(mymax, mymset.get_max_attained());

    return true;
}

// tests the collapse-on-key
static bool test_collapsekey1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmSettings mymopt;

    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    om_doccount mymsize1 = mymset1.size();

    for (int key_no = 1; key_no < 7; ++key_no) {
	mymopt.set("match_collapse_key", key_no);
	OmMSet mymset = enquire.get_mset(0, 100, 0, &mymopt);

	TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
			 "Had no fewer items when performing collapse: don't know whether it worked.");

	std::map<string, om_docid> keys;
	OmMSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    OmKey key = i.get_document().get_key(key_no);
	    TEST(keys[key.value] == 0 || key.value == "");
	    keys[key.value] = *i;
	}
    }

    return true;
}

// tests the collapse-on-key for DA databases
static bool test_collapsekey2()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmSettings mymopt;

    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    om_doccount mymsize1 = mymset1.size();

    const int key_no = 0;
    mymopt.set("match_collapse_key", key_no);
    OmMSet mymset = enquire.get_mset(0, 100, 0, &mymopt);

    TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
		     "Had no fewer items when performing collapse: don't know whether it worked.");

    std::map<string, om_docid> keys;
    OmMSetIterator i = mymset.begin();
    for ( ; i != mymset.end(); ++i) {
	OmKey key = i.get_document().get_key(key_no);
	TEST(keys[key.value] == 0 || key.value == "");
	keys[key.value] = *i;
    }

    return true;
}

// tests a reversed boolean query
static bool test_reversebool1()
{
    OmEnquire enquire(get_simple_database());
    OmQuery query("thi");
    init_simple_enquire(enquire, query);

    OmSettings mymopt;
    mymopt.set("match_weighting_scheme", "bool");
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    mymopt.set("match_sort_forward", true);
    OmMSet mymset2 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set("match_sort_forward", false);
    OmMSet mymset3 = enquire.get_mset(0, 100, 0, &mymopt);

    // mymset1 and mymset2 should be identical
    TEST_EQUAL(mymset1.size(), mymset2.size());

    {
	OmMSetIterator i = mymset1.begin();
	OmMSetIterator j = mymset2.begin();
	for ( ; i != mymset1.end(), j != mymset2.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
    }

    // mymset1 and mymset3 should be same but reversed
    TEST_EQUAL(mymset1.size(), mymset3.size());

    {
	OmMSetIterator i = mymset1.begin();
	std::vector<om_docid> rev(mymset3.begin(), mymset3.end());
	// Next iterator not const because of compiler brokenness (egcs 1.1.2)
	std::vector<om_docid>::reverse_iterator j = rev.rbegin();
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
    OmEnquire enquire(get_simple_database());
    OmQuery query("thi");
    init_simple_enquire(enquire, query);

    OmSettings mymopt;
    mymopt.set("match_weighting_scheme", "bool");
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);

    TEST_AND_EXPLAIN(mymset1.size() > 1,
		     "Mset was too small to test properly");

    mymopt.set("match_sort_forward", true);
    om_doccount msize = mymset1.size() / 2;
    OmMSet mymset2 = enquire.get_mset(0, msize, 0, &mymopt);
    mymopt.set("match_sort_forward", false);
    OmMSet mymset3 = enquire.get_mset(0, msize, 0, &mymopt);

    // mymset2 should be first msize items of mymset1
    TEST_EQUAL(msize, mymset2.size());

    {
	OmMSetIterator i = mymset1.begin();
	OmMSetIterator j = mymset2.begin();
	for ( ; i != mymset1.end(), j != mymset2.end(); ++i, j++) {
	    // if this fails, then setting match_sort_forward=true was not
	    // the same as the default.
	    TEST_EQUAL(*i, *j);
	}
    }

    // mymset3 should be last msize items of mymset1, in reverse order
    TEST_EQUAL(msize, mymset3.size());
    {
	std::vector<om_docid> rev(mymset1.begin(), mymset1.end());
	// Next iterator not const because of compiler brokenness (egcs 1.1.2)
	std::vector<om_docid>::reverse_iterator i = rev.rbegin();
	OmMSetIterator j = mymset3.begin();
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

    OmDatabase mydb(get_database("apitest_termorder"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery(OmQuery::OP_OR,
	    OmQuery(OmQuery::OP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OmQuery::OP_OR,
		    OmQuery("four", 1, 4),
		    OmQuery("two", 1, 2)));

    enquire.set_query(myquery);

    OmMSet mymset = enquire.get_mset(0, 10);

    TEST_MSET_SIZE(mymset, 1);
    om_termname_list list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
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

    OmDatabase mydb(get_database("apitest_termorder"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery(OmQuery::OP_OR,
	    OmQuery(OmQuery::OP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OmQuery::OP_OR,
		    OmQuery("one", 1, 4),
		    OmQuery("two", 1, 2)));

    enquire.set_query(myquery);

    OmMSet mymset = enquire.get_mset(0, 10);

    TEST_MSET_SIZE(mymset, 1);
    om_termname_list list(enquire.get_matching_terms_begin(mymset.begin()),
			  enquire.get_matching_terms_end(mymset.begin()));
    TEST(list == answers_list);

    return true;
}

// tests that specifying a nonexistent input file throws an exception.
static bool test_absentfile1()
{
    TEST_EXCEPTION(OmOpeningError,
		   OmDatabase mydb(get_database("/this_does_not_exist"));
		   OmEnquire enquire(make_dbgrp(&mydb));
		   
		   OmQuery myquery("cheese");
		   enquire.set_query(myquery);
		   
		   OmMSet mymset = enquire.get_mset(0, 10);)
    return true;
}

// tests that the collapsing on termpos optimisation works
static bool test_poscollapse1()
{
    OmQuery myquery1(OmQuery::OP_OR,
		     OmQuery("thi", 1, 1),
		     OmQuery("thi", 1, 1));
    OmQuery myquery2("thi", 2, 1);

    if (verbose) {
	tout << myquery1.get_description() << "\n";
	tout << myquery2.get_description() << "\n";
    }

    OmMSet mymset1 = do_get_simple_query_mset(myquery1);
    OmMSet mymset2 = do_get_simple_query_mset(myquery2);

    TEST_EQUAL(mymset1, mymset2);

    return true;
}

// tests that the collapsing on termpos optimisation gives correct query length
static bool test_poscollapse2()
{
    OmQuery q(OmQuery::OP_OR, OmQuery("thi", 1, 1), OmQuery("thi", 1, 1));
    TEST_EQUAL(q.get_length(), 2);
    return true;
}

#if 0
// test that the batch query functionality works
static bool test_batchquery1()
{
    OmBatchEnquire::query_desc mydescs[3];
    mydescs[0] = OmBatchEnquire::query_desc(query("this"), 0, 10, 0, 0, 0);
    mydescs[1] = OmBatchEnquire::query_desc(OmQuery(), 0, 10, 0, 0, 0);
    mydescs[2] = OmBatchEnquire::query_desc(OmQuery("word"), 0, 10, 0, 0, 0);

    OmBatchEnquire benq(get_simple_database());

    OmBatchEnquire::query_batch myqueries(mydescs, mydescs+3);

    benq.set_queries(myqueries);

    OmBatchEnquire::mset_batch myresults = benq.get_msets();

    TEST_EQUAL(myresults.size(), 3);
    TEST_EQUAL(myresults[0].value(), do_get_simple_query_mset(query("this")));
    TEST(!myresults[1].is_valid());
    TEST_EXCEPTION(OmInvalidResultError, OmMSet unused = myresults[1].value());
    TEST_EQUAL(myresults[2].value(), do_get_simple_query_mset(OmQuery("word")));

    return true;
}
#endif

// test that running a query twice returns the same results
static bool test_repeatquery1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    enquire.set_query(query(OmQuery::OP_OR, "this", "word"));

    OmMSet mymset1 = enquire.get_mset(0, 10);
    OmMSet mymset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset1, mymset2);

    return true;
}

// test that prefetching documents works (at least, gives same results)
static bool test_fetchdocs1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    enquire.set_query(query(OmQuery::OP_OR, "this", "word"));

    OmMSet mymset1 = enquire.get_mset(0, 10);
    OmMSet mymset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mymset1, mymset2);
    mymset2.fetch(mymset2[0], mymset2[mymset2.size() - 1]);
    mymset2.fetch(mymset2.begin(), mymset2.end());
    mymset2.fetch(mymset2.begin());
    mymset2.fetch();

    OmMSetIterator it1 = mymset1.begin();
    OmMSetIterator it2 = mymset2.begin();

    while(it1 != mymset1.end() && it2 != mymset2.end()) {
	TEST_EQUAL(it1.get_document().get_data().value,
		   it2.get_document().get_data().value);
	TEST_NOT_EQUAL(it1.get_document().get_data().value, "");
	TEST_NOT_EQUAL(it2.get_document().get_data().value, "");
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
    OmEnquire enquire(get_database("apitest_space"));
    OmMSet mymset;
    std::vector<om_docid> docs;
    OmStem stemmer("english");

    init_simple_enquire(enquire, OmQuery(stemmer.stem_word("space man")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    docs = std::vector<om_docid>(mymset.begin(), mymset.end());
    TEST_EQUAL(docs.size(), 1);

    for (int key_no = 1; key_no < 7; ++key_no) {
	TEST_NOT_EQUAL(mymset.begin().get_document().get_data().value, "");
	TEST_NOT_EQUAL(mymset.begin().get_document().get_key(key_no).value, "");
    }

    init_simple_enquire(enquire, OmQuery(stemmer.stem_word("tab\tby")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    docs = std::vector<om_docid>(mymset.begin(), mymset.end());
    TEST_EQUAL(docs.size(), 1);

    for (int key_no = 1; key_no < 7; ++key_no) {
	OmKey key = mymset.begin().get_document().get_key(key_no);
	TEST_NOT_EQUAL(key.value, "");
	if (key_no == 0) {
	    TEST(key.value.size() > 262);
	    TEST_EQUAL((unsigned char)(key.value[261]), 255);
	}
    }
    
    init_simple_enquire(enquire, OmQuery(stemmer.stem_word("back\\slash")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    docs = std::vector<om_docid>(mymset.begin(), mymset.end());
    TEST_EQUAL(docs.size(), 1);

    return true;
}

// test that XOR queries work
static bool test_xor1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);
    OmStem stemmer("english");

    std::vector<om_termname> terms;
    terms.push_back(stemmer.stem_word("this"));
    terms.push_back(stemmer.stem_word("word"));
    terms.push_back(stemmer.stem_word("of"));

    OmQuery query(OmQuery::OP_XOR, terms.begin(), terms.end());
    OmSettings mymopt;
    mymopt.set("match_weighting_scheme", "bool");
    enquire.set_query(query);

    OmMSet mymset = enquire.get_mset(0, 10, 0, &mymopt);
    mset_expect_order(mymset, 1, 2, 5, 6);

    return true;
}

// test OmDatabase::get_document()
static bool test_getdoc1()
{
    OmDatabase db(get_database("apitest_onedoc"));
    db.get_document(1);
    TEST_EXCEPTION(OmInvalidArgumentError, db.get_document(0));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(999999999));    
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(123456789));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(3));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(2));
    return true;
}

// test whether operators with no elements work as a null query
static bool test_emptyop1()
{
    OmEnquire enquire(get_simple_database());
    std::vector<OmQuery> nullvec;
    
    OmQuery query1(OmQuery::OP_XOR, nullvec.begin(), nullvec.end());

    OmMSet mymset = do_get_simple_query_mset(query1);
    TEST_MSET_SIZE(mymset, 0);
    TEST_EXCEPTION(OmInvalidArgumentError, enquire.get_matching_terms_begin(1));

    return true;
}

// test for keepalives
static bool test_keepalive1()
{
    OmDatabase db(get_network_database("apitest_simpledata", 5000));

    /* Test that keep-alives work */
    for (int i=0; i<10; ++i) {
	sleep(2);
	db.keep_alive();
    }
    OmEnquire enquire(db);
    OmQuery myquery("word");
    enquire.set_query(myquery);
    enquire.get_mset(0, 10);

    /* Test that things break without keepalives */
    sleep(10);
    enquire.set_query(myquery);
    TEST_EXCEPTION(OmNetworkError,
		   enquire.get_mset(0, 10));

    return true;
}

// test that iterating through all terms in a database works.
static bool test_allterms1()
{
    OmDatabase db(get_database("apitest_allterms"));
    OmTermIterator ati = db.allterms_begin();
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    OmTermIterator ati2 = ati;

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
    
#if 0 // FIXME: why isn't this used?
// test that iterating through all terms in two databases works.
static bool test_allterms2()
{
    OmDatabase db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));
    OmTermIterator ati = db.allterms_begin();

    TEST(ati != db.allterms_end());
    TEST(*ati == "five");
    TEST(ati.get_termfreq() == 2);
    ati++;

    TEST(ati != db.allterms_end());
    TEST(*ati == "four");
    TEST(ati.get_termfreq() == 1);

    OmTermIterator ati2 = ati;

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "four");
    TEST(ati2.get_termfreq() == 1);

    ++ati;
    ++ati2;
    TEST(ati != db.allterms_end());
    TEST(*ati == "six");
    TEST(ati.get_termfreq() == 3);

    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "one");
    TEST(ati2.get_termfreq() == 1);

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
#endif

// test that skip_to sets at_end (regression test)
static bool test_allterms3()
{
    OmDatabase db;
    db.add_database(get_database("apitest_allterms"));
    OmTermIterator ati = db.allterms_begin();

    ati.skip_to(om_termname("zzzzzz"));
    TEST(ati == db.allterms_end());

    return true;
}

// test that searching for a term with a special characters in it works
static bool test_specialterms1()
{
    OmEnquire enquire(get_database("apitest_space"));
    OmMSet mymset;
    std::vector<om_docid> docs;
    OmStem stemmer("english");

    init_simple_enquire(enquire, OmQuery(stemmer.stem_word("new\nline")));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    docs = std::vector<om_docid>(mymset.begin(), mymset.end());
    TEST_EQUAL(docs.size(), 1);

    for (int key_no = 0; key_no < 7; ++key_no) {
	OmKey key = mymset.begin().get_document().get_key(key_no);
	TEST_NOT_EQUAL(key.value, "");
	if (key_no == 0) {
	    TEST(key.value.size() > 263);
	    TEST_EQUAL((unsigned char)(key.value[262]), 255);
	    for (int k = 0; k < 256; k++) {
		TEST_EQUAL((unsigned char)(key.value[k+7]), k);
	    }
	}
    }
    
    init_simple_enquire(enquire,
			OmQuery(stemmer.stem_word(std::string("big\0zero", 8))));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    docs = std::vector<om_docid>(mymset.begin(), mymset.end());
    TEST_EQUAL(docs.size(), 1);

    return true;
}

// test that searching for a term not in the database fails nicely
static bool test_absentterm1()
{
    OmEnquire enquire(get_simple_database());
    OmQuery query("frink");
    OmSettings mymopt;
    mymopt.set("match_weighting_scheme", "bool");
    init_simple_enquire(enquire, query);

    OmMSet mymset = enquire.get_mset(0, 10, 0, &mymopt);
    mset_expect_order(mymset);

    return true;
}

// as absentterm1, but setting query from a vector of terms
static bool test_absentterm2()
{
    OmEnquire enquire(get_simple_database());
    vector<om_termname> terms;
    terms.push_back("frink");

    OmQuery query(OmQuery::OP_OR, terms.begin(), terms.end());
    enquire.set_query(query);

    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    return true;
}

// test that rsets do sensible things
static bool test_rset1()
{
    OmDatabase mydb(get_database("apitest_rset"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmQuery myquery = query(OmQuery::OP_OR, "giraffe", "tiger");
    enquire.set_query(myquery);

    OmMSet mymset1 = enquire.get_mset(0, 10);

    OmRSet myrset;
    myrset.add_document(1);

    OmMSet mymset2 = enquire.get_mset(0, 10, &myrset);

    // We should have the same documents turn up, but 1 and 3 should
    // have higher weights with the RSet.
    TEST_MSET_SIZE(mymset1, 3);
    TEST_MSET_SIZE(mymset2, 3);

    return true;
}

// test that rsets do more sensible things
static bool test_rset2()
{
    OmDatabase mydb(get_database("apitest_rset"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmQuery myquery = query(OmQuery::OP_OR, "cuddly", "people");
    enquire.set_query(myquery);

    OmMSet mymset1 = enquire.get_mset(0, 10);

    OmRSet myrset;
    myrset.add_document(2);

    OmMSet mymset2 = enquire.get_mset(0, 10, &myrset);

    mset_expect_order(mymset1, 1, 2);
    mset_expect_order(mymset2, 2, 1);

    return true;
}

// test that rsets behave correctly with multiDBs
static bool test_rsetmultidb1()
{
    OmDatabase mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    OmDatabase mydb2(get_database("apitest_rset"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));

    OmEnquire enquire1(make_dbgrp(&mydb1));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    OmQuery myquery = query(OmQuery::OP_OR, "cuddly", "multiple");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    OmRSet myrset1;
    OmRSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    OmMSet mymset1a = enquire1.get_mset(0, 10);
    OmMSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    OmMSet mymset2a = enquire2.get_mset(0, 10);
    OmMSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

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
    OmDatabase mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    OmDatabase mydb2(get_database("apitest_rset"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));

    OmEnquire enquire1(make_dbgrp(&mydb1));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    OmQuery myquery = query("is");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    OmRSet myrset1;
    OmRSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    OmMSet mymset1a = enquire1.get_mset(0, 10);
    OmMSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    OmMSet mymset2a = enquire2.get_mset(0, 10);
    OmMSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

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
    OmEnquire enquire(get_database("apitest_simpledata2"));
    enquire.set_query(query(OmQuery::OP_OR, "cuddly", "people"));
    OmMSet mset = enquire.get_mset(0, 10); // used to fail assertion
    return true;
}

/// Simple test of the elite set operator.
static bool test_eliteset1()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery1 = query(OmQuery::OP_OR, "word");
    myquery1.set_length(2); // so the query lengths are the same

    OmQuery myquery2 = query(OmQuery::OP_ELITE_SET, "simple", "word");
    myquery2.set_elite_set_size(1);

    enquire.set_query(myquery1);
    OmMSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    OmMSet mymset2 = enquire.get_mset(0, 10);

    TEST_EQUAL(mymset1, mymset2);
    return true;
}

/// Test that the elite set operator works if the set contains
/// sub-expressions (regression test)
static bool test_eliteset2()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery1 = query(OmQuery::OP_AND, "word", "search");

    OmQuery myquery2(OmQuery::OP_ELITE_SET,
		     query("this"),
		     query(OmQuery::OP_AND, "word", "search"));
    myquery2.set_elite_set_size(1);

    enquire.set_query(myquery1);
    OmMSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    OmMSet mymset2 = enquire.get_mset(0, 10);

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
    OmDatabase mydb1(get_database("apitest_simpledata"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmEnquire enquire2(make_dbgrp(&mydb2));

    // make a query
    OmStem stemmer("english");

    std::string term1 = stemmer.stem_word("word");
    std::string term2 = stemmer.stem_word("rubbish");
    std::string term3 = stemmer.stem_word("banana");

    std::vector<om_termname> terms;
    terms.push_back(term1);
    terms.push_back(term2);
    terms.push_back(term3);

    OmQuery myquery1(OmQuery::OP_OR, terms.begin(), terms.end());
    enquire1.set_query(myquery1);

    OmQuery myquery2(OmQuery::OP_ELITE_SET, terms.begin(), terms.end());
    myquery2.set_elite_set_size(3);
    enquire2.set_query(myquery2);

    // retrieve the results
    OmMSet mymset1 = enquire1.get_mset(0, 10);
    OmMSet mymset2 = enquire2.get_mset(0, 10);

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
    OmDatabase mydb1(get_database("apitest_simpledata"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmEnquire enquire2(make_dbgrp(&mydb2));

    OmQuery myquery1 = query("rubbish");
    OmQuery myquery2 = query(OmQuery::OP_ELITE_SET, "word", "rubbish", "fibble");
    myquery2.set_elite_set_size(1);
    enquire1.set_query(myquery1);
    enquire2.set_query(myquery2);

    // retrieve the results
    OmMSet mymset1 = enquire1.get_mset(0, 10);
    OmMSet mymset2 = enquire2.get_mset(0, 10);

    TEST_NOT_EQUAL(mymset2.size(), 0);
    TEST_EQUAL(mymset1, mymset2);
//    TEST_EQUAL(mymset1, mymset2);

    return true;
}

/// Test that the termfreq returned by termlists is correct.
static bool test_termlisttermfreq1()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    OmRSet rset1;
    OmRSet rset2;
    rset1.add_document(5);
    rset2.add_document(6);

    OmESet eset1 = enquire.get_eset(1000, rset1);
    OmESet eset2 = enquire.get_eset(1000, rset2);

    // search for weight of term 'another'
    std::string theterm = stemmer.stem_word("another");

    om_weight wt1 = 0;
    om_weight wt2 = 0;
    {
	OmESetIterator i = eset1.begin();
	for ( ; i != eset1.end(); i++) {
	    if (*i == theterm) {
		wt1 = i.get_weight();
		break;
	    }
	}
    }
    {
	OmESetIterator i = eset2.begin();
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

// tests an expand accross multiple databases
static bool test_multiexpand1()
{
    OmDatabase mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make simple equivalent rsets, with a document from each database in each.
    OmRSet rset1;
    OmRSet rset2;
    rset1.add_document(1);
    rset1.add_document(7);
    rset2.add_document(1);
    rset2.add_document(2);

    // retrieve the top ten results from each method of accessing
    // multiple text files

    // This is the single database one.
    OmESet eset1 = enquire1.get_eset(1000, rset1);

    // This is the multi database with approximation
    OmESet eset2 = enquire2.get_eset(1000, rset2);

    OmSettings eopts;
    eopts.set("expand_use_exact_termfreq", true);
    // This is the multi database without approximation
    OmESet eset3 = enquire2.get_eset(1000, rset2, &eopts);

    TEST_EQUAL(eset1.size(), eset2.size());
    TEST_EQUAL(eset1.size(), eset3.size());

    OmESetIterator i = eset1.begin();
    OmESetIterator j = eset2.begin();
    OmESetIterator k = eset3.begin();
    bool all_iwts_equal_jwts = true;
    for ( ; i != eset1.end(), j != eset2.end(), k != eset3.end();
	 i++, j++, k++) {
	if (i.get_weight() != j.get_weight()) all_iwts_equal_jwts = false;
	TEST_EQUAL(i.get_weight(), k.get_weight());
	TEST_EQUAL(*i, *k);
    }
    TEST(!all_iwts_equal_jwts);
    return true;
}

/// Test the termfrequency and termweight info returned for query terms
static bool test_qterminfo1()
{
    OmDatabase mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make a query
    OmStem stemmer("english");

    std::string term1 = stemmer.stem_word("word");
    std::string term2 = stemmer.stem_word("inmemory");
    std::string term3 = stemmer.stem_word("flibble");

    OmQuery myquery(OmQuery::OP_OR,
		    OmQuery(term1),
		    OmQuery(OmQuery::OP_OR,
			    OmQuery(term2),
			    OmQuery(term3)));
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the results
    OmMSet mymset1a = enquire1.get_mset(0, 0);
    OmMSet mymset2a = enquire2.get_mset(0, 0);

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

    TEST_EXCEPTION(OmInvalidArgumentError,
		   mymset1a.get_termfreq("sponge"));

    return true;
}

// tests that when specifiying that no items are to be returned, those
// statistics which should be the same are.
static bool test_msetzeroitems1()
{
    OmMSet mymset1 = do_get_simple_query_mset(query("this"), 0);
    OmMSet mymset2 = do_get_simple_query_mset(query("this"), 1);

    TEST_EQUAL(mymset1.get_max_possible(), mymset2.get_max_possible());

    return true;
}

// test that the matches_* of a simple query are as expected
static bool test_matches1()
{
    OmQuery myquery;
    OmMSet mymset;

    myquery = query("word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(OmQuery::OP_OR, "inmemory", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(OmQuery::OP_AND, "inmemory", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_estimated(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 0);

    myquery = query(OmQuery::OP_AND, "simple", "word");
    mymset = do_get_simple_query_mset(myquery);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mymset.get_matches_estimated(), 2);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 2);

    myquery = query(OmQuery::OP_AND, "simple", "word");
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

    myquery = query(OmQuery::OP_AND, "paragraph", "another");
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
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1, doc2, doc3;

    // doc1 should come top, but if term "foo" gets wdf of 1, doc2 will beat it
    // doc3 should beat both
    // Note: all docs have same length
    doc1.set_data(std::string("tom"));
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 3);
    doc1.add_posting("bar", 4);
    db.add_document(doc1);
    
    doc2.set_data(std::string("dick"));
    doc2.add_posting("foo", 1);
    doc2.add_posting("foo", 2);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    db.add_document(doc2);

    doc3.set_data(std::string("harry"));
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 2);
    doc3.add_posting("foo", 2);
    doc3.add_posting("bar", 3);
    db.add_document(doc3);

    OmQuery query("foo");

    OmEnquire enq(make_dbgrp(&db));
    enq.set_query(query);

    OmMSet mset = enq.get_mset(0, 10);

    mset_expect_order(mset, 3, 1, 2);

    return true;    
}

// test that indexing a term more than once at the same position increases
// the wdf
static bool test_adddoc2()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);
    om_docid did;

    OmDocument doc2 = db.get_document(did = db.add_document(doc1));
    TEST_EQUAL(did, 1);

    OmTermIterator iter1 = doc1.termlist_begin();
    OmTermIterator iter2 = doc2.termlist_begin();
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bar");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 4);
    TEST_EQUAL(iter2.get_wdf(), 4);
    TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "gone");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 == doc1.termlist_end());
    TEST(iter2 == doc2.termlist_end());

    doc2.remove_posting("foo", 1, 5);
    doc2.add_term_nopos("bat", 0);
    doc2.add_term_nopos("bar", 8);
    doc2.add_term_nopos("bag", 0);
    doc2.remove_term("gone");

    // Should have (doc,wdf) pairs: (bag,0)(bar,9)(bat,0)(foo,0)
    // positionlists (bag,none)(bar,3)(bat,none)(foo,2)

    iter2 = doc2.termlist_begin();
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bag");
    TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bar");
    TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bat");
    TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "foo");
    TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter2.get_termfreq(), 0);

    OmPositionListIterator pi1 = iter1.positionlist_begin();
    OmPositionListIterator pi2 = iter2.positionlist_begin();
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
    TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter2.get_termfreq(), 0);

    OmPositionListIterator temp1 = iter1.positionlist_begin();
    pi1 = temp1;
    OmPositionListIterator temp2 = iter2.positionlist_begin();
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

static bool test_poslist1()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc;
    doc.add_term_nopos("nopos");
    om_docid did = db.add_document(doc);

    TEST_EXCEPTION(OmRangeError,
	// Check what happens when term doesn't exist
	OmPositionListIterator i = db.positionlist_begin(did, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );

    TEST_EXCEPTION(OmDocNotFoundError,
        // Check what happens when the document doesn't even exist
        OmPositionListIterator i = db.positionlist_begin(123, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );            
    
    {
	OmPositionListIterator i = db.positionlist_begin(did, "nopos");
	TEST_EQUAL(i, db.positionlist_end(did, "nopos"));
    }
    
    OmDocument doc2 = db.get_document(did);
   
    OmTermIterator term = doc2.termlist_begin();

    {
	OmPositionListIterator i = term.positionlist_begin(); 
	TEST_EQUAL(i, term.positionlist_end());
    }

    OmDocument doc3;
    doc3.add_posting("hadpos", 1);
    om_docid did2 = db.add_document(doc3);

    OmDocument doc4 = db.get_document(did2);
    doc4.remove_posting("hadpos", 1);
    db.replace_document(did2, doc4);
   
    {
	OmPositionListIterator i = db.positionlist_begin(did2, "hadpos");
	TEST_EQUAL(i, db.positionlist_end(did2, "hadpos"));
    }

    db.delete_document(did);
    TEST_EXCEPTION(OmDocNotFoundError,
        // Check what happens when the document doesn't even exist
	// (but once did)
	OmPositionListIterator i = db.positionlist_begin(did, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );

    return true;
}

// tests that database destructors flush if it isn't done explicitly
static bool test_implicitendsession1()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc;

    doc.set_data(std::string("top secret"));
    doc.add_posting("cia", 1);
    doc.add_posting("nsa", 2);
    doc.add_posting("fbi", 3);
    db.add_document(doc);

    return true;
}

// tests that assignment of OmDatabase and OmWritableDatabase work as expected
static bool test_databaseassign1()
{
    OmWritableDatabase wdb = get_writable_database("");
    OmDatabase db = get_database("");
    OmDatabase actually_wdb = wdb;
    OmWritableDatabase w1(wdb);
    w1 = wdb;
    OmDatabase d1(wdb);
    OmDatabase d2(actually_wdb);
    d2 = wdb;
    d2 = actually_wdb;
    wdb = wdb; // check assign to itself works
    db = db; // check assign to itself works
    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc1()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);
    om_docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("gone");

    db.add_document(doc1);

    doc1.add_term_nopos("new", 1);
    db.add_document(doc1);

    db.delete_document(1);

    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(1));

    doc1 = db.get_document(2);
    doc1.remove_term("foo");
    doc1.add_term_nopos("fwing");
    db.replace_document(2, doc1);

    OmDocument doc2 = db.get_document(2);
    OmTermIterator tit = doc2.termlist_begin();
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
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone",3);
    doc1.add_posting("bar", 4);
    doc1.add_posting("foo", 5);
    om_docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    OmDocument doc2;

    doc2.add_posting("foo", 1);
    doc2.add_posting("pipco", 2);
    doc2.add_posting("bar", 4);
    doc2.add_posting("foo", 5);

    db.replace_document(did, doc2);

    OmDocument doc3 = db.get_document(did);
    OmTermIterator tIter = doc3.termlist_begin();
    TEST_EQUAL(*tIter, "bar");
    OmPositionListIterator pIter = tIter.positionlist_begin();
    TEST_EQUAL(*pIter, 4);
    ++tIter;
    TEST_EQUAL(*tIter, "foo");
    OmPositionListIterator qIter = tIter.positionlist_begin();
    TEST_EQUAL(*qIter, 1);
    ++qIter;
    TEST_EQUAL(*qIter, 5);
    ++tIter;
    TEST_EQUAL(*tIter, "pipco");
    OmPositionListIterator rIter = tIter.positionlist_begin();
    TEST_EQUAL(*rIter, 2);
    ++tIter;
    TEST_EQUAL(tIter, doc3.termlist_end());
    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc2()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);
    om_docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("one");
    doc1.add_posting("three", 4);

    db.add_document(doc1);

    doc1.add_posting("one", 7);
    doc1.remove_term("two");

    db.add_document(doc1);

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

    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(2));
    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(3));
    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(4));
    
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

    TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(2));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(3));

    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(2));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(3));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// another test of deletion of documents, a cut-down version of deldoc2
static bool test_deldoc3()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("one", 1);

    om_docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    db.flush();

    db.reopen();

    db.delete_document(1);

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));

    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(2));
    
    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);

    TEST_EQUAL(db.term_exists("one"), false);

    TEST_EQUAL(db.get_collection_freq("one"), 0);

    TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(2));

    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(OmDocNotFoundError, db.get_document(2));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// tests that deletion and updating of (lots of) documents works as expected
static bool test_deldoc4()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);

    OmDocument doc2 = doc1;
    doc2.remove_term("one");
    doc2.add_posting("three", 4);

    OmDocument doc3 = doc2;
    doc3.add_posting("one", 7);
    doc3.remove_term("two");

    const int maxdoc = 1000 * 3;
    om_docid did;
    for (unsigned int i=0; i<maxdoc / 3; ++i) {
	did = db.add_document(doc1);
	TEST_EQUAL(did, i*3+1);
	did = db.add_document(doc2);
	TEST_EQUAL(did, i*3+2);
	did = db.add_document(doc3);
	TEST_EQUAL(did, i*3+3);

	bool is_power_of_two = false;
	unsigned int temp = i;
	int count = 0;
	while (temp > 0) {
	    int next = temp >> 1;
	    if (next == 0) {
		is_power_of_two = (temp << count) == i;
		break;
	    }
	    count++;
	    temp = next;
	}
	if (is_power_of_two) {
	    db.flush();
	    db.reopen();
	}
    }
    db.flush();
    db.reopen();

    /* delete the documents in a peculiar order */
    for (unsigned int i=0; i < (maxdoc/3); i++) {
	db.delete_document(maxdoc - i);
	db.delete_document(maxdoc/3 + i + 1);
	db.delete_document(i+1);
    }

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    for (int i=1; i<=maxdoc; ++i) {
	TEST_EXCEPTION(OmDocNotFoundError, db.termlist_begin(i));
	TEST_EXCEPTION(OmDocNotFoundError, db.get_doclength(i));
	TEST_EXCEPTION(OmDocNotFoundError, db.get_document(i));
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
    OmQuery q1("word", 2);
    OmQuery q2("word");
    q2.set_length(2);
    OmMSet mset1 = do_get_simple_query_mset(q1);
    OmMSet mset2 = do_get_simple_query_mset(q2);
    // Check the weights
    TEST(mset1.begin().get_weight() > mset2.begin().get_weight());
    return true;
}

// tests that query length affects the document weights
static bool test_qlen1()
{
    OmQuery q1("word");
    OmQuery q2("word");
    q2.set_length(2);
    OmMSet mset1 = do_get_simple_query_mset(q1);
    OmMSet mset2 = do_get_simple_query_mset(q2);
    // Check the weights
    //TEST(mset1.begin().get_weight() < mset2.begin().get_weight());
    TEST(mset1.begin().get_weight() == mset2.begin().get_weight());
    return true;
}

// tests that opening a non-existant termlist throws the correct exception
static bool test_termlist1()
{
    OmDatabase db(get_database("apitest_onedoc"));
    TEST_EXCEPTION(OmInvalidArgumentError,
		   OmTermIterator t = db.termlist_begin(0));
    TEST_EXCEPTION(OmDocNotFoundError,
		   OmTermIterator t = db.termlist_begin(2));
    /* Cause the database to be used properly, showing up problems
     * with the link being in a bad state.  CME */
    OmTermIterator temp = db.termlist_begin(1);
    TEST_EXCEPTION(OmDocNotFoundError,
		   OmTermIterator t = db.termlist_begin(999999999));
    return true;
}

// tests that an OmTermIterator works as an STL iterator
static bool test_termlist2()
{
    OmDatabase db(get_database("apitest_onedoc"));
    OmTermIterator t = db.termlist_begin(1);
    OmTermIterator tend = db.termlist_end(1);

    // test operator= creates a copy which compares equal
    OmTermIterator t_copy = t;
    TEST_EQUAL(t, t_copy);

    // test copy constructor creates a copy which compares equal
    OmTermIterator t_clone(t);
    TEST_EQUAL(t, t_clone);

    std::vector<om_termname> v(t, tend);

    t = db.termlist_begin(1);    
    tend = db.termlist_end(1);
    std::vector<om_termname>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	TEST_NOT_EQUAL(t, tend);
	TEST_EQUAL(*i, *t);
	t++;
    }
    TEST_EQUAL(t, tend);
    return true;
}

static OmTermIterator
test_termlist3_helper()
{
    OmDatabase db(get_database("apitest_onedoc"));
    return db.termlist_begin(1);
}

// tests that an OmTermIterator still works when the DB is deleted
static bool test_termlist3()
{
    OmTermIterator u = test_termlist3_helper();
    OmDatabase db(get_database("apitest_onedoc"));
    OmTermIterator t = db.termlist_begin(1);
    OmTermIterator tend = db.termlist_end(1);

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
    OmDatabase db(get_database("apitest_onedoc"));
    OmTermIterator i = db.termlist_begin(1);
    i.skip_to("");
    i.skip_to("\xff");
    return true;
}

// tests that opening a non-existant postlist return an empty list
static bool test_postlist1()
{
    OmDatabase db(get_database("apitest_simpledata"));

    TEST_EXCEPTION(OmInvalidArgumentError, db.postlist_begin(""));

    TEST_EQUAL(db.postlist_begin("rosebud"), db.postlist_end("rosebud"));

    std::string s = "let_us_see_if_we_can_break_it_with_a_really_really_long_term.";
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

// tests that an OmPostListIterator works as an STL iterator
static bool test_postlist2()
{
    OmDatabase db(get_database("apitest_simpledata"));
    OmPostListIterator p = db.postlist_begin("thi");
    OmPostListIterator pend = db.postlist_end("thi");

    // test operator= creates a copy which compares equal
    OmPostListIterator p_copy = p;
    TEST_EQUAL(p, p_copy);

    // test copy constructor creates a copy which compares equal
    OmPostListIterator p_clone(p);
    TEST_EQUAL(p, p_clone);

    std::vector<om_docid> v(p, pend);

    p = db.postlist_begin("thi");
    pend = db.postlist_end("thi");
    std::vector<om_docid>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	TEST_NOT_EQUAL(p, pend);
	TEST_EQUAL(*i, *p);
	p++;
    }
    TEST_EQUAL(p, pend);
    return true;
}

static OmPostListIterator
test_postlist3_helper()
{
    OmDatabase db(get_database("apitest_simpledata"));
    return db.postlist_begin("thi");
}

// tests that an OmPostListIterator still works when the DB is deleted
static bool test_postlist3()
{
    OmPostListIterator u = test_postlist3_helper();
    OmDatabase db(get_database("apitest_simpledata"));
    OmPostListIterator p = db.postlist_begin("thi");
    OmPostListIterator pend = db.postlist_end("thi");

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
    OmDatabase db(get_database("apitest_simpledata"));
    OmPostListIterator i = db.postlist_begin("thi");
    i.skip_to(1);
    i.skip_to(999999999);
    TEST(i == db.postlist_end("thi"));
    return true;
}

// tests long postlists
static bool test_postlist5()
{
    OmDatabase db(get_database("apitest_manydocs"));
    if(db.get_avlength() != 1) // Allow for databases which don't support length
	TEST_EQUAL_DOUBLE(db.get_avlength(), 4);
    OmPostListIterator i = db.postlist_begin("thi");
    unsigned int j = 1;
    while(i != db.postlist_end("thi")) {
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
    OmDatabase db(get_database("apitest_simpledata"));
    OmPostListIterator i = db.postlist_begin("thi");
    TEST(i != db.postlist_end("thi"));
    while(i != db.postlist_end("thi")) {
	TEST_EQUAL(i.get_doclength(), db.get_doclength(*i));
	i++;
    }
    return true;
}

// tests collection frequency
static bool test_collfreq1()
{
    OmDatabase db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.get_collection_freq("thi"), 11);
    TEST_EQUAL(db.get_collection_freq("first"), 1);
    TEST_EQUAL(db.get_collection_freq("last"), 0);
    TEST_EQUAL(db.get_collection_freq("word"), 9);

    OmDatabase db1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmDatabase db2(get_database("apitest_simpledata"));
    db2.add_database(get_database("apitest_simpledata2"));

    TEST_EQUAL(db1.get_collection_freq("thi"), 15);
    TEST_EQUAL(db1.get_collection_freq("first"), 1);
    TEST_EQUAL(db1.get_collection_freq("last"), 0);
    TEST_EQUAL(db1.get_collection_freq("word"), 11);
    TEST_EQUAL(db2.get_collection_freq("thi"), 15);
    TEST_EQUAL(db2.get_collection_freq("first"), 1);
    TEST_EQUAL(db2.get_collection_freq("last"), 0);
    TEST_EQUAL(db2.get_collection_freq("word"), 11);

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
    //{"batchquery1",	   test_batchquery1}, OmBatchEnquire temporarily removed
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

/// The tests which require a database which supports keys > 0 sensibly
test_desc multikey_tests[] = {
    {"collapsekey1",	   test_collapsekey1},
    {0, 0}
};

test_desc mus36_tests[] = {
    {"collapsekey2",       test_collapsekey2},
    {0, 0}
};

/// The tests which need a backend which supports iterating over all terms
test_desc allterms_tests[] = {
    {"allterms1",	   test_allterms1},
//    {"allterms2",	   test_allterms2},
    {"allterms3",	   test_allterms3},
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
// FIXME: quartz needs fixing!    {"poslist1",	   test_poslist1},
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
    {"esetiterator1",	   test_esetiterator1},
    {"esetiterator2",	   test_esetiterator2},
    {"multiexpand1",       test_multiexpand1},
    {"postlist1",	   test_postlist1},
    {"postlist2",	   test_postlist2},
    {"postlist3",	   test_postlist3},
    {"postlist4",	   test_postlist4},
    {"postlist5",	   test_postlist5},
    {"postlist6",	   test_postlist6},
    {0, 0}
};

test_desc remotedb_tests[] = {
    {"multierrhandler1",   test_multierrhandler1},
    {"keepalive1",	   test_keepalive1},
    {0, 0}
};
