/** @file api_matchspy.cc
 * @brief tests of MatchSpy usage
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009 Olly Betts
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

#include "api_matchspy.h"

#include <xapian.h>

#include "str.h"
#include <cmath>
#include <map>
#include <vector>

#include "backendmanager.h"
#include "testsuite.h"
#include "testutils.h"
#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

class SimpleMatchSpy : public Xapian::MatchSpy {
  public:
    // Vector which will be filled with all the document contents seen.
    std::vector<std::string> seen;

    void operator()(const Xapian::Document &doc,
		    Xapian::weight) {
	// Note that this is not recommended usage of get_data() - you
	// generally shouldn't call get_data() from inside a MatchSpy, because
	// it is (likely to be) a slow operation resulting in considerable IO.
	seen.push_back(doc.get_data());
    }
};

// Basic test of a matchspy.
DEFINE_TESTCASE(matchspy1, backend && !remote) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("this"));

    SimpleMatchSpy myspy;

    Xapian::MSet nospymset = enquire.get_mset(0, 100);
    enquire.add_matchspy(&myspy);
    Xapian::MSet spymset = enquire.get_mset(0, 100);

    // Check that the match estimates aren't affected by the matchspy.
    TEST_EQUAL(nospymset, spymset);

    vector<bool> docid_checked(db.get_lastdocid());

    // Check that we get the expected number of matches, and that the stored
    // document contents are right.
    Xapian::MSetIterator i = spymset.begin();
    TEST(i != spymset.end());
    TEST_EQUAL(spymset.size(), 6);
    TEST_EQUAL(myspy.seen.size(), spymset.size());

    std::sort(myspy.seen.begin(), myspy.seen.end());

    std::vector<std::string> seen2;
    for ( ; i != spymset.end(); ++i) {
	const Xapian::Document doc(i.get_document());
	seen2.push_back(doc.get_data());
    }
    std::sort(seen2.begin(), seen2.end());

    TEST_EQUAL(myspy.seen.size(), seen2.size());
    std::vector<std::string>::const_iterator j = myspy.seen.begin();
    std::vector<std::string>::const_iterator j2 = seen2.begin();
    for (; j != myspy.seen.end(); ++j, ++j2) {
	TEST_EQUAL(*j, *j2);
    }

    return true;
}

static string values_to_repr(const Xapian::ValueCountMatchSpy & spy) {
    string resultrepr("|");
    for (Xapian::TermIterator i = spy.values_begin();
	 i != spy.values_end();
	 ++i) {
	resultrepr += *i;
	resultrepr += ':';
	resultrepr += str(i.get_termfreq());
	resultrepr += '|';
    }
    return resultrepr;
}

DEFINE_TESTCASE(matchspy2, writable)
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database();
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + str(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + str(factor));
		++factors;
	    }
	}

	// Number of factors.
	doc.add_value(0, str(factors));
	// Units digits.
	doc.add_value(1, str(c % 10));
	// Constant.
	doc.add_value(2, "fish");
	// Number of digits.
	doc.add_value(3, str(str(c).size()));

	db.add_document(doc);
    }

    Xapian::ValueCountMatchSpy spy0(0);
    Xapian::ValueCountMatchSpy spy1(1);
    Xapian::ValueCountMatchSpy spy3(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    enq.add_matchspy(&spy0);
    enq.add_matchspy(&spy1);
    enq.add_matchspy(&spy3);
    Xapian::MSet mset = enq.get_mset(0, 10);

    TEST_EQUAL(spy0.get_total(), 25);
    TEST_EQUAL(spy1.get_total(), 25);
    TEST_EQUAL(spy3.get_total(), 25);

    static const char * results[] = {
	"|1:1|2:9|3:3|4:7|5:1|6:3|8:1|",
	"|0:2|1:3|2:3|3:3|4:3|5:3|6:2|7:2|8:2|9:2|",
	"|1:9|2:16|",
    };
    TEST_STRINGS_EQUAL(values_to_repr(spy0), results[0]);
    TEST_STRINGS_EQUAL(values_to_repr(spy1), results[1]);
    TEST_STRINGS_EQUAL(values_to_repr(spy3), results[2]);
		       
    return true;
}

DEFINE_TESTCASE(matchspy3, writable)
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database();
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + str(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + str(factor));
		++factors;
	    }
	}

	// Number of factors.
	doc.add_value(0, Xapian::sortable_serialise(factors));
	// Units digits.
	doc.add_value(1, Xapian::sortable_serialise(c % 10));
	// (x + 1/3)*(x + 1/3).
	doc.add_value(2, Xapian::sortable_serialise((c + 1.0/3.0) * (c + 1.0/3.0)));
	// Reciprocal.
	doc.add_value(3, Xapian::sortable_serialise(floor(100.0 / c)));

	db.add_document(doc);
    }

    Xapian::ValueCountMatchSpy spy0(0);
    Xapian::ValueCountMatchSpy spy1(1);
    Xapian::ValueCountMatchSpy spy2(2);
    Xapian::ValueCountMatchSpy spy3(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    enq.add_matchspy(&spy0);
    enq.add_matchspy(&spy1);
    enq.add_matchspy(&spy2);
    enq.add_matchspy(&spy3);
    Xapian::MSet mset = enq.get_mset(0, 10);

    TEST_EQUAL(spy0.get_total(), 25);
    TEST_EQUAL(spy1.get_total(), 25);
    TEST_EQUAL(spy2.get_total(), 25);
    TEST_EQUAL(spy3.get_total(), 25);

    static const string results[] = {
	"|100:1|200:9|300:3|400:7|500:1|600:3|800:1|",
	"|0..200:8|300..400:6|500..700:7|800..900:4|",
	"|177..8711:9|10677..17777:4|20544..26677:3|30044..37377:3|41344..49877:3|54444..59211:2|64177:1|",
	"|400..900:15|1000..1600:5|2000..2500:2|3300:1|5000:1|10000:1|",
	""
    };
    std::vector<Xapian::ValueCountMatchSpy *> spies;
    spies.push_back(&spy0);
    spies.push_back(&spy1);
    spies.push_back(&spy2);
    spies.push_back(&spy3);
    for (Xapian::valueno v = 0; !results[v].empty(); ++v) {
	Xapian::UnbiasedNumericRanges ranges(*(spies[v]), 7);
	if (results[v] == "|") {
	    TEST_EQUAL(ranges.get_values_seen(), 0);
	    continue;
	}
	TEST_NOT_EQUAL(ranges.get_values_seen(), 0);
	TEST(ranges.get_ranges().size() <= 7);
	string resultrepr("|");
	map<Xapian::NumericRange, Xapian::doccount>::const_iterator i;
	for (i = ranges.get_ranges().begin();
	     i != ranges.get_ranges().end(); ++i) {
	    if (i->first.get_lower() != i->first.get_upper()) {
		resultrepr += str(floor(i->first.get_lower() * 100));
		resultrepr += "..";
		resultrepr += str(floor(i->first.get_upper() * 100));
	    } else {
		double start = floor(i->first.get_lower() * 100);
		resultrepr += str(start);
	    }
	    resultrepr += ':';
	    resultrepr += str(i->second);
	    resultrepr += '|';
	}
	tout << "value " << v << endl;
	TEST_STRINGS_EQUAL(resultrepr, results[v]);
    }

    return true;
}

DEFINE_TESTCASE(matchspy4, writable)
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database();
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + str(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + str(factor));
		++factors;
	    }
	}

	// Number of factors.
	doc.add_value(0, str(factors));
	// Units digits.
	doc.add_value(1, str(c % 10));
	// Constant.
	doc.add_value(2, "fish");
	// Number of digits.
	doc.add_value(3, str(str(c).size()));

	db.add_document(doc);
    }

    // We're going to run the match twice - once sorted by relevance, and once
    // sorted by a value.  This is a regression test - the matcher used to fail
    // to show some documents to the spy when sorting by non-pure-relevance.
    Xapian::ValueCountMatchSpy spya0(0);
    Xapian::ValueCountMatchSpy spya1(1);
    Xapian::ValueCountMatchSpy spya3(3);
    Xapian::ValueCountMatchSpy spyb0(0);
    Xapian::ValueCountMatchSpy spyb1(1);
    Xapian::ValueCountMatchSpy spyb3(3);

    Xapian::Enquire enqa(db);
    Xapian::Enquire enqb(db);

    enqa.set_query(Xapian::Query("all"));
    enqb.set_query(Xapian::Query("all"));

    enqa.add_matchspy(&spya0);
    enqa.add_matchspy(&spya1);
    enqa.add_matchspy(&spya3);
    enqb.add_matchspy(&spyb0);
    enqb.add_matchspy(&spyb1);
    enqb.add_matchspy(&spyb3);

    Xapian::MSet mseta = enqa.get_mset(0, 10);
    enqb.set_sort_by_value(0, false);
    Xapian::MSet msetb = enqb.get_mset(0, 10, 100);

    TEST_EQUAL(spya0.get_total(), 25);
    TEST_EQUAL(spya1.get_total(), 25);
    TEST_EQUAL(spya3.get_total(), 25);
    TEST_EQUAL(spyb0.get_total(), 25);
    TEST_EQUAL(spyb1.get_total(), 25);
    TEST_EQUAL(spyb3.get_total(), 25);

    static const char * results[] = {
	"|2:9|4:7|3:3|6:3|1:1|5:1|8:1|",
	"|1:3|2:3|3:3|4:3|5:3|0:2|6:2|7:2|8:2|9:2|",
	"|",
	"|2:16|1:9|",
	"|2:9|4:7|3:3|6:3|1:1|5:1|8:1|",
	"|1:3|2:3|3:3|4:3|5:3|0:2|6:2|7:2|8:2|9:2|",
	"|",
	"|2:16|1:9|",
	NULL
    };
    std::vector<Xapian::ValueCountMatchSpy *> spies;
    spies.push_back(&spya0);
    spies.push_back(&spya1);
    spies.push_back(NULL);
    spies.push_back(&spya3);
    spies.push_back(&spyb0);
    spies.push_back(&spyb1);
    spies.push_back(NULL);
    spies.push_back(&spyb3);
    for (Xapian::valueno v = 0; results[v]; ++v) {
	tout << "value " << v << endl;
	Xapian::ValueCountMatchSpy * spy = spies[v];
	string allvals_str("|");
	if (spy != NULL) {
	    size_t allvals_size = 0;
	    for (Xapian::TermIterator i = spy->top_values_begin(100);
		 i != spy->top_values_end(100);
		 ++i, ++allvals_size) {
		allvals_str += *i;
		allvals_str += ':';
		allvals_str += str(i.get_termfreq());
		allvals_str += '|';
	    }
	    tout << allvals_str << endl;
	    TEST_STRINGS_EQUAL(allvals_str, results[v]);

	    for (size_t count = 0; count < allvals_size; ++count) {
		tout << "count " << count << endl;
		for (Xapian::TermIterator i = spy->top_values_begin(100),
		     j = spy->top_values_begin(count);
		     i != spy->top_values_end(100) &&
		     j != spy->top_values_end(count);
		     ++i, ++j) {
		    tout << "j " << j << endl;
		    TEST_EQUAL(*i, *j);
		    TEST_EQUAL(i.get_termfreq(), j.get_termfreq());
		}
	    }
	}
    }

    return true;
}

// Test builtin match spies
DEFINE_TESTCASE(matchspy5, backend)
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("this"));

    Xapian::ValueCountMatchSpy myspy1(1);
    Xapian::ValueCountMatchSpy myspy2(1);

    enquire.add_matchspy(&myspy1);
    enquire.add_matchspy(&myspy2);
    Xapian::MSet mymset = enquire.get_mset(0, 100);
    TEST_EQUAL(mymset.size(), 6);

    Xapian::TermIterator i = myspy1.values_begin();
    TEST(i != myspy1.values_end());
    TEST(*i == "h");
    TEST_EQUAL(i.get_termfreq(), 5);
    ++i;
    TEST(i != myspy1.values_end());
    TEST(*i == "n");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == myspy1.values_end());

    i = myspy2.values_begin();
    TEST(i != myspy2.values_end());
    TEST(*i == "h");
    TEST_EQUAL(i.get_termfreq(), 5);
    ++i;
    TEST(i != myspy2.values_end());
    TEST(*i == "n");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == myspy2.values_end());

    return true;
}

class MySpy : public Xapian::MatchSpy {
    void operator()(const Xapian::Document &, Xapian::weight) {
    }
};

// Test exceptions from matchspy base class, and get_description method.
DEFINE_TESTCASE(matchspy6, !backend)
{
    MySpy spy;

    TEST_EXCEPTION(Xapian::UnimplementedError, spy.clone());
    TEST_EXCEPTION(Xapian::UnimplementedError, spy.name());
    TEST_EXCEPTION(Xapian::UnimplementedError, spy.serialise());
    TEST_EXCEPTION(Xapian::UnimplementedError,
		   spy.unserialise(std::string(), Xapian::Registry()));
    TEST_EXCEPTION(Xapian::UnimplementedError, spy.serialise_results());
    TEST_EXCEPTION(Xapian::UnimplementedError,
		   spy.merge_results(std::string()));
    TEST_EQUAL(spy.get_description(), "Xapian::MatchSpy()");

    return true;
}

/// Test that NumericRange comparisons work correctly.
DEFINE_TESTCASE(numericrange1, !backend)
{
    Xapian::NumericRange n1(0, 0);
    Xapian::NumericRange n2(0, 1);
    Xapian::NumericRange n3(1, 1);
    Xapian::NumericRange n4(2, 1);

    TEST(!(n1 < n1));
    TEST(n1 < n2);
    TEST(!(n2 < n1));
    TEST(n2 < n3);
    TEST(!(n3 < n2));
    TEST(n3 < n4);
    TEST(!(n4 < n3));
    return true;
}

// Regression test for underflow in numeric ranges.
// See ticket #321 for more details.
DEFINE_TESTCASE(numericrange2, writable)
{
    Xapian::WritableDatabase db = get_writable_database();
    static const double values[] = { 11.95, 14.50, 60.00 };

    int j;
    for (j = 0; j != 3; ++j) {
	Xapian::Document doc;
	doc.add_value(0, Xapian::sortable_serialise(values[j]));
	db.add_document(doc);
    }
    db.flush();
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query::MatchAll);
    Xapian::ValueCountMatchSpy spy(0);
    enq.add_matchspy(&spy);
    enq.get_mset(0, 10);

    Xapian::UnbiasedNumericRanges ranges(spy, 1000);
    const std::map<Xapian::NumericRange, Xapian::doccount> & r = ranges.get_ranges();

    TEST_EQUAL(r.size(), 3);
    std::map<Xapian::NumericRange, Xapian::doccount>::const_iterator i;
    for (j = 0, i = r.begin(); i != r.end(); ++i, ++j) {
	TEST_EQUAL(i->first.get_lower(), i->first.get_upper());
	TEST_EQUAL(i->second, 1);
	TEST_EQUAL(i->first.get_lower(), values[j]);
    }

    return true;
}

static void
make_matchspy7_db(Xapian::WritableDatabase &db, const string &)
{
    {
	Xapian::Document doc;
	doc.add_term("all");
	Xapian::StringListSerialiser s;
	s.append("foo");
	s.append(Xapian::sortable_serialise(2));
	s.append("bar");
	s.append(Xapian::sortable_serialise(3));
	doc.add_value(0, s.get());
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("all");
	Xapian::StringListSerialiser s;
	s.append("food");
	s.append(Xapian::sortable_serialise(1));
	s.append("bar");
	s.append(Xapian::sortable_serialise(4));
	doc.add_value(0, s.get());
	db.add_document(doc);
    }
}

// Test MultiValueSumMatchSpy.
DEFINE_TESTCASE(matchspy7, generated)
{
    Xapian::Database db = get_database("matchspy7", make_matchspy7_db);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("all"));

    Xapian::MultiValueSumMatchSpy myspy(0);

    enquire.add_matchspy(&myspy);
    Xapian::MSet mymset = enquire.get_mset(0, 100);
    TEST_EQUAL(mymset.size(), 2);

    Xapian::TermIterator i = myspy.values_begin();
    TEST(i != myspy.values_end());
    TEST(*i == "bar");
    TEST_EQUAL(i.get_termfreq(), 7);
    ++i;
    TEST(i != myspy.values_end());
    TEST(*i == "foo");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST(i != myspy.values_end());
    TEST(*i == "food");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == myspy.values_end());

    return true;
}
