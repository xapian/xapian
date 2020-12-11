/** @file
 * @brief tests which need a backend with positional information
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2009,2016 Olly Betts
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

#include "api_posdb.h"

#include <string>
#include <vector>

using namespace std;

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

/// Simple test of NEAR
DEFINE_TESTCASE(near1, positional) {
    Xapian::Database mydb(get_database("apitest_phrase"));
    Xapian::Enquire enquire(mydb);
    Xapian::Stem stemmer("english");
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    // make a query
    vector<Xapian::Query> subqs;
    Xapian::Query q;
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 3);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2, 3);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 4);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 7);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 8);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    // test really large window size
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 999999999);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
}

/// Test NEAR over operators
DEFINE_TESTCASE(near2, positional) {
    Xapian::Database mydb(get_database("apitest_phrase"));
    Xapian::Enquire enquire(mydb);
    Xapian::Stem stemmer("english");
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::MSet mymset;

    // make a query
    vector<Xapian::Query> subqs;
    Xapian::Query q;
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    subqs.push_back(Xapian::Query(stemmer("and")));
    TEST_EXCEPTION(Xapian::UnimplementedError,
	q = Xapian::Query(q.OP_NEAR, subqs.begin(), subqs.end(), 2);
	enquire.set_query(q);

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10)
    );
#if 0 // Disable until we reimplement this.
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    subqs.push_back(Xapian::Query(stemmer("operator")));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("operator")));
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    q = Xapian::Query(Xapian::Query::OP_NEAR, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);
#endif
}

/// Simple test of PHRASE
DEFINE_TESTCASE(phrase1, positional) {
    Xapian::Database mydb(get_database("apitest_phrase"));
    Xapian::Enquire enquire(mydb);
    Xapian::Stem stemmer("english");
    enquire.set_weighting_scheme(Xapian::BoolWeight());

    // make a query
    vector<Xapian::Query> subqs;
    Xapian::Query q;
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    Xapian::MSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("phrase")));
    subqs.push_back(Xapian::Query(stemmer("near")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 4);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 7);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 8);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // test really large window size
    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("leave")));
    subqs.push_back(Xapian::Query(stemmer("fridge")));
    subqs.push_back(Xapian::Query(stemmer("on")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 999999999);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // regression test (was matching doc 15, should fail)
    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("first")));
    subqs.push_back(Xapian::Query(stemmer("second")));
    subqs.push_back(Xapian::Query(stemmer("third")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 9);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    // regression test (should match doc 15, make sure still does with fix)
    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("first")));
    subqs.push_back(Xapian::Query(stemmer("second")));
    subqs.push_back(Xapian::Query(stemmer("third")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 10);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 15);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("milk")));
    subqs.push_back(Xapian::Query(stemmer("rare")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 16);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("rare")));
    subqs.push_back(Xapian::Query(stemmer("milk")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 17);
}

/// Test PHRASE over operators
DEFINE_TESTCASE(phrase2, positional) {
    Xapian::Database mydb(get_database("apitest_phrase"));
    Xapian::Enquire enquire(mydb);
    Xapian::Stem stemmer("english");
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::MSet mymset;

    // make a query
    vector<Xapian::Query> subqs;
    Xapian::Query q;
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    subqs.push_back(Xapian::Query(stemmer("and")));
    TEST_EXCEPTION(Xapian::UnimplementedError,
	q = Xapian::Query(q.OP_PHRASE, subqs.begin(), subqs.end(), 2);
	enquire.set_query(q);

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10)
    );
#if 0 // Disable until we reimplement this.
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    subqs.push_back(Xapian::Query(stemmer("operator")));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(Xapian::Query(stemmer("operator")));
    subqs.push_back(Xapian::Query(Xapian::Query::OP_AND,
			    Xapian::Query(stemmer("phrase")),
			    Xapian::Query(stemmer("near"))));
    q = Xapian::Query(Xapian::Query::OP_PHRASE, subqs.begin(), subqs.end(), 2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);
#endif
}

/// Test getting position lists from databases
DEFINE_TESTCASE(poslist1, positional) {
    Xapian::Database mydb(get_database("apitest_poslist"));

    Xapian::Stem stemmer("english");
    string term = stemmer("sponge");

    Xapian::PositionIterator pli = mydb.positionlist_begin(2, term);

    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 1);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 2);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 3);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 5);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 8);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 13);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 21);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST_EQUAL(*pli, 34);
    pli++;
    TEST(pli == mydb.positionlist_end(2, term));
}

DEFINE_TESTCASE(poslist2, positional && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    doc.add_term("nopos");
    Xapian::docid did = db.add_document(doc);

    // Check what happens when term doesn't exist - should give an empty list.
    // Threw RangeError in Xapian < 1.1.0.
    TEST_EQUAL(db.positionlist_begin(did, "nosuchterm"),
	       db.positionlist_end(did, "nosuchterm"));

    // Check what happens when the document doesn't even exist - should give
    // an empty list.  Threw DocNotFoundError in Xapian < 1.1.0.
    TEST_EQUAL(db.positionlist_begin(123, "nosuchterm"),
	       db.positionlist_end(123, "nosuchterm"));

    TEST_EQUAL(db.positionlist_begin(did, "nopos"),
	       db.positionlist_end(did, "nopos"));

    Xapian::Document doc2 = db.get_document(did);

    Xapian::TermIterator term = doc2.termlist_begin();

    {
	Xapian::PositionIterator i = term.positionlist_begin();
	TEST_EQUAL(i, term.positionlist_end());
    }

    Xapian::Document doc3;
    doc3.add_posting("hadpos", 1);
    Xapian::docid did2 = db.add_document(doc3);

    Xapian::Document doc4 = db.get_document(did2);
    doc4.remove_posting("hadpos", 1);
    db.replace_document(did2, doc4);

    {
	Xapian::PositionIterator i = db.positionlist_begin(did2, "hadpos");
	TEST_EQUAL(i, db.positionlist_end(did2, "hadpos"));
    }

    db.delete_document(did);
    // Check what happens when the document doesn't exist (but once did).
    TEST_EQUAL(db.positionlist_begin(did, "nosuchterm"),
	       db.positionlist_end(did, "nosuchterm"));
}

/// Test playing with a positionlist, testing skip_to in particular.
/// (used to be quartztest's test_positionlist1).
DEFINE_TESTCASE(poslist3, positional && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document document;
    document.add_posting("foo", 5);
    document.add_posting("foo", 8);
    document.add_posting("foo", 10);
    document.add_posting("foo", 12);
    db.add_document(document);

    Xapian::PositionIterator pl = db.positionlist_begin(1, "foo");
    Xapian::PositionIterator pl_end = db.positionlist_end(1, "foo");

    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 5);
    ++pl;
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 8);
    ++pl;
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 10);
    ++pl;
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 12);
    ++pl;
    TEST(pl == pl_end);

    pl = db.positionlist_begin(1, "foo");
    pl.skip_to(5);
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 5);

    pl.skip_to(9);
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 10);

    ++pl;
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 12);

    pl.skip_to(12);
    TEST(pl != pl_end);
    TEST_EQUAL(*pl, 12);

    pl.skip_to(13);
    TEST(pl == pl_end);
}

// Regression test - in 0.9.4 (and many previous versions) you couldn't get a
// PositionIterator from a TermIterator from Database::termlist_begin().
//
// Also test that positionlist_count() is implemented for this case, which it
// wasn't in 1.0.2 and earlier.
DEFINE_TESTCASE(positfromtermit1, positional) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::TermIterator t(db.termlist_begin(7));
    TEST_NOT_EQUAL(t, db.termlist_end(7));
    Xapian::PositionIterator p = t.positionlist_begin();
    TEST_NOT_EQUAL(p, t.positionlist_end());

    try {
	TEST_EQUAL(t.positionlist_count(), 1);
	t.skip_to("on");
	TEST_NOT_EQUAL(t, db.termlist_end(7));
	TEST_EQUAL(t.positionlist_count(), 2);
    } catch (const Xapian::UnimplementedError &) {
	SKIP_TEST("TermList::positionlist_count() not yet implemented for this backend");
    }
}
