/** @file api_opsynonym.cc
 * @brief tests of MatchSpy usage
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
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
    Xapian::MSet spymset = enquire.get_mset_with_matchspy(0, 100, 0, NULL, &myspy);

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

DEFINE_TESTCASE(matchspy2, writable)
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
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

    Xapian::CategorySelectMatchSpy spy;

    spy.add_slot(0);
    spy.add_slot(1);
    spy.add_slot(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    Xapian::MSet mset = enq.get_mset_with_matchspy(0, 10, 0, NULL, &spy);

    TEST_EQUAL(spy.get_total(), 25);

    static const char * results[] = {
	"|1:1|2:9|3:3|4:7|5:1|6:3|8:1|",
	"|0:2|1:3|2:3|3:3|4:3|5:3|6:2|7:2|8:2|9:2|",
	"|",
	"|1:9|2:16|",
	"|",
	NULL
    };
    for (Xapian::valueno v = 0; results[v]; ++v) {
	const map<string, Xapian::doccount> & cat = spy.get_values(v);
	string resultrepr("|");
	map<string, Xapian::doccount>::const_iterator i;
	for (i = cat.begin(); i != cat.end(); ++i) {
	    resultrepr += i->first;
	    resultrepr += ':';
	    resultrepr += str(i->second);
	    resultrepr += '|';
	}
	tout << "value " << v << endl;
	TEST_STRINGS_EQUAL(resultrepr, results[v]);
    }

    {
	// Test scoring evenness returns scores with the natural ordering.
	double score0 = spy.score_categorisation(0);
	tout << "score0 = " << score0 << endl;
	double score1 = spy.score_categorisation(1);
	tout << "score1 = " << score1 << endl;
	double score3 = spy.score_categorisation(3);
	tout << "score3 = " << score3 << endl;
	// 1 is obviously best, and 0 obviously worst.
	TEST(score1 < score3);
	TEST(score3 < score0);
    }

    {
	// Test scoring evenness and about 7 categories returns scores with the
	// natural ordering.
	double score0 = spy.score_categorisation(0, 7);
	tout << "score0 = " << score0 << endl;
	double score1 = spy.score_categorisation(1, 7);
	tout << "score1 = " << score1 << endl;
	double score3 = spy.score_categorisation(3, 7);
	tout << "score3 = " << score3 << endl;
	// 3 is clearly worst - 0 is arguably a little better than 1 (0 is the
	// requested size, but 1 has a much more even split).
	TEST(score0 < score1);
	TEST(score1 < score3);
    }

    return true;
}

DEFINE_TESTCASE(matchspy3, writable)
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
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
	doc.add_value(3, Xapian::sortable_serialise(1.0 / c));

	db.add_document(doc);
    }

    Xapian::CategorySelectMatchSpy spy;

    spy.add_slot(0);
    spy.add_slot(1);
    spy.add_slot(2);
    spy.add_slot(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    Xapian::MSet mset = enq.get_mset_with_matchspy(0, 10, 0, NULL, &spy);

    TEST_EQUAL(spy.get_total(), 25);

    static const string results[] = {
	"|100:1|200:9|300:3|400:7|500:1|600:3|800:1|",
	"|0..200:8|300..400:6|500..700:7|800..900:4|",
	"|177..8711:9|10677..17777:4|20544..26677:3|30044..37377:3|41344..49877:3|54444..59211:2|64177:1|",
	"|4..9:15|10..16:5|20..25:2|33:1|50:1|100:1|",
	"|",
	""
    };
    for (Xapian::valueno v = 0; !results[v].empty(); ++v) {
	bool result = spy.build_numeric_ranges(v, 7);
	if (results[v] == "|") {
	    TEST(!result);
	    continue;
	}
	TEST(result);
	const map<string, Xapian::doccount> & cat = spy.get_values(v);
	TEST(cat.size() <= 7);
	string resultrepr("|");
	map<string, Xapian::doccount>::const_iterator i;
	for (i = cat.begin(); i != cat.end(); ++i) {
	    if (i->first.size() > 9) {
		double start = Xapian::sortable_unserialise((i->first).substr(0, 9));
		double end = Xapian::sortable_unserialise((i->first).substr(9));
		start = floor(start * 100);
		end = floor(end * 100);
		resultrepr += str(start);
		resultrepr += "..";
		resultrepr += str(end);
	    } else {
		double start = Xapian::sortable_unserialise((i->first).substr(0, 9));
		start = floor(start * 100);
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

    Xapian::WritableDatabase db = get_writable_database("");
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

    Xapian::ValueCountMatchSpy spy;

    spy.add_slot(0);
    spy.add_slot(1);
    spy.add_slot(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    Xapian::MSet mset = enq.get_mset_with_matchspy(0, 10, 0, NULL, &spy);

    TEST_EQUAL(spy.get_total(), 25);

    static const char * results[] = {
	"|2:9|4:7|3:3|6:3|1:1|5:1|8:1|",
	"|1:3|2:3|3:3|4:3|5:3|0:2|6:2|7:2|8:2|9:2|",
	"|",
	"|2:16|1:9|",
	"|",
	NULL
    };
    for (Xapian::valueno v = 0; results[v]; ++v) {
	tout << "value " << v << endl;
	std::vector<Xapian::StringAndFrequency> allvals;

	spy.get_top_values(allvals, v, 100);
	string allvals_str("|");
	for (size_t i = 0; i < allvals.size(); i++) {
	    allvals_str += allvals[i].str;
	    allvals_str += ':';
	    allvals_str += str(allvals[i].frequency);
	    allvals_str += '|';
	}
	tout << allvals_str << endl;
	TEST_STRINGS_EQUAL(allvals_str, results[v]);

	std::vector<Xapian::StringAndFrequency> vals;
	for (size_t i = 0; i < allvals.size(); i++) {
	    tout << "i " << i << endl;
	    spy.get_top_values(vals, v, i);
	    for (size_t j = 0; j < vals.size(); j++) {
		tout << "j " << j << endl;
		TEST_EQUAL(vals[j].str, allvals[j].str);
		TEST_EQUAL(vals[j].frequency, allvals[j].frequency);
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
    Xapian::TermCountMatchSpy myspy3("h");
    Xapian::MultipleMatchSpy multispy;
    multispy.append(&myspy1);
    multispy.append(&myspy2);
    multispy.append(&myspy3);

    Xapian::MSet mymset = enquire.get_mset_with_matchspy(0, 100, 0, NULL, &multispy);
    TEST_EQUAL(mymset.size(), 6);

    const std::map<std::string, Xapian::doccount> & vals1 = myspy1.get_values(1);
    const std::map<std::string, Xapian::doccount> & vals2 = myspy2.get_values(1);
    const std::map<std::string, Xapian::doccount> & vals3 = myspy3.get_terms("h");

    TEST_EQUAL(vals1.size(), 2);
    TEST(vals1.find("h") != vals1.end());
    TEST(vals1.find("n") != vals1.end());
    TEST_EQUAL(vals1.find("h")->second, 5);
    TEST_EQUAL(vals1.find("n")->second, 1);

    TEST_EQUAL(vals2.size(), 2);
    TEST(vals2.find("h") != vals2.end());
    TEST(vals2.find("n") != vals2.end());
    TEST_EQUAL(vals2.find("h")->second, 5);
    TEST_EQUAL(vals2.find("n")->second, 1);

    TEST_EQUAL(vals3.size(), 3);
    TEST(vals3.find("ack") != vals3.end());
    TEST(vals3.find("as") != vals3.end());
    TEST(vals3.find("ow") != vals3.end());
    TEST_EQUAL(vals3.find("ack")->second, 1);
    TEST_EQUAL(vals3.find("as")->second, 1);
    TEST_EQUAL(vals3.find("ow")->second, 1);

    return true;
}
