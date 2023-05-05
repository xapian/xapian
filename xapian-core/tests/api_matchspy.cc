/** @file
 * @brief tests of MatchSpy usage
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2012,2015,2019 Olly Betts
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

#include <vector>

#include "backendmanager.h"
#include "str.h"
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

    void operator()(const Xapian::Document &doc, double) {
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

static void
make_matchspy2_db(Xapian::WritableDatabase &db, const string &)
{
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
}

DEFINE_TESTCASE(matchspy2, backend)
{
    Xapian::Database db = get_database("matchspy2", make_matchspy2_db);

    Xapian::ValueCountMatchSpy spy0(0);
    Xapian::ValueCountMatchSpy spy1(1);
    Xapian::ValueCountMatchSpy spy3(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));
    if (db.size() > 1) {
	// Without this, we short-cut on the second shard because we don't get
	// the documents in ascending weight order.
	enq.set_weighting_scheme(Xapian::CoordWeight());
    }

    enq.add_matchspy(&spy0);
    enq.add_matchspy(&spy1);
    enq.add_matchspy(&spy3);
    Xapian::MSet mset = enq.get_mset(0, 10);

    TEST_EQUAL(spy0.get_total(), 25);
    TEST_EQUAL(spy1.get_total(), 25);
    TEST_EQUAL(spy3.get_total(), 25);

    static const char * const results[] = {
	"|1:1|2:9|3:3|4:7|5:1|6:3|8:1|",
	"|0:2|1:3|2:3|3:3|4:3|5:3|6:2|7:2|8:2|9:2|",
	"|1:9|2:16|",
    };
    TEST_STRINGS_EQUAL(values_to_repr(spy0), results[0]);
    TEST_STRINGS_EQUAL(values_to_repr(spy1), results[1]);
    TEST_STRINGS_EQUAL(values_to_repr(spy3), results[2]);
}

DEFINE_TESTCASE(matchspy4, backend)
{
    XFAIL_FOR_BACKEND("multi_remote",
		      "Matchspy counts hits on remote and locally");
    XFAIL_FOR_BACKEND("multi_glass_remote",
		      "Matchspy counts hits on remote and locally");

    Xapian::Database db = get_database("matchspy2", make_matchspy2_db);

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
    if (db.size() > 1) {
	// Without this, we short-cut on the second shard because we don't get
	// the documents in ascending weight order.
	enqa.set_weighting_scheme(Xapian::CoordWeight());
    }
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

    static const char * const results[] = {
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
	tout << "value " << v << '\n';
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
	    tout << allvals_str << '\n';
	    TEST_STRINGS_EQUAL(allvals_str, results[v]);

	    for (size_t count = 0; count < allvals_size; ++count) {
		tout << "count " << count << '\n';
		for (Xapian::TermIterator i = spy->top_values_begin(100),
		     j = spy->top_values_begin(count);
		     i != spy->top_values_end(100) &&
		     j != spy->top_values_end(count);
		     ++i, ++j) {
		    tout << "j " << j << '\n';
		    TEST_EQUAL(*i, *j);
		    TEST_EQUAL(i.get_termfreq(), j.get_termfreq());
		}
	    }
	}
    }
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
}

class MySpy : public Xapian::MatchSpy {
    void operator()(const Xapian::Document &, double) {
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
}

/// Regression test for bug fixed in 1.4.12.
DEFINE_TESTCASE(matchspy7, !backend)
{
    Xapian::ValueCountMatchSpy myspy(1);
    string s = myspy.serialise_results();
    s += 'x';
    // This merge_results() call used to enter an infinite loop.
    TEST_EXCEPTION(Xapian::NetworkError, myspy.merge_results(s));
}
