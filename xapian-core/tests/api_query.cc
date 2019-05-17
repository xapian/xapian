/** @file api_query.cc
 * @brief Query-related tests.
 */
/* Copyright (C) 2008,2009,2012,2013,2015,2016,2017,2018,2019 Olly Betts
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

#include "api_query.h"

#include <xapian.h>

#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

/// Regression test - in 1.0.10 and earlier "" was included in the list.
DEFINE_TESTCASE(queryterms1, !backend) {
    Xapian::Query query = Xapian::Query::MatchAll;
    TEST(query.get_terms_begin() == query.get_terms_end());
    query = Xapian::Query(query.OP_AND_NOT, query, Xapian::Query("fair"));
    TEST_EQUAL(*query.get_terms_begin(), "fair");
    return true;
}

DEFINE_TESTCASE(matchall2, !backend) {
    TEST_STRINGS_EQUAL(Xapian::Query::MatchAll.get_description(),
		       "Query(<alldocuments>)");
    return true;
}

DEFINE_TESTCASE(matchnothing1, !backend) {
    TEST_STRINGS_EQUAL(Xapian::Query::MatchNothing.get_description(),
		       "Query()");
    vector<Xapian::Query> subqs;
    subqs.push_back(Xapian::Query("foo"));
    subqs.push_back(Xapian::Query::MatchNothing);
    Xapian::Query q(Xapian::Query::OP_AND, subqs.begin(), subqs.end());
    TEST_STRINGS_EQUAL(q.get_description(), "Query()");

    Xapian::Query q2(Xapian::Query::OP_AND,
		     Xapian::Query("foo"), Xapian::Query::MatchNothing);
    TEST_STRINGS_EQUAL(q2.get_description(), "Query()");
    return true;
}

DEFINE_TESTCASE(overload1, !backend) {
    Xapian::Query q;
    q = Xapian::Query("foo") & Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo AND bar))");

    // Test &= appends a same-type subquery (since Xapian 1.4.10).
    q &= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo AND bar AND baz))");
    // But not if the RHS is the same query:
    q = Xapian::Query("foo") & Xapian::Query("bar");
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
    // Suppress warning from newer clang about self-assignment so we can
    // test that self-assignment works!
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wself-assign-overloaded"
# endif
#endif
    q &= q;
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
#  pragma clang diagnostic pop
# endif
#endif
    TEST_STRINGS_EQUAL(q.get_description(), "Query(((foo AND bar) AND (foo AND bar)))");
    {
	// Also not if the query has a refcount > 1.
	q = Xapian::Query("foo") & Xapian::Query("bar");
	Xapian::Query qcopy = q;
	qcopy &= Xapian::Query("baz");
	TEST_STRINGS_EQUAL(qcopy.get_description(), "Query(((foo AND bar) AND baz))");
	// And q shouldn't change.
	TEST_STRINGS_EQUAL(q.get_description(), "Query((foo AND bar))");
    }
    // Check that MatchNothing still results in MatchNothing:
    q = Xapian::Query("foo") & Xapian::Query("bar");
    q &= Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query()");
    // Check we don't combine for other operators:
    q = Xapian::Query("foo") | Xapian::Query("bar");
    q &= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query(((foo OR bar) AND baz))");

    // Test |= appends a same-type subquery (since Xapian 1.4.10).
    q = Xapian::Query("foo") | Xapian::Query("bar");
    q |= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo OR bar OR baz))");
    // But not if the RHS is the same query:
    q = Xapian::Query("foo") | Xapian::Query("bar");
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
    // Suppress warning from newer clang about self-assignment so we can
    // test that self-assignment works!
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wself-assign-overloaded"
# endif
#endif
    q |= q;
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
#  pragma clang diagnostic pop
# endif
#endif
    TEST_STRINGS_EQUAL(q.get_description(), "Query(((foo OR bar) OR (foo OR bar)))");
    {
	// Also not if the query has a refcount > 1.
	q = Xapian::Query("foo") | Xapian::Query("bar");
	Xapian::Query qcopy = q;
	qcopy |= Xapian::Query("baz");
	TEST_STRINGS_EQUAL(qcopy.get_description(), "Query(((foo OR bar) OR baz))");
	// And q shouldn't change.
	TEST_STRINGS_EQUAL(q.get_description(), "Query((foo OR bar))");
    }
    // Check that MatchNothing still results in no change:
    q = Xapian::Query("foo") | Xapian::Query("bar");
    q |= Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo OR bar))");
    // Check we don't combine for other operators:
    q = Xapian::Query("foo") & Xapian::Query("bar");
    q |= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query(((foo AND bar) OR baz))");

    // Test ^= appends a same-type subquery (since Xapian 1.4.10).
    q = Xapian::Query("foo") ^ Xapian::Query("bar");
    q ^= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo XOR bar XOR baz))");
    // But a query ^= itself gives an empty query.
    q = Xapian::Query("foo") ^ Xapian::Query("bar");
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
    // Suppress warning from newer clang about self-assignment so we can
    // test that self-assignment works!
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wself-assign-overloaded"
# endif
#endif
    q ^= q;
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
#  pragma clang diagnostic pop
# endif
#endif
    TEST_STRINGS_EQUAL(q.get_description(), "Query()");
    {
	// Even if the reference count > 1.
	q = Xapian::Query("foo") ^ Xapian::Query("bar");
	Xapian::Query qcopy = q;
	q ^= qcopy;
	TEST_STRINGS_EQUAL(q.get_description(), "Query()");
    }
    {
	// Also not if the query has a refcount > 1.
	q = Xapian::Query("foo") ^ Xapian::Query("bar");
	Xapian::Query qcopy = q;
	qcopy ^= Xapian::Query("baz");
	TEST_STRINGS_EQUAL(qcopy.get_description(), "Query(((foo XOR bar) XOR baz))");
	// And q shouldn't change.
	TEST_STRINGS_EQUAL(q.get_description(), "Query((foo XOR bar))");
    }
    // Check that MatchNothing still results in no change:
    q = Xapian::Query("foo") ^ Xapian::Query("bar");
    q ^= Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo XOR bar))");
    // Check we don't combine for other operators:
    q = Xapian::Query("foo") & Xapian::Query("bar");
    q ^= Xapian::Query("baz");
    TEST_STRINGS_EQUAL(q.get_description(), "Query(((foo AND bar) XOR baz))");

    q = Xapian::Query("foo") &~ Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo AND_NOT bar))");
    // In 1.4.9 and earlier this gave (foo AND (<alldocuments> AND_NOT bar)).
    q = Xapian::Query("foo");
    q &= ~Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo AND_NOT bar))");
    q = ~Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((<alldocuments> AND_NOT bar))");
    q = Xapian::Query("foo") & Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query()");
    q = Xapian::Query("foo") | Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo OR bar))");
    q = Xapian::Query("foo") | Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query(foo)");
    q = Xapian::Query("foo") ^ Xapian::Query("bar");
    TEST_STRINGS_EQUAL(q.get_description(), "Query((foo XOR bar))");
    q = Xapian::Query("foo") ^ Xapian::Query::MatchNothing;
    TEST_STRINGS_EQUAL(q.get_description(), "Query(foo)");
    q = 1.25 * (Xapian::Query("one") | Xapian::Query("two"));
    TEST_STRINGS_EQUAL(q.get_description(), "Query(1.25 * (one OR two))");
    q = (Xapian::Query("one") & Xapian::Query("two")) * 42;
    TEST_STRINGS_EQUAL(q.get_description(), "Query(42 * (one AND two))");
    q = Xapian::Query("one") / 2.0;
    TEST_STRINGS_EQUAL(q.get_description(), "Query(0.5 * one)");
    return true;
}

/** Regression test and feature test.
 *
 *  This threw AssertionError in 1.0.9 and earlier (bug#201) and gave valgrind
 *  errors in 1.0.11 and earlier (bug#349).
 *
 *  Currently the OR-subquery case is supported, other operators aren't.
 */
DEFINE_TESTCASE(possubqueries1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_posting("a", 1);
    doc.add_posting("b", 2);
    doc.add_posting("c", 3);
    db.add_document(doc);

    Xapian::Query a_or_b(Xapian::Query::OP_OR,
			 Xapian::Query("a"),
			 Xapian::Query("b"));
    Xapian::Query near(Xapian::Query::OP_NEAR, a_or_b, a_or_b);
    // As of 1.3.0, we no longer rearrange queries at this point, so check
    // that we don't.
    TEST_STRINGS_EQUAL(near.get_description(),
		       "Query(((a OR b) NEAR 2 (a OR b)))");
    Xapian::Query phrase(Xapian::Query::OP_PHRASE, a_or_b, a_or_b);
    TEST_STRINGS_EQUAL(phrase.get_description(),
		       "Query(((a OR b) PHRASE 2 (a OR b)))");

    Xapian::Query a_and_b(Xapian::Query::OP_AND,
			  Xapian::Query("a"),
			  Xapian::Query("b"));
    Xapian::Query a_near_b(Xapian::Query::OP_NEAR,
			   Xapian::Query("a"),
			   Xapian::Query("b"));
    Xapian::Query a_phrs_b(Xapian::Query::OP_PHRASE,
			   Xapian::Query("a"),
			   Xapian::Query("b"));
    Xapian::Query c("c");

    // FIXME: The plan is to actually try to support the cases below, but
    // for now at least ensure they are cleanly rejected.
    Xapian::Enquire enq(db);

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_NEAR, a_and_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_NEAR, a_near_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_NEAR, a_phrs_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_PHRASE, a_and_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_PHRASE, a_near_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::Query q(Xapian::Query::OP_PHRASE, a_phrs_b, c);
	enq.set_query(q);
	(void)enq.get_mset(0, 10));

    return true;
}

/// Test that XOR handles all remaining subqueries running out at the same
//  time.
DEFINE_TESTCASE(xor3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    static const char * const subqs[] = {
	"hack", "which", "paragraph", "is", "return"
    };
    // Document where the subqueries run out *does* match XOR:
    Xapian::Query q(Xapian::Query::OP_XOR, subqs, subqs + 5);
    Xapian::Enquire enq(db);
    enq.set_query(q);
    Xapian::MSet mset = enq.get_mset(0, 10);

    TEST_EQUAL(mset.size(), 3);
    TEST_EQUAL(*mset[0], 4);
    TEST_EQUAL(*mset[1], 2);
    TEST_EQUAL(*mset[2], 3);

    // Document where the subqueries run out *does not* match XOR:
    q = Xapian::Query(Xapian::Query::OP_XOR, subqs, subqs + 4);
    enq.set_query(q);
    mset = enq.get_mset(0, 10);

    TEST_EQUAL(mset.size(), 4);
    TEST_EQUAL(*mset[0], 5);
    TEST_EQUAL(*mset[1], 4);
    TEST_EQUAL(*mset[2], 2);
    TEST_EQUAL(*mset[3], 3);

    return true;
}

/// Check encoding of non-UTF8 terms in query descriptions.
DEFINE_TESTCASE(nonutf8termdesc1, !backend) {
    TEST_EQUAL(Xapian::Query("\xc0\x80\xf5\x80\x80\x80\xfe\xff").get_description(),
	       "Query(\\xc0\\x80\\xf5\\x80\\x80\\x80\\xfe\\xff)");
    TEST_EQUAL(Xapian::Query(string("\x00\x1f", 2)).get_description(),
	       "Query(\\x00\\x1f)");
    // Check that backslashes are encoded so output isn't ambiguous.
    TEST_EQUAL(Xapian::Query("back\\slash").get_description(),
	       "Query(back\\x5cslash)");
    // Check that \x7f is escaped.
    TEST_EQUAL(Xapian::Query("D\x7f_\x7f~").get_description(),
	       "Query(D\\x7f_\\x7f~)");
    return true;
}

/// Test introspection on Query objects.
DEFINE_TESTCASE(queryintro1, !backend) {
    TEST_EQUAL(Xapian::Query::MatchAll.get_type(), Xapian::Query::LEAF_MATCH_ALL);
    TEST_EQUAL(Xapian::Query::MatchAll.get_num_subqueries(), 0);
    TEST_EQUAL(Xapian::Query::MatchNothing.get_type(), Xapian::Query::LEAF_MATCH_NOTHING);
    TEST_EQUAL(Xapian::Query::MatchNothing.get_num_subqueries(), 0);

    Xapian::Query q;
    q = Xapian::Query(q.OP_AND_NOT, Xapian::Query::MatchAll, Xapian::Query("fair"));
    TEST_EQUAL(q.get_type(), q.OP_AND_NOT);
    TEST_EQUAL(q.get_num_subqueries(), 2);
    TEST_EQUAL(q.get_subquery(0).get_type(), q.LEAF_MATCH_ALL);
    TEST_EQUAL(q.get_subquery(1).get_type(), q.LEAF_TERM);

    q = Xapian::Query("foo", 2, 1);
    TEST_EQUAL(q.get_leaf_wqf(), 2);
    TEST_EQUAL(q.get_leaf_pos(), 1);

    q = Xapian::Query("bar");
    TEST_EQUAL(q.get_leaf_wqf(), 1);
    TEST_EQUAL(q.get_leaf_pos(), 0);

    q = Xapian::Query("foo") & Xapian::Query("bar");
    TEST_EQUAL(q.get_type(), q.OP_AND);

    q = Xapian::Query("foo") &~ Xapian::Query("bar");
    TEST_EQUAL(q.get_type(), q.OP_AND_NOT);

    q = ~Xapian::Query("bar");
    TEST_EQUAL(q.get_type(), q.OP_AND_NOT);

    q = Xapian::Query("foo") | Xapian::Query("bar");
    TEST_EQUAL(q.get_type(), q.OP_OR);

    q = Xapian::Query("foo") ^ Xapian::Query("bar");
    TEST_EQUAL(q.get_type(), q.OP_XOR);

    q = 1.25 * (Xapian::Query("one") | Xapian::Query("two"));
    TEST_EQUAL(q.get_type(), q.OP_SCALE_WEIGHT);
    TEST_EQUAL(q.get_num_subqueries(), 1);
    TEST_EQUAL(q.get_subquery(0).get_type(), q.OP_OR);

    q = Xapian::Query("one") / 2.0;
    TEST_EQUAL(q.get_type(), q.OP_SCALE_WEIGHT);
    TEST_EQUAL(q.get_num_subqueries(), 1);
    TEST_EQUAL(q.get_subquery(0).get_type(), q.LEAF_TERM);

    q = Xapian::Query(q.OP_NEAR, Xapian::Query("a"), Xapian::Query("b"));
    TEST_EQUAL(q.get_type(), q.OP_NEAR);
    TEST_EQUAL(q.get_num_subqueries(), 2);
    TEST_EQUAL(q.get_subquery(0).get_type(), q.LEAF_TERM);
    TEST_EQUAL(q.get_subquery(1).get_type(), q.LEAF_TERM);

    q = Xapian::Query(q.OP_PHRASE, Xapian::Query("c"), Xapian::Query("d"));
    TEST_EQUAL(q.get_type(), q.OP_PHRASE);
    TEST_EQUAL(q.get_num_subqueries(), 2);
    TEST_EQUAL(q.get_subquery(0).get_type(), q.LEAF_TERM);
    TEST_EQUAL(q.get_subquery(1).get_type(), q.LEAF_TERM);

    return true;
}

/// Regression test for bug introduced in 1.3.1 and fixed in 1.3.3.
//  We were incorrectly converting a term which indexed all docs and was used
//  in an unweighted phrase into an all docs postlist, so check that this
//  case actually works.
DEFINE_TESTCASE(phrasealldocs1, backend) {
    Xapian::Database db = get_database("apitest_declen");
    Xapian::Query q;
    static const char * const phrase[] = { "this", "is", "the" };
    q = Xapian::Query(q.OP_AND_NOT,
	    Xapian::Query("paragraph"),
	    Xapian::Query(q.OP_PHRASE, phrase, phrase + 3));
    Xapian::Enquire enq(db);
    enq.set_query(q);
    Xapian::MSet mset = enq.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 3);

    return true;
}

struct wildcard_testcase {
    const char * pattern;
    Xapian::termcount max_expansion;
    char max_type;
    const char * terms[4];
};

#define WILDCARD_EXCEPTION { 0, 0, 0, "" }
static const
wildcard_testcase wildcard1_testcases[] = {
    // Tries to expand to 7 terms.
    { "th",	6, 'E', WILDCARD_EXCEPTION },
    { "thou",	1, 'E', { "though", 0, 0, 0 } },
    { "s",	2, 'F', { "say", "search", 0, 0 } },
    { "s",	2, 'M', { "simpl", "so", 0, 0 } }
};

DEFINE_TESTCASE(wildcard1, backend) {
    // FIXME: The counting of terms the wildcard expands to is per subdatabase,
    // so the wildcard may expand to more terms than the limit if some aren't
    // in all subdatabases.  Also WILDCARD_LIMIT_MOST_FREQUENT uses the
    // frequency from the subdatabase, and so may select different terms in
    // each subdatabase.
    SKIP_TEST_FOR_BACKEND("multi");
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    const Xapian::Query::op o = Xapian::Query::OP_WILDCARD;

    for (auto&& test : wildcard1_testcases) {
	tout << test.pattern << endl;
	auto tend = test.terms + 4;
	while (tend[-1] == NULL) --tend;
	bool expect_exception = (tend - test.terms == 4 && tend[-1][0] == '\0');
	Xapian::Query q;
	if (test.max_type) {
	    int max_type;
	    switch (test.max_type) {
		case 'E':
		    max_type = Xapian::Query::WILDCARD_LIMIT_ERROR;
		    break;
		case 'F':
		    max_type = Xapian::Query::WILDCARD_LIMIT_FIRST;
		    break;
		case 'M':
		    max_type = Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT;
		    break;
		default:
		    return false;
	    }
	    q = Xapian::Query(o, test.pattern, test.max_expansion, max_type);
	} else {
	    q = Xapian::Query(o, test.pattern, test.max_expansion);
	}
	enq.set_query(q);
	try {
	    Xapian::MSet mset = enq.get_mset(0, 10);
	    TEST(!expect_exception);
	    q = Xapian::Query(q.OP_SYNONYM, test.terms, tend);
	    enq.set_query(q);
	    Xapian::MSet mset2 = enq.get_mset(0, 10);
	    TEST_EQUAL(mset.size(), mset2.size());
	    TEST(mset_range_is_same(mset, 0, mset2, 0, mset.size()));
	} catch (const Xapian::WildcardError &) {
	    TEST(expect_exception);
	}
    }

    return true;
}

/// Regression test for #696, fixed in 1.3.4.
DEFINE_TESTCASE(wildcard2, backend) {
    // FIXME: The counting of terms the wildcard expands to is per subdatabase,
    // so the wildcard may expand to more terms than the limit if some aren't
    // in all subdatabases.  Also WILDCARD_LIMIT_MOST_FREQUENT uses the
    // frequency from the subdatabase, and so may select different terms in
    // each subdatabase.
    SKIP_TEST_FOR_BACKEND("multi");
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    const Xapian::Query::op o = Xapian::Query::OP_WILDCARD;

    const int max_type = Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT;
    Xapian::Query q0(o, "w", 2, max_type);
    Xapian::Query q(o, "s", 2, max_type);
    Xapian::Query q2(o, "t", 2, max_type);
    q = Xapian::Query(q.OP_OR, q0, q);
    q = Xapian::Query(q.OP_OR, q, q2);
    enq.set_query(q);
    Xapian::MSet mset = enq.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 6);

    return true;
}

/// Regression test for bug fixed in 1.4.12 - if any terms start with A-Z then
//  the next term that doesn't isn't considered.
DEFINE_TESTCASE(wildcard3, generated) {
    Xapian::Database db = get_database("wildcard3",
				       [](Xapian::WritableDatabase& wdb,
					  const string&)
				       {
					   Xapian::Document doc;
					   doc.add_term("Zfoo");
					   doc.add_term("a");
					   wdb.add_document(doc);
					   doc.add_term("abc");
					   wdb.add_document(doc);
				       });

    Xapian::Enquire enq(db);
    Xapian::Query q(Xapian::Query::OP_WILDCARD, "?", 0,
		    Xapian::Query::WILDCARD_PATTERN_GLOB);
    enq.set_query(q);
    Xapian::MSet mset = enq.get_mset(0, 10);
    TEST_EQUAL(mset.size(), 2);

    return true;
}

DEFINE_TESTCASE(dualprefixwildcard1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Query q(Xapian::Query::OP_SYNONYM,
		    Xapian::Query(Xapian::Query::OP_WILDCARD, "fo"),
		    Xapian::Query(Xapian::Query::OP_WILDCARD, "Sfo"));
    tout << q.get_description() << endl;
    Xapian::Enquire enq(db);
    enq.set_query(q);
    TEST_EQUAL(enq.get_mset(0, 5).size(), 2);
    return true;
}

/// Test special case wildcards.
DEFINE_TESTCASE(specialwildcard1, !backend) {
    const Xapian::Query::op o = Xapian::Query::OP_WILDCARD;
    const auto f = Xapian::Query::WILDCARD_PATTERN_GLOB;

    // Empty wildcard -> MatchNothing.
    TEST_EQUAL(Xapian::Query(o, "", 0, f).get_description(), "Query()");

    // "*", "?*", etc -> MatchAll.
#define QUERY_ALLDOCS "Query(<alldocuments>)"
    TEST_EQUAL(Xapian::Query(o, "*", 0, f).get_description(), QUERY_ALLDOCS);
    TEST_EQUAL(Xapian::Query(o, "**", 0, f).get_description(), QUERY_ALLDOCS);
    TEST_EQUAL(Xapian::Query(o, "?*", 0, f).get_description(), QUERY_ALLDOCS);
    TEST_EQUAL(Xapian::Query(o, "*?", 0, f).get_description(), QUERY_ALLDOCS);
    TEST_EQUAL(Xapian::Query(o, "*?*", 0, f).get_description(), QUERY_ALLDOCS);

    return true;
}

/// Test `?` extended wildcard.
DEFINE_TESTCASE(singlecharwildcard1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    {
	Xapian::Document doc;
	doc.add_term("test");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("t\xc3\xaast");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("t\xe1\x80\x80st");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("t\xf3\x80\x80\x80st");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("toast");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("t*t");
	db.add_document(doc);
    }
    db.commit();

    Xapian::Enquire enq(db);
    enq.set_weighting_scheme(Xapian::BoolWeight());

    const Xapian::Query::op o = Xapian::Query::OP_WILDCARD;
    const auto f = Xapian::Query::WILDCARD_PATTERN_SINGLE;

    {
	// Check that `?` matches one Unicode character.
	enq.set_query(Xapian::Query(o, "t?st", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 1, 2, 3, 4);
    }

    {
	// Check that `??` doesn't match a single two-byte UTF-8 character.
	enq.set_query(Xapian::Query(o, "t??st", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 5);
    }

    {
	// Check that `*` is handled as a literal character not a wildcard.
	enq.set_query(Xapian::Query(o, "t*t", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 6);
    }

    return true;
}

/// Test `*` extended wildcard.
DEFINE_TESTCASE(multicharwildcard1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    {
	Xapian::Document doc;
	doc.add_term("ananas");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("annas");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("bananas");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("banannas");
	db.add_document(doc);
    }
    {
	Xapian::Document doc;
	doc.add_term("b?nanas");
	db.add_document(doc);
    }
    db.commit();

    Xapian::Enquire enq(db);
    enq.set_weighting_scheme(Xapian::BoolWeight());

    const Xapian::Query::op o = Xapian::Query::OP_WILDCARD;
    const auto f = Xapian::Query::WILDCARD_PATTERN_MULTI;

    {
	// Check `*` can handle partial matches before and after.
	enq.set_query(Xapian::Query(o, "b*anas", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 3, 5);
    }

    {
	// Check leading `*` works.
	enq.set_query(Xapian::Query(o, "*anas", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 1, 3, 5);
    }

    {
	// Check more than one `*` works.
	enq.set_query(Xapian::Query(o, "*ann*", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 2, 4);
    }

    {
	// Check that `?` is handled as a literal character not a wildcard.
	enq.set_query(Xapian::Query(o, "b?n*", 0, f));
	Xapian::MSet mset = enq.get_mset(0, 100);
	mset_expect_order(mset, 5);
    }

    return true;
}

struct editdist_testcase {
    const char* target;
    unsigned edit_distance;
    Xapian::termcount max_expansion;
    char max_type;
    const char* terms[4];
};

#define EDITDIST_EXCEPTION { 0, 0, 0, "" }
static const
editdist_testcase editdist1_testcases[] = {
    // Tries to expand to 9 terms.
    { "muse",	2, 8, 'E', EDITDIST_EXCEPTION },
    { "museum",	3, 3, 'E', { "mset", "must", "use", 0 } },
    { "thou",	0, 9, 'E', { 0, 0, 0, 0 } },
    { "though",	0, 9, 'E', { "though", 0, 0, 0 } },
    { "museum",	3, 1, 'F', { "mset", 0, 0, 0 } },
    { "museum",	3, 1, 'M', { "use", 0, 0, 0 } },
};

DEFINE_TESTCASE(editdist1, backend) {
    // FIXME: The counting of terms the subquery expands to is per subdatabase,
    // so it may expand to more terms than the limit if some aren't in all
    // subdatabases.  Also WILDCARD_LIMIT_MOST_FREQUENT uses the frequency from
    // the subdatabase, and so may select different terms in each subdatabase.
    SKIP_TEST_FOR_BACKEND("multi");
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    const Xapian::Query::op o = Xapian::Query::OP_EDIT_DISTANCE;

    for (auto&& test : editdist1_testcases) {
	tout << test.target << endl;
	auto tend = test.terms + 4;
	while (tend > test.terms && tend[-1] == NULL) --tend;
	bool expect_exception = (tend - test.terms == 4 && tend[-1][0] == '\0');
	Xapian::Query q;
	int max_type;
	switch (test.max_type) {
	    case 'E':
		max_type = Xapian::Query::WILDCARD_LIMIT_ERROR;
		break;
	    case 'F':
		max_type = Xapian::Query::WILDCARD_LIMIT_FIRST;
		break;
	    case 'M':
		max_type = Xapian::Query::WILDCARD_LIMIT_MOST_FREQUENT;
		break;
	    default:
		return false;
	}
	q = Xapian::Query(o, test.target, test.max_expansion, max_type,
			  q.OP_SYNONYM, test.edit_distance);
	enq.set_query(q);
	tout << q.get_description() << endl;
	try {
	    Xapian::MSet mset = enq.get_mset(0, 10);
	    TEST(!expect_exception);
	    q = Xapian::Query(q.OP_SYNONYM, test.terms, tend);
	    enq.set_query(q);
	    Xapian::MSet mset2 = enq.get_mset(0, 10);
	    TEST_EQUAL(mset.size(), mset2.size());
	    TEST(mset_range_is_same(mset, 0, mset2, 0, mset.size()));
	} catch (const Xapian::WildcardError&) {
	    TEST(expect_exception);
	}
    }

    return true;
}

DEFINE_TESTCASE(dualprefixeditdist1, generated) {
    Xapian::Database db = get_database("dualprefixeditdist1",
				       [](Xapian::WritableDatabase& wdb,
					  const string&)
				       {
					   Xapian::Document doc;
					   doc.add_term("opossum");
					   doc.add_term("possum");
					   wdb.add_document(doc);
					   doc.clear_terms();
					   doc.add_term("Spossums");
					   wdb.add_document(doc);
				       });

    auto OP_EDIT_DISTANCE = Xapian::Query::OP_EDIT_DISTANCE;
    auto OP_SYNONYM = Xapian::Query::OP_SYNONYM;
    Xapian::Query q0(OP_EDIT_DISTANCE, "possum");
    Xapian::Query q1(OP_EDIT_DISTANCE, "Spossum", 0, 0, OP_SYNONYM, 2, 1);
    Xapian::Query q(OP_SYNONYM, q0, q1);
    tout << q.get_description() << endl;
    Xapian::Enquire enq(db);
    enq.set_query(q0);
    Xapian::MSet mset = enq.get_mset(0, 5);
    TEST_EQUAL(mset.size(), 1);
    TEST_EQUAL(*mset[0], 1);
    enq.set_query(q1);
    mset = enq.get_mset(0, 5);
    TEST_EQUAL(mset.size(), 1);
    TEST_EQUAL(*mset[0], 2);
    enq.set_query(q);
    mset = enq.get_mset(0, 5);
    TEST_EQUAL(mset.size(), 2);

    return true;
}

struct positional_testcase {
    int window;
    const char * terms[4];
    Xapian::docid result;
};

static const
positional_testcase loosephrase1_testcases[] = {
    { 5, { "expect", "to", "mset", 0 }, 0 },
    { 5, { "word", "well", "the", 0 }, 2 },
    { 5, { "if", "word", "doesnt", 0 }, 0 },
    { 5, { "at", "line", "three", 0 }, 0 },
    { 5, { "paragraph", "other", "the", 0 }, 0 },
    { 5, { "other", "the", "with", 0 }, 0 }
};

/// Regression test for bug fixed in 1.3.3 and 1.2.21.
DEFINE_TESTCASE(loosephrase1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);

    for (auto&& test : loosephrase1_testcases) {
	auto tend = test.terms + 4;
	while (tend[-1] == NULL) --tend;
	auto OP_PHRASE = Xapian::Query::OP_PHRASE;
	Xapian::Query q(OP_PHRASE, test.terms, tend, test.window);
	enq.set_query(q);
	Xapian::MSet mset = enq.get_mset(0, 10);
	if (test.result == 0) {
	    TEST(mset.empty());
	} else {
	    TEST_EQUAL(mset.size(), 1);
	    TEST_EQUAL(*mset[0], test.result);
	}
    }

    return true;
}

static const
positional_testcase loosenear1_testcases[] = {
    { 4, { "test", "the", "with", 0 }, 1 },
    { 4, { "expect", "word", "the", 0 }, 2 },
    { 4, { "line", "be", "blank", 0 }, 1 },
    { 2, { "banana", "banana", 0, 0 }, 0 },
    { 3, { "banana", "banana", 0, 0 }, 0 },
    { 2, { "word", "word", 0, 0 }, 2 },
    { 4, { "work", "meant", "work", 0 }, 0 },
    { 4, { "this", "one", "yet", "one" }, 0 }
};

/// Regression tests for bugs fixed in 1.3.3 and 1.2.21.
DEFINE_TESTCASE(loosenear1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);

    for (auto&& test : loosenear1_testcases) {
	auto tend = test.terms + 4;
	while (tend[-1] == NULL) --tend;
	Xapian::Query q(Xapian::Query::OP_NEAR, test.terms, tend, test.window);
	enq.set_query(q);
	Xapian::MSet mset = enq.get_mset(0, 10);
	if (test.result == 0) {
	    TEST(mset.empty());
	} else {
	    TEST_EQUAL(mset.size(), 1);
	    TEST_EQUAL(*mset[0], test.result);
	}
    }

    return true;
}

/// Regression test for bug fixed in 1.3.6 - the first case segfaulted in 1.3.x.
DEFINE_TESTCASE(complexphrase1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_PHRASE,
	    Xapian::Query("a") | Xapian::Query("b"),
	    Xapian::Query("i"));
    enq.set_query(query);
    TEST(enq.get_mset(0, 10).empty());
    Xapian::Query query2(Xapian::Query::OP_PHRASE,
	    Xapian::Query("a") | Xapian::Query("b"),
	    Xapian::Query("c"));
    enq.set_query(query2);
    TEST(enq.get_mset(0, 10).empty());
    return true;
}

/// Regression test for bug fixed in 1.3.6 - the first case segfaulted in 1.3.x.
DEFINE_TESTCASE(complexnear1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_NEAR,
	    Xapian::Query("a") | Xapian::Query("b"),
	    Xapian::Query("i"));
    enq.set_query(query);
    TEST(enq.get_mset(0, 10).empty());
    Xapian::Query query2(Xapian::Query::OP_NEAR,
	    Xapian::Query("a") | Xapian::Query("b"),
	    Xapian::Query("c"));
    enq.set_query(query2);
    TEST(enq.get_mset(0, 10).empty());
    return true;
}

/// Check subqueries of MatchAll, MatchNothing and PostingSource are supported.
DEFINE_TESTCASE(complexphrase2, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::ValueWeightPostingSource ps(0);
    Xapian::Query subqs[3] = {
	Xapian::Query(Xapian::Query::OP_PHRASE,
	    Xapian::Query("a"),
	    Xapian::Query(&ps)),
	Xapian::Query(Xapian::Query::OP_PHRASE,
	    Xapian::Query("and"),
	    Xapian::Query::MatchAll),
	Xapian::Query(Xapian::Query::OP_PHRASE,
	    Xapian::Query("at"),
	    Xapian::Query::MatchNothing)
    };
    Xapian::Query query(Xapian::Query::OP_OR, subqs, subqs + 3);
    enq.set_query(query);
    (void)enq.get_mset(0, 10);
    return true;
}

/// Check subqueries of MatchAll, MatchNothing and PostingSource are supported.
DEFINE_TESTCASE(complexnear2, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::ValueWeightPostingSource ps(0);
    Xapian::Query subqs[3] = {
	Xapian::Query(Xapian::Query::OP_NEAR,
	    Xapian::Query("a"),
	    Xapian::Query(&ps)),
	Xapian::Query(Xapian::Query::OP_NEAR,
	    Xapian::Query("and"),
	    Xapian::Query::MatchAll),
	Xapian::Query(Xapian::Query::OP_NEAR,
	    Xapian::Query("at"),
	    Xapian::Query::MatchNothing)
    };
    Xapian::Query query(Xapian::Query::OP_OR, subqs, subqs + 3);
    enq.set_query(query);
    (void)enq.get_mset(0, 10);
    return true;
}

/// A zero estimated number of matches broke the code to round the estimate.
DEFINE_TESTCASE(zeroestimate1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Query phrase(Xapian::Query::OP_PHRASE,
			 Xapian::Query("absolute"),
			 Xapian::Query("rubbish"));
    enquire.set_query(phrase &~ Xapian::Query("queri"));
    Xapian::MSet mset = enquire.get_mset(0, 0);
    TEST_EQUAL(mset.get_matches_estimated(), 0);
    return true;
}

/// Feature test for OR under OP_PHRASE support added in 1.4.3.
DEFINE_TESTCASE(complexphrase3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_PHRASE,
	    Xapian::Query("is") | Xapian::Query("as") | Xapian::Query("be"),
	    Xapian::Query("a"));
    enq.set_query(query);
    mset_expect_order(enq.get_mset(0, 10), 1);
    Xapian::Query query2(Xapian::Query::OP_PHRASE,
	    Xapian::Query("a"),
	    Xapian::Query("is") | Xapian::Query("as") | Xapian::Query("be"));
    enq.set_query(query2);
    mset_expect_order(enq.get_mset(0, 10));
    Xapian::Query query3(Xapian::Query::OP_PHRASE,
	    Xapian::Query("one") | Xapian::Query("with"),
	    Xapian::Query("the") | Xapian::Query("of") | Xapian::Query("line"));
    enq.set_query(query3);
    mset_expect_order(enq.get_mset(0, 10), 1, 4, 5);
    Xapian::Query query4(Xapian::Query::OP_PHRASE,
	    Xapian::Query("the") | Xapian::Query("of") | Xapian::Query("line"),
	    Xapian::Query("one") | Xapian::Query("with"));
    enq.set_query(query4);
    mset_expect_order(enq.get_mset(0, 10));
    return true;
}

/// Feature test for OR under OP_NEAR support added in 1.4.3.
DEFINE_TESTCASE(complexnear3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::Query query(Xapian::Query::OP_NEAR,
	    Xapian::Query("is") | Xapian::Query("as") | Xapian::Query("be"),
	    Xapian::Query("a"));
    enq.set_query(query);
    mset_expect_order(enq.get_mset(0, 10), 1);
    Xapian::Query query2(Xapian::Query::OP_NEAR,
	    Xapian::Query("a"),
	    Xapian::Query("is") | Xapian::Query("as") | Xapian::Query("be"));
    enq.set_query(query2);
    mset_expect_order(enq.get_mset(0, 10), 1);
    Xapian::Query query3(Xapian::Query::OP_NEAR,
	    Xapian::Query("one") | Xapian::Query("with"),
	    Xapian::Query("the") | Xapian::Query("of") | Xapian::Query("line"));
    enq.set_query(query3);
    mset_expect_order(enq.get_mset(0, 10), 1, 4, 5);
    Xapian::Query query4(Xapian::Query::OP_NEAR,
	    Xapian::Query("the") | Xapian::Query("of") | Xapian::Query("line"),
	    Xapian::Query("one") | Xapian::Query("with"));
    enq.set_query(query4);
    mset_expect_order(enq.get_mset(0, 10), 1, 4, 5);
    return true;
}

static void
gen_subdbwithoutpos1_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_term("this");
    doc.add_term("paragraph");
    doc.add_term("wibble", 5);
    db.add_document(doc);
}

DEFINE_TESTCASE(subdbwithoutpos1, generated) {
    Xapian::Database db(get_database("apitest_simpledata"));

    Xapian::Query q(Xapian::Query::OP_PHRASE,
		    Xapian::Query("this"),
		    Xapian::Query("paragraph"));

    Xapian::Enquire enq1(db);
    enq1.set_query(q);
    Xapian::MSet mset1 = enq1.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 3);

    Xapian::Database db2 =
	get_database("subdbwithoutpos1", gen_subdbwithoutpos1_db);

    // If a database has no positional info, OP_PHRASE -> OP_AND.
    Xapian::Enquire enq2(db2);
    enq2.set_query(q);
    Xapian::MSet mset2 = enq2.get_mset(0, 10);
    TEST_EQUAL(mset2.size(), 1);

    // If one sub-database in a combined database has no positional info but
    // other sub-databases do, then we shouldn't convert OP_PHRASE to OP_AND
    // (but prior to 1.4.3 we did).
    db.add_database(db2);
    Xapian::Enquire enq3(db);
    enq3.set_query(q);
    Xapian::MSet mset3 = enq3.get_mset(0, 10);
    TEST_EQUAL(mset3.size(), 3);
    // Regression test for bug introduced in 1.4.3 which led to a division by
    // zero and then (at least on Linux) we got 1% here.
    TEST_EQUAL(mset3[0].get_percent(), 100);

    // Regression test for https://trac.xapian.org/ticket/752
    enq3.set_query((Xapian::Query("this") & q) | Xapian::Query("wibble"));
    mset3 = enq3.get_mset(0, 10);
    TEST_EQUAL(mset3.size(), 4);

    return true;
}

// Regression test for bug fixed in 1.4.4 and 1.2.25.
DEFINE_TESTCASE(notandor1, backend) {
    Xapian::Database db(get_database("etext"));
    using Xapian::Query;
    Query q = Query("the") &~ (Query("friedrich") &
			       (Query("day") | Query("night")));
    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset = enq.get_mset(0, 10, db.get_doccount());
    TEST_EQUAL(mset.get_matches_estimated(), 344);

    return true;
}

// Regression test for bug fixed in git master before 1.5.0.
DEFINE_TESTCASE(boolorbug1, backend) {
    Xapian::Database db(get_database("etext"));
    using Xapian::Query;
    Query q = Query("the") &~ Query(Query::OP_WILDCARD, "pru");
    Xapian::Enquire enq(db);
    enq.set_query(q);

    Xapian::MSet mset = enq.get_mset(0, 10, db.get_doccount());
    // Due to a bug in BoolOrPostList this returned 330 results.
    TEST_EQUAL(mset.get_matches_estimated(), 331);

    return true;
}
