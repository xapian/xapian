/* api_posdb.cc: tests which need a backend with positional information
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

using std::cout;
using std::endl;
using std::vector;
using std::string;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "textfile_indexer.h"
#include "../indexer/index_utils.h"
#include "utils.h"

#include "apitest.h"

/// Simple test of NEAR
static bool test_near1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 3);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 5);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 6);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 3);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 4);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 5);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 6);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 7);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 8);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    // test really large window size
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 999999999);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    return true;
}

/// Test NEAR over operators
static bool test_near2()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("and")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    return true;
}

/// Simple test of PHRASE
static bool test_phrase1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 3);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 5);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 6);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 3);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 4);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 5);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 6);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 7);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 8);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // test really large window size
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 999999999);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // regression test (was matching doc 15, should fail)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("first")));
    subqs.push_back(OmQuery(stemmer.stem_word("second")));
    subqs.push_back(OmQuery(stemmer.stem_word("third")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 9);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    // regression test (should match doc 15, make sure still does with fix)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("first")));
    subqs.push_back(OmQuery(stemmer.stem_word("second")));
    subqs.push_back(OmQuery(stemmer.stem_word("third")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 10);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 15);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("milk")));
    subqs.push_back(OmQuery(stemmer.stem_word("rare")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 16);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("rare")));
    subqs.push_back(OmQuery(stemmer.stem_word("milk")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 17);

    return true;
}

/// Test PHRASE over operators
static bool test_phrase2()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("and")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    q.set_bool(true);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    return true;
}

// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which need a backend which supports positional information
test_desc positionaldb_tests[] = {
    {"near1",		   test_near1},
    {"near2",		   test_near2},
    {"phrase1",		   test_phrase1},
    {"phrase2",		   test_phrase2},
    {0, 0}
};
