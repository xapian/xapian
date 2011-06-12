/** @file api_opvalue.cc
 * @brief Tests of the OP_VALUE_* query operators.
 */
/* Copyright 2007,2008,2009,2010,2010 Olly Betts
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
    static const char * vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z", NULL
    };
    for (const char **start = vals; *start; ++start) {
	for (const char **end = vals; *end; ++end) {
	    Xapian::Query query(Xapian::Query::OP_VALUE_RANGE, 1, *start, *end);
	    enq.set_query(query);
	    Xapian::MSet mset = enq.get_mset(0, 20);
	    // Check that documents in the MSet match the value range filter.
	    set<Xapian::docid> matched;
	    Xapian::MSetIterator i;
	    for (i = mset.begin(); i != mset.end(); ++i) {
		matched.insert(*i);
		string value = db.get_document(*i).get_value(1);
		TEST_REL(value,>=,*start);
		TEST_REL(value,<=,*end);
	    }
	    // Check that documents not in the MSet don't match the value range filter.
	    for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
		if (matched.find(j) == matched.end()) {
		    string value = db.get_document(j).get_value(1);
		    tout << value << " < '" << *start << "' or > '" << *end << "'" << endl;
		    TEST(value < *start || value > *end);
		}
	    }
	}
    }
    return true;
}

// Regression test for Query::OP_VALUE_LE - used to return document IDs for
// non-existent documents.
DEFINE_TESTCASE(valuerange2, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.set_data("5");
    doc.add_value(0, "5");
    db.replace_document(5, doc);
    Xapian::Enquire enq(db);

    Xapian::Query query(Xapian::Query::OP_VALUE_LE, 0, "6");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 20);

    TEST_EQUAL(mset.size(), 1);
    TEST_EQUAL(*(mset[0]), 5);
    return true;
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
DEFINE_TESTCASE(valuerange5, generated) {
    Xapian::Database db = get_database("valuerange5", make_valuerange5);
    if (db.get_value_lower_bound(0).empty()) {
	// This backend (i.e. flint) doesn't support value bounds, or else
	// there are no instances of this value.  In the former case,
	// get_value_upper_bound() should throw an exception.
	TEST_EXCEPTION(Xapian::UnimplementedError,
		db.get_value_upper_bound(0));
	return true;
    }

    Xapian::Enquire enq(db);

    Xapian::Query query(Xapian::Query::OP_VALUE_RANGE, 0, "APPLE", "BANANA");
    enq.set_query(query);
    Xapian::MSet mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);

    Xapian::Query query2(Xapian::Query::OP_VALUE_RANGE, 0, "WALRUS", "ZEBRA");
    enq.set_query(query2);
    mset = enq.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);

    return true;
}

// Feature test for Query::OP_VALUE_GE.
DEFINE_TESTCASE(valuege1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    static const char * vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z", NULL
    };
    for (const char **start = vals; *start; ++start) {
	Xapian::Query query(Xapian::Query::OP_VALUE_GE, 1, *start);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, 20);
	// Check that documents in the MSet match the value range filter.
	set<Xapian::docid> matched;
	Xapian::MSetIterator i;
	for (i = mset.begin(); i != mset.end(); ++i) {
	    matched.insert(*i);
	    string value = db.get_document(*i).get_value(1);
	    tout << "'" << *start << "' <= '" << value << "'" << endl;
	    TEST_REL(value,>=,*start);
	}
	// Check that documents not in the MSet don't match the value range
	// filter.
	for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
	    if (matched.find(j) == matched.end()) {
		string value = db.get_document(j).get_value(1);
		tout << value << " < '" << *start << "'" << endl;
		TEST_REL(value,<,*start);
	    }
	}
    }
    return true;
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
    return true;
}

// Feature test for Query::OP_VALUE_LE.
DEFINE_TESTCASE(valuele1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    static const char * vals[] = {
	"", " ", "a", "aa", "abcd", "e", "g", "h", "hzz", "i", "l", "z", NULL
    };
    for (const char **end = vals; *end; ++end) {
	Xapian::Query query(Xapian::Query::OP_VALUE_LE, 1, *end);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, 20);
	// Check that documents in the MSet match the value range filter.
	set<Xapian::docid> matched;
	Xapian::MSetIterator i;
	for (i = mset.begin(); i != mset.end(); ++i) {
	    matched.insert(*i);
	    string value = db.get_document(*i).get_value(1);
	    TEST_REL(value,<=,*end);
	}
	// Check that documents not in the MSet don't match the value range
	// filter.
	for (Xapian::docid j = db.get_lastdocid(); j != 0; --j) {
	    if (matched.find(j) == matched.end()) {
		string value = db.get_document(j).get_value(1);
		TEST_REL(value,>,*end);
	    }
	}
    }
    return true;
}

// Check that Query(OP_VALUE_GE, 0, "") -> Query::MatchAll.
DEFINE_TESTCASE(valuege3, !backend) {
    Xapian::Query query(Xapian::Query::OP_VALUE_GE, 0, "");
    TEST_STRINGS_EQUAL(query.get_description(), Xapian::Query::MatchAll.get_description());
    return true;
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
    return true;
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
    return true;
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
    return true;
}
