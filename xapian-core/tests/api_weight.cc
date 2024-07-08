/** @file
 * @brief tests of Xapian::Weight subclasses
 */
/* Copyright (C) 2004-2024 Olly Betts
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
#include <memory>

#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "apitest.h"
#include "heap.h"
#include "testutils.h"

using namespace std;

template<class W>
static inline void
test_weight_class_no_params(const char* classname, const char* name)
{
    tout << classname << '\n';
    W obj;
    // Check name() returns the class name.
    TEST_EQUAL(obj.name(), name);
    // If there are no parameters, there's nothing to serialise.
    string obj_serialised = obj.serialise();
    TEST_EQUAL(obj_serialised.size(), 0);
    // Check serialising and unserialising gives object with same serialisation.
    unique_ptr<Xapian::Weight> wt(W().unserialise(obj_serialised));
    TEST_EQUAL(obj_serialised, wt->serialise());
    // Check that unserialise() throws suitable error for bad serialisation.
    // The easy case to test is extra junk after the serialised weight.
    try {
	unique_ptr<Xapian::Weight> bad(W().unserialise(obj_serialised + "X"));
	FAIL_TEST(classname << " did not throw for unserialise with junk "
		  "appended");
    } catch (const Xapian::SerialisationError& e) {
	// Check the exception message contains the weighting scheme name
	// (regression test for TradWeight's exception saying "BM25").
	string target = classname + CONST_STRLEN("Xapian::");
	TEST(e.get_msg().find(target) != string::npos);
    }
}

#define TEST_WEIGHT_CLASS_NO_PARAMS(W, N) test_weight_class_no_params<W>(#W, N)

template<class W>
static inline void
test_weight_class(const char* classname, const char* name,
		  const W& obj_default, const W& obj_other)
{
    tout << classname << '\n';
    W obj;
    // Check name() returns the class name.
    TEST_EQUAL(obj.name(), name);
    TEST_EQUAL(obj_default.name(), name);
    TEST_EQUAL(obj_other.name(), name);
    // Check serialisation matches that of object constructed with explicit
    // parameter values of what the defaults are meant to be.
    string obj_serialised = obj.serialise();
    TEST_EQUAL(obj_serialised, obj_default.serialise());
    // Check serialisation is different to object with different parameters.
    string obj_other_serialised = obj_other.serialise();
    TEST_NOT_EQUAL(obj_serialised, obj_other_serialised);
    // Check serialising and unserialising gives object with same serialisation.
    unique_ptr<Xapian::Weight> wt(W().unserialise(obj_serialised));
    TEST_EQUAL(obj_serialised, wt->serialise());
    // Check serialising and unserialising of object with different parameters.
    unique_ptr<Xapian::Weight> wt2(W().unserialise(obj_other_serialised));
    TEST_EQUAL(obj_other_serialised, wt2->serialise());
    // Check that unserialise() throws suitable error for bad serialisation.
    // The easy case to test is extra junk after the serialised weight.
    try {
	unique_ptr<Xapian::Weight> bad(W().unserialise(obj_serialised + "X"));
	FAIL_TEST(classname << " did not throw for unserialise with junk "
		  "appended");
    } catch (const Xapian::SerialisationError& e) {
	// Check the exception message contains the correct weighting scheme
	// name (originally a regression test for TradWeight's exception saying
	// "BM25", but not TradWeight is just a thin subclass of BM25Weight so
	// it's expected it reports as BM25Weight now!)
	string target = classname + CONST_STRLEN("Xapian::");
	if (target == "TradWeight") target = "BM25Weight";
	TEST(e.get_msg().find(target) != string::npos);
    }
}

// W Should be the class name.
//
// DEFAULT should be a parenthesised parameter list to explicitly construct
// an object of class W with the documented default parameters.
//
// OTHER should be a parenthesised parameter list to construct an object with
// non-default parameters.
#define TEST_WEIGHT_CLASS(W, N, DEFAULT, OTHER) \
    test_weight_class<W>(#W, N, W DEFAULT, W OTHER)

/// Test serialisation and introspection of built-in weighting schemes.
DEFINE_TESTCASE(weightserialisation1, !backend) {
    // Parameter-free weighting schemes.
    TEST_WEIGHT_CLASS_NO_PARAMS(Xapian::BoolWeight, "bool");
    TEST_WEIGHT_CLASS_NO_PARAMS(Xapian::CoordWeight, "coord");
    TEST_WEIGHT_CLASS_NO_PARAMS(Xapian::DLHWeight, "dlh");
    TEST_WEIGHT_CLASS_NO_PARAMS(Xapian::DPHWeight, "dph");
    TEST_WEIGHT_CLASS_NO_PARAMS(Xapian::DiceCoeffWeight, "dicecoeff");

    // Parameterised weighting schemes.
    TEST_WEIGHT_CLASS(Xapian::TradWeight, "bm25", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::BM25Weight, "bm25",
		      (1, 0, 1, 0.5, 0.5),
		      (1, 0.5, 1, 0.5, 0.5));
    TEST_WEIGHT_CLASS(Xapian::BM25PlusWeight, "bm25+",
		      (1, 0, 1, 0.5, 0.5, 1.0),
		      (1, 0, 1, 0.5, 0.5, 2.0));
    TEST_WEIGHT_CLASS(Xapian::TfIdfWeight, "tfidf", ("ntn"), ("bpn"));
    TEST_WEIGHT_CLASS(Xapian::InL2Weight, "inl2", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::IfB2Weight, "ifb2", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::IneB2Weight, "ineb2", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::BB2Weight, "bb2", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::PL2Weight, "pl2", (1.0), (2.0));
    TEST_WEIGHT_CLASS(Xapian::PL2PlusWeight, "pl2+",
		      (1.0, 0.8),
		      (2.0, 0.9));
    TEST_WEIGHT_CLASS(Xapian::LM2StageWeight, "lm2stage",
		      (0.7, 2000.0),
		      (0.5, 2000.0));
    TEST_WEIGHT_CLASS(Xapian::LMAbsDiscountWeight, "lmabsdiscount",
		      (0.7),
		      (0.75));
    TEST_WEIGHT_CLASS(Xapian::LMDirichletWeight, "lmdirichlet",
		      (2000.0, 0.05),
		      (2034.0, 0.0));
    TEST_WEIGHT_CLASS(Xapian::LMJMWeight, "lmjm", (0.0), (0.5));
}

/// Basic test of using weighting schemes.
DEFINE_TESTCASE(weight1, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    Xapian::Enquire enquire_scaled(db);
    auto term = "robinson";
    Xapian::Query q{term};
    enquire.set_query(q);
    enquire_scaled.set_query(q * 15.0);
    auto expected_matches = db.get_termfreq(term);
    auto helper = [&](const Xapian::Weight& weight,
		      string_view name,
		      string_view params) {
	tout << name << '(' << params << ")\n";
	enquire.set_weighting_scheme(weight);
	enquire_scaled.set_weighting_scheme(weight);
	Xapian::MSet mset = enquire.get_mset(0, expected_matches + 1);
	TEST_EQUAL(mset.size(), expected_matches);
	if (name == "Xapian::BoolWeight") {
	    /* All weights should be zero. */
	    TEST_EQUAL(mset[0].get_weight(), 0.0);
	    TEST_EQUAL(mset.back().get_weight(), 0.0);
	} else if (name == "Xapian::CoordWeight") {
	    /* All weights should be 1 for a single term query. */
	    TEST_EQUAL(mset[0].get_weight(), 1.0);
	    TEST_EQUAL(mset.back().get_weight(), 1.0);
	} else if (!params.empty()) {
	    /* All weights should be equal with these particular parameters. */
	    TEST_NOT_EQUAL(mset[0].get_weight(), 0.0);
	    TEST_EQUAL(mset[0].get_weight(), mset.back().get_weight());
	} else {
	    TEST_NOT_EQUAL(mset[0].get_weight(), 0.0);
	    TEST_NOT_EQUAL(mset[0].get_weight(), mset.back().get_weight());
	}
	Xapian::MSet mset_scaled = enquire_scaled.get_mset(0, expected_matches);
	TEST_EQUAL(mset_scaled.size(), expected_matches);
	auto lm = name.find("::LM");
	// All the LM* schemes have sumextra except LMJMWeight.
	//
	// BM25 and BM25+ have sumextra, but by default k2 is 0 which means
	// sumextra is zero too.
	bool has_sumextra = lm != string::npos && name[lm + 4] != 'J';
	for (Xapian::doccount i = 0; i < expected_matches; ++i) {
	    double w = mset[i].get_weight();
	    double ws = mset_scaled[i].get_weight();
	    if (has_sumextra) {
		// sumextra is not scaled, so we can't test for (near)
		// equality, but we can test that the weight is affected by the
		// scaling, and that it's between the unscaled weight and the
		// fully scaled weight.
		TEST_NOT_EQUAL_DOUBLE(ws, w);
		TEST_REL(ws, <=, w * 15.0);
		TEST_REL(ws, >=, w);
	    } else {
		TEST_EQUAL_DOUBLE(ws, w * 15.0);
	    }
	}
    };

    // MSVC gives nothing for #__VA_ARGS__ when there are no varargs.
#define TEST_WEIGHTING_SCHEME(W, ...) \
	helper(W(__VA_ARGS__), #W, "" #__VA_ARGS__)

    TEST_WEIGHTING_SCHEME(Xapian::BoolWeight);
    TEST_WEIGHTING_SCHEME(Xapian::CoordWeight);
    TEST_WEIGHTING_SCHEME(Xapian::DLHWeight);
    TEST_WEIGHTING_SCHEME(Xapian::DPHWeight);
    TEST_WEIGHTING_SCHEME(Xapian::DiceCoeffWeight);
    TEST_WEIGHTING_SCHEME(Xapian::TradWeight);
    TEST_WEIGHTING_SCHEME(Xapian::BM25Weight);
    TEST_WEIGHTING_SCHEME(Xapian::BM25PlusWeight);
    TEST_WEIGHTING_SCHEME(Xapian::TfIdfWeight);
    TEST_WEIGHTING_SCHEME(Xapian::InL2Weight);
    TEST_WEIGHTING_SCHEME(Xapian::IfB2Weight);
    TEST_WEIGHTING_SCHEME(Xapian::IneB2Weight);
    TEST_WEIGHTING_SCHEME(Xapian::BB2Weight);
    TEST_WEIGHTING_SCHEME(Xapian::PL2Weight);
    TEST_WEIGHTING_SCHEME(Xapian::PL2PlusWeight);
    TEST_WEIGHTING_SCHEME(Xapian::LM2StageWeight);
    TEST_WEIGHTING_SCHEME(Xapian::LMAbsDiscountWeight);
    TEST_WEIGHTING_SCHEME(Xapian::LMDirichletWeight);
    TEST_WEIGHTING_SCHEME(Xapian::LMJMWeight);
    // Regression test for bug fixed in 1.2.4.
    TEST_WEIGHTING_SCHEME(Xapian::BM25Weight, 0, 0, 0, 0, 1);
    /* As mentioned in the documentation, when parameter k is 0, wdf and
     * document length don't affect the weights.  Regression test for bug fixed
     * in 1.2.4.
     */
    TEST_WEIGHTING_SCHEME(Xapian::TradWeight, 0);
#undef TEST_WEIGHTING_SCHEME
}

/// Feature tests for Weight::create().
DEFINE_TESTCASE(weightcreate1, !backend) {
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	delete Xapian::Weight::create(""));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	delete Xapian::Weight::create("invalid"));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	delete Xapian::Weight::create("invalid 1.0"));
}

/** Regression test for bug fixed in 1.0.5.
 *
 * This test would fail under valgrind because it used an uninitialised value.
 */
DEFINE_TESTCASE(bm25weight1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::BM25Weight(1, 25, 1, 0.01, 0.5));
    enquire.set_query(Xapian::Query("word"));

    Xapian::MSet mset = enquire.get_mset(0, 25);
}

/// Test Weight::create() for BM25Weight.
DEFINE_TESTCASE(bm25weight2, !backend) {
    {
	auto wt_ptr = Xapian::Weight::create("bm25");
	auto wt = Xapian::BM25Weight();
	TEST_EQUAL(wt_ptr->serialise(), wt.serialise());
	delete wt_ptr;
    }

    {
	auto wt_ptr = Xapian::Weight::create("bm25 1 0 1 0.5 0.5");
	auto wt = Xapian::BM25Weight(1, 0, 1, 0.5, 0.5);
	TEST_EQUAL(wt_ptr->serialise(), wt.serialise());
	delete wt_ptr;
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

// Test for invalid values of c.
DEFINE_TESTCASE(inl2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::InL2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::InL2Weight wt2(0.0));
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
}

// Test for invalid values of c.
DEFINE_TESTCASE(ifb2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IfB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IfB2Weight wt2(0.0));
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
}

// Test for invalid values of c.
DEFINE_TESTCASE(ineb2weight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IneB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::IneB2Weight wt2(0.0));
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
}

// Test for invalid values of c.
DEFINE_TESTCASE(bb2weight2, !backend) {
    // InvalidArgumentError should be thrown if the parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::BB2Weight wt(-2.0));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::BB2Weight wt2(0.0));
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

// Test for invalid values of c.
DEFINE_TESTCASE(pl2weight2, !backend) {
    // InvalidArgumentError should be thrown if parameter c is invalid.
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::PL2Weight wt(-2.0));
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

// Feature Test 1 for PL2PlusWeight.
DEFINE_TESTCASE(pl2plusweight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("to"));
    Xapian::MSet mset;

    enquire.set_weighting_scheme(Xapian::PL2PlusWeight(2.0, 0.8));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 3);
    // Expected weight difference calculated in Python using stats from the
    // test database.
    TEST_EQUAL_DOUBLE(mset[1].get_weight(),
		      mset[2].get_weight() + 0.016760925252262027);
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

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::Weight::create("tfidf FUN NONE NONE"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::Weight::create("tfidf NONE FUN NONE"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::Weight::create("tfidf NONE NONE FUN"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::Weight::create("tfidf NONE"));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	Xapian::Weight::create("tfidf NONE NONE"));
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
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset2, 2, 4);
    // wqf is 2, so weights should be doubled.
    TEST_EQUAL_DOUBLE(mset[0].get_weight() * 2, mset2[0].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight() * 2, mset2[1].get_weight());

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

    // Check for "mnn".
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("mnn"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 / 8);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1.0 / 4);

    // Check for "ann".
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("ann"));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 0.5 + 0.5 * 8.0 / 8);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 0.5 + 0.5 * 1.0 / 4);

    // Check for NONE, TFIDF, NONE when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::TFIDF,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 * log(6.0 / 2));

    // Check that wqf is taken into account.
    enquire.set_query(Xapian::Query("word", 2));
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 2);
    // doc 2 should have higher weight than 4 as only tf(wdf) will dominate.
    mset_expect_order(mset2, 2, 4);
    // wqf is 2, so weights should be doubled.
    TEST_EQUAL_DOUBLE(mset[0].get_weight() * 2, mset2[0].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight() * 2, mset2[1].get_weight());

    // check for NONE, FREQ, NONE when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::FREQ,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 / 2);

    // check for NONE, SQUARE, NONE when termfreq != N
    enquire.set_query(query);
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::SQUARE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 * pow(log(6.0 / 2), 2.0));

    // Check for BOOLEAN, NONE, NONE and for both branches of BOOLEAN.
    enquire.set_query(Xapian::Query("test"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::BOOLEAN,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 1);
    mset_expect_order(mset, 1);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 1.0);

    // Check for LOG, NONE, NONE and for both branches of LOG.
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::LOG,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 1 + log(8.0));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1.0);

    // Check for SQUARE, NONE, NONE.
    enquire.set_query(Xapian::Query("paragraph"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::SQUARE,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE)); // idf=1 and tfn=tf*tf
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    mset_expect_order(mset, 2, 1, 4, 3, 5);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 9.0);
    TEST_EQUAL_DOUBLE(mset[4].get_weight(), 1.0);

    // Check for NONE, TFIDF, NONE when termfreq=N
    enquire.set_query(Xapian::Query("this"));
    // N=termfreq and so idfn=0 for TFIDF
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::TFIDF,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 6);
    mset_expect_order(mset, 1, 2, 3, 4, 5, 6);
    for (int i = 0; i < 6; ++i) {
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), 0.0);
    }

    // Check for NONE, PROB, NONE and for both branches of PROB
    enquire.set_query(Xapian::Query("this"));
    // N=termfreq and so idfn=0 for PROB
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::PROB,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 6);
    mset_expect_order(mset, 1, 2, 3, 4, 5, 6);
    for (int i = 0; i < 6; ++i) {
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), 0.0);
    }

    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::PROB,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * log((6.0 - 2) / 2));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * log((6.0 - 2) / 2));

    // Check for LOG_AVERAGE, NONE, NONE.
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::LOG_AVERAGE,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(),
		      (1 + log(8.0)) / (1 + log(81.0 / 56.0)));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(),
		      (1 + log(1.0)) / (1 + log(31.0 / 26.0)));

    // Check for AUG_LOG, NONE, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::AUG_LOG,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 0.2 + 0.8 * log(1.0 + 8));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 0.2 + 0.8 * log(1.0 + 1));

    // Check for NONE, GLOBAL_FREQ, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::GLOBAL_FREQ,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * (9.0 / 2));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * (9.0 / 2));

    // Check for SQRT, NONE, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::SQRT,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), sqrt(8 - 0.5) + 1);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), sqrt(1 - 0.5) + 1);

    // Check for NONE, LOG_GLOBAL_FREQ, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::LOG_GLOBAL_FREQ,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * log(9.0 / 2 + 1));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * log(9.0 / 2 + 1));

    // Check for NONE, INCREMENTED_GLOBAL_FREQ, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::INCREMENTED_GLOBAL_FREQ,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * (9.0 / 2 + 1));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * (9.0 / 2 + 1));

    // Check for NONE, SQRT_GLOBAL_FREQ, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::SQRT_GLOBAL_FREQ,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8 * sqrt(9.0 / 2 - 0.9));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1 * sqrt(9.0 / 2 - 0.9));

    // Check for AUG_AVERAGE, NONE, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::AUG_AVERAGE,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 0.9 + 0.1 * (8.0 / (81.0 / 56.0)));
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 0.9 + 0.1 * (1.0 / (31.0 / 26.0)));

    // Check for MAX, NONE, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::MAX,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 8.0 / 8);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 1.0 / 4);

    // Check for AUG, NONE, NONE.
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::AUG,
	    Xapian::TfIdfWeight::idf_norm::NONE,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    mset_expect_order(mset, 2, 4);
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 0.5 + 0.5 * 8.0 / 8);
    TEST_EQUAL_DOUBLE(mset[1].get_weight(), 0.5 + 0.5 * 1.0 / 4);
}

// Feature tests for pivoted normalization functions.
DEFINE_TESTCASE(tfidfweight4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query query("paragraph");
    Xapian::MSet mset;

    // Check for "PPn" normalization string.
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("PPn", 0.2, 1.0));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Shorter docs should ranker higher if wqf is equal among all the docs.
    TEST_REL(mset[0].get_weight(),>,mset[1].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());

    // Check that wqf is taken into account.
    enquire.set_query(Xapian::Query("paragraph", 2));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("PPn", 0.2, 1.0));
    Xapian::MSet mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);
    // wqf is 2, so weights should be doubled.
    TEST_EQUAL_DOUBLE(mset[0].get_weight() * 2, mset2[0].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight() * 2, mset2[1].get_weight());

    // check for "nPn" which represents "xPx"
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("nPn", 0.2, 1.0));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // Expect doc 2 with query "word" to have higher weight than doc 4.
    mset_expect_order(mset, 2, 4);

    // check for "Ptn" which represents "Pxx"
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(Xapian::TfIdfWeight("Ptn", 0.2, 1.0));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // Expect doc 2 with query "word" to have higher weight than doc 4.
    mset_expect_order(mset, 2, 4);

    // Check for PIVOTED, PIVOTED, NONE normalization string.
    enquire.set_query(query);
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::PIVOTED,
	    Xapian::TfIdfWeight::idf_norm::PIVOTED,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 5);
    // Shorter docs should ranker higher if wqf is equal among all the docs.
    TEST_REL(mset[0].get_weight(),>,mset[1].get_weight());
    TEST_REL(mset[2].get_weight(),>,mset[3].get_weight());

    // Check that wqf is taken into account.
    enquire.set_query(Xapian::Query("paragraph", 2));
    mset2 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 5);
    // wqf is 2, so weights should be doubled.
    TEST_EQUAL_DOUBLE(mset[0].get_weight() * 2, mset2[0].get_weight());
    TEST_EQUAL_DOUBLE(mset[1].get_weight() * 2, mset2[1].get_weight());

    // check for NONE, PIVOTED, NONE
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::NONE,
	    Xapian::TfIdfWeight::idf_norm::PIVOTED,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // Expect doc 2 with query "word" to have higher weight than doc 4.
    mset_expect_order(mset, 2, 4);

    // check for PIVOTED, TFIDF, NONE
    enquire.set_query(Xapian::Query("word"));
    enquire.set_weighting_scheme(
	Xapian::TfIdfWeight(
	    Xapian::TfIdfWeight::wdf_norm::PIVOTED,
	    Xapian::TfIdfWeight::idf_norm::TFIDF,
	    Xapian::TfIdfWeight::wt_norm::NONE));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);
    // Expect doc 2 with query "word" to have higher weight than doc 4.
    mset_expect_order(mset, 2, 4);
}

// Check that create_from_parameters() creates the correct object.
DEFINE_TESTCASE(tfidfweight5, !backend) {
    auto wt_ptr = Xapian::Weight::create("tfidf NONE TFIDF NONE");
    auto wt = Xapian::TfIdfWeight(Xapian::TfIdfWeight::wdf_norm::NONE,
				  Xapian::TfIdfWeight::idf_norm::TFIDF,
				  Xapian::TfIdfWeight::wt_norm::NONE);
    TEST_EQUAL(wt_ptr->serialise(), wt.serialise());
    delete wt_ptr;

    auto wt_ptr2 = Xapian::Weight::create("tfidf SQRT PIVOTED NONE");
    auto wt2 = Xapian::TfIdfWeight(Xapian::TfIdfWeight::wdf_norm::SQRT,
				   Xapian::TfIdfWeight::idf_norm::PIVOTED,
				   Xapian::TfIdfWeight::wt_norm::NONE);
    TEST_EQUAL(wt_ptr2->serialise(), wt2.serialise());
    delete wt_ptr2;
}

class CheckInitWeight : public Xapian::Weight {
  public:
    double factor;

    unsigned & zero_inits, & non_zero_inits;

    CheckInitWeight(unsigned &z, unsigned &n)
	: factor(-1.0), zero_inits(z), non_zero_inits(n) {
	need_stat(DOC_LENGTH);
    }

    void init(double factor_) override {
	factor = factor_;
	if (factor == 0.0)
	    ++zero_inits;
	else
	    ++non_zero_inits;
    }

    Weight* clone() const override {
	return new CheckInitWeight(zero_inits, non_zero_inits);
    }

    double get_sumpart(Xapian::termcount, Xapian::termcount,
		       Xapian::termcount, Xapian::termcount) const override {
	return 1.0;
    }

    double get_maxpart() const override { return 1.0; }

    double get_sumextra(Xapian::termcount doclen,
			Xapian::termcount,
			Xapian::termcount) const override {
	return 1.0 / doclen;
    }

    double get_maxextra() const override { return 1.0; }
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
    double factor = -1.0;

    Xapian::Database db;

    string term1;

    // When testing OP_SYNONYM, term2 is also set.
    // When testing OP_WILDCARD, term2 == "*".
    // When testing a repeated term, term2 == "=" for the first occurrence and
    // "_" for subsequent occurrences.
    mutable string term2;

    Xapian::termcount & sum;
    Xapian::termcount & sum_squares;

    mutable Xapian::termcount len_upper = 0;
    mutable Xapian::termcount len_lower = Xapian::termcount(-1);
    mutable Xapian::termcount uniqueterms_upper = 0;
    mutable Xapian::termcount uniqueterms_lower = Xapian::termcount(-1);
    mutable Xapian::termcount wdf_upper = 0;

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term1_,
		     const string & term2_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: db(db_), term1(term1_), term2(term2_),
	  sum(sum_), sum_squares(sum_squares_)
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
	need_stat(DB_DOC_LENGTH_MIN);
	need_stat(DB_DOC_LENGTH_MAX);
	need_stat(WDF_MAX);
	need_stat(COLLECTION_FREQ);
	need_stat(UNIQUE_TERMS);
	need_stat(UNIQUE_TERMS_MIN);
	need_stat(UNIQUE_TERMS_MAX);
	need_stat(DB_UNIQUE_TERMS_MIN);
	need_stat(DB_UNIQUE_TERMS_MAX);
	need_stat(TOTAL_LENGTH);
	need_stat(WDF_DOC_MAX);
    }

    CheckStatsWeight(const Xapian::Database & db_,
		     const string & term_,
		     Xapian::termcount & sum_,
		     Xapian::termcount & sum_squares_)
	: CheckStatsWeight(db_, term_, string(), sum_, sum_squares_) { }

    void init(double factor_) override {
	factor = factor_;
    }

    Weight* clone() const override {
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

    double get_sumpart(Xapian::termcount wdf,
		       Xapian::termcount doclen,
		       Xapian::termcount uniqueterms,
		       Xapian::termcount wdfdocmax) const override {
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
	if (doclen > 0) {
	    TEST_REL(uniqueterms,>=,1);
	    TEST_REL(uniqueterms_lower,>=,1);
	    TEST_REL(wdfdocmax,>=,1);
	}
	TEST_REL(uniqueterms,>=,uniqueterms_lower);
	TEST_REL(uniqueterms,<=,uniqueterms_upper);
	TEST_REL(uniqueterms,<=,doclen);
	TEST_REL(uniqueterms_upper,<=,len_upper);
	TEST_REL(wdf,<=,wdf_upper);
	TEST_REL(wdfdocmax,<=,doclen);
	TEST_REL(wdfdocmax,>=,wdf);

	auto db_len_lower = db.get_doclength_lower_bound();
	auto db_len_upper = db.get_doclength_upper_bound();
	auto db_uniqueterms_lower = db.get_unique_terms_lower_bound();
	auto db_uniqueterms_upper = db.get_unique_terms_upper_bound();
	TEST_EQUAL(get_db_doclength_lower_bound(), db_len_lower);
	TEST_EQUAL(get_db_doclength_upper_bound(), db_len_upper);
	TEST_EQUAL(get_db_unique_terms_lower_bound(), db_uniqueterms_lower);
	TEST_EQUAL(get_db_unique_terms_upper_bound(), db_uniqueterms_upper);
	if (db.size() == 1) {
	    TEST_EQUAL(len_lower, db_len_lower);
	    TEST_EQUAL(len_upper, db_len_upper);
	    TEST_EQUAL(uniqueterms_lower, db_uniqueterms_lower);
	    TEST_EQUAL(uniqueterms_upper, db_uniqueterms_upper);
	} else {
	    TEST_REL(len_lower,>=,db_len_lower);
	    TEST_REL(len_upper,<=,db_len_upper);
	    TEST_REL(uniqueterms_lower,>=,db_uniqueterms_lower);
	    TEST_REL(uniqueterms_upper,<=,db_uniqueterms_upper);
	}
	if (term2 != "_") {
	    sum += wdf;
	    sum_squares += wdf * wdf;
	}
	return 1.0;
    }

    double get_maxpart() const override {
	if (len_upper == 0) {
	    len_lower = get_doclength_lower_bound();
	    len_upper = get_doclength_upper_bound();
	    uniqueterms_lower = get_unique_terms_lower_bound();
	    uniqueterms_upper = get_unique_terms_upper_bound();
	    wdf_upper = get_wdf_upper_bound();
	}
	return 1.0;
    }

    double get_sumextra(Xapian::termcount doclen,
			Xapian::termcount,
			Xapian::termcount) const override {
	return 1.0 / doclen;
    }

    double get_maxextra() const override { return 1.0; }
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
DEFINE_TESTCASE(checkstatsweight3, backend && !remote) {
    // The most correct thing to do would be to collate termfreqs across shards
    // for this, but if that's too hard to do efficiently we could at least
    // scale up the termfreqs proportional to the size of the shard.
    XFAIL_FOR_BACKEND("multi", "OP_WILDCARD+OP_SYNONYM use shard termfreqs");

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
	tout.str(string{});
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
	Heap::make(postlists.begin(), postlists.end(), PlCmp());
	Xapian::docid did = 0;
	Xapian::termcount wdf = 0;
	while (!postlists.empty()) {
	    Xapian::docid did_new = *postlists.front();
	    Xapian::termcount wdf_new = postlists.front().get_wdf();
	    if (++(postlists.front()) == Xapian::PostingIterator()) {
		Heap::pop(postlists.begin(), postlists.end(), PlCmp());
		postlists.pop_back();
	    } else {
		Heap::replace(postlists.begin(), postlists.end(), PlCmp());
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

class CheckStatsWeight5 : public Xapian::Weight {
  public:
    mutable Xapian::docid did = 0;

    double factor;

    Xapian::Database db;

    char stat_code;

    explicit
    CheckStatsWeight5(const Xapian::Database& db_, char stat_code_ = '\0')
	: factor(-1.0), db(db_), stat_code(stat_code_)
    {
	switch (stat_code) {
	    case 'w':
		need_stat(WDF);
		break;
	    case 'd':
		need_stat(DOC_LENGTH);
		break;
	}
	need_stat(WDF_DOC_MAX);
    }

    void init(double factor_) override {
	factor = factor_;
    }

    Weight* clone() const override {
	return new CheckStatsWeight5(db, stat_code);
    }

    double get_sumpart(Xapian::termcount,
		       Xapian::termcount,
		       Xapian::termcount,
		       Xapian::termcount wdfdocmax) const override {
	// The query is a synonym of all terms, so should match all documents.
	++did;
	TEST_REL(wdfdocmax,==,db.get_doclength(did));
	return 1.0 / wdfdocmax;
    }

    double get_maxpart() const override {
	return 1.0;
    }
};

/// Check wdfdocmax is clamped to doclen even if wdf and doclen aren't wanted.
DEFINE_TESTCASE(checkstatsweight5, backend && !multi && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire(db);
    Xapian::Query q{Xapian::Query::OP_SYNONYM,
		    db.allterms_begin(),
		    db.allterms_end()};
    enquire.set_query(q);
    enquire.set_weighting_scheme(CheckStatsWeight5(db));
    Xapian::MSet mset1 = enquire.get_mset(0, db.get_doccount());
    enquire.set_weighting_scheme(CheckStatsWeight5(db, 'w'));
    Xapian::MSet mset2 = enquire.get_mset(0, db.get_doccount());
    enquire.set_weighting_scheme(CheckStatsWeight5(db, 'd'));
    Xapian::MSet mset3 = enquire.get_mset(0, db.get_doccount());
}

// Feature test for Dir+ weighting.
DEFINE_TESTCASE(lmdirichletweight1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire1(db);
    Xapian::Enquire enquire2(db);
    enquire1.set_query(Xapian::Query("paragraph"));
    enquire2.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset1;
    Xapian::MSet mset2;

    enquire1.set_weighting_scheme(Xapian::LMDirichletWeight(2000, 0));
    enquire2.set_weighting_scheme(Xapian::LMDirichletWeight(2000, 0.05));

    mset1 = enquire1.get_mset(0, 10);
    mset2 = enquire2.get_mset(0, 10);

    // mset size should be 5
    TEST_EQUAL(mset1.size(), 5);
    TEST_EQUAL(mset2.size(), 5);

    // Expect mset weights from Dir+ to be less than mset weights from
    // Dirichlet for this testcase.
    TEST_REL(mset2[0].get_weight(),<,mset1[0].get_weight());
    TEST_REL(mset2[1].get_weight(),<,mset1[1].get_weight());
    TEST_REL(mset2[2].get_weight(),<,mset1[2].get_weight());
    TEST_REL(mset2[3].get_weight(),<,mset1[3].get_weight());
    TEST_REL(mset2[4].get_weight(),<,mset1[4].get_weight());
}

// Feature test for CoordWeight.
DEFINE_TESTCASE(coordweight1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(Xapian::CoordWeight());
    static const char * const terms[] = {
	"this", "line", "paragraph", "rubbish"
    };
    Xapian::Query query(Xapian::Query::OP_OR, terms, std::end(terms));
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
}

// Feature test.
DEFINE_TESTCASE(dicecoeffweight2, backend) {
    Xapian::Database db = get_database("apitest_simpledata3");
    Xapian::Enquire enquire(db);
    static const char * const terms[] = {
	"one", "three"
    };
    Xapian::Query query(Xapian::Query::OP_OR, terms, std::end(terms));
    enquire.set_query(query);
    enquire.set_weighting_scheme(Xapian::DiceCoeffWeight());

    Xapian::MSet mset1;
    mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 4);

    /* The weight value has been manually calculated by using the statistics
     * of the test database. */
    TEST_EQUAL_DOUBLE(mset1[0].get_weight(), 0.571428571428571);
    TEST_EQUAL_DOUBLE(mset1[1].get_weight(), 0.5);
    TEST_EQUAL_DOUBLE(mset1[2].get_weight(), 0.2);
    TEST_EQUAL_DOUBLE(mset1[3].get_weight(), 0.181818181818182);
}

// Test handling of a term with zero wdf.
DEFINE_TESTCASE(dicecoeffweight3, backend) {
    Xapian::Database db = get_database("dicecoeffweight3",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   Xapian::Document doc;
					   doc.add_term("radio", 2);
					   doc.add_term("seahorse");
					   doc.add_term("zebra");
					   doc.add_boolean_term("false");
					   doc.add_boolean_term("true");
					   wdb.add_document(doc);
				       });
    Xapian::Enquire enquire(db);
    enquire.set_weighting_scheme(Xapian::DiceCoeffWeight());

    // OP_SYNONYM gives wdf zero is need_stat(WDF) isn't specified (and
    // it isn't by DiceCoeffWeight).
    Xapian::Query q(Xapian::Query::OP_SYNONYM,
		    Xapian::Query("false"), Xapian::Query("true"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
				    q, 6.0), 2);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 1);

    // factor * 2.0 * wqf / (query_length + unique_term_count)
    // = 6.0 * 2.0 * 1 / (2 + 4) = 2.0
    TEST_EQUAL_DOUBLE(mset[0].get_weight(), 2.0);
}
