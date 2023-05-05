/** @file
 * @brief Tests of the OP_VALUE_* query operators.
 */
/* Copyright 2007,2008,2009,2010,2010,2011,2017,2019 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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

#include "api_opvalue.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

#include <string>

using namespace std;

// Feature test for Query::OP_VALUE_RANGE.
DEFINE_TESTCASE(valuerange1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    static const char * const vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z"
    };
    for (auto start : vals) {
	for (auto end : vals) {
	    Xapian::Query query(Xapian::Query::OP_VALUE_RANGE, 1, start, end);
	    enq.set_query(query);
	    Xapian::MSet mset = enq.get_mset(0, 20);
	    // Check that documents in the MSet match the value range filter.
	    set<Xapian::docid> matched;
	    Xapian::MSetIterator i;
	    for (i = mset.begin(); i != mset.end(); ++i) {
		matched.insert(*i);
		string value = db.get_document(*i).get_value(1);
		TEST_REL(value,>=,start);
		TEST_REL(value,<=,end);
	    }
	    // Check that documents not in the MSet don't match the value range filter.
	    for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
		if (matched.find(j) == matched.end()) {
		    string value = db.get_document(j).get_value(1);
		    tout << value << " < '" << start << "' or > '" << end << "'\n";
		    TEST(value < start || value > end);
		}
	    }
	}
    }
}

// Regression test for Query::OP_VALUE_LE - used to return document IDs for
// non-existent documents.
DEFINE_TESTCASE(valuerange2, backend) {
    Xapian::Database db = get_database("valuerange2",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   Xapian::Document doc;
					   doc.set_data("5");
					   doc.add_value(0, "5");
					   wdb.replace_document(5, doc);
				       });
    Xapian::Enquire enq(db);

    Xapian::Query query(Xapian::Query::OP_VALUE_LE, 0, "6");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);

    TEST_EQUAL(mset.size(), 1);
    TEST_EQUAL(*(mset[0]), 5);
}

static void
make_valuerange5(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_value(0, "BOOK");
    db.add_document(doc);
    doc.add_value(0, "VOLUME");
    db.add_document(doc);
}

// Check that lower and upper bounds are used.
DEFINE_TESTCASE(valuerange5, backend) {
    Xapian::Database db = get_database("valuerange5", make_valuerange5);

    // If the lower bound is empty, either the specified value slot is
    // never used in the database, or the backend doesn't track value bounds.
    // Neither should be true here.
    TEST(!db.get_value_lower_bound(0).empty());

    Xapian::Enquire enq(db);

    Xapian::Query query(Xapian::Query::OP_VALUE_RANGE, 0, "APPLE", "BANANA");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);

    Xapian::Query query2(Xapian::Query::OP_VALUE_RANGE, 0, "WALRUS", "ZEBRA");
    enq.set_query(query2);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
}

static void
make_singularvalue_db(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    db.add_document(doc);
    doc.add_value(0, "SINGULAR");
    db.add_document(doc);
    db.add_document(doc);
}

// Check handling of bounds when bounds are equal.
DEFINE_TESTCASE(valuerange6, backend) {
    const auto OP_VALUE_RANGE = Xapian::Query::OP_VALUE_RANGE;
    Xapian::Database db = get_database("singularvalue", make_singularvalue_db);

    Xapian::Enquire enq(db);

    Xapian::Query query;
    query = Xapian::Query(OP_VALUE_RANGE, 0, "SATSUMA", "SLOE");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "PEACH", "PLUM");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "PEACH", "PEACH");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "PEACH", "PEACHERINE");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SING", "SINGULARITY");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SING", "SINGULAR");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULAR", "SINGULARITY");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULAR", "SINGULAR");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULARITY", "SINGULARITY");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULARITY", "SINGULARITIES");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULARITY", "SINNER");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGULARITY", "ZEBRA");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "SINGE", "SINGER");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);

    // Check no assertions when slot is empty.  Regression test for bug
    // introduced and fixed between 1.4.5 and 1.4.6.
    query = Xapian::Query(OP_VALUE_RANGE, 1, "MONK", "MONKEY");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    TEST_EQUAL(mset.get_matches_upper_bound(), 0);
}

static void
make_valprefixbounds_db(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    db.add_document(doc);
    doc.add_value(0, "ZERO");
    db.add_document(doc);
    doc.add_value(0, string("ZERO\0", 5));
    db.add_document(doc);
}

// Check handling of bounds when low is a prefix of high.
DEFINE_TESTCASE(valuerange7, backend) {
    const auto OP_VALUE_RANGE = Xapian::Query::OP_VALUE_RANGE;
    Xapian::Database db = get_database("valprefixbounds", make_valprefixbounds_db);

    Xapian::Enquire enq(db);

    Xapian::Query query;
    query = Xapian::Query(OP_VALUE_RANGE, 0, "ZAP", "ZOO");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);
    TEST_EQUAL(mset.get_matches_estimated(), 2);
    TEST_EQUAL(mset.get_matches_upper_bound(), 2);

    query = Xapian::Query(OP_VALUE_RANGE, 0, "ZAP", "ZERO");
    enq.set_query(query);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 1);
    if (db.size() > 1) {
	// The second shard will just have one document with "ZERO" in the slot
	// so we can tell there's exactly one match there, and the first shard
	// has one "ZERO\0" and one empty entry, so we can tell that can't
	// match.
	TEST_EQUAL(mset.get_matches_lower_bound(), 1);
	TEST_EQUAL(mset.get_matches_upper_bound(), 1);
    } else {
	TEST_EQUAL(mset.get_matches_lower_bound(), 0);
	TEST_EQUAL(mset.get_matches_upper_bound(), 2);
    }
}

// Feature test for Query::OP_VALUE_GE.
DEFINE_TESTCASE(valuege1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    static const char * const vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z"
    };
    for (auto start : vals) {
	Xapian::Query query(Xapian::Query::OP_VALUE_GE, 1, start);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, 20);
	// Check that documents in the MSet match the value range filter.
	set<Xapian::docid> matched;
	Xapian::MSetIterator i;
	for (i = mset.begin(); i != mset.end(); ++i) {
	    matched.insert(*i);
	    string value = db.get_document(*i).get_value(1);
	    tout << "'" << start << "' <= '" << value << "'\n";
	    TEST_REL(value,>=,start);
	}
	// Check that documents not in the MSet don't match the value range
	// filter.
	for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
	    if (matched.find(j) == matched.end()) {
		string value = db.get_document(j).get_value(1);
		tout << value << " < '" << start << "'\n";
		TEST_REL(value,<,start);
	    }
	}
    }
}

// Regression test for Query::OP_VALUE_GE - used to segfault if check() got
// called.
DEFINE_TESTCASE(valuege2, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_AND,
			Xapian::Query("what"),
			Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "aa"));
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);
}

// Feature test for Query::OP_VALUE_LE.
DEFINE_TESTCASE(valuele1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    static const char * const vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z"
    };
    for (auto end : vals) {
	Xapian::Query query(Xapian::Query::OP_VALUE_LE, 1, end);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, 20);
	// Check that documents in the MSet match the value range filter.
	set<Xapian::docid> matched;
	Xapian::MSetIterator i;
	for (i = mset.begin(); i != mset.end(); ++i) {
	    matched.insert(*i);
	    string value = db.get_document(*i).get_value(1);
	    TEST_REL(value,<=,end);
	}
	// Check that documents not in the MSet don't match the value range
	// filter.
	for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
	    if (matched.find(j) == matched.end()) {
		string value = db.get_document(j).get_value(1);
		TEST_REL(value,>,end);
	    }
	}
    }
}

// Check that Query(OP_VALUE_GE, 0, "") -> Query::MatchAll.
DEFINE_TESTCASE(valuege3, !backend) {
    Xapian::Query query(Xapian::Query::OP_VALUE_GE, 0, "");
    TEST_STRINGS_EQUAL(query.get_description(), Xapian::Query::MatchAll.get_description());
}

// Test Query::OP_VALUE_GE in a query which causes its skip_to() to be used.
DEFINE_TESTCASE(valuege4, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);

    // This query should put the ValueGePostList on the LHS of the AND because
    // it has a lower estimated termfreq than the term "fridg".  As a result,
    // the skip_to() method is used to advance the ValueGePostList.
    Xapian::Query query(Xapian::Query::OP_AND,
			Xapian::Query("fridg"),
			Xapian::Query(Xapian::Query::OP_VALUE_GE, 1, "aa"));
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);
}

// Test Query::OP_VALUE_RANGE in a query which causes its check() to be used.
DEFINE_TESTCASE(valuerange3, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_AND,
			Xapian::Query("what"),
			Xapian::Query(Xapian::Query::OP_VALUE_RANGE, 1,
				      "aa", "z"));
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);
}

// Test Query::OP_VALUE_RANGE in a query which causes its skip_to() to be used.
DEFINE_TESTCASE(valuerange4, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_AND,
			Xapian::Query("fridg"),
			Xapian::Query(Xapian::Query::OP_VALUE_RANGE, 1,
				      "aa", "z"));
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);
}

/// Test improved upper bound and estimate in 1.4.3.
DEFINE_TESTCASE(valuerangematchesub1, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enq(db);
    // Values present in slot 10 range from 'e' to 'w'.
    Xapian::Query query(Xapian::Query(Xapian::Query::OP_VALUE_RANGE, 10,
				      "h", "i"));
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 0);
    // The upper bound used to be db.size().
    TEST_EQUAL(mset.get_matches_upper_bound(), db.get_value_freq(10));
    TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    // The estimate used to be db.size() / 2, now it's calculated
    // proportional to the possible range.
    TEST_REL(mset.get_matches_estimated(), <=, db.get_doccount() / 3);
}
