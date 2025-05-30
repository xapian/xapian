/** @file
 * @brief tests which don't use any of the backends
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002-2024 Olly Betts
 * Copyright 2006 Lemur Consulting Ltd
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

#include "api_nodb.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

#include <list>
#include <string>
#include <vector>

using namespace std;

// tests that get_query_terms() returns the terms in the right order
DEFINE_TESTCASE(getqterms1, !backend) {
    list<string> answers_list;
    answers_list.push_back("one");
    answers_list.push_back("two");
    answers_list.push_back("three");
    answers_list.push_back("four");

    Xapian::Query myquery(Xapian::Query::OP_OR,
	    Xapian::Query(Xapian::Query::OP_AND,
		    Xapian::Query("one", 1, 1),
		    Xapian::Query("three", 1, 3)),
	    Xapian::Query(Xapian::Query::OP_OR,
		    Xapian::Query("four", 1, 4),
		    Xapian::Query("two", 1, 2)));

    list<string> list1;
    {
	Xapian::TermIterator t;
	for (t = myquery.get_terms_begin(); t != myquery.get_terms_end(); ++t)
	    list1.push_back(*t);
    }
    TEST(list1 == answers_list);
    list<string> list2(myquery.get_terms_begin(), myquery.get_terms_end());
    TEST(list2 == answers_list);
}

// tests that get_query_terms() doesn't SEGV on an empty query
// (regression test for bug in 0.9.0)
DEFINE_TESTCASE(getqterms2, !backend) {
    Xapian::Query empty_query;
    TEST_EQUAL(empty_query.get_terms_begin(), empty_query.get_terms_end());
    TEST_EQUAL(empty_query.get_unique_terms_begin(),
	       empty_query.get_unique_terms_end());
}

// tests that empty queries work correctly
DEFINE_TESTCASE(emptyquery2, !backend) {
    // test that Query::empty() is true for an empty query.
    TEST(Xapian::Query().empty());
    // test that an empty query has length 0
    TEST(Xapian::Query().get_length() == 0);
    vector<Xapian::Query> v;
    TEST(Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end()).empty());
    TEST(Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end()).get_length() == 0);
}

/// Regression test for behaviour for an empty query with AND_NOT.
DEFINE_TESTCASE(emptyquery3, !backend) {
    static const Xapian::Query::op ops[] = {
	Xapian::Query::OP_AND,
	Xapian::Query::OP_OR,
	Xapian::Query::OP_XOR,
	Xapian::Query::OP_AND_MAYBE,
	Xapian::Query::OP_AND_NOT
    };

    for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); ++i) {
	tout << "Testing op #" << i << '\n';
	Xapian::Query empty;
	Xapian::Query q("test");
	Xapian::Query qcombine(ops[i], empty, q);
	tout << qcombine.get_description() << '\n';
	Xapian::Query qcombine2(ops[i], q, empty);
	tout << qcombine2.get_description() << '\n';
	Xapian::Query qcombine3(ops[i], empty, empty);
	tout << qcombine3.get_description() << '\n';
    }
}

// tests that query lengths are calculated correctly
DEFINE_TESTCASE(querylen1, !backend) {
    // test that a simple query has the right length
    Xapian::Query myquery;
    myquery = Xapian::Query(Xapian::Query::OP_OR,
		      Xapian::Query("foo"),
		      Xapian::Query("bar"));
    myquery = Xapian::Query(Xapian::Query::OP_AND,
		      myquery,
		      Xapian::Query(Xapian::Query::OP_OR,
			      Xapian::Query("wibble"),
			      Xapian::Query("spoon")));

    TEST_EQUAL(myquery.get_length(), 4);
    TEST(!myquery.empty());
}

// tests that query lengths are calculated correctly
DEFINE_TESTCASE(querylen2, !backend) {
    // test with an even bigger and strange query
    string terms[3] = {
	"foo",
	"bar",
	"baz"
    };
    Xapian::Query queries[3] = {
	Xapian::Query("wibble"),
	Xapian::Query("wobble"),
	Xapian::Query(Xapian::Query::OP_OR, string("jelly"), string("belly"))
    };

    Xapian::Query myquery;
    vector<string> v1(terms, terms + 3);
    vector<Xapian::Query> v2(queries, queries + 3);
    vector<Xapian::Query *> v3;
    Xapian::Query query1(Xapian::Query::OP_AND, string("ball"), string("club"));
    Xapian::Query query2("ring");
    v3.push_back(&query1);
    v3.push_back(&query2);

    Xapian::Query myq1 = Xapian::Query(Xapian::Query::OP_AND, v1.begin(), v1.end());
    tout << "myq1=" << myq1 << "\n";
    TEST_EQUAL(myq1.get_length(), 3);

    Xapian::Query myq2_1 = Xapian::Query(Xapian::Query::OP_OR, v2.begin(), v2.end());
    tout << "myq2_1=" << myq2_1 << "\n";
    TEST_EQUAL(myq2_1.get_length(), 4);

    Xapian::Query myq2_2 = Xapian::Query(Xapian::Query::OP_AND, v3.begin(), v3.end());
    tout << "myq2_2=" << myq2_2 << "\n";
    TEST_EQUAL(myq2_2.get_length(), 3);

    Xapian::Query myq2 = Xapian::Query(Xapian::Query::OP_OR, myq2_1, myq2_2);
    tout << "myq2=" << myq2 << "\n";
    TEST_EQUAL(myq2.get_length(), 7);

    myquery = Xapian::Query(Xapian::Query::OP_OR, myq1, myq2);
    tout << "myquery=" << myquery << "\n";
    TEST_EQUAL(myquery.get_length(), 10);
}

/** Check we no longer flatten subqueries combined with the same operator.
 *
 *  Prior to 1.3.0 we did flatten these, but it's simpler to just handle this
 *  when we convert the query to a PostList tree, and that works better with
 *  Query objects being immutable.
 */
DEFINE_TESTCASE(dontflattensubqueries1, !backend) {
    Xapian::Query queries1[3] = {
	Xapian::Query("wibble"),
	Xapian::Query("wobble"),
	Xapian::Query(Xapian::Query::OP_OR, string("jelly"), string("belly"))
    };

    Xapian::Query queries2[3] = {
	Xapian::Query(Xapian::Query::OP_AND, string("jelly"), string("belly")),
	Xapian::Query("wibble"),
	Xapian::Query("wobble")
    };

    vector<Xapian::Query> vec1(queries1, queries1 + 3);
    Xapian::Query myquery1(Xapian::Query::OP_OR, vec1.begin(), vec1.end());
    TEST_EQUAL(myquery1.get_description(),
	       "Query((wibble OR wobble OR (jelly OR belly)))");

    vector<Xapian::Query> vec2(queries2, queries2 + 3);
    Xapian::Query myquery2(Xapian::Query::OP_AND, vec2.begin(), vec2.end());
    TEST_EQUAL(myquery2.get_description(),
	       "Query(((jelly AND belly) AND wibble AND wobble))");
}

// test behaviour when creating a query from an empty vector
DEFINE_TESTCASE(emptyquerypart1, !backend) {
    vector<string> emptyterms;
    Xapian::Query query(Xapian::Query::OP_OR, emptyterms.begin(), emptyterms.end());
    TEST(Xapian::Query(Xapian::Query::OP_AND, query, Xapian::Query("x")).empty());
    TEST(Xapian::Query(Xapian::Query::OP_AND, query, Xapian::Query("x")).get_length() == 0);
    TEST(!Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("x")).empty());
    TEST(Xapian::Query(Xapian::Query::OP_OR, query, Xapian::Query("x")).get_length() == 1);
}

DEFINE_TESTCASE(stemlangs1, !backend) {
    string langs = Xapian::Stem::get_available_languages();
    tout << "available languages '" << langs << "'\n";
    TEST(!langs.empty());

    // Also test the language codes.
    langs += " ar hy eu ca da nl en fi fr de hu id ga it lt ne nb nn no pt ro"
	     " ru es sv ta tr";

    string::size_type i = 0;
    while (true) {
	string::size_type spc = langs.find(' ', i);
	// The only spaces in langs should be a single one between each pair
	// of language names.
	TEST_NOT_EQUAL(i, spc);

	// Try making a stemmer for this language.  We should be able to create
	// it without an exception being thrown.
	string language(langs, i, spc - i);
	tout << "checking language code '" << language << "' works\n";
	Xapian::Stem stemmer(language);
	TEST(!stemmer.is_none());
	if (language.size() > 2) {
	    string expected("Xapian::Stem(");
	    expected += language;
	    expected += ')';
	    TEST_EQUAL(stemmer.get_description(), expected);
	}

	if (spc == string::npos) break;
	i = spc + 1;
    }

    {
	// Stem("none") should give a no-op stemmer.
	Xapian::Stem stem_nothing = Xapian::Stem("none");
	TEST(stem_nothing.is_none());
	TEST_EQUAL(stem_nothing.get_description(), "Xapian::Stem(none)");
    }

    {
	// Stem("") should be equivalent.
	Xapian::Stem stem_nothing = Xapian::Stem("");
	TEST(stem_nothing.is_none());
	TEST_EQUAL(stem_nothing.get_description(), "Xapian::Stem(none)");
    }
}

// Regression test.
DEFINE_TESTCASE(nosuchdb1, !backend) {
    // This is a "nodb" test because it doesn't test a particular backend.
    try {
	Xapian::Database db("NOsuChdaTabASe");
	FAIL_TEST("Managed to open 'NOsuChdaTabASe'");
    } catch (const Xapian::DatabaseOpeningError & e) {
	// We don't really require this exact message, but in Xapian <= 1.1.0
	// this gave "Couldn't detect type of database".
	TEST_STRINGS_EQUAL(e.get_msg(), "Couldn't stat 'NOsuChdaTabASe'");
    }

    try {
	Xapian::Database::check("NOsuChdaTabASe");
	FAIL_TEST("Managed to check 'NOsuChdaTabASe'");
    } catch (const Xapian::DatabaseOpeningError & e) {
	// In 1.4.3 and earlier, this threw DatabaseError with the message:
	// "File is not a Xapian database or database table" (confusing as
	// there is no file).
	TEST_STRINGS_EQUAL(e.get_msg(),
			   "Couldn't find Xapian database or table to check");
    }
}

// Feature tests for value manipulations.
DEFINE_TESTCASE(addvalue1, !backend) {
    // Regression test for add_value on an existing value (bug#82).
    Xapian::Document doc;
    doc.add_value(1, "original");
    doc.add_value(1, "replacement");
    TEST_EQUAL(doc.get_value(1), "replacement");

    doc.add_value(2, "too");
    doc.add_value(3, "free");
    doc.add_value(4, "for");

    doc.remove_value(2);
    doc.remove_value(4);
    TEST_EQUAL(doc.get_value(0), "");
    TEST_EQUAL(doc.get_value(1), "replacement");
    TEST_EQUAL(doc.get_value(2), "");
    TEST_EQUAL(doc.get_value(3), "free");
    TEST_EQUAL(doc.get_value(4), "");
}

// tests that the collapsing on termpos optimisation gives correct query length
DEFINE_TESTCASE(poscollapse2, !backend) {
    Xapian::Query q(Xapian::Query::OP_OR, Xapian::Query("this", 1, 1), Xapian::Query("this", 1, 1));
    TEST_EQUAL(q.get_length(), 2);
}

// Regression test: query on an uninitialised database segfaulted with 1.0.0.
// As of 1.5.0, this is just handled as an empty database.
DEFINE_TESTCASE(uninitdb1, !backend) {
    Xapian::Database db;
    Xapian::Enquire enq(db);
}

// Test a scaleweight query applied to a match nothing query
DEFINE_TESTCASE(scaleweight3, !backend) {
    Xapian::Query matchnothing(Xapian::Query::MatchNothing);
    Xapian::Query query(Xapian::Query::OP_SCALE_WEIGHT, matchnothing, 3.0);
    TEST_EQUAL(query.get_description(), "Query()");
}

// Regression test - before 1.1.0, you could add docid 0 to an RSet.
DEFINE_TESTCASE(rset3, !backend) {
    Xapian::RSet rset;
    TEST_EXCEPTION(Xapian::InvalidArgumentError, rset.add_document(0));
    TEST(rset.empty());
    TEST_EQUAL(rset.size(), 0);
    rset.add_document(1);
    rset.add_document(static_cast<Xapian::docid>(-1));
    TEST_EXCEPTION(Xapian::InvalidArgumentError, rset.add_document(0));
    TEST(!rset.empty());
    TEST_EQUAL(rset.size(), 2);
}

// Regression test - RSet::get_description() gave a malformed answer in 1.0.7.
DEFINE_TESTCASE(rset4, !backend) {
    Xapian::RSet rset;
    TEST_STRINGS_EQUAL(rset.get_description(), "RSet()");
    rset.add_document(2);
    // In 1.0.7 this gave: RSet(RSet(RSet::Internal(, 2))
    TEST_STRINGS_EQUAL(rset.get_description(), "RSet(2)");
    rset.add_document(1);
    TEST_STRINGS_EQUAL(rset.get_description(), "RSet(1,2)");
}

// Direct test of ValueSetMatchDecider
DEFINE_TESTCASE(valuesetmatchdecider1, !backend) {
    Xapian::ValueSetMatchDecider vsmd1(0, true);
    vsmd1.add_value("42");
    Xapian::ValueSetMatchDecider vsmd2(0, false);
    vsmd2.remove_value("nosuch"); // Test removing a value which isn't present.
    vsmd2.add_value("42");
    Xapian::ValueSetMatchDecider vsmd3(0, true);
    vsmd3.add_value("42");
    vsmd3.add_value("blah");

    Xapian::Document doc;
    TEST(!vsmd1(doc));
    TEST(vsmd2(doc));
    TEST(!vsmd3(doc));
    doc.add_value(0, "42");
    TEST(vsmd1(doc));
    TEST(!vsmd2(doc));
    TEST(vsmd3(doc));
    doc.add_value(0, "blah");
    TEST(!vsmd1(doc));
    TEST(vsmd2(doc));
    TEST(vsmd3(doc));

    vsmd3.remove_value("nosuch"); // Test removing a value which isn't present.
    vsmd3.remove_value("blah");
    TEST(!vsmd1(doc));
    TEST(vsmd2(doc));
    TEST(!vsmd3(doc));
    doc.add_value(0, "42");
    TEST(vsmd1(doc));
    TEST(!vsmd2(doc));
    TEST(vsmd3(doc));
}

// Test that requesting termfreq or termweight on an empty mset returns 0.
// New behaviour as of 1.5.0 - previously both methods threw
// Xapian::InvalidOperationError.
DEFINE_TESTCASE(emptymset1, !backend) {
    Xapian::MSet emptymset;
    TEST_EQUAL(emptymset.get_termfreq("foo"), 0);
    TEST_EQUAL(emptymset.get_termweight("foo"), 0.0);
}

DEFINE_TESTCASE(expanddeciderfilterprefix1, !backend) {
    string prefix = "tw";
    Xapian::ExpandDeciderFilterPrefix decider(prefix);
    TEST(!decider("one"));
    TEST(!decider("t"));
    TEST(!decider(""));
    TEST(!decider("Two"));
    TEST(decider("two"));
    TEST(decider("twitter"));
    TEST(decider(prefix));
}
