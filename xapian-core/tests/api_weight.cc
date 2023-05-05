/** @file
 * @brief tests of Xapian::Weight subclasses
 */
/* Copyright (C) 2004,2012,2013,2016,2017,2019 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2016 Vivek Pal
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

#include "api_weight.h"
#include <cmath>

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(tradweight3, !backend) {
    Xapian::TradWeight wt(42);
    try {
	Xapian::TradWeight t;
	Xapian::TradWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised TradWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised TradWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	// Regression test for error in exception message fixed in 1.2.11 and
	// 1.3.1.
	TEST(e.get_msg().find("BM25") == string::npos);
	TEST(e.get_msg().find("Trad") != string::npos);
    }
}

// Test Exception for junk after serialised weight.
DEFINE_TESTCASE(unigramlmweight3, !backend) {
    Xapian::LMWeight wt(79898.0, Xapian::Weight::JELINEK_MERCER_SMOOTHING, 0.5, 1.0);
    try {
	Xapian::LMWeight t;
	Xapian::LMWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised LMWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised LMWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("LM") != string::npos);
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(bm25weight3, !backend) {
    Xapian::BM25Weight wt(2.0, 0.5, 1.3, 0.6, 0.01);
    try {
	Xapian::BM25Weight b;
	Xapian::BM25Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised BM25Weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised BM25Weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("BM25") != string::npos);
    }
}

// Test parameter combinations which should be unaffected by doclength.
DEFINE_TESTCASE(bm25weight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::BM25Weight(1, 0, 1, 0, 0.5));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: wdf has an effect on weight, but doclen doesn't.
    TEST_REL(mset[0].get_weight(),>,mset[1].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), mset[2].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());
    TEST_EQUAL_DOUBLE(mset[3].get_weight(), mset[4].get_weight());

    enquire.set_weighting_scheme(Xapian::BM25Weight(0, 0, 1, 1, 0.5));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: neither wdf nor doclen affects weight.
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), mset[4].get_weight());
}

/// Test non-zero k2 with zero k1.
// Regression test for bug fixed in 1.2.17 and 1.3.2.
DEFINE_TESTCASE(bm25weight5, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::BM25Weight(0, 1, 1, 0.5, 0.5));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: wdf has no effect on weight; shorter docs rank higher.
    mset_expect_order(mset, 3, 5, 1, 4, 2);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), mset[1].get_weight());
    TEST_REL(mset[1].get_weight(),>,mset[2].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());
    TEST_REL(mset[3].get_weight(),>,mset[4].get_weight());
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(bm25plusweight1, !backend) {
    Xapian::BM25PlusWeight wt(2.0, 0.1, 1.3, 0.6, 0.01, 0.5);
    try {
	Xapian::BM25PlusWeight b;
	Xapian::BM25PlusWeight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised BM25PlusWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised BM25PlusWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("BM25Plus") != string::npos);
    }
}

// Test parameter combinations which should be unaffected by doclength.
DEFINE_TESTCASE(bm25plusweight2, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::BM25PlusWeight(1, 0, 1, 0, 0.5, 1));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: wdf has an effect on weight, but doclen doesn't.
    TEST_REL(mset[0].get_weight(),>,mset[1].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), mset[2].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());
    TEST_EQUAL_DOUBLE(mset[3].get_weight(), mset[4].get_weight());

    enquire.set_weighting_scheme(Xapian::BM25PlusWeight(0, 0, 1, 1, 0.5, 1));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expect: neither wdf nor doclen affects weight.
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), mset[4].get_weight());
}

// Regression test for a mistake corrected in the BM25+ implementation.
DEFINE_TESTCASE(bm25plusweight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::BM25PlusWeight(1, 0, 1, 0.5, 0.5, 1));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);

    // The value of each doc weight calculated manually from the BM25+ formulae
    // by using the respective document statistics.
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 0.7920796567487473);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 0.7846980783848447);
    TEST_EQUAL_DOUBLE(mset[2].get_weight(), 0.7558817623365934);
    TEST_EQUAL_DOUBLE(mset[3].get_weight(), 0.7210119356168847);
    TEST_EQUAL_DOUBLE(mset[4].get_weight(), 0.7210119356168847);
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(inl2weight1, !backend) {
    Xapian::InL2Weight wt(2.0);
    try {
	Xapian::InL2Weight b;
	Xapian::InL2Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised inl2weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised inl2weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("InL2") != string::npos);
    }
}

// Test for invalid values of c.
DEFINE_TESTCASE(inl2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::InL2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::InL2Weight wt2(0.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::InL2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::InL2Weight(1.0).serialise());
}

// Feature tests for Inl2Weight
DEFINE_TESTCASE(inl2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("banana");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::InL2Weight(2.0));

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 1);
    mset_expect_order(mset1, 6);

    /* The value has been calculated in the python interpreter by looking at the
     * database statistics. */
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 1.559711143842063);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::InL2Weight(2.0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 1);
    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    TEST_EQUAL_DOUBLE(15.0 * mset1[0].get_weight(), mset2[0].get_weight());
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(ifb2weight1, !backend) {
    Xapian::IfB2Weight wt(2.0);
    try {
	Xapian::IfB2Weight b;
	Xapian::IfB2Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised IfB2Weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised IfB2Weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("IfB2") != string::npos);
    }
}

// Test for invalid values of c.
DEFINE_TESTCASE(ifb2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IfB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IfB2Weight wt2(0.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::IfB2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::IfB2Weight(1.0).serialise());
}

// Feature test
DEFINE_TESTCASE(ifb2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("banana");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::IfB2Weight(2.0));

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 1);

    /* The value of the weight has been manually calculated using the statistics
     * of the test database. */
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 3.119422287684126);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::IfB2Weight(2.0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 1);
    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    TEST_EQUAL_DOUBLE(15.0 * mset1[0].get_weight(), mset2[0].get_weight());
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(ineb2weight1, !backend) {
    Xapian::IneB2Weight wt(2.0);
    try {
	Xapian::IneB2Weight b;
	Xapian::IneB2Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised ineb2weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised ineb2weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("IneB2") != string::npos);
    }
}

// Test for invalid values of c.
DEFINE_TESTCASE(ineb2weight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IneB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IneB2Weight wt2(0.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::IneB2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::IneB2Weight(1.0).serialise());
}

// Feature test.
DEFINE_TESTCASE(ineb2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::IneB2Weight(2.0));

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);

    // The third document in the database is 4th in the ranking.
    /* The weight value has been manually calculated by using the statistics
     * of the test database. */
    TEST_EQUAL_DOUBLE(mset1[4].get_weight(), 0.61709730297692400036);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::IneB2Weight(2.0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);

    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(bb2weight1, !backend) {
    Xapian::BB2Weight wt(2.0);
    try {
	Xapian::BB2Weight b;
	Xapian::BB2Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised BB2Weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised BB2Weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("BB2") != string::npos);
    }
}

// Test for invalid values of c.
DEFINE_TESTCASE(bb2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::BB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::BB2Weight wt2(0.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::BB2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::BB2Weight(1.0).serialise());
}

// Feature test
DEFINE_TESTCASE(bb2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::BB2Weight(2.0));

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);
    /* The third document in the database has the highest weight and is the
     * first in the mset. */
    // Value calculated manually by using the statistics of the test database.
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 1.6823696969784483);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::BB2Weight(2.0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);

    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }

    // Test with OP_SCALE_WEIGHT and a small factor (regression test, as we
    // were applying the factor to the upper bound twice).
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 1.0 / 1024));
    enquire.set_weighting_scheme(Xapian::BB2Weight(2.0));

    Xapian::MSet mset3;
    mset3 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset3.size(), 5);

    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(), mset3[i].get_weight() * 1024);
    }
}

// Regression test: we used to calculate log2(0) when there was only one doc.
DEFINE_TESTCASE(bb2weight4, backend) {
    Xapian::Database db = get_database("apitest_onedoc");
    Xapian::Enquire enquire(db);
    Xapian::Query query("word");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::BB2Weight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 1);
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 3.431020621347435);
}

// Feature test.
DEFINE_TESTCASE(dlhweight1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("a");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DLHWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 3);
    mset_expect_order(mset1, 3, 1, 2);
    // Weights calculated manually using stats from the database.
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 1.0046477754371292362);
    TEST_EQUAL_DOUBLE(mset1[1].get_weight(), 0.97621929514640352757);
    // The following weight would be negative but gets clamped to 0.
    TEST_EQUAL_DOUBLE(mset1[2].get_weight(), 0.0);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::DLHWeight());

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 3);

    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    for (Xapian::doccount i = 0; i < mset2.size(); ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(dlhweight2, !backend) {
    Xapian::DLHWeight wt;
    try {
	Xapian::DLHWeight t;
	Xapian::DLHWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised DLHWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised DLHWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("DLH") != string::npos);
    }
}

static void
gen_wdf_eq_doclen_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_term("solo", 37);
    db.add_document(doc);
}

// Test wdf == doclen.
DEFINE_TESTCASE(dlhweight3, backend) {
    Xapian::Database db = get_database("wdf_eq_doclen", gen_wdf_eq_doclen_db);
    Xapian::Enquire enquire(db);
    Xapian::Query query("solo");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DLHWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 1);
    // Weight gets clamped to zero.
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(pl2weight1, !backend) {
    Xapian::PL2Weight wt(2.0);
    try {
	Xapian::PL2Weight b;
	Xapian::PL2Weight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised PL2Weight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised PL2Weight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("PL2") != string::npos);
    }
}

// Test for invalid values of c.
DEFINE_TESTCASE(pl2weight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::PL2Weight wt(-2.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::PL2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::PL2Weight(1.0).serialise());
}

// Feature Test.
DEFINE_TESTCASE(pl2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");
    enquire.set_query(query);
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::PL2Weight(2.0));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expected weight difference calculated in extended precision using stats
    // from the test database.
    TEST_EQUAL_DOUBLE(mset[2].get_weight(),
		      mset[3].get_weight() + 0.0086861771701328694);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::PL2Weight(2.0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);
    TEST_NOT_EQUAL_DOUBLE(mset[0].get_weight(), 0.0);
    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset[i].get_weight(), mset2[i].get_weight());
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(pl2plusweight1, !backend) {
    Xapian::PL2PlusWeight wt(2.0, 0.9);
    try {
	Xapian::PL2PlusWeight b;
	Xapian::PL2PlusWeight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised PL2PlusWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised PL2PlusWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("PL2Plus") != string::npos);
    }
}

// Test for invalid values of parameters, c and delta.
DEFINE_TESTCASE(pl2plusweight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::PL2PlusWeight wt(-2.0, 0.9));

    // InvalidArgumentError should be thrown if parameter delta is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::PL2PlusWeight wt(1.0, -1.9));
}

// Test for default values of parameters, c and delta.
DEFINE_TESTCASE(pl2plusweight3, !backend) {
    Xapian::PL2PlusWeight weight2;

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    TEST_EQUAL(weight2.serialise(), Xapian::PL2PlusWeight(1.0, 0.8).serialise());

    /* Parameter delta should be set to 0.8 by constructor if none is given. */
    TEST_EQUAL(weight2.serialise(), Xapian::PL2PlusWeight(1.0, 0.8).serialise());
}

// Feature Test 1 for PL2PlusWeight.
DEFINE_TESTCASE(pl2plusweight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::PL2PlusWeight(2.0, 0.8));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expected weight difference calculated in extended precision using stats
    // from the test database.
    TEST_EQUAL_DOUBLE(mset[2].get_weight(),
		      mset[3].get_weight() + 0.0086861771701328694);
}

// Feature Test 2 for PL2PlusWeight
DEFINE_TESTCASE(pl2plusweight5, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("word");
    enquire.set_query(query);
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::PL2PlusWeight(1.0, 0.8));
    mset = enquire.get_mset(0, 10);
    // Expect MSet contains two documents having query "word".
    TEST_EQUAL(mset.size(), 2);
    // Expect Document 2 has higher weight than document 4 because
    // "word" appears more no. of times in document 2 than document 4.
    mset_expect_order(mset, 2, 4);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::PL2PlusWeight(1.0, 0.8));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), mset.size());
    TEST_NOT_EQUAL_DOUBLE(mset[0].get_weight(), 0.0);
    for (Xapian::doccount i = 0; i < mset.size(); ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset[i].get_weight(), mset2[i].get_weight());
    }
}

// Feature test
DEFINE_TESTCASE(dphweight1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DPHWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);
    /* The weight has been calculated manually by using the statistics of the
     * test database. */
    TEST_EQUAL_DOUBLE(mset1[2].get_weight() - mset1[4].get_weight(), 0.542623617687990167);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::DPHWeight());

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);
    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(dphweight2, !backend) {
    Xapian::DPHWeight wt;
    try {
	Xapian::DPHWeight t;
	Xapian::DPHWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised DPHWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised DPHWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("DPH") != string::npos);
    }
}

// Test wdf == doclen.
DEFINE_TESTCASE(dphweight3, backend) {
    Xapian::Database db = get_database("wdf_eq_doclen", gen_wdf_eq_doclen_db);
    Xapian::Enquire enquire(db);
    Xapian::Query query("solo");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DPHWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 1);
    // Weight gets clamped to zero.
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
}

// Test for various cases of normalization string.
DEFINE_TESTCASE(tfidfweight1, !backend) {
    // InvalidArgumentError should be thrown if normalization string is invalid
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::TfIdfWeight b("JOHN_LENNON"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::TfIdfWeight b("LOL"));

    /* Normalization string should be set to "ntn" by constructor if none is
      given. */
    Xapian::TfIdfWeight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::TfIdfWeight("ntn").serialise());
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(tfidfweight2, !backend) {
    Xapian::TfIdfWeight wt("ntn");
    try {
	Xapian::TfIdfWeight b;
	Xapian::TfIdfWeight * b2 = b.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = b2->name().empty();
	delete b2;
	if (empty)
	    FAIL_TEST("Serialised TfIdfWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised TfIdfWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("TfIdf") != string::npos);
    }
}

// Feature tests for various normalization functions.
DEFINE_TESTCASE(tfidfweight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("word");
    Xapian::MSet mset;

    // Check for "ntn" when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ntn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 * log(6.0 / 2));

    // Check that wqf is taken into account.
    enquire.set_query(Xapian::Query("word", 2));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ntn"));
    Xapian::MSet mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 2);
    // wqf is 2, so weights should be doubled.
    TEST_EQUAL_DOUBLE(mset[0].get_weight() * 2, mset2[0].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight() * 2, mset2[1].get_weight());

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ntn"));
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 2);
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset2, 2, 4);
    TEST_NOT_EQUAL_DOUBLE(mset[0].get_weight(), 0.0);
    TEST_EQUAL_DOUBLE(15 * mset[0].get_weight(), mset2[0].get_weight());

    // check for "nfn" when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("nfn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 / 2);

    // check for "nsn" when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("nsn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 * pow(log(6.0 / 2), 2.0));

    // Check for "bnn" and for both branches of 'b'.
    enquire.set_query(Xapian::Query("test"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("bnn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 1);
    mset_expect_order(mset, 1);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 1.0);

    // Check for "lnn" and for both branches of 'l'.
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("lnn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 1 + log(8.0)); // idfn=1 and so wt=tfn=1+log(tf)
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1.0);         // idfn=1 and wt=tfn=1+log(tf)=1+log(1)=1

    // Check for "snn"
    enquire.set_query(Xapian::Query("paragraph"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("snn")); // idf=1 and tfn=tf*tf
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    mset_expect_order(mset, 2, 1, 4, 3, 5);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 9.0);
    TEST_EQUAL_DOUBLE(mset[4].get_weight(), 1.0);

    // Check for "ntn" when termfreq=N
    enquire.set_query(Xapian::Query("this"));  // N=termfreq and so idfn=0 for "t"
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ntn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 6);
    mset_expect_order(mset, 1, 2, 3, 4, 5, 6);
    for (int i = 0; i < 6; ++i) {
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), 0.0);
    }

    // Check for "npn" and for both branches of 'p'
    enquire.set_query(Xapian::Query("this"));  // N=termfreq and so idfn=0 for "p"
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("npn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 6);
    mset_expect_order(mset, 1, 2, 3, 4, 5, 6);
    for (int i = 0; i < 6; ++i) {
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), 0.0);
    }

    // Check for "Lnn".
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("Lnn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), (1 + log(8.0)) / (1 + log(81.0 / 56.0)));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), (1 + log(1.0)) / (1 + log(31.0 / 26.0)));

    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("npn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * log((6.0 - 2) / 2));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * log((6.0 - 2) / 2));
}

class CheckInitWeight : public Xapian::Weight {
  public:
    double factor;

    unsigned & zero_inits, & non_zero_inits;

    CheckInitWeight(unsigned &z, unsigned &n)
	: factor(-1.0), zero_inits(z), non_zero_inits(n) { }

    void init(double factor_) {
	factor = factor_;
	if (factor == 0.0)
	    ++zero_inits;
	else
	    ++non_zero_inits;
    }

    Weight * clone() const {
	return new CheckInitWeight(zero_inits, non_zero_inits);
    }

    double get_sumpart(Xapian::termcount, Xapian::termcount,
		       Xapian::termcount) const {
	return 1.0;
    }

    double get_maxpart() const { return 1.0; }

    double get_sumextra(Xapian::termcount doclen, Xapian::termcount) const {
	return 1.0 / doclen;
    }

    double get_maxextra() const { return 1.0; }
};

/// Regression test - check init() is called for the term-indep Weight obj.
DEFINE_TESTCASE(checkinitweight1, backend && !multi && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query q(Xapian::Query::OP_AND,
		    Xapian::Query("this"), Xapian::Query("paragraph"));
    enquire.set_query(q);
    unsigned zero_inits = 0, non_zero_inits = 0;
    CheckInitWeight wt(zero_inits, non_zero_inits);
    enquire.set_weighting_scheme(wt);
    Xapian::MSet mset = enquire.get_mset(0, 3);
    TEST_EQUAL(zero_inits, 1);
    TEST_EQUAL(non_zero_inits, 2);
}

class CheckStatsWeight : public Xapian::Weight {
  public:
    double factor;

    Xapian::Database db;

    string term1;

    // When testing OP_SYNONYM, term2 is also set.
    // When testing OP_WILDCARD, term2 == "*".
    // When testing a repeated term, term2 == "=" for the first occurrence and
    // "_" for subsequent occurrences.
    mutable string term2;

    Xapian::termcount & sum;
    Xapian::termcount & sum_squares;

    mutable Xapian::termcount len_upper;
    mutable Xapian::termcount len_lower;
    mutable Xapian::termcount wdf_upper;

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term1_,
		     const string & term2_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: factor(-1.0), db(db_), term1(term1_), term2(term2_),
	  sum(sum_), sum_squares(sum_squares_),
	  len_upper(0), len_lower(Xapian::termcount(-1)), wdf_upper(0)
    {
	need_stat(COLLECTION_SIZE);
	need_stat(RSET_SIZE);
	need_stat(AVERAGE_LENGTH);
	need_stat(TERMFREQ);
	need_stat(RELTERMFREQ);
	need_stat(QUERY_LENGTH);
	need_stat(WQF);
	need_stat(WDF);
	need_stat(DOC_LENGTH);
	need_stat(DOC_LENGTH_MIN);
	need_stat(DOC_LENGTH_MAX);
	need_stat(WDF_MAX);
	need_stat(COLLECTION_FREQ);
	need_stat(UNIQUE_TERMS);
	need_stat(TOTAL_LENGTH);
    }

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: CheckStatsWeight(db_, term_, string(), sum_, sum_squares_) { }

    void init(double factor_) {
	factor = factor_;
    }

    Weight * clone() const {
	auto res = new CheckStatsWeight(db, term1, term2, sum, sum_squares);
	if (term2 == "=") {
	    // The object passed to Enquire::set_weighting_scheme() is cloned
	    // right away, and then cloned again for each term, and then
	    // potentially once more for the term-independent weight
	    // contribution.  In the repeated case, we want to handle the first
	    // actual term specially, so we arrange for that to have "=" for
	    // term2, and subsequent clones to have "_", so that we accumulate
	    // sum and sum_squares on the first occurrence only.
	    term2 = "_";
	}
	return res;
    }

    double get_sumpart(Xapian::termcount wdf, Xapian::termcount doclen,
		       Xapian::termcount uniqueterms) const {
	Xapian::doccount num_docs = db.get_doccount();
	TEST_EQUAL(get_collection_size(), num_docs);
	TEST_EQUAL(get_rset_size(), 0);
	TEST_EQUAL(get_average_length(), db.get_avlength());
	Xapian::totallength totlen = get_total_length();
	TEST_EQUAL(totlen, db.get_total_length());
	double total_term_occurences = get_average_length() * num_docs;
	TEST_EQUAL(Xapian::totallength(total_term_occurences + 0.5), totlen);
	if (term2.empty() || term2 == "=" || term2 == "_") {
	    TEST_EQUAL(get_termfreq(), db.get_termfreq(term1));
	    TEST_EQUAL(get_collection_freq(), db.get_collection_freq(term1));
	    if (term2.empty()) {
		TEST_EQUAL(get_query_length(), 1);
	    } else {
		TEST_EQUAL(get_query_length(), 2);
	    }
	} else {
	    Xapian::doccount tfmax = 0, tfsum = 0;
	    Xapian::termcount cfmax = 0, cfsum = 0;
	    if (term2 == "*") {
		// OP_WILDCARD case.
		for (auto&& t = db.allterms_begin(term1);
		     t != db.allterms_end(term1); ++t) {
		    Xapian::doccount tf = t.get_termfreq();
		    tout << "->" << *t << " " << tf << '\n';
		    tfsum += tf;
		    tfmax = max(tfmax, tf);
		    Xapian::termcount cf = db.get_collection_freq(*t);
		    cfsum += cf;
		    cfmax = max(cfmax, cf);
		}
		TEST_EQUAL(get_query_length(), 1);
	    } else {
		// OP_SYNONYM case.
		Xapian::doccount tf1 = db.get_termfreq(term1);
		Xapian::doccount tf2 = db.get_termfreq(term2);
		tfsum = tf1 + tf2;
		tfmax = max(tf1, tf2);
		Xapian::termcount cf1 = db.get_collection_freq(term1);
		Xapian::termcount cf2 = db.get_collection_freq(term2);
		cfsum = cf1 + cf2;
		cfmax = max(cf1, cf2);
		TEST_EQUAL(get_query_length(), 2);
	    }
	    // Synonym occurs at least as many times as any term.
	    TEST_REL(get_termfreq(), >=, tfmax);
	    TEST_REL(get_collection_freq(), >=, cfmax);
	    // Synonym can't occur more times than the terms do.
	    TEST_REL(get_termfreq(), <=, tfsum);
	    TEST_REL(get_collection_freq(), <=, cfsum);
	    // Synonym can't occur more times than there are documents/terms.
	    TEST_REL(get_termfreq(), <=, num_docs);
	    TEST_REL(get_collection_freq(), <=, totlen);
	}
	TEST_EQUAL(get_reltermfreq(), 0);
	TEST_EQUAL(get_wqf(), 1);
	TEST_REL(doclen,>=,len_lower);
	TEST_REL(doclen,<=,len_upper);
	TEST_REL(uniqueterms,>=,1);
	TEST_REL(uniqueterms,<=,doclen);
	TEST_REL(wdf,<=,wdf_upper);
	if (term2 != "_") {
	    sum += wdf;
	    sum_squares += wdf * wdf;
	}
	return 1.0;
    }

    double get_maxpart() const {
	if (len_upper == 0) {
	    len_lower = get_doclength_lower_bound();
	    len_upper = get_doclength_upper_bound();
	    wdf_upper = get_wdf_upper_bound();
	}
	return 1.0;
    }

    double get_sumextra(Xapian::termcount doclen, Xapian::termcount) const {
	return 1.0 / doclen;
    }

    double get_maxextra() const { return 1.0; }
};

/// Check the weight subclass gets the correct stats.
DEFINE_TESTCASE(checkstatsweight1, backend && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::TermIterator a;
    for (a = db.allterms_begin(); a != db.allterms_end(); ++a) {
	const string & term = *a;
	enquire.set_query(Xapian::Query(term));
	Xapian::termcount sum = 0;
	Xapian::termcount sum_squares = 0;
	CheckStatsWeight wt(db, term, sum, sum_squares);
	enquire.set_weighting_scheme(wt);
	Xapian::MSet mset = enquire.get_mset(0, db.get_doccount());

	// The document order in the multi-db case isn't the same as the
	// postlist order on the combined DB, so it's hard to compare the
	// wdf for each document in the Weight objects, but we can sum
	// the wdfs and the squares of the wdfs which provides a decent
	// check that we're not getting the wrong wdf values (it ensures
	// they have the right mean and standard deviation).
	Xapian::termcount expected_sum = 0;
	Xapian::termcount expected_sum_squares = 0;
	Xapian::PostingIterator i;
	for (i = db.postlist_begin(term); i != db.postlist_end(term); ++i) {
	    Xapian::termcount wdf = i.get_wdf();
	    expected_sum += wdf;
	    expected_sum_squares += wdf * wdf;
	}
	TEST_EQUAL(sum, expected_sum);
	TEST_EQUAL(sum_squares, expected_sum_squares);
    }
}

/// Check the weight subclass gets the correct stats with OP_SYNONYM.
// Regression test for bugs fixed in 1.4.1.
DEFINE_TESTCASE(checkstatsweight2, backend && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::TermIterator a;
    for (a = db.allterms_begin(); a != db.allterms_end(); ++a) {
	const string & term1 = *a;
	if (++a == db.allterms_end()) break;
	const string & term2 = *a;
	Xapian::Query q(Xapian::Query::OP_SYNONYM,
			Xapian::Query(term1), Xapian::Query(term2));
	tout << q.get_description() << '\n';
	enquire.set_query(q);
	Xapian::termcount sum = 0;
	Xapian::termcount sum_squares = 0;
	CheckStatsWeight wt(db, term1, term2, sum, sum_squares);
	enquire.set_weighting_scheme(wt);
	Xapian::MSet mset = enquire.get_mset(0, db.get_doccount());

	// The document order in the multi-db case isn't the same as the
	// postlist order on the combined DB, so it's hard to compare the
	// wdf for each document in the Weight objects, but we can sum
	// the wdfs and the squares of the wdfs which provides a decent
	// check that we're not getting the wrong wdf values (it ensures
	// they have the right mean and standard deviation).
	Xapian::termcount expected_sum = 0;
	Xapian::termcount expected_sum_squares = 0;
	Xapian::PostingIterator i = db.postlist_begin(term1);
	Xapian::PostingIterator j = db.postlist_begin(term2);
	Xapian::docid did1 = *i, did2 = *j;
	while (true) {
	    // To calculate expected_sum_squares correctly we need to square
	    // the sum per document.
	    Xapian::termcount wdf;
	    if (did1 == did2) {
		wdf = i.get_wdf() + j.get_wdf();
		did1 = did2 = 0;
	    } else if (did1 < did2) {
		wdf = i.get_wdf();
		did1 = 0;
	    } else {
		wdf = j.get_wdf();
		did2 = 0;
	    }
	    expected_sum += wdf;
	    expected_sum_squares += wdf * wdf;

	    if (did1 == 0) {
		if (++i != db.postlist_end(term1)) {
		    did1 = *i;
		} else {
		    if (did2 == Xapian::docid(-1)) break;
		    did1 = Xapian::docid(-1);
		}
	    }
	    if (did2 == 0) {
		if (++j != db.postlist_end(term2)) {
		    did2 = *j;
		} else {
		    if (did1 == Xapian::docid(-1)) break;
		    did2 = Xapian::docid(-1);
		}
	    }
	}
	// The OP_SYNONYM's wdf should be equal to the sum of the wdfs of
	// the individual terms.
	TEST_EQUAL(sum, expected_sum);
	TEST_REL(sum_squares, >=, expected_sum_squares);
    }
}

/// Check the weight subclass gets the correct stats with OP_WILDCARD.
// Regression test for bug fixed in 1.4.1.
// Don't run with multi-database, as the termfreq checks don't work
// there - FIXME: Investigate this - it smells like a bug.
DEFINE_TESTCASE(checkstatsweight3, backend && !remote && !multi) {
    struct PlCmp {
	bool operator()(const Xapian::PostingIterator& a,
			const Xapian::PostingIterator& b) {
	    return *a < *b;
	}
    };

    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::TermIterator a;
    static const char * const testcases[] = {
	"a", // a* matches all documents, but no term matches all.
	"pa", // Expands to only "paragraph", matching 5.
	"zulu", // No matches.
	"th", // Term "this" matches all documents.
    };
    for (auto pattern : testcases) {
	Xapian::Query q(Xapian::Query::OP_WILDCARD, pattern);
	tout << q.get_description() << '\n';
	enquire.set_query(q);
	Xapian::termcount sum = 0;
	Xapian::termcount sum_squares = 0;
	CheckStatsWeight wt(db, pattern, "*", sum, sum_squares);
	enquire.set_weighting_scheme(wt);
	Xapian::MSet mset = enquire.get_mset(0, db.get_doccount());

	// The document order in the multi-db case isn't the same as the
	// postlist order on the combined DB, so it's hard to compare the
	// wdf for each document in the Weight objects, but we can sum
	// the wdfs and the squares of the wdfs which provides a decent
	// check that we're not getting the wrong wdf values (it ensures
	// they have the right mean and standard deviation).
	Xapian::termcount expected_sum = 0;
	Xapian::termcount expected_sum_squares = 0;
	vector<Xapian::PostingIterator> postlists;
	for (auto&& t = db.allterms_begin(pattern);
	     t != db.allterms_end(pattern); ++t) {
	    postlists.emplace_back(db.postlist_begin(*t));
	}
	make_heap(postlists.begin(), postlists.end(), PlCmp());
	Xapian::docid did = 0;
	Xapian::termcount wdf = 0;
	while (!postlists.empty()) {
	    pop_heap(postlists.begin(), postlists.end(), PlCmp());
	    Xapian::docid did_new = *postlists.back();
	    Xapian::termcount wdf_new = postlists.back().get_wdf();
	    if (++(postlists.back()) == Xapian::PostingIterator()) {
		postlists.pop_back();
	    } else {
		push_heap(postlists.begin(), postlists.end(), PlCmp());
	    }
	    if (did_new != did) {
		expected_sum += wdf;
		expected_sum_squares += wdf * wdf;
		wdf = 0;
		did = did_new;
	    }
	    wdf += wdf_new;
	}
	expected_sum += wdf;
	expected_sum_squares += wdf * wdf;
	// The OP_SYNONYM's wdf should be equal to the sum of the wdfs of
	// the individual terms.
	TEST_EQUAL(sum, expected_sum);
	TEST_REL(sum_squares, >=, expected_sum_squares);
    }
}

/// Check the stats for a repeated term are correct.
// Regression test for bug fixed in 1.4.6.  Doesn't work with
// multi as the weight object is cloned more times.
DEFINE_TESTCASE(checkstatsweight4, backend && !remote && !multi) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::TermIterator a;
    for (a = db.allterms_begin(); a != db.allterms_end(); ++a) {
	const string & term = *a;
	enquire.set_query(Xapian::Query(term, 1, 1) |
			  Xapian::Query(term, 1, 2));
	Xapian::termcount sum = 0;
	Xapian::termcount sum_squares = 0;
	CheckStatsWeight wt(db, term, "=", sum, sum_squares);
	enquire.set_weighting_scheme(wt);
	Xapian::MSet mset = enquire.get_mset(0, db.get_doccount());

	// The document order in the multi-db case isn't the same as the
	// postlist order on the combined DB, so it's hard to compare the
	// wdf for each document in the Weight objects, but we can sum
	// the wdfs and the squares of the wdfs which provides a decent
	// check that we're not getting the wrong wdf values (it ensures
	// they have the right mean and standard deviation).
	Xapian::termcount expected_sum = 0;
	Xapian::termcount expected_sum_squares = 0;
	Xapian::PostingIterator i;
	for (i = db.postlist_begin(term); i != db.postlist_end(term); ++i) {
	    Xapian::termcount wdf = i.get_wdf();
	    expected_sum += wdf;
	    expected_sum_squares += wdf * wdf;
	}
	TEST_EQUAL(sum, expected_sum);
	TEST_EQUAL(sum_squares, expected_sum_squares);
    }
}

// Two stage should perform same as Jelinek mercer if smoothing parameter for mercer is kept 1 in both.
DEFINE_TESTCASE(unigramlmweight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire1(db);
    Xapian::Enquire enquire2(db);
    enquire1.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset1;
    enquire2.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset2;
    // 5 documents available with term paragraph so mset size should be 5
    enquire1.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::TWO_STAGE_SMOOTHING, 1, 0));
    enquire2.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::JELINEK_MERCER_SMOOTHING, 1, 0));
    mset1 = enquire1.get_mset(0, 10);
    mset2 = enquire2.get_mset(0, 10);

    TEST_EQUAL(mset1.size(), 5);
    TEST_EQUAL_DOUBLE(mset1[1].get_weight(), mset2[1].get_weight());
}

/* Test for checking if we don't use smoothing all
 * of them should give same result i.e wdf_double/len_double */
DEFINE_TESTCASE(unigramlmweight5, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire1(db);
    Xapian::Enquire enquire2(db);
    Xapian::Enquire enquire3(db);
    Xapian::Enquire enquire4(db);
    enquire1.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset1;
    enquire2.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset2;
    enquire3.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset3;
    enquire4.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset4;
    // 5 documents available with term paragraph so mset size should be 5
    enquire1.set_weighting_scheme(Xapian::LMWeight(10000.0, Xapian::Weight::TWO_STAGE_SMOOTHING, 0, 0));
    enquire2.set_weighting_scheme(Xapian::LMWeight(10000.0, Xapian::Weight::JELINEK_MERCER_SMOOTHING, 0, 0));
    enquire3.set_weighting_scheme(Xapian::LMWeight(10000.0, Xapian::Weight::ABSOLUTE_DISCOUNT_SMOOTHING, 0, 0));
    enquire4.set_weighting_scheme(Xapian::LMWeight(10000.0, Xapian::Weight::DIRICHLET_SMOOTHING, 0, 0));

    mset1 = enquire1.get_mset(0, 10);
    mset2 = enquire2.get_mset(0, 10);
    mset3 = enquire3.get_mset(0, 10);
    mset4 = enquire4.get_mset(0, 10);

    TEST_EQUAL(mset1.size(), 5);
    TEST_EQUAL(mset2.size(), 5);
    TEST_EQUAL(mset3.size(), 5);
    TEST_EQUAL(mset4.size(), 5);
    for (Xapian::doccount i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(mset3[i].get_weight(), mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset2[i].get_weight(), mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(), mset2[i].get_weight());
	TEST_EQUAL_DOUBLE(mset3[i].get_weight(), mset2[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(), mset4[i].get_weight());
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(), mset3[i].get_weight());
    }
}

// Test Exception for junk after serialised weight (with Dir+ enabled).
DEFINE_TESTCASE(unigramlmweight6, !backend) {
    Xapian::LMWeight wt(0, Xapian::Weight::DIRICHLET_SMOOTHING, 0.5, 1.0);
    try {
	Xapian::LMWeight d;
	Xapian::LMWeight * d2 = d.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = d2->name().empty();
	delete d2;
	if (empty)
	    FAIL_TEST("Serialised LMWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised LMWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("LM") != string::npos);
    }
}

// Feature test for Dir+ function.
DEFINE_TESTCASE(unigramlmweight7, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire1(db);
    Xapian::Enquire enquire2(db);
    enquire1.set_query(Xapian::Query("paragraph"));
    enquire2.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset1;
    Xapian::MSet mset2;

    enquire1.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::DIRICHLET_SMOOTHING, 2000, 0));
    enquire2.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::DIRICHLET_PLUS_SMOOTHING, 2000, 0.05));

    mset1 = enquire1.get_mset(0, 10);
    mset2 = enquire2.get_mset(0, 10);

    // mset size should be 5
    TEST_EQUAL(mset1.size(), 5);
    TEST_EQUAL(mset2.size(), 5);

    // Expect mset weights associated with Dir+ more than mset weights by Dir
    // because of the presence of extra weight component in Dir+ function.
    TEST_REL(mset2[0].get_weight(),>,mset1[0].get_weight());
    TEST_REL(mset2[1].get_weight(),>,mset1[1].get_weight());
    TEST_REL(mset2[2].get_weight(),>,mset1[2].get_weight());
    TEST_REL(mset2[3].get_weight(),>,mset1[3].get_weight());
    TEST_REL(mset2[4].get_weight(),>,mset1[4].get_weight());
}

// Regression test that OP_SCALE_WEIGHT works with LMWeight (fixed in 1.4.1).
DEFINE_TESTCASE(unigramlmweight8, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::DIRICHLET_SMOOTHING, 2000, 0));

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);

    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::LMWeight(0, Xapian::Weight::DIRICHLET_SMOOTHING, 2000, 0));

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), mset1.size());
    TEST_NOT_EQUAL_DOUBLE(mset1[0].get_weight(), 0.0);
    for (Xapian::doccount i = 0; i < mset1.size(); ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }
}

// Feature test for BoolWeight.
// Test exception for junk after serialised weight.
DEFINE_TESTCASE(boolweight1, !backend) {
    Xapian::BoolWeight wt;
    try {
	Xapian::BoolWeight t;
	Xapian::BoolWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised BoolWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised BoolWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("Bool") != string::npos);
    }
}

// Feature test for CoordWeight.
DEFINE_TESTCASE(coordweight1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::CoordWeight());
    static const char * const terms[] = {
	"this", "line", "paragraph", "rubbish"
    };
    Xapian::Query query(Xapian::Query::OP_OR,
			terms, terms + sizeof(terms) / sizeof(terms[0]));
    enquire.set_query(query);
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    // CoordWeight scores 1 for each matching term, so the weight should equal
    // the number of matching terms.
    for (Xapian::MSetIterator i = mymset1.begin(); i != mymset1.end(); ++i) {
	Xapian::termcount matching_terms = 0;
	Xapian::TermIterator t = enquire.get_matching_terms_begin(i);
	while (t != enquire.get_matching_terms_end(i)) {
	    ++matching_terms;
	    ++t;
	}
	TEST_EQUAL(i.get_weight(), matching_terms);
    }

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    Xapian::MSet mymset2 = enquire.get_mset(0, 100);
    TEST_EQUAL(mymset1.size(), mymset2.size());
    for (Xapian::doccount i = 0; i != mymset1.size(); ++i) {
	TEST_EQUAL(15.0 * mymset1[i].get_weight(), mymset2[i].get_weight());
    }
}

// Test exception for junk after serialised weight.
DEFINE_TESTCASE(coordweight2, !backend) {
    Xapian::CoordWeight wt;
    try {
	Xapian::CoordWeight t;
	Xapian::CoordWeight * t2 = t.unserialise(wt.serialise() + "X");
	// Make sure we actually use the weight.
	bool empty = t2->name().empty();
	delete t2;
	if (empty)
	    FAIL_TEST("Serialised CoordWeight with junk appended unserialised to empty name!");
	FAIL_TEST("Serialised CoordWeight with junk appended unserialised OK");
    } catch (const Xapian::SerialisationError &e) {
	TEST(e.get_msg().find("Coord") != string::npos);
    }
}
