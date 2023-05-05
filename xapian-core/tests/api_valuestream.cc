/** @file
 * @brief Tests of valuestream functionality.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#include "api_valuestream.h"

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

/// Feature test simple valuestream iteration.
DEFINE_TESTCASE(valuestream1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    for (Xapian::valueno slot = 0; slot < 15; ++slot) {
	tout << "testing valuestream iteration for slot " << slot << '\n';
	Xapian::ValueIterator it = db.valuestream_begin(slot);
	while (it != db.valuestream_end(slot)) {
	    TEST_EQUAL(it.get_valueno(), slot);
	    string value = *it;
	    Xapian::docid did = it.get_docid();

	    Xapian::Document doc = db.get_document(did);
	    TEST_EQUAL(doc.get_value(slot), value);

	    ++it;
	}
    }
}

/// Test skip_to() on a valuestream iterator.
DEFINE_TESTCASE(valuestream2, backend) {
    Xapian::Database db = get_database("etext");

    for (Xapian::valueno slot = 0; slot < 15; ++slot) {
	unsigned interval = 1;
	while (interval < 1999) {
	    tout.str(string());
	    tout << "testing valuestream skip_to for slot " << slot
		 << " with interval " << interval << '\n';
	    Xapian::docid did = 1;
	    Xapian::ValueIterator it = db.valuestream_begin(slot);
	    if (it == db.valuestream_end(slot)) break;
	    while (it.skip_to(did), it != db.valuestream_end(slot)) {
		TEST_EQUAL(it.get_valueno(), slot);
		string value = *it;

		// Check that the skipped documents had no values.
		Xapian::docid actual_did = it.get_docid();
		TEST_REL(actual_did,>=,did);
		while (did < actual_did) {
		    Xapian::Document doc = db.get_document(did);
		    TEST(doc.get_value(slot).empty());
		    ++did;
		}

		Xapian::Document doc = db.get_document(actual_did);
		TEST_EQUAL(doc.get_value(slot), value);
		did += interval;
	    }
	    interval = interval * 3 - 1;
	}
    }
}

/// Test check() on a valuestream iterator.
DEFINE_TESTCASE(valuestream3, backend) {
    Xapian::Database db = get_database("etext");

    // Check combinations of check with other operations.
    typedef enum {
	CHECK, CHECK_AND_NEXT, CHECK2, SKIP_TO, CHECK_AND_LOOP
    } test_op;
    test_op operation = CHECK;

    for (Xapian::valueno slot = 0; slot < 15; ++slot) {
	unsigned interval = 1;
	while (interval < 1999) {
	    tout << "testing valuestream check for slot " << slot
		 << " with interval " << interval << '\n';
	    Xapian::docid did = 1;
	    Xapian::ValueIterator it = db.valuestream_begin(slot);
	    if (it == db.valuestream_end(slot)) break;
	    while (true) {
		bool positioned = true;
		switch (operation) {
		    case CHECK_AND_LOOP:
			operation = CHECK;
			// FALLTHRU.
		    case CHECK: case CHECK2:
			positioned = it.check(did);
			break;
		    case CHECK_AND_NEXT: {
			bool was_skip_to = it.check(did);
			if (!was_skip_to) ++it;
			break;
		    }
		    case SKIP_TO:
			it.skip_to(did);
			break;
		}
		operation = test_op(operation + 1);
		if (positioned) {
		    if (it == db.valuestream_end(slot)) break;
		    TEST_EQUAL(it.get_valueno(), slot);
		    string value = *it;

		    // Check that the skipped documents had no values.
		    Xapian::docid actual_did = it.get_docid();
		    while (did < actual_did) {
			Xapian::Document doc = db.get_document(did);
			TEST(doc.get_value(slot).empty());
			++did;
		    }

		    Xapian::Document doc = db.get_document(actual_did);
		    TEST_EQUAL(doc.get_value(slot), value);
		}
		did += interval;
	    }
	    interval = interval * 3 - 1;
	}
    }
}

static void
gen_valueweightsource5_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_value(1, Xapian::sortable_serialise(3.14));
    db.replace_document(1, doc);
    db.replace_document(0xffffffff, doc);
}

/** Check that valueweightsource handles last_docid of 0xffffffff.
 *
 *  The original implementation went into an infinite loop in this case.
 */
DEFINE_TESTCASE(valueweightsource5, valuestats) {
    // inmemory's memory use is currently O(last_docid)!
    SKIP_TEST_FOR_BACKEND("inmemory");
    // remote's value slot iteration is very slow for this case currently
    // because it throws and catches DocNotFoundError across the link 2^32-3
    // times.
    if (contains(get_dbtype(), "remote"))
	SKIP_TEST("Testcase is too slow with remote shards");
    XFAIL_FOR_BACKEND("honey", "compaction needs to split sparse document length chunks");

    Xapian::Database db = get_database("valueweightsource5",
				       gen_valueweightsource5_db);
    Xapian::ValueWeightPostingSource src(1);
    src.init(db);
    src.next(0.0);
    TEST(!src.at_end());
    TEST_EQUAL(src.get_docid(), 1);
    src.next(0.0);
    TEST(!src.at_end());
    TEST_EQUAL(src.get_docid(), 0xffffffff);
    src.next(0.0);
    TEST(src.at_end());
}

// Check that ValueMapPostingSource works correctly.
// the test db has value 13 set to:
//      1   Thi
//      2   The
//      3   You
//      4   War
//      5   Fri
//      6   Ins
//      7   Whi
//      8   Com
//      9   A p
//      10  Tel
//      11  Tel
//      12  Enc
//      13  Get
//      14  Doe
//      15  fir
//      16  Pad
//      17  Pad
//
DEFINE_TESTCASE(valuemapsource1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);

    Xapian::ValueMapPostingSource src(13);
    src.add_mapping("Thi", 2.0);
    src.add_mapping("The", 1.0);
    src.add_mapping("You", 3.0);
    src.add_mapping("War", 4.0);
    src.add_mapping("Fri", 5.0);

    // check mset size and order
    enq.set_query(Xapian::Query(&src));
    Xapian::MSet mset = enq.get_mset(0, 5);

    TEST(mset.size() == 5);
    mset_expect_order(mset, 5, 4, 3, 1, 2);

    // and with default weight
    src.clear_mappings();
    src.set_default_weight(3.5);
    src.add_mapping("Thi", 2.0);
    src.add_mapping("The", 1.0);
    src.add_mapping("You", 3.0);
    src.add_mapping("War", 4.0);
    src.add_mapping("Fri", 5.0);

    enq.set_query(Xapian::Query(&src));
    mset = enq.get_mset(0, 5);

    TEST(mset.size() == 5);
    mset_expect_order(mset, 5, 4, 6, 7, 8);
}

// Regression test for valuepostingsource subclasses: used to segfault if skip_to()
// called on an empty list.
DEFINE_TESTCASE(valuemapsource2, backend && !multi) {
    Xapian::Database db(get_database("apitest_phrase"));

    {
	Xapian::ValueMapPostingSource src(100);
	src.init(db);
	TEST(src.at_end() == false);
	src.next(0.0);
	TEST(src.at_end() == true);
    }

    {
	Xapian::ValueMapPostingSource src(100);
	src.init(db);
	TEST(src.at_end() == false);
	src.skip_to(1, 0.0);
	TEST(src.at_end() == true);
    }

    {
	Xapian::ValueMapPostingSource src(100);
	src.init(db);
	TEST(src.at_end() == false);
	src.check(1, 0.0);
	TEST(src.at_end() == true);
    }
}

// Regression test for fixedweightpostingsource: used to segfault if skip_to()
// called on an empty list.
DEFINE_TESTCASE(fixedweightsource2, !backend) {
    Xapian::Database db;

    {
	Xapian::FixedWeightPostingSource src(5.0);
	src.init(db);
	TEST(src.at_end() == false);
	src.next(0.0);
	TEST(src.at_end() == true);
    }

    {
	Xapian::FixedWeightPostingSource src(5.0);
	src.init(db);
	TEST(src.at_end() == false);
	src.skip_to(1, 0.0);
	TEST(src.at_end() == true);
    }

    // No need to test behaviour of check() - check is only allowed to be
    // called with document IDs which exist, so can never be called for a
    // FixedWeightPostingSource with an empty database.
}

// Test DecreasingValueWeightPostingSource.
DEFINE_TESTCASE(decvalwtsource1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    doc.add_value(1, Xapian::sortable_serialise(3));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(2));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
    db.commit();

    // Check basic function
    {
	Xapian::DecreasingValueWeightPostingSource src(1);
	src.init(db);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 3);

	src.next(0.0);
	TEST(src.at_end());
    }

    // Check skipping to end of list due to weight
    {
	Xapian::DecreasingValueWeightPostingSource src(1);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(1.5);
	TEST(src.at_end());
    }

    // Check behaviour with a restricted range
    doc.add_value(1, Xapian::sortable_serialise(2));
    db.add_document(doc);

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 1, 3);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 4);

	src.next(1.5);
	TEST(src.at_end());
    }

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 1, 3);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.skip_to(3, 1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 4);

	src.next(1.5);
	TEST(src.at_end());
    }

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 1, 3);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	TEST(src.check(3, 1.5));
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 4);

	src.next(1.5);
	TEST(src.at_end());
    }
}

// Test DecreasingValueWeightPostingSource with out-of-order sections at
// start, and with repeated weights.
DEFINE_TESTCASE(decvalwtsource2, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(3));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(3));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
    db.commit();

    // Check basic function
    {
	Xapian::DecreasingValueWeightPostingSource src(1);
	src.init(db);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 3);

	src.next(0.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 4);

	src.next(0.0);
	TEST(src.at_end());
    }

    // Check skipping to end of list due to weight
    {
	Xapian::DecreasingValueWeightPostingSource src(1, 2);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 3);

	src.next(1.5);
	TEST(src.at_end());
    }

    // Check behaviour with a restricted range
    doc.add_value(1, Xapian::sortable_serialise(2));
    db.add_document(doc);

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 2, 4);
	src.init(db);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 3);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);

	src.next(1.5);
	TEST(src.at_end());
    }

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 2, 4);
	src.init(db);

	TEST(src.check(1, 1.5));
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	src.skip_to(4, 1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);

	src.next(1.5);
	TEST(src.at_end());
    }

    {
	Xapian::DecreasingValueWeightPostingSource src(1, 2, 4);
	src.init(db);

	TEST(src.check(1, 1.5));
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);

	src.next(1.5);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);

	TEST(src.check(4, 1.5));
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);

	src.next(1.5);
	TEST(src.at_end());
    }
}

static void
gen_decvalwtsource3_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_term("foo");
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(3));
    db.add_document(doc);
    doc.add_term("bar");
    doc.add_value(1, Xapian::sortable_serialise(3));
    db.add_document(doc);
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
}

// Test DecreasingValueWeightPostingSource with an actual query.
DEFINE_TESTCASE(decvalwtsource3, backend) {
    Xapian::Database db = get_database("decvalwtsource3",
				       gen_decvalwtsource3_db);

    Xapian::DecreasingValueWeightPostingSource ps(1, 2, 5);
    Xapian::Query q(&ps);
    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset1(enq.get_mset(0, 1));
    Xapian::MSet mset2(enq.get_mset(0, 2));
    Xapian::MSet mset3(enq.get_mset(0, 3));
    Xapian::MSet mset4(enq.get_mset(0, 4));

    TEST_EQUAL(mset1.size(), 1);
    TEST_EQUAL(mset2.size(), 2);
    TEST_EQUAL(mset3.size(), 3);
    TEST_EQUAL(mset4.size(), 4);

    TEST(mset_range_is_same(mset1, 0, mset2, 0, 1));
    TEST(mset_range_is_same(mset2, 0, mset3, 0, 2));
    TEST(mset_range_is_same(mset3, 0, mset4, 0, 3));
}

// Test DecreasingValueWeightPostingSource with an actual query on a fixed
// dataset (this was to cover the remote backend before we supported generated
// databases for remote databases).
DEFINE_TESTCASE(decvalwtsource4, backend && !multi) {
    Xapian::Database db = get_database("apitest_declen");

    Xapian::DecreasingValueWeightPostingSource ps(11, 2, 5);
    Xapian::Query q(&ps);
    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset1(enq.get_mset(0, 1));
    Xapian::MSet mset2(enq.get_mset(0, 2));
    Xapian::MSet mset3(enq.get_mset(0, 3));
    Xapian::MSet mset4(enq.get_mset(0, 4));

    TEST_EQUAL(mset1.size(), 1);
    TEST_EQUAL(mset2.size(), 2);
    TEST_EQUAL(mset3.size(), 3);
    TEST_EQUAL(mset4.size(), 4);

    TEST(mset_range_is_same(mset1, 0, mset2, 0, 1));
    TEST(mset_range_is_same(mset2, 0, mset3, 0, 2));
    TEST(mset_range_is_same(mset3, 0, mset4, 0, 3));
}

static void
gen_decvalwtsource5_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_value(1, Xapian::sortable_serialise(1));
    db.add_document(doc);
    doc.add_value(2, Xapian::sortable_serialise(1));
    db.add_document(doc);
}

// Regression test - used to get segfaults if
// DecreasingValueWeightPostingSource was pointed at an empty slot.
DEFINE_TESTCASE(decvalwtsource5, writable) {
    Xapian::Database db = get_database("decvalwtsource5",
				       gen_decvalwtsource5_db);

    {
	Xapian::DecreasingValueWeightPostingSource ps(1);
	Xapian::Query q(&ps);
	Xapian::Enquire enq(db);
	enq.set_query(q);
	Xapian::MSet mset1(enq.get_mset(0, 3));
	TEST_EQUAL(mset1.size(), 2);
    }
    {
	Xapian::DecreasingValueWeightPostingSource ps(2);
	Xapian::Query q(&ps);
	Xapian::Enquire enq(db);
	enq.set_query(q);
	Xapian::MSet mset1(enq.get_mset(0, 3));
	TEST_EQUAL(mset1.size(), 1);
    }
    {
	Xapian::DecreasingValueWeightPostingSource ps(3);
	Xapian::Query q(&ps);
	Xapian::Enquire enq(db);
	enq.set_query(q);
	Xapian::MSet mset1(enq.get_mset(0, 3));
	TEST_EQUAL(mset1.size(), 0);
    }
}
