/** @file api_none.cc
 * @brief tests which don't need a backend
 */
/* Copyright (C) 2009 Richard Boulton
 * Copyright (C) 2009,2010,2011,2013,2014,2015,2016,2017 Olly Betts
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

#include "api_none.h"

#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "apitest.h"
#include "str.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;

// Check the version functions give consistent results.
DEFINE_TESTCASE(version1, !backend) {
    string version = str(Xapian::major_version());
    version += '.';
    version += str(Xapian::minor_version());
    version += '.';
    version += str(Xapian::revision());
    TEST_EQUAL(Xapian::version_string(), version);
    return true;
}

// Regression test: various methods on Database() used to segfault or cause
// division by 0.  Fixed in 1.1.4 and 1.0.18.  Ticket#415.
DEFINE_TESTCASE(nosubdatabases1, !backend) {
    Xapian::Database db;
    TEST(db.get_metadata("foo").empty());
    TEST_EQUAL(db.metadata_keys_begin(), db.metadata_keys_end());
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.termlist_begin(1));
    TEST_EQUAL(db.allterms_begin(), db.allterms_end());
    TEST_EQUAL(db.allterms_begin("foo"), db.allterms_end("foo"));
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.positionlist_begin(1, "foo"));
    TEST_EQUAL(db.get_lastdocid(), 0);
    TEST_EQUAL(db.valuestream_begin(7), db.valuestream_end(7));
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.get_unique_terms(1));
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.get_document(1));
    return true;
}

/// Feature test for Document::add_boolean_term(), new in 1.0.18/1.1.4.
DEFINE_TESTCASE(document1, !backend) {
    Xapian::Document doc;
    doc.add_boolean_term("Hxapian.org");
    TEST_EQUAL(doc.termlist_count(), 1);
    Xapian::TermIterator t = doc.termlist_begin();
    TEST(t != doc.termlist_end());
    TEST_EQUAL(*t, "Hxapian.org");
    TEST_EQUAL(t.get_wdf(), 0);
    TEST(++t == doc.termlist_end());
    doc.remove_term("Hxapian.org");
    TEST_EQUAL(doc.termlist_count(), 0);
    TEST(doc.termlist_begin() == doc.termlist_end());
    return true;
}

/// Regression test - the docid wasn't initialised prior to 1.0.22/1.2.4.
DEFINE_TESTCASE(document2, !backend) {
    Xapian::Document doc;
    // The return value is uninitialised, so running under valgrind this
    // will fail reliably prior to the fix.
    TEST_EQUAL(doc.get_docid(), 0);
    return true;
}

/// Feature tests for Document::clear_terms().
DEFINE_TESTCASE(documentclearterms1, !backend) {
    {
	Xapian::Document doc;
	doc.add_boolean_term("Hlocalhost");
	doc.add_term("hello");
	doc.add_term("there", 2);
	doc.add_posting("positional", 1);
	doc.add_posting("information", 2, 3);
	TEST_EQUAL(doc.termlist_count(), 5);
	TEST(doc.termlist_begin() != doc.termlist_end());
	doc.clear_terms();
	TEST_EQUAL(doc.termlist_count(), 0);
	TEST(doc.termlist_begin() == doc.termlist_end());
	// Test clear_terms() when there are no terms.
	doc.clear_terms();
	TEST_EQUAL(doc.termlist_count(), 0);
	TEST(doc.termlist_begin() == doc.termlist_end());
    }

    {
	// Test clear_terms() when there have never been any terms.
	Xapian::Document doc;
	doc.clear_terms();
	TEST_EQUAL(doc.termlist_count(), 0);
	TEST(doc.termlist_begin() == doc.termlist_end());
    }

    return true;
}

/// Feature tests for Document::clear_values().
DEFINE_TESTCASE(documentclearvalues1, !backend) {
    {
	Xapian::Document doc;
	doc.add_value(37, "hello");
	doc.add_value(42, "world");
	TEST_EQUAL(doc.values_count(), 2);
	TEST(doc.values_begin() != doc.values_end());
	doc.clear_values();
	TEST_EQUAL(doc.values_count(), 0);
	TEST(doc.values_begin() == doc.values_end());
	// Test clear_values() when there are no values.
	doc.clear_values();
	TEST_EQUAL(doc.values_count(), 0);
	TEST(doc.values_begin() == doc.values_end());
    }

    {
	// Test clear_values() when there have never been any values.
	Xapian::Document doc;
	doc.clear_values();
	TEST_EQUAL(doc.values_count(), 0);
	TEST(doc.termlist_begin() == doc.termlist_end());
    }

    return true;
}

/// Feature tests for errors for empty terms.
DEFINE_TESTCASE(documentemptyterm1, !backend) {
    Xapian::Document doc;
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.add_boolean_term(string()));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.add_term(string()));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.add_posting(string(), 1));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.add_posting(string(), 2, 3));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.remove_term(string()));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.remove_posting(string(), 1));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
	    doc.remove_posting(string(), 2, 3));
    return true;
}

DEFINE_TESTCASE(emptyquery4, !backend) {
    // Test we get an empty query from applying any of the following ops to
    // an empty list of subqueries.
    Xapian::Query q;
    TEST(Xapian::Query(q.OP_AND, &q, &q).empty());
    TEST(Xapian::Query(q.OP_OR, &q, &q).empty());
    TEST(Xapian::Query(q.OP_AND_NOT, &q, &q).empty());
    TEST(Xapian::Query(q.OP_XOR, &q, &q).empty());
    TEST(Xapian::Query(q.OP_AND_MAYBE, &q, &q).empty());
    TEST(Xapian::Query(q.OP_FILTER, &q, &q).empty());
    TEST(Xapian::Query(q.OP_NEAR, &q, &q).empty());
    TEST(Xapian::Query(q.OP_PHRASE, &q, &q).empty());
    TEST(Xapian::Query(q.OP_ELITE_SET, &q, &q).empty());
    TEST(Xapian::Query(q.OP_SYNONYM, &q, &q).empty());
    TEST(Xapian::Query(q.OP_MAX, &q, &q).empty());
    return true;
}

DEFINE_TESTCASE(singlesubquery1, !backend) {
    // Test that we get just the subquery if we apply any of the following
    // ops to just that subquery.
#define singlesubquery1_(OP) \
    TEST_STRINGS_EQUAL(Xapian::Query(q->OP, q, q + 1).get_description(),\
	"Query(test)")
    Xapian::Query q[1] = { Xapian::Query("test") };
    singlesubquery1_(OP_AND);
    singlesubquery1_(OP_OR);
    singlesubquery1_(OP_AND_NOT);
    singlesubquery1_(OP_XOR);
    singlesubquery1_(OP_AND_MAYBE);
    singlesubquery1_(OP_FILTER);
    singlesubquery1_(OP_NEAR);
    singlesubquery1_(OP_PHRASE);
    singlesubquery1_(OP_ELITE_SET);
    singlesubquery1_(OP_SYNONYM);
    singlesubquery1_(OP_MAX);
    return true;
}

DEFINE_TESTCASE(singlesubquery2, !backend) {
    // Like the previous test, but using MatchNothing as the subquery.
#define singlesubquery2_(OP) \
    TEST_STRINGS_EQUAL(Xapian::Query(q->OP, q, q + 1).get_description(),\
	"Query()")
    Xapian::Query q[1] = { Xapian::Query::MatchNothing };
    singlesubquery2_(OP_AND);
    singlesubquery2_(OP_OR);
    singlesubquery2_(OP_AND_NOT);
    singlesubquery2_(OP_XOR);
    singlesubquery2_(OP_AND_MAYBE);
    singlesubquery2_(OP_FILTER);
    singlesubquery2_(OP_NEAR);
    singlesubquery2_(OP_PHRASE);
    singlesubquery2_(OP_ELITE_SET);
    singlesubquery2_(OP_SYNONYM);
    singlesubquery2_(OP_MAX);
    return true;
}

DEFINE_TESTCASE(singlesubquery3, !backend) {
    // Like the previous test, but using MatchAll as the subquery.
#define singlesubquery3_(OP) \
    TEST_STRINGS_EQUAL(Xapian::Query(q->OP, q, q + 1).get_description(),\
	"Query(<alldocuments>)")
    Xapian::Query q[1] = { Xapian::Query::MatchAll };
    singlesubquery3_(OP_AND);
    singlesubquery3_(OP_OR);
    singlesubquery3_(OP_AND_NOT);
    singlesubquery3_(OP_XOR);
    singlesubquery3_(OP_AND_MAYBE);
    singlesubquery3_(OP_FILTER);
    // OP_NEAR and OP_PHRASE over MatchAll doesn't really make sense.
    singlesubquery3_(OP_ELITE_SET);
    singlesubquery3_(OP_SYNONYM);
    singlesubquery3_(OP_MAX);
    return true;
}

/// Check we no longer combine wqf for same term at the same position.
DEFINE_TESTCASE(combinewqfnomore1, !backend) {
    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("beer", 1, 1),
		    Xapian::Query("beer", 1, 1));
    // Prior to 1.3.0, we would have given beer@2, but we decided that wasn't
    // really useful or helpful.
    TEST_EQUAL(q.get_description(), "Query((beer@1 OR beer@1))");
    return true;
}

class DestroyedFlag {
    bool & destroyed;

  public:
    DestroyedFlag(bool & destroyed_) : destroyed(destroyed_) {
	destroyed = false;
    }

    ~DestroyedFlag() {
	destroyed = true;
    }
};

class TestRangeProcessor : public Xapian::RangeProcessor {
    DestroyedFlag destroyed;

  public:
    TestRangeProcessor(bool & destroyed_)
	: Xapian::RangeProcessor(0), destroyed(destroyed_) { }

    Xapian::Query operator()(const std::string&, const std::string&)
    {
	return Xapian::Query::MatchAll;
    }
};

/// Check reference counting of user-subclassable classes.
DEFINE_TESTCASE(subclassablerefcount1, !backend) {
    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::RangeProcessor * rp = new TestRangeProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_rangeprocessor(rp->release());
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::RangeProcessor * rp = new TestRangeProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_rangeprocessor(rp->release());
	rp->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a RangeProcessor with automatic
    // storage works OK.
    {
	TestRangeProcessor rp_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::QueryParser qp1;
	    {
		Xapian::QueryParser qp2;
		Xapian::RangeProcessor * rp;
		rp = new TestRangeProcessor(gone);
		TEST(!gone);
		qp1.add_rangeprocessor(rp->release());
		TEST(!gone);
		qp2.add_rangeprocessor(rp);
		TEST(!gone);
		qp2.add_rangeprocessor(&rp_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    // Regression test for initial implementation, where ~opt_intrusive_ptr()
    // checked the reference of the object, which may have already been deleted
    // if it wasn't been reference counted.
    {
	Xapian::QueryParser qp;
	{
	    Xapian::RangeProcessor * rp = new TestRangeProcessor(gone);
	    TEST(!gone);
	    qp.add_rangeprocessor(rp);
	    delete rp;
	    TEST(gone);
	}
	// At the end of this block, qp is destroyed, but mustn't dereference
	// the pointer it has to rp.  If it does, that should get caught
	// when tests are run under valgrind.
    }

    return true;
}

class TestFieldProcessor : public Xapian::FieldProcessor {
    DestroyedFlag destroyed;

  public:
    TestFieldProcessor(bool & destroyed_) : destroyed(destroyed_) { }

    Xapian::Query operator()(const string &str) {
	return Xapian::Query(str);
    }
};

/// Check reference counting of user-subclassable classes.
DEFINE_TESTCASE(subclassablerefcount2, !backend) {
    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::FieldProcessor * proc = new TestFieldProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_prefix("foo", proc->release());
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::FieldProcessor * proc = new TestFieldProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_prefix("foo", proc->release());
	proc->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a FieldProcessor with automatic
    // storage works OK.
    {
	TestFieldProcessor proc_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::QueryParser qp1;
	    {
		Xapian::QueryParser qp2;
		Xapian::FieldProcessor * proc;
		proc = new TestFieldProcessor(gone);
		TEST(!gone);
		qp1.add_prefix("foo", proc->release());
		TEST(!gone);
		qp2.add_prefix("foo", proc);
		TEST(!gone);
		qp2.add_prefix("bar", &proc_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    return true;
}

class TestMatchSpy : public Xapian::MatchSpy {
    DestroyedFlag destroyed;

  public:
    TestMatchSpy(bool & destroyed_) : destroyed(destroyed_) { }

    void operator()(const Xapian::Document &, double) { }
};

/// Check reference counting of MatchSpy.
DEFINE_TESTCASE(subclassablerefcount3, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::MatchSpy * spy = new TestMatchSpy(gone);
	TEST(!gone);
	Xapian::Enquire enquire(db);
	enquire.add_matchspy(spy->release());
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::MatchSpy * spy = new TestMatchSpy(gone);
	TEST(!gone);
	Xapian::Enquire enquire(db);
	enquire.add_matchspy(spy->release());
	spy->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a MatchSpy with automatic
    // storage works OK.
    {
	TestMatchSpy spy_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::Enquire enq1(db);
	    {
		Xapian::Enquire enq2(db);
		Xapian::MatchSpy * spy;
		spy = new TestMatchSpy(gone);
		TEST(!gone);
		enq1.add_matchspy(spy->release());
		TEST(!gone);
		enq2.add_matchspy(spy);
		TEST(!gone);
		enq2.add_matchspy(&spy_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    return true;
}

class TestStopper : public Xapian::Stopper {
    DestroyedFlag destroyed;

  public:
    TestStopper(bool & destroyed_) : destroyed(destroyed_) { }

    bool operator()(const std::string&) const { return true; }
};

/// Check reference counting of Stopper with QueryParser.
DEFINE_TESTCASE(subclassablerefcount4, !backend) {
    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.set_stopper(stopper->release());
	TEST(!gone);
    }
    TEST(gone);

    // Test that setting a new stopper causes the previous one to be released.
    {
	bool gone0;
	Xapian::Stopper * stopper0 = new TestStopper(gone0);
	TEST(!gone0);
	Xapian::QueryParser qp;
	qp.set_stopper(stopper0->release());
	TEST(!gone0);

	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	qp.set_stopper(stopper->release());
	TEST(gone0);
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.set_stopper(stopper->release());
	stopper->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a Stopper with automatic
    // storage works OK.
    {
	TestStopper stopper_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::QueryParser qp1;
	    {
		Xapian::QueryParser qp2;
		Xapian::Stopper * stopper;
		stopper = new TestStopper(gone);
		TEST(!gone);
		qp1.set_stopper(stopper->release());
		TEST(!gone);
		qp2.set_stopper(stopper);
		TEST(!gone);
		qp2.set_stopper(&stopper_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    return true;
}

/// Check reference counting of Stopper with TermGenerator.
DEFINE_TESTCASE(subclassablerefcount5, !backend) {
    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	Xapian::TermGenerator indexer;
	indexer.set_stopper(stopper->release());
	TEST(!gone);
    }
    TEST(gone);

    // Test that setting a new stopper causes the previous one to be released.
    {
	bool gone0;
	Xapian::Stopper * stopper0 = new TestStopper(gone0);
	TEST(!gone0);
	Xapian::TermGenerator indexer;
	indexer.set_stopper(stopper0->release());
	TEST(!gone0);

	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	indexer.set_stopper(stopper->release());
	TEST(gone0);
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::Stopper * stopper = new TestStopper(gone);
	TEST(!gone);
	Xapian::TermGenerator indexer;
	indexer.set_stopper(stopper->release());
	stopper->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a Stopper with automatic
    // storage works OK.
    {
	TestStopper stopper_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::TermGenerator indexer1;
	    {
		Xapian::TermGenerator indexer2;
		Xapian::Stopper * stopper;
		stopper = new TestStopper(gone);
		TEST(!gone);
		indexer1.set_stopper(stopper->release());
		TEST(!gone);
		indexer2.set_stopper(stopper);
		TEST(!gone);
		indexer2.set_stopper(&stopper_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    return true;
}

class TestKeyMaker : public Xapian::KeyMaker {
    DestroyedFlag destroyed;

  public:
    TestKeyMaker(bool & destroyed_) : destroyed(destroyed_) { }

    string operator()(const Xapian::Document&) const { return string(); }
};

/// Check reference counting of KeyMaker.
DEFINE_TESTCASE(subclassablerefcount6, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::KeyMaker * keymaker = new TestKeyMaker(gone);
	TEST(!gone);
	Xapian::Enquire enq(db);
	enq.set_sort_by_key(keymaker->release(), false);
	TEST(!gone);
    }
    TEST(gone);

    // Test that setting a new keymaker causes the previous one to be released.
    {
	bool gone0;
	Xapian::KeyMaker * keymaker0 = new TestKeyMaker(gone0);
	TEST(!gone0);
	Xapian::Enquire enq(db);
	enq.set_sort_by_key(keymaker0->release(), false);
	TEST(!gone0);

	Xapian::KeyMaker * keymaker = new TestKeyMaker(gone);
	TEST(!gone);
	enq.set_sort_by_key_then_relevance(keymaker->release(), false);
	TEST(gone0);
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::KeyMaker * keymaker = new TestKeyMaker(gone);
	TEST(!gone);
	Xapian::Enquire enq(db);
	enq.set_sort_by_key(keymaker->release(), false);
	keymaker->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a KeyMaker with automatic
    // storage works OK.
    {
	TestKeyMaker keymaker_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::Enquire enq1(db);
	    {
		Xapian::Enquire enq2(db);
		Xapian::KeyMaker * keymaker;
		keymaker = new TestKeyMaker(gone);
		TEST(!gone);
		enq1.set_sort_by_key(keymaker->release(), false);
		TEST(!gone);
		enq2.set_sort_by_relevance_then_key(keymaker, false);
		TEST(!gone);
		enq2.set_sort_by_key_then_relevance(&keymaker_auto, false);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    return true;
}

class TestExpandDecider : public Xapian::ExpandDecider {
    DestroyedFlag destroyed;

  public:
    TestExpandDecider(bool & destroyed_) : destroyed(destroyed_) { }

    bool operator()(const string&) const { return true; }
};

/// Check reference counting of ExpandDecider.
DEFINE_TESTCASE(subclassablerefcount7, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::RSet rset;
    rset.add_document(1);

    bool gone_auto, gone;

    for (int flags = 0;
	 flags <= Xapian::Enquire::INCLUDE_QUERY_TERMS;
	 flags += Xapian::Enquire::INCLUDE_QUERY_TERMS) {
	// Test of auto lifetime ExpandDecider.
	{
	    TestExpandDecider edecider_auto(gone_auto);
	    TEST(!gone_auto);
	    (void)enq.get_eset(5, rset, 0, &edecider_auto);
	    TEST(!gone_auto);
	}
	TEST(gone_auto);

	// Simple test of release().
	{
	    Xapian::ExpandDecider * edecider = new TestExpandDecider(gone);
	    TEST(!gone);
	    (void)enq.get_eset(5, rset, 0, edecider);
	    TEST(!gone);
	    delete edecider;
	    TEST(gone);
	}

	// Test that a released ExpandDecider gets cleaned up by get_eset().
	{
	    Xapian::ExpandDecider * edecider = new TestExpandDecider(gone);
	    TEST(!gone);
	    (void)enq.get_eset(5, rset, 0, edecider->release());
	    TEST(gone);
	}

	// Check a second call to release() has no effect.
	{
	    Xapian::ExpandDecider * edecider = new TestExpandDecider(gone);
	    TEST(!gone);
	    edecider->release();
	    TEST(!gone);
	    (void)enq.get_eset(5, rset, 0, edecider->release());
	    TEST(gone);
	}
    }

    // Test combinations of released/non-released with ExpandDeciderAnd.
    {
	TestExpandDecider edecider_auto(gone_auto);
	TEST(!gone_auto);
	Xapian::ExpandDecider * edecider = new TestExpandDecider(gone);
	TEST(!gone);
	(void)enq.get_eset(5, rset, 0,
		(new Xapian::ExpandDeciderAnd(
		    &edecider_auto,
		    edecider->release()))->release());
	TEST(!gone_auto);
	TEST(gone);
    }
    TEST(gone_auto);
    {
	TestExpandDecider edecider_auto(gone_auto);
	TEST(!gone_auto);
	Xapian::ExpandDecider * edecider = new TestExpandDecider(gone);
	TEST(!gone);
	(void)enq.get_eset(5, rset, 0,
		(new Xapian::ExpandDeciderAnd(
		    edecider->release(),
		    &edecider_auto))->release());
	TEST(!gone_auto);
	TEST(gone);
    }
    TEST(gone_auto);

    return true;
}

class TestValueRangeProcessor : public Xapian::ValueRangeProcessor {
    DestroyedFlag destroyed;

  public:
    TestValueRangeProcessor(bool & destroyed_) : destroyed(destroyed_) { }

    Xapian::valueno operator()(std::string &, std::string &) {
	return 42;
    }
};

/// Check reference counting of user-subclassable classes.
DEFINE_TESTCASE(subclassablerefcount8, !backend) {
    bool gone_auto, gone;

    // Simple test of release().
    {
	Xapian::ValueRangeProcessor * vrp = new TestValueRangeProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_valuerangeprocessor(vrp->release());
	TEST(!gone);
    }
    TEST(gone);

    // Check a second call to release() has no effect.
    {
	Xapian::ValueRangeProcessor * vrp = new TestValueRangeProcessor(gone);
	TEST(!gone);
	Xapian::QueryParser qp;
	qp.add_valuerangeprocessor(vrp->release());
	vrp->release();
	TEST(!gone);
    }
    TEST(gone);

    // Test reference counting works, and that a VRP with automatic storage
    // works OK.
    {
	TestValueRangeProcessor vrp_auto(gone_auto);
	TEST(!gone_auto);
	{
	    Xapian::QueryParser qp1;
	    {
		Xapian::QueryParser qp2;
		Xapian::ValueRangeProcessor * vrp;
		vrp = new TestValueRangeProcessor(gone);
		TEST(!gone);
		qp1.add_valuerangeprocessor(vrp->release());
		TEST(!gone);
		qp2.add_valuerangeprocessor(vrp);
		TEST(!gone);
		qp2.add_valuerangeprocessor(&vrp_auto);
		TEST(!gone);
		TEST(!gone_auto);
	    }
	    TEST(!gone);
	}
	TEST(gone);
	TEST(!gone_auto);
    }
    TEST(gone_auto);

    // Regression test for initial implementation, where ~opt_intrusive_ptr()
    // checked the reference of the object, which may have already been deleted
    // if it wasn't been reference counted.
    {
	Xapian::QueryParser qp;
	{
	    Xapian::ValueRangeProcessor * vrp =
		new TestValueRangeProcessor(gone);
	    TEST(!gone);
	    qp.add_valuerangeprocessor(vrp);
	    delete vrp;
	    TEST(gone);
	}
	// At the end of this block, qp is destroyed, but mustn't dereference
	// the pointer it has to vrp.  If it does, that should get caught
	// when tests are run under valgrind.
    }

    return true;
}

/// Check encoding of non-UTF8 document data.
DEFINE_TESTCASE(nonutf8docdesc1, !backend) {
    Xapian::Document doc;
    doc.set_data("\xc0\x80\xf5\x80\x80\x80\xfe\xff");
    TEST_EQUAL(doc.get_description(),
	      "Document(data='\\xc0\\x80\\xf5\\x80\\x80\\x80\\xfe\\xff')");
    doc.set_data(string("\x00\x1f", 2));
    TEST_EQUAL(doc.get_description(),
	      "Document(data='\\x00\\x1f')");
    // Check that backslashes are encoded so output isn't ambiguous.
    doc.set_data("back\\slash");
    TEST_EQUAL(doc.get_description(),
	      "Document(data='back\\x5cslash')");
    return true;
}
