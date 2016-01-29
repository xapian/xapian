/** @file api_none.cc
 * @brief tests which don't need a backend
 */
/* Copyright (C) 2009 Richard Boulton
 * Copyright (C) 2009,2010,2011,2013,2014,2015 Olly Betts
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
    // Fails to compile with g++ 3.3.5 on OpenBSD (ticket#458):
    // TEST_EQUAL(db.get_metadata("foo"), std::string());
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
    // Like the previous test, but using MatchNothing as the subquery.
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
    singlesubquery3_(OP_NEAR);
    singlesubquery3_(OP_PHRASE);
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

class TestValueRangeProcessor : public Xapian::ValueRangeProcessor {
    DestroyedFlag destroyed;

  public:
    TestValueRangeProcessor(bool & destroyed_) : destroyed(destroyed_) { }

    Xapian::valueno operator()(std::string &, std::string &) {
	return 42;
    }
};

/// Check reference counting of user-subclassable classes.
DEFINE_TESTCASE(subclassablerefcount1, !backend) {
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

    // Regression test for initial implementation, where ~opt_instrusive_ptr()
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
