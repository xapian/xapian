/* api_valuestats.cc: tests of the value statistics functions.
 *
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008,2009,2011 Olly Betts
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

#include "api_valuestats.h"

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

/// Test of value statistics methods.
DEFINE_TESTCASE(valuestats1, writable && valuestats) {
    Xapian::WritableDatabase db_w = get_writable_database();

    // Check that counts are initially zero.
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");

    Xapian::Document doc;
    doc.add_value(0, "hello");

    // Check that statistics for the correct value slot increase when document
    // is added.  (Check slot 1 first, so that cache invalidation of the last
    // slot read also gets checked.)
    db_w.add_document(doc);
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");
    TEST_EQUAL(db_w.get_value_freq(0), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "hello");

    // Check that statistics work correctly when second document is added.
    doc = Xapian::Document();
    doc.add_value(0, "world");
    doc.add_value(1, "cheese");
    db_w.replace_document(2, doc);
    TEST_EQUAL(db_w.get_value_freq(0), 2);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "world");
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "cheese");

    // Deleting a document affects the count, but not the bounds.
    db_w.delete_document(1);
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_freq(0), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "world");

    // Deleting all the documents returns the bounds to their original value.
    db_w.delete_document(2);
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");

    // Adding a document with a value in one of the old slots should still
    // end up with tight bounds on it.
    doc = Xapian::Document();
    doc.add_value(1, "newval");
    db_w.replace_document(2, doc);
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "newval");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "newval");
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");

    return true;
}

/// Test that value statistics stuff obeys transactions.
DEFINE_TESTCASE(valuestats2, transactions && valuestats) {
    Xapian::WritableDatabase db_w = get_writable_database();
    Xapian::Database db = get_writable_database_as_database();

    // Check that counts are initially zero.
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");
    TEST_EQUAL(db.get_value_freq(0), 0);
    TEST_EQUAL(db.get_value_lower_bound(0), "");
    TEST_EQUAL(db.get_value_upper_bound(0), "");
    TEST_EQUAL(db.get_value_freq(1), 0);
    TEST_EQUAL(db.get_value_lower_bound(1), "");
    TEST_EQUAL(db.get_value_upper_bound(1), "");

    Xapian::Document doc;
    doc.add_value(0, "hello");

    // Check that statistics for the correct value slot increase when document
    // is added.  (Check slot 1 first, so that cache invalidation of the last
    // slot read also gets checked.)
    db_w.add_document(doc);
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");
    TEST_EQUAL(db_w.get_value_freq(0), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "hello");

    // The readonly database shouldn't change, though.
    TEST_EQUAL(db.get_value_freq(1), 0);
    TEST_EQUAL(db.get_value_lower_bound(1), "");
    TEST_EQUAL(db.get_value_upper_bound(1), "");
    TEST_EQUAL(db.get_value_freq(0), 0);
    TEST_EQUAL(db.get_value_lower_bound(0), "");
    TEST_EQUAL(db.get_value_upper_bound(0), "");

    // Check that statistics work correctly when second document is added.
    doc = Xapian::Document();
    doc.add_value(0, "world");
    doc.add_value(1, "cheese");
    db_w.replace_document(2, doc);
    TEST_EQUAL(db_w.get_value_freq(0), 2);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "world");
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "cheese");

    // The readonly database shouldn't change, though.
    TEST_EQUAL(db.get_value_freq(0), 0);
    TEST_EQUAL(db.get_value_lower_bound(0), "");
    TEST_EQUAL(db.get_value_upper_bound(0), "");
    TEST_EQUAL(db.get_value_freq(1), 0);
    TEST_EQUAL(db.get_value_lower_bound(1), "");
    TEST_EQUAL(db.get_value_upper_bound(1), "");

    // Check that readonly database catches up when a commit is done.
    db_w.commit();
    TEST(db.reopen());
    TEST_EQUAL(db.get_value_freq(1), 1);
    TEST_EQUAL(db.get_value_lower_bound(1), "cheese");
    TEST_EQUAL(db.get_value_upper_bound(1), "cheese");
    TEST_EQUAL(db.get_value_freq(0), 2);
    TEST_EQUAL(db.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db.get_value_upper_bound(0), "world");

    // Deleting a document affects the count, but not the bounds.
    db_w.delete_document(1);
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "cheese");
    TEST_EQUAL(db_w.get_value_freq(0), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "hello");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "world");

    // Deleting all the documents returns the bounds to their original value.
    db_w.delete_document(2);
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");
    TEST_EQUAL(db_w.get_value_freq(1), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "");

    // Adding a document with a value in one of the old slots should still
    // end up with tight bounds on it.
    doc = Xapian::Document();
    doc.add_value(1, "newval");
    db_w.replace_document(2, doc);
    TEST_EQUAL(db_w.get_value_freq(1), 1);
    TEST_EQUAL(db_w.get_value_lower_bound(1), "newval");
    TEST_EQUAL(db_w.get_value_upper_bound(1), "newval");
    TEST_EQUAL(db_w.get_value_freq(0), 0);
    TEST_EQUAL(db_w.get_value_lower_bound(0), "");
    TEST_EQUAL(db_w.get_value_upper_bound(0), "");

    // Check that a readonly database gets the right statistics, too.
    db_w.commit();
    TEST(db.reopen());
    TEST_EQUAL(db.get_value_freq(0), 0);
    TEST_EQUAL(db.get_value_lower_bound(0), "");
    TEST_EQUAL(db.get_value_upper_bound(0), "");
    TEST_EQUAL(db.get_value_freq(1), 1);
    TEST_EQUAL(db.get_value_lower_bound(1), "newval");
    TEST_EQUAL(db.get_value_upper_bound(1), "newval");

    return true;
}

/// Test reading value statistics from prebuilt databases.
DEFINE_TESTCASE(valuestats3, valuestats) {
    Xapian::Database db = get_database("apitest_simpledata");

    TEST_EQUAL(db.get_value_freq(1), 6);
    TEST_EQUAL(db.get_value_lower_bound(1), "h");
    TEST_EQUAL(db.get_value_upper_bound(1), "n");
    TEST_EQUAL(db.get_value_freq(2), 6);
    TEST_EQUAL(db.get_value_lower_bound(2), "d");
    TEST_EQUAL(db.get_value_upper_bound(2), "i");
    TEST_EQUAL(db.get_value_freq(3), 6);
    TEST_EQUAL(db.get_value_lower_bound(3), " ");
    TEST_EQUAL(db.get_value_upper_bound(3), "s");
    TEST_EQUAL(db.get_value_freq(4), 6);
    TEST_EQUAL(db.get_value_lower_bound(4), " ");
    TEST_EQUAL(db.get_value_upper_bound(4), "y");
    TEST_EQUAL(db.get_value_freq(5), 6);
    TEST_EQUAL(db.get_value_lower_bound(5), "e");
    TEST_EQUAL(db.get_value_upper_bound(5), "p");
    TEST_EQUAL(db.get_value_freq(6), 6);
    TEST_EQUAL(db.get_value_lower_bound(6), "a");
    TEST_EQUAL(db.get_value_upper_bound(6), "t");
    TEST_EQUAL(db.get_value_freq(7), 6);
    TEST_EQUAL(db.get_value_lower_bound(7), " ");
    TEST_EQUAL(db.get_value_upper_bound(7), "r");
    TEST_EQUAL(db.get_value_freq(8), 6);
    TEST_EQUAL(db.get_value_lower_bound(8), "a");
    TEST_EQUAL(db.get_value_upper_bound(8), "t");
    TEST_EQUAL(db.get_value_freq(9), 6);
    TEST_EQUAL(db.get_value_lower_bound(9), " ");
    TEST_EQUAL(db.get_value_upper_bound(9), "n");
    TEST_EQUAL(db.get_value_freq(10), 6);
    TEST_EQUAL(db.get_value_lower_bound(10), "e");
    TEST_EQUAL(db.get_value_upper_bound(10), "w");
    TEST_EQUAL(db.get_value_freq(11), 6);
    TEST_EQUAL(db.get_value_lower_bound(11), "\xb9P");
    TEST_EQUAL(db.get_value_upper_bound(11), "\xc7\x04");

    return true;
}

DEFINE_TESTCASE(valuestats4, transactions && valuestats) {
    const size_t FLUSH_THRESHOLD = 10000;
    {
	Xapian::WritableDatabase db_w = get_writable_database();
	Xapian::Document doc;
	doc.add_value(1, "test");
	for (size_t i = 0; i < FLUSH_THRESHOLD; ++i) {
	    db_w.add_document(doc);
	}

	Xapian::Database db = get_writable_database_as_database();
	// Check that we had an automatic-commit.
	TEST_EQUAL(db.get_doccount(), FLUSH_THRESHOLD);
	// Check that the value stats are there.
	TEST_EQUAL(db.get_value_freq(1), FLUSH_THRESHOLD);
	TEST_EQUAL(db.get_value_lower_bound(1), "test");
	TEST_EQUAL(db.get_value_upper_bound(1), "test");

	db_w.begin_transaction();
	doc.add_value(1, "umbrella");
	db_w.cancel_transaction();
    }

    {
	Xapian::Database db = get_writable_database_as_database();
	// Check that we had an automatic-commit.
	TEST_EQUAL(db.get_doccount(), FLUSH_THRESHOLD);
	// Check that the value stats are there.
	TEST_EQUAL(db.get_value_freq(1), FLUSH_THRESHOLD);
	TEST_EQUAL(db.get_value_lower_bound(1), "test");
	TEST_EQUAL(db.get_value_upper_bound(1), "test");
    }

    return true;
}

/// Regression test for bug fixed in 1.1.1 which led to incorrect valuestats.
DEFINE_TESTCASE(valuestats5, !backend) {
    Xapian::Document doc;
    doc.add_value(0, "zero");
    doc.add_value(1, "one");
    doc.add_value(2, "two");
    doc.add_value(3, "three");
    doc.add_value(4, "");
    doc.add_value(5, "five");
    doc.remove_value(3);
    doc.add_value(1, "");

    // Check that we don't have any empty values reported.
    size_t c = 0;
    Xapian::ValueIterator v = doc.values_begin();
    while (v != doc.values_end()) {
	TEST(!(*v).empty());
	++c;
	++v;
    }
    TEST_EQUAL(c, 3); // 0, 2, 5

    return true;
}
