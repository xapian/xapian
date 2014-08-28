/** @file api_weight.cc
 * @brief tests of Xapian::Weight subclasses
 */
/* Copyright (C) 2012,2013 Olly Betts
 * Copyright (C) 2013 Aarsh Shah
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
    }
    return true;
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
    } catch (const Xapian::SerialisationError &) {
	// Good!
    }
    return true;
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

    return true;
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

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }

    return true;
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

    return true;
}

//Feature tests for Inl2Weight
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
    TEST_EQUAL_DOUBLE(15.0 * mset1[0].get_weight(), mset2[0].get_weight());

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }
    return true;
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

    return true;
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
    TEST_EQUAL_DOUBLE(15.0 * mset1[0].get_weight(), mset2[0].get_weight());

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }

    return true;
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

    return true;
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

    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }
    return true;
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

    return true;
}

//Feature test
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

    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }

    // Test with OP_SCALE_WEIGHT and a small factor (regression test, as we
    // were applying the factor to the upper bound twice).
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 1.0/1024));
    enquire.set_weighting_scheme(Xapian::BB2Weight(2.0));

    Xapian::MSet mset3;
    mset3 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset3.size(), 5);

    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(mset1[i].get_weight(), mset3[i].get_weight() * 1024);
    }

    return true;
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
    // Zero weight is a bit bogus, but what we currently give.
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 0);

    return true;
}

// Feature test.
DEFINE_TESTCASE(dlhweight1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");

    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DLHWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);
    /* Weight has been calculated manually by obtaining the statistics from the
     * database.*/
    TEST_EQUAL_DOUBLE(mset1[0].get_weight() - mset1[4].get_weight(), 1.17790202016936130);

    // Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::DLHWeight());

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);

    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }
    return true;
}

// Test for invalid values of c.
DEFINE_TESTCASE(pl2weight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::PL2Weight wt(-2.0));

    /* Parameter c should be set to 1.0 by constructor if none is given. */
    Xapian::PL2Weight weight2;
    TEST_EQUAL(weight2.serialise(), Xapian::PL2Weight(1.0).serialise());

    return true;
}

// Feature Test.
DEFINE_TESTCASE(pl2weight3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::PL2Weight(2.0));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Expected weight difference calculated in extended precision using stats
    // from the test database.
    TEST_EQUAL_DOUBLE(mset[2].get_weight(),
		      mset[3].get_weight() + 0.0086861771701328694);

    return true;
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

    //Test with OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::DPHWeight());

    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);
    for (int i = 0; i < 5; ++i) {
	TEST_EQUAL_DOUBLE(15.0 * mset1[i].get_weight(), mset2[i].get_weight());
    }

    return true;
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

    return true;
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
    } catch (const Xapian::SerialisationError &) {

    }
    return true;
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

    //Test for OP_SCALE_WEIGHT.
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 15.0));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ntn"));
    Xapian::MSet mset2;
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 2);
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset2, 2, 4);
    TEST_EQUAL_DOUBLE(15 * mset[0].get_weight(), mset2[0].get_weight());

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
    enquire.set_query(Xapian::Query("this"));  // N=termfreq amd so idfn=0 for "t"
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

    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("npn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * log((6.0 - 2) / 2));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * log((6.0 - 2) / 2));

    return true;
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
    return true;
}

class CheckStatsWeight : public Xapian::Weight {
  public:
    double factor;

    Xapian::Database db;

    string term;

    Xapian::termcount & sum;
    Xapian::termcount & sum_squares;

    mutable Xapian::termcount len_upper;
    mutable Xapian::termcount len_lower;
    mutable Xapian::termcount wdf_upper;

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: factor(-1.0), db(db_), term(term_),
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
    }

    void init(double factor_) {
	factor = factor_;
    }

    Weight * clone() const {
	return new CheckStatsWeight(db, term, sum, sum_squares);
    }

    double get_sumpart(Xapian::termcount wdf, Xapian::termcount doclen,
		       Xapian::termcount uniqueterms) const {
	TEST_EQUAL(get_collection_size(), db.get_doccount());
	TEST_EQUAL(get_collection_freq(), db.get_collection_freq(term));
	TEST_EQUAL(get_rset_size(), 0);
	TEST_EQUAL(get_average_length(), db.get_avlength());
	TEST_EQUAL(get_termfreq(), db.get_termfreq(term));
	TEST_EQUAL(get_reltermfreq(), 0);
	TEST_EQUAL(get_query_length(), 1);
	TEST_EQUAL(get_wqf(), 1);
	TEST_REL(doclen,>=,len_lower);
	TEST_REL(doclen,<=,len_upper);
	TEST_REL(uniqueterms,>=,1);
	TEST_REL(uniqueterms,<=,doclen);
	TEST_REL(wdf,<=,wdf_upper);
	sum += wdf;
	sum_squares += wdf * wdf;
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
	// wdf for each document in the Weight objects, so we can sum
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
    return true;
}
