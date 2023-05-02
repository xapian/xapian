/** @file
 * @brief Tests of serialisation functionality.
 */
/* Copyright 2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2012,2013 Olly Betts
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

#include "api_serialise.h"

#include <xapian.h>

#include <exception>
#include <stdexcept>

#include "apitest.h"
#include "testutils.h"

using namespace std;

// Test for serialising a document
DEFINE_TESTCASE(serialise_document1, !backend) {
    Xapian::Document doc;

    // Test serialising and unserialising an empty document.
    Xapian::Document doc1 = Xapian::Document::unserialise(doc.serialise());
    TEST_EQUAL(doc1.termlist_count(), 0);
    TEST_EQUAL(doc1.termlist_begin(), doc1.termlist_end());
    TEST_EQUAL(doc1.values_count(), 0);
    TEST_EQUAL(doc1.values_begin(), doc1.values_end());
    TEST_EQUAL(doc1.get_data(), "");

    // Test serialising a document with things in.
    doc.add_term("foo", 2);
    doc.add_posting("foo", 10);
    doc.add_value(1, "bar");
    doc.set_data("baz");

    Xapian::Document doc2 = Xapian::Document::unserialise(doc.serialise());

    TEST_EQUAL(doc.termlist_count(), doc2.termlist_count());
    TEST_EQUAL(doc.termlist_count(), 1);
    Xapian::TermIterator i;
    Xapian::PositionIterator j;
    Xapian::ValueIterator k;

    i = doc.termlist_begin();
    TEST_NOT_EQUAL(i, doc.termlist_end());
    TEST_EQUAL(i.get_wdf(), 3);
    TEST_EQUAL(*i, "foo");
    TEST_EQUAL(i.positionlist_count(), 1);
    j = i.positionlist_begin();
    TEST_NOT_EQUAL(j, i.positionlist_end());
    TEST_EQUAL(*j, 10);
    ++j;
    TEST_EQUAL(j, i.positionlist_end());
    ++i;
    TEST_EQUAL(i, doc.termlist_end());

    TEST_EQUAL(doc.values_count(), 1);
    k = doc.values_begin();
    TEST_NOT_EQUAL(k, doc.values_end());
    TEST_EQUAL(k.get_valueno(), 1);
    TEST_EQUAL(*k, "bar");
    ++k;
    TEST_EQUAL(k, doc.values_end());

    TEST_EQUAL(doc.get_data(), "baz");

    i = doc2.termlist_begin();
    TEST_NOT_EQUAL(i, doc2.termlist_end());
    TEST_EQUAL(i.get_wdf(), 3);
    TEST_EQUAL(*i, "foo");
    TEST_EQUAL(i.positionlist_count(), 1);
    j = i.positionlist_begin();
    TEST_NOT_EQUAL(j, i.positionlist_end());
    TEST_EQUAL(*j, 10);
    ++j;
    TEST_EQUAL(j, i.positionlist_end());
    ++i;
    TEST_EQUAL(i, doc2.termlist_end());

    TEST_EQUAL(doc2.values_count(), 1);
    k = doc2.values_begin();
    TEST_NOT_EQUAL(k, doc2.values_end());
    TEST_EQUAL(k.get_valueno(), 1);
    TEST_EQUAL(*k, "bar");
    ++k;
    TEST_EQUAL(k, doc2.values_end());

    TEST_EQUAL(doc2.get_data(), "baz");
}

// Test for serialising a document obtained from a database.
DEFINE_TESTCASE(serialise_document2, backend) {
    Xapian::Database db = get_database("serialise_document2",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   Xapian::Document doc;
					   doc.add_term("foo", 2);
					   doc.add_posting("foo", 10);
					   doc.add_value(1, "bar");
					   doc.set_data("baz");
					   wdb.add_document(doc);
				       });

    Xapian::Document doc = db.get_document(1);

    Xapian::Document doc2 = Xapian::Document::unserialise(doc.serialise());

    TEST_EQUAL(doc.termlist_count(), doc2.termlist_count());
    TEST_EQUAL(doc.termlist_count(), 1);
    Xapian::TermIterator i;
    Xapian::PositionIterator j;
    Xapian::ValueIterator k;

    i = doc.termlist_begin();
    TEST_NOT_EQUAL(i, doc.termlist_end());
    TEST_EQUAL(i.get_wdf(), 3);
    TEST_EQUAL(*i, "foo");
    TEST_EQUAL(i.positionlist_count(), 1);
    j = i.positionlist_begin();
    TEST_NOT_EQUAL(j, i.positionlist_end());
    TEST_EQUAL(*j, 10);
    ++j;
    TEST_EQUAL(j, i.positionlist_end());
    ++i;
    TEST_EQUAL(i, doc.termlist_end());

    TEST_EQUAL(doc.values_count(), 1);
    k = doc.values_begin();
    TEST_NOT_EQUAL(k, doc.values_end());
    TEST_EQUAL(k.get_valueno(), 1);
    TEST_EQUAL(*k, "bar");
    ++k;
    TEST_EQUAL(k, doc.values_end());

    TEST_EQUAL(doc.get_data(), "baz");

    i = doc2.termlist_begin();
    TEST_NOT_EQUAL(i, doc2.termlist_end());
    TEST_EQUAL(i.get_wdf(), 3);
    TEST_EQUAL(*i, "foo");
    TEST_EQUAL(i.positionlist_count(), 1);
    j = i.positionlist_begin();
    TEST_NOT_EQUAL(j, i.positionlist_end());
    TEST_EQUAL(*j, 10);
    ++j;
    TEST_EQUAL(j, i.positionlist_end());
    ++i;
    TEST_EQUAL(i, doc2.termlist_end());

    TEST_EQUAL(doc2.values_count(), 1);
    k = doc2.values_begin();
    TEST_NOT_EQUAL(k, doc2.values_end());
    TEST_EQUAL(k.get_valueno(), 1);
    TEST_EQUAL(*k, "bar");
    ++k;
    TEST_EQUAL(k, doc2.values_end());

    TEST_EQUAL(doc2.get_data(), "baz");
}

// Test for serialising a query
DEFINE_TESTCASE(serialise_query1, !backend) {
    Xapian::Query q;
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query()");

    q = Xapian::Query("hello");
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(hello)");

    q = Xapian::Query("hello", 1, 1);
    q2 = Xapian::Query::unserialise(q.serialise());
    // Regression test for fix in Xapian 1.0.0.
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(hello@1)");

    q = Xapian::Query(q.OP_OR, Xapian::Query("hello"), Xapian::Query("world"));
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query((hello OR world))");

    q = Xapian::Query(q.OP_OR,
		      Xapian::Query("hello", 1, 1),
		      Xapian::Query("world", 1, 1));
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query((hello@1 OR world@1))");

    static const char * const phrase[] = { "shaken", "not", "stirred" };
    q = Xapian::Query(q.OP_PHRASE, phrase, phrase + 3);
    q = Xapian::Query(q.OP_OR, Xapian::Query("007"), q);
    q = Xapian::Query(q.OP_SCALE_WEIGHT, q, 3.14);
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
}

// Test for serialising a query which contains a PostingSource.
DEFINE_TESTCASE(serialise_query2, !backend) {
    Xapian::ValueWeightPostingSource s1(10);
    Xapian::Query q(&s1);
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::ValueWeightPostingSource(slot=10)))");

    Xapian::ValueMapPostingSource s2(11);
    s2.set_default_weight(5.0);
    q = Xapian::Query(&s2);
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::ValueMapPostingSource(slot=11)))");

    Xapian::FixedWeightPostingSource s3(5.5);
    q = Xapian::Query(&s3);
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::FixedWeightPostingSource(wt=5.5)))");
}

// Test for unserialising a query using the default registry.
DEFINE_TESTCASE(serialise_query3, !backend) {
    Xapian::ValueWeightPostingSource s1(10);
    Xapian::Query q(&s1);
    Xapian::Registry reg;
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise(), reg);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::ValueWeightPostingSource(slot=10)))");

    Xapian::ValueMapPostingSource s2(11);
    s2.set_default_weight(5.0);
    q = Xapian::Query(&s2);
    q2 = Xapian::Query::unserialise(q.serialise(), reg);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::ValueMapPostingSource(slot=11)))");

    Xapian::FixedWeightPostingSource s3(5.5);
    q = Xapian::Query(&s3);
    q2 = Xapian::Query::unserialise(q.serialise(), reg);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Query(PostingSource(Xapian::FixedWeightPostingSource(wt=5.5)))");
}

class MyPostingSource2 : public Xapian::ValuePostingSource {
    std::string desc;
  public:
    MyPostingSource2(const std::string & desc_)
	    : Xapian::ValuePostingSource(0), desc(desc_)
    {
    }

    MyPostingSource2 * clone() const
    {
	return new MyPostingSource2(desc);
    }

    std::string name() const {
	return "MyPostingSource2";
    }

    std::string serialise() const {
	return desc;
    }

    MyPostingSource2 * unserialise(const std::string & s) const {
	return new MyPostingSource2(s);
    }

    double get_weight() const { return 1.0; }

    std::string get_description() const {
	return "MyPostingSource2(" + desc + ")";
    }
};

// Test for unserialising a query which contains a custom PostingSource.
DEFINE_TESTCASE(serialise_query4, !backend) {
    MyPostingSource2 s1("foo");
    Xapian::Query q(&s1);
    TEST_EQUAL(q.get_description(), "Query(PostingSource(MyPostingSource2(foo)))");
    std::string serialised = q.serialise();

    TEST_EXCEPTION(Xapian::SerialisationError, Xapian::Query::unserialise(serialised));
    Xapian::Registry reg;
    TEST_EXCEPTION(Xapian::SerialisationError, Xapian::Query::unserialise(serialised, reg));

    reg.register_posting_source(s1);
    Xapian::Query q2 = Xapian::Query::unserialise(serialised, reg);
    TEST_EQUAL(q.get_description(), q2.get_description());
}

/// Test for memory leaks when registering posting sources or weights twice.
DEFINE_TESTCASE(double_register_leak, !backend) {
    MyPostingSource2 s1("foo");
    Xapian::BM25Weight w1;

    Xapian::Registry reg;
    reg.register_posting_source(s1);
    reg.register_posting_source(s1);
    reg.register_posting_source(s1);

    reg.register_weighting_scheme(w1);
    reg.register_weighting_scheme(w1);
    reg.register_weighting_scheme(w1);
}

class ExceptionalPostingSource : public Xapian::PostingSource {
  public:
    typedef enum { NONE, CLONE } failmode;

    failmode fail;

    ExceptionalPostingSource(failmode fail_) : fail(fail_) { }

    string name() const {
	return "ExceptionalPostingSource";
    }

    PostingSource * clone() const {
	if (fail == CLONE)
	    throw bad_alloc();
	return new ExceptionalPostingSource(fail);
    }

    void init(const Xapian::Database &) { }

    Xapian::doccount get_termfreq_min() const { return 0; }
    Xapian::doccount get_termfreq_est() const { return 1; }
    Xapian::doccount get_termfreq_max() const { return 2; }

    void next(double) { }

    void skip_to(Xapian::docid, double) { }

    bool at_end() const { return true; }
    Xapian::docid get_docid() const { return 0; }
};

/// Check that exceptions when registering a postingsource are handled well.
DEFINE_TESTCASE(registry1, !backend) {
    // Test that a replacement object throwing bad_alloc is handled.
    {
	Xapian::Registry reg;

	ExceptionalPostingSource eps(ExceptionalPostingSource::NONE);
	TEST_EXCEPTION(Xapian::UnimplementedError, eps.serialise());
	TEST_EXCEPTION(Xapian::UnimplementedError, eps.unserialise(string()));
	reg.register_posting_source(eps);
	try {
	    ExceptionalPostingSource eps_clone(ExceptionalPostingSource::CLONE);
	    reg.register_posting_source(eps_clone);
	    FAIL_TEST("Expected bad_alloc exception to be thrown");
	} catch (const bad_alloc &) {
	}

	// Either the old entry should be removed, or it should work.
	const Xapian::PostingSource * p;
	p = reg.get_posting_source("ExceptionalPostingSource");
	if (p) {
	    TEST_EQUAL(p->name(), "ExceptionalPostingSource");
	}
    }
}

class ExceptionalWeight : public Xapian::Weight {
  public:
    typedef enum { NONE, CLONE } failmode;

    failmode fail;

    ExceptionalWeight(failmode fail_) : fail(fail_) { }

    string name() const {
	return "ExceptionalWeight";
    }

    Weight * clone() const {
	if (fail == CLONE)
	    throw bad_alloc();
	return new ExceptionalWeight(fail);
    }

    void init(double) { }

    double get_sumpart(Xapian::termcount, Xapian::termcount, Xapian::termcount) const {
	return 0;
    }
    double get_maxpart() const { return 0; }

    double get_sumextra(Xapian::termcount, Xapian::termcount) const { return 0; }
    double get_maxextra() const { return 0; }
};

/// Check that exceptions when registering are handled well.
DEFINE_TESTCASE(registry2, !backend) {
    // Test that a replacement object throwing bad_alloc is handled.
    {
	Xapian::Registry reg;

	ExceptionalWeight ewt(ExceptionalWeight::NONE);
	reg.register_weighting_scheme(ewt);
	try {
	    ExceptionalWeight ewt_clone(ExceptionalWeight::CLONE);
	    reg.register_weighting_scheme(ewt_clone);
	    FAIL_TEST("Expected bad_alloc exception to be thrown");
	} catch (const bad_alloc &) {
	}

	// Either the old entry should be removed, or it should work.
	const Xapian::Weight * p;
	p = reg.get_weighting_scheme("ExceptionalWeight");
	if (p) {
	    TEST_EQUAL(p->name(), "ExceptionalWeight");
	}
    }
}

class ExceptionalMatchSpy : public Xapian::MatchSpy {
  public:
    typedef enum { NONE, CLONE } failmode;

    failmode fail;

    ExceptionalMatchSpy(failmode fail_) : fail(fail_) { }

    string name() const {
	return "ExceptionalMatchSpy";
    }

    MatchSpy * clone() const {
	if (fail == CLONE)
	    throw bad_alloc();
	return new ExceptionalMatchSpy(fail);
    }

    void operator()(const Xapian::Document &, double) {
    }
};

/// Check that exceptions when registering are handled well.
DEFINE_TESTCASE(registry3, !backend) {
    // Test that a replacement object throwing bad_alloc is handled.
    {
	Xapian::Registry reg;

	ExceptionalMatchSpy ems(ExceptionalMatchSpy::NONE);
	reg.register_match_spy(ems);
	try {
	    ExceptionalMatchSpy ems_clone(ExceptionalMatchSpy::CLONE);
	    reg.register_match_spy(ems_clone);
	    FAIL_TEST("Expected bad_alloc exception to be thrown");
	} catch (const bad_alloc &) {
	}

	// Either the old entry should be removed, or it should work.
	const Xapian::MatchSpy * p;
	p = reg.get_match_spy("ExceptionalMatchSpy");
	if (p) {
	    TEST_EQUAL(p->name(), "ExceptionalMatchSpy");
	}
    }
}
