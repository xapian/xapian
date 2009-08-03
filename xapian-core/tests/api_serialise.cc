/** @file api_serialise.cc
 * @brief Tests of serialisation functionality.
 */
/* Copyright 2009 Lemur Consulting Ltd
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

#include "apitest.h"
#include "testutils.h"

using namespace std;

// Test for serialising a document
DEFINE_TESTCASE(serialise_document1, !backend) {
    Xapian::Document doc;
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
 
    return true;
}

// Test for serialising a document obtained from a database.
DEFINE_TESTCASE(serialise_document2, backend && writable) {
    Xapian::Document origdoc;
    origdoc.add_term("foo", 2);
    origdoc.add_posting("foo", 10);
    origdoc.add_value(1, "bar");
    origdoc.set_data("baz");
    Xapian::WritableDatabase db = get_writable_database();
    db.add_document(origdoc);

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
 
    return true;
}

// Test for serialising a query
DEFINE_TESTCASE(serialise_query1, !backend) {
    Xapian::Query q;
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query()");

    q = Xapian::Query("hello");
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(hello)");

    q = Xapian::Query(Xapian::Query::OP_OR, Xapian::Query("hello"), Xapian::Query("world"));
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query((hello OR world))");

    return true;
}

// Test for serialising a query which contains a PostingSource.
DEFINE_TESTCASE(serialise_query2, !backend) {
    Xapian::ValueWeightPostingSource s1(10);
    Xapian::Query q(&s1);
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::ValueWeightPostingSource(slot=10)))");

    Xapian::ValueMapPostingSource s2(11);
    s2.set_default_weight(5.0);
    q = Xapian::Query(&s2);
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::ValueMapPostingSource(slot=11)))");

    Xapian::FixedWeightPostingSource s3(5.5);
    q = Xapian::Query(&s3);
    q2 = Xapian::Query::unserialise(q.serialise());
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::FixedWeightPostingSource(wt=5.5)))");

    return true;
}

// Test for unserialising a query using the default context.
DEFINE_TESTCASE(serialise_query3, !backend) {
    Xapian::ValueWeightPostingSource s1(10);
    Xapian::Query q(&s1);
    Xapian::SerialisationContext ctx;
    Xapian::Query q2 = Xapian::Query::unserialise(q.serialise(), ctx);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::ValueWeightPostingSource(slot=10)))");

    Xapian::ValueMapPostingSource s2(11);
    s2.set_default_weight(5.0);
    q = Xapian::Query(&s2);
    q2 = Xapian::Query::unserialise(q.serialise(), ctx);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::ValueMapPostingSource(slot=11)))");

    Xapian::FixedWeightPostingSource s3(5.5);
    q = Xapian::Query(&s3);
    q2 = Xapian::Query::unserialise(q.serialise(), ctx);
    TEST_EQUAL(q.get_description(), q2.get_description());
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(Xapian::FixedWeightPostingSource(wt=5.5)))");

    return true;
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

    Xapian::weight get_weight() const { return 1.0; }

    std::string get_description() const {
	return "MyPostingSource2(" + desc + ")";
    }
};

// Test for unserialising a query which contains a custom PostingSource.
DEFINE_TESTCASE(serialise_query4, !backend) {
    MyPostingSource2 s1("foo");
    Xapian::Query q(&s1);
    TEST_EQUAL(q.get_description(), "Xapian::Query(PostingSource(MyPostingSource2(foo)))");
    std::string serialised = q.serialise();

    TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::Query::unserialise(serialised));
    Xapian::SerialisationContext ctx;
    TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::Query::unserialise(serialised, ctx));

    ctx.register_posting_source(s1);
    Xapian::Query q2 = Xapian::Query::unserialise(serialised, ctx);
    TEST_EQUAL(q.get_description(), q2.get_description());

    return true;
}

// Test for memory leaks when registering posting sources or weights twice.
DEFINE_TESTCASE(double_register_leak, !backend) {
    MyPostingSource2 s1("foo");
    Xapian::BM25Weight w1;

    Xapian::SerialisationContext ctx;
    ctx.register_posting_source(s1);
    ctx.register_posting_source(s1);
    ctx.register_posting_source(s1);

    ctx.register_weighting_scheme(w1);
    ctx.register_weighting_scheme(w1);
    ctx.register_weighting_scheme(w1);

    return true;
}
