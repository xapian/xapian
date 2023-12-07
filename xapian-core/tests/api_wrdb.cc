/** @file
 * @brief tests which need a writable backend
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002-2023 Olly Betts
 * Copyright 2006 Richard Boulton
 * Copyright 2007 Lemur Consulting Ltd
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

#include "api_wrdb.h"

#include <xapian.h>

#include "filetests.h"
#include "omassert.h"
#include "str.h"
#include "stringutils.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"

#include "apitest.h"

#include "safeunistd.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <string>

using namespace std;

// #######################################################################
// # Tests start here

// test that indexing a term more than once at the same position increases
// the wdf
DEFINE_TESTCASE(adddoc1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1, doc2, doc3;

    // doc1 should come top, but if term "foo" gets wdf of 1, doc2 will beat it
    // doc3 should beat both
    // Note: all docs have same length
    doc1.set_data(string("tom"));
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 3);
    doc1.add_posting("bar", 4);
    db.add_document(doc1);

    doc2.set_data(string("dick"));
    doc2.add_posting("foo", 1);
    doc2.add_posting("foo", 2);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    doc2.add_posting("bar", 3);
    db.add_document(doc2);

    doc3.set_data(string("harry"));
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 1);
    doc3.add_posting("foo", 2);
    doc3.add_posting("foo", 2);
    doc3.add_posting("bar", 3);
    db.add_document(doc3);

    Xapian::Query query("foo");

    Xapian::Enquire enq(db);
    enq.set_query(query);

    Xapian::MSet mset = enq.get_mset(0, 10);

    mset_expect_order(mset, 3, 1, 2);
}

// test that removing a posting and removing a term works
DEFINE_TESTCASE(adddoc2, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    // get_termfreq() on a TermIterator from a Document returns the termfreq
    // for just the shard the doc is in.
    bool sharded = (db.size() > 1);

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);
    // Quartz had a bug handling a term >= 128 characters longer than the
    // preceding term in the sort order - this is "foo" + 130 "X"s
    doc1.add_posting("fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 1);
    Xapian::docid did;

    Xapian::Document doc2 = db.get_document(did = db.add_document(doc1));
    TEST_EQUAL(did, 1);

    Xapian::TermIterator iter1 = doc1.termlist_begin();
    Xapian::TermIterator iter2 = doc2.termlist_begin();
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bar");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    if (!sharded) {
	// TEST_EQUAL(iter1.get_termfreq(), 0);
	TEST_EQUAL(iter2.get_termfreq(), 1);
    }

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 4);
    TEST_EQUAL(iter2.get_wdf(), 4);
    if (!sharded) {
	// TEST_EQUAL(iter1.get_termfreq(), 0);
	TEST_EQUAL(iter2.get_termfreq(), 1);
    }

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    if (!sharded) {
	// assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
	TEST_EQUAL(iter2.get_termfreq(), 1);
    }

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "gone");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    if (!sharded) {
	// assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
	TEST_EQUAL(iter2.get_termfreq(), 1);
    }

    iter1++;
    iter2++;
    TEST(iter1 == doc1.termlist_end());
    TEST(iter2 == doc2.termlist_end());

    doc2.remove_posting("foo", 1, 5);
    doc2.add_term("bat", 0);
    doc2.add_term("bar", 8);
    doc2.add_term("bag", 0);
    doc2.remove_term("gone");
    doc2.remove_term("fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

    // Should have (doc,wdf) pairs: (bag,0)(bar,9)(bat,0)(foo,0)
    // positionlists (bag,none)(bar,3)(bat,none)(foo,2)

    iter2 = doc2.termlist_begin();
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bag");
    // TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bar");
    // TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bat");
    // TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "foo");
    // TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 == doc2.termlist_end());

    doc1 = db.get_document(did = db.add_document(doc2));
    TEST_EQUAL(did, 2);

    iter1 = doc1.termlist_begin();
    iter2 = doc2.termlist_begin();
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bag");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    if (!sharded) {
	TEST_EQUAL(iter1.get_termfreq(), 1);
	// TEST_EQUAL(iter2.get_termfreq(), 0);
    }
    TEST(iter1.positionlist_begin() == iter1.positionlist_end());
    TEST(iter2.positionlist_begin() == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bar");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 9);
    TEST_EQUAL(iter2.get_wdf(), 9);
    if (!sharded) {
	TEST_EQUAL(iter1.get_termfreq(), 2);
	// TEST_EQUAL(iter2.get_termfreq(), 0);
    }

    Xapian::PositionIterator pi1;
    pi1 = iter1.positionlist_begin();
    Xapian::PositionIterator pi2 = iter2.positionlist_begin();
    TEST_EQUAL(*pi1, 3); pi1++;
    TEST_EQUAL(*pi2, 3); pi2++;
    TEST(pi1 == iter1.positionlist_end());
    TEST(pi2 == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "bat");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    if (!sharded) {
	TEST_EQUAL(iter1.get_termfreq(), 1);
	// TEST_EQUAL(iter2.get_termfreq(), 0);
    }
    TEST(iter1.positionlist_begin() == iter1.positionlist_end());
    TEST(iter2.positionlist_begin() == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 0);
    TEST_EQUAL(iter2.get_wdf(), 0);
    if (!sharded) {
	TEST_EQUAL(iter1.get_termfreq(), 2);
	// TEST_EQUAL(iter2.get_termfreq(), 0);
    }

    Xapian::PositionIterator temp1 = iter1.positionlist_begin();
    pi1 = temp1;
    Xapian::PositionIterator temp2 = iter2.positionlist_begin();
    pi2 = temp2;
    TEST_EQUAL(*pi1, 2); pi1++;
    TEST_EQUAL(*pi2, 2); pi2++;
    TEST(pi1 == iter1.positionlist_end());
    TEST(pi2 == iter2.positionlist_end());

    iter1++;
    iter2++;
    TEST(iter1 == doc1.termlist_end());
    TEST(iter2 == doc2.termlist_end());
}

// test that adding lots of documents works, and doesn't leak memory
// REGRESSION FIXED:2003-09-07
DEFINE_TESTCASE(adddoc3, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    for (Xapian::doccount i = 0; i < 2100; ++i) {
	Xapian::Document doc;
	for (Xapian::termcount t = 0; t < 100; ++t) {
	    string term("foo");
	    term += char(t ^ 70 ^ i);
	    doc.add_posting(term, t);
	}
	db.add_document(doc);
    }
}

// We originally wanted to test that a termlist starting with a 48 character
// long term worked since that required special handling in flint for
// historical reasons.  That's no longer relevant, but it seems useful to
// continue to test term lists starting with various term lengths work.
DEFINE_TESTCASE(adddoc4, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    for (Xapian::doccount i = 1; i <= 240; ++i) {
	Xapian::Document doc;
	string term(i, 'X');
	doc.add_term(term);
	db.add_document(doc);
    }
    db.add_document(Xapian::Document());
    db.commit();

    for (Xapian::doccount i = 1; i <= 240; ++i) {
	Xapian::Document doc = db.get_document(i);
	Xapian::TermIterator t = doc.termlist_begin();
	TEST(t != doc.termlist_end());
	TEST_EQUAL((*t).size(), i);
	++t;
	TEST(t == doc.termlist_end());
    }

    // And test a document with no terms.
    Xapian::Document doc = db.get_document(241);
    TEST(doc.termlist_begin() == doc.termlist_end());
}

// Test adding a document, and checking that it got added correctly.
// This testcase used to be adddoc2 in quartztest.
DEFINE_TESTCASE(adddoc5, writable) {
    // FIXME: With multi, get_termfreq() on a TermIterator from a Document
    // currently returns the termfreq for just the shard the doc is in.

    // Inmemory doesn't support get_writable_database_as_database().
    SKIP_TEST_FOR_BACKEND("inmemory");

    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);

    Xapian::Document document_in2;
    document_in2.set_data("Foobar falling");
    document_in2.add_posting("foobar", 1);
    document_in2.add_posting("falling", 2);
    {
	Xapian::WritableDatabase database(get_writable_database());

	TEST_EQUAL(database.get_doccount(), 0);
	TEST_EQUAL(database.get_avlength(), 0);

	did = database.add_document(document_in);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 3);

	TEST_EQUAL(database.get_termfreq("foobar"), 1);
	TEST_EQUAL(database.get_collection_freq("foobar"), 2);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 0);
	TEST_EQUAL(database.get_collection_freq("falling"), 0);

	Xapian::docid did2 = database.add_document(document_in2);
	TEST_EQUAL(database.get_doccount(), 2);
	TEST_NOT_EQUAL(did, did2);
	TEST_EQUAL(database.get_avlength(), 5.0 / 2.0);

	TEST_EQUAL(database.get_termfreq("foobar"), 2);
	TEST_EQUAL(database.get_collection_freq("foobar"), 3);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);

	database.delete_document(did);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 2);

	TEST_EQUAL(database.get_termfreq("foobar"), 1);
	TEST_EQUAL(database.get_collection_freq("foobar"), 1);
	TEST_EQUAL(database.get_termfreq("rising"), 0);
	TEST_EQUAL(database.get_collection_freq("rising"), 0);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);

	did = database.add_document(document_in);
	TEST_EQUAL(database.get_doccount(), 2);
	TEST_EQUAL(database.get_avlength(), 5.0 / 2.0);

	TEST_EQUAL(database.get_termfreq("foobar"), 2);
	TEST_EQUAL(database.get_collection_freq("foobar"), 3);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);
    }

    {
	Xapian::Database database(get_writable_database_as_database());
	Xapian::Document document_out = database.get_document(did);

	// get_termfreq() on a TermIterator from a Document returns the
	// termfreq for just the shard the doc is in.
	bool sharded = (database.size() > 1);

	TEST_EQUAL(document_in.get_data(), document_out.get_data());

	{
	    Xapian::ValueIterator i(document_in.values_begin());
	    Xapian::ValueIterator j(document_out.values_begin());
	    for (; i != document_in.values_end(); i++, j++) {
		TEST_NOT_EQUAL(j, document_out.values_end());
		TEST_EQUAL(*i, *j);
		TEST_EQUAL(i.get_valueno(), j.get_valueno());
	    }
	    TEST_EQUAL(j, document_out.values_end());
	}

	{
	    // Regression test for bug fixed in 1.0.5 - values_begin() didn't
	    // ensure that values had been read.  However, values_end() did
	    // (and so did values_count()) so this wasn't generally an issue
	    // but it shouldn't happen anyway.
	    Xapian::Document doc_tmp = database.get_document(did);
	    Xapian::ValueIterator i = document_in.values_begin();
	    Xapian::ValueIterator j = doc_tmp.values_begin();
	    TEST_EQUAL(*i, *j);
	}

	{
	    Xapian::TermIterator i(document_in.termlist_begin());
	    Xapian::TermIterator j(document_out.termlist_begin());
	    for (; i != document_in.termlist_end(); i++, j++) {
		TEST_NOT_EQUAL(j, document_out.termlist_end());
		TEST_EQUAL(*i, *j);
		TEST_EQUAL(i.get_wdf(), j.get_wdf());
		if (!sharded) {
		    // Actually use termfreq to stop compiler optimising away the
		    // call to get_termfreq().
		    TEST_EXCEPTION(Xapian::InvalidOperationError,
				   if (i.get_termfreq()) FAIL_TEST("?"));
		    TEST_NOT_EQUAL(0, j.get_termfreq());
		    if (*i == "foobar") {
			// termfreq of foobar is 2
			TEST_EQUAL(2, j.get_termfreq());
		    } else {
			// termfreq of rising is 1
			TEST_EQUAL(*i, "rising");
			TEST_EQUAL(1, j.get_termfreq());
		    }
		}
		Xapian::PositionIterator k(i.positionlist_begin());
		Xapian::PositionIterator l(j.positionlist_begin());
		for (; k != i.positionlist_end(); k++, l++) {
		    TEST_NOT_EQUAL(l, j.positionlist_end());
		    TEST_EQUAL(*k, *l);
		}
		TEST_EQUAL(l, j.positionlist_end());
	    }
	    TEST_EQUAL(j, document_out.termlist_end());
	}
    }
}

// Test adding a document, and checking that it got added correctly.
// This testcase used to be adddoc3 in quartztest.
DEFINE_TESTCASE(adddoc6, writable) {
    // Inmemory doesn't support get_writable_database_again().
    SKIP_TEST_FOR_BACKEND("inmemory");

    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foo", 1);
    document_in.add_posting("bar", 2);

    {
	Xapian::WritableDatabase database(get_writable_database());

	did = database.add_document(document_in);
	TEST_EQUAL(did, 1);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 2);
    }

    {
	Xapian::WritableDatabase database(get_writable_database_again());

	document_in.remove_term("foo");
	document_in.add_posting("baz", 1);

	database.replace_document(1, document_in);

	database.delete_document(1);

	TEST_EQUAL(database.get_doccount(), 0);
	TEST_EQUAL(database.get_avlength(), 0);
	TEST_EQUAL(database.get_termfreq("foo"), 0);
	TEST_EQUAL(database.get_collection_freq("foo"), 0);
	TEST_EQUAL(database.get_termfreq("bar"), 0);
	TEST_EQUAL(database.get_collection_freq("bar"), 0);
	TEST_EQUAL(database.get_termfreq("baz"), 0);
	TEST_EQUAL(database.get_collection_freq("baz"), 0);
    }
}

// tests that database destructors commit if it isn't done explicitly
DEFINE_TESTCASE(implicitendsession1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;

    doc.set_data(string("top secret"));
    doc.add_posting("cia", 1);
    doc.add_posting("nsa", 2);
    doc.add_posting("fbi", 3);
    db.add_document(doc);
}

// tests that assignment of Xapian::Database and Xapian::WritableDatabase works
// as expected
DEFINE_TESTCASE(databaseassign1, writable) {
    Xapian::WritableDatabase wdb = get_writable_database();
    Xapian::Database db = get_database("");
    Xapian::Database actually_wdb = wdb;
    Xapian::WritableDatabase w1(wdb);
    w1 = wdb;
    Xapian::Database d1(wdb);
    Xapian::Database d2(actually_wdb);
    d2 = wdb;
    d2 = actually_wdb;
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
    // Suppress warning from newer clang about self-assignment so we can
    // test that self-assignment works!
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wself-assign-overloaded"
# endif
#endif
    wdb = wdb; // check assign to itself works
    db = db; // check assign to itself works
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
#  pragma clang diagnostic pop
# endif
#endif
}

// tests that deletion and updating of documents works as expected
DEFINE_TESTCASE(deldoc1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("foo", 2);
    doc1.add_posting("bar", 3);
    doc1.add_posting("gone", 1);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("gone");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_term("new", 1);
    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.delete_document(1);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));

    doc1 = db.get_document(2);
    doc1.remove_term("foo");
    doc1.add_term("fwing");
    db.replace_document(2, doc1);

    Xapian::Document doc2 = db.get_document(2);
    Xapian::TermIterator tit = doc2.termlist_begin();
    TEST_NOT_EQUAL(tit, doc2.termlist_end());
    TEST_EQUAL(*tit, "bar");
    tit++;
    TEST_NOT_EQUAL(tit, doc2.termlist_end());
    TEST_EQUAL(*tit, "fwing");
    tit++;
    TEST_EQUAL(tit, doc2.termlist_end());
}

// tests that deletion and updating of documents works as expected
DEFINE_TESTCASE(deldoc2, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);
    Xapian::docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("one");
    doc1.add_posting("three", 4);

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_posting("one", 7);
    doc1.remove_term("two");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.commit();

    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    db.delete_document(1);
    db.delete_document(2);
    db.delete_document(3);

    db.commit();

    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(3));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(4));

    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);
    TEST_EQUAL(db.get_termfreq("two"), 0);
    TEST_EQUAL(db.get_termfreq("three"), 0);

    TEST(!db.term_exists("one"));
    TEST(!db.term_exists("two"));
    TEST(!db.term_exists("three"));

    TEST_EQUAL(db.get_collection_freq("one"), 0);
    TEST_EQUAL(db.get_collection_freq("two"), 0);
    TEST_EQUAL(db.get_collection_freq("three"), 0);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(3));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(3));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(3));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());
}

// another test of deletion of documents, a cut-down version of deldoc2
DEFINE_TESTCASE(deldoc3, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("one", 1);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    db.commit();

    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    db.delete_document(1);

    db.commit();

    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(2));

    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);

    TEST(!db.term_exists("one"));

    TEST_EQUAL(db.get_collection_freq("one"), 0);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(2));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(2));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());
}

// tests that deletion and updating of (lots of) documents works as expected
DEFINE_TESTCASE(deldoc4, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("one", 1);
    doc1.add_posting("two", 2);
    doc1.add_posting("two", 3);

    Xapian::Document doc2 = doc1;
    doc2.remove_term("one");
    doc2.add_posting("three", 4);

    Xapian::Document doc3 = doc2;
    doc3.add_posting("one", 7);
    doc3.remove_term("two");

    const Xapian::docid maxdoc = 1000 * 3;
    Xapian::docid did;
    for (Xapian::docid i = 0; i < maxdoc / 3; ++i) {
	did = db.add_document(doc1);
	TEST_EQUAL(did, i * 3 + 1);
	did = db.add_document(doc2);
	TEST_EQUAL(did, i * 3 + 2);
	did = db.add_document(doc3);
	TEST_EQUAL(did, i * 3 + 3);

	bool is_power_of_two = ((i & (i - 1)) == 0);
	if (is_power_of_two) {
	    db.commit();
	    // reopen() on a writable database shouldn't do anything.
	    TEST(!db.reopen());
	}
    }
    db.commit();
    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    /* delete the documents in a peculiar order */
    for (Xapian::docid i = 0; i < maxdoc / 3; ++i) {
	db.delete_document(maxdoc - i);
	db.delete_document(maxdoc / 3 + i + 1);
	db.delete_document(i + 1);
    }

    db.commit();
    // reopen() on a writable database shouldn't do anything.
    TEST(!db.reopen());

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    for (Xapian::docid i = 1; i <= maxdoc; ++i) {
	// TEST_EXCEPTION writes to tout each time if the test is run
	// in verbose mode and some string stream implementations get
	// very inefficient with large strings, so clear tout on each pass of
	// the loop to speed up the test since the older information isn't
	// interesting anyway.
	tout.str(string());
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(i));
    }

    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);
    TEST_EQUAL(db.get_termfreq("two"), 0);
    TEST_EQUAL(db.get_termfreq("three"), 0);

    TEST(!db.term_exists("one"));
    TEST(!db.term_exists("two"));
    TEST(!db.term_exists("three"));

    TEST_EQUAL(db.get_collection_freq("one"), 0);
    TEST_EQUAL(db.get_collection_freq("two"), 0);
    TEST_EQUAL(db.get_collection_freq("three"), 0);

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());
}

// Test deleting a document which was added in the same batch.
DEFINE_TESTCASE(deldoc5, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 2);
    doc1.add_posting("aardvark", 3);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("bar");
    doc1.add_term("hello");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_term("world", 1);
    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.delete_document(2);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    db.commit();

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    TEST_EQUAL(db.get_termfreq("foo"), 2);
    TEST_EQUAL(db.get_termfreq("aardvark"), 2);
    TEST_EQUAL(db.get_termfreq("hello"), 1);

    Xapian::PostingIterator p = db.postlist_begin("foo");
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 1);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 3);
    ++p;
    TEST_EQUAL(p, db.postlist_end("foo"));
}

// Regression test for bug in quartz and flint, fixed in 1.0.2.
DEFINE_TESTCASE(deldoc6, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 2);
    doc1.add_posting("aardvark", 3);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("bar");
    doc1.add_term("hello");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    db.commit();

    db.delete_document(2);
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.delete_document(3));

    db.commit();

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));
}

DEFINE_TESTCASE(replacedoc1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone", 3);
    doc1.add_posting("bar", 4);
    doc1.add_posting("foo", 5);
    Xapian::docid did;

    did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    Xapian::Document doc2;

    doc2.add_posting("foo", 1);
    doc2.add_posting("pipco", 2);
    doc2.add_posting("bar", 4);
    doc2.add_posting("foo", 5);

    db.replace_document(did, doc2);

    Xapian::Document doc3 = db.get_document(did);
    Xapian::TermIterator t_iter = doc3.termlist_begin();
    TEST_EQUAL(*t_iter, "bar");
    Xapian::PositionIterator p_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*p_iter, 4);
    ++t_iter;
    TEST_EQUAL(*t_iter, "foo");
    Xapian::PositionIterator q_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*q_iter, 1);
    ++q_iter;
    TEST_EQUAL(*q_iter, 5);
    ++t_iter;
    TEST_EQUAL(*t_iter, "pipco");
    Xapian::PositionIterator r_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*r_iter, 2);
    ++t_iter;
    TEST_EQUAL(t_iter, doc3.termlist_end());
}

// Test of new feature: WritableDatabase::replace_document accepts a docid
// which doesn't yet exist as of Xapian 0.8.2.
DEFINE_TESTCASE(replacedoc2, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone", 3);
    doc1.add_posting("bar", 4);
    doc1.add_posting("foo", 5);
    Xapian::docid did = 31770;

    db.replace_document(did, doc1);

    // Regression tests for bug in the InMemory backend - fixed in 1.0.2.
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    Xapian::PostingIterator postit = db.postlist_begin("");
    TEST(postit != db.postlist_end(""));
    TEST_EQUAL(*postit, 31770);

    Xapian::Document doc2;

    doc2.add_posting("foo", 1);
    doc2.add_posting("pipco", 2);
    doc2.add_posting("bar", 4);
    doc2.add_posting("foo", 5);

    db.replace_document(did, doc2);
    TEST_EQUAL(db.get_doccount(), 1);

    Xapian::Document doc3 = db.get_document(did);
    Xapian::TermIterator t_iter = doc3.termlist_begin();
    TEST_EQUAL(*t_iter, "bar");
    Xapian::PositionIterator p_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*p_iter, 4);
    ++t_iter;
    TEST_EQUAL(*t_iter, "foo");
    Xapian::PositionIterator q_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*q_iter, 1);
    ++q_iter;
    TEST_EQUAL(*q_iter, 5);
    ++t_iter;
    TEST_EQUAL(*t_iter, "pipco");
    Xapian::PositionIterator r_iter = t_iter.positionlist_begin();
    TEST_EQUAL(*r_iter, 2);
    ++t_iter;
    TEST_EQUAL(t_iter, doc3.termlist_end());

    did = db.add_document(doc1);
    TEST_EQUAL(did, 31771);
    TEST_EQUAL(db.get_doccount(), 2);

    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.replace_document(0, doc2));
}

// Test replacing a document which was added in the same batch.
DEFINE_TESTCASE(replacedoc3, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 2);
    doc1.add_posting("aardvark", 3);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("bar");
    doc1.add_term("hello");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_term("world", 1);
    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    Xapian::Document doc2;
    doc2.add_term("world");
    db.replace_document(2, doc2);

    db.commit();

    // Check that the document exists (no DocNotFoundError).
    doc2 = db.get_document(2);

    TEST_EQUAL(db.get_termfreq("foo"), 2);
    TEST_EQUAL(db.get_termfreq("aardvark"), 2);
    TEST_EQUAL(db.get_termfreq("hello"), 1);
    TEST_EQUAL(db.get_termfreq("world"), 2);

    TEST_EQUAL(db.get_doclength(1), 3);
    TEST_EQUAL(db.get_doclength(2), 1);
    TEST_EQUAL(db.get_doclength(3), 4);

    TEST_EQUAL(db.get_unique_terms(1), 3);
    TEST_EQUAL(db.get_unique_terms(2), 1);
    TEST_EQUAL(db.get_unique_terms(3), 4);

    Xapian::PostingIterator p = db.postlist_begin("foo");
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 1);
    TEST_EQUAL(p.get_doclength(), 3);
    TEST_EQUAL(p.get_unique_terms(), 3);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 3);
    TEST_EQUAL(p.get_doclength(), 4);
    TEST_EQUAL(p.get_unique_terms(), 4);
    ++p;
    TEST_EQUAL(p, db.postlist_end("foo"));

    p = db.postlist_begin("world");
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 2);
    TEST_EQUAL(p.get_doclength(), 1);
    TEST_EQUAL(p.get_unique_terms(), 1);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 3);
    TEST_EQUAL(p.get_doclength(), 4);
    TEST_EQUAL(p.get_unique_terms(), 4);
    ++p;
    TEST_EQUAL(p, db.postlist_end("world"));
}

// Test replacing a document which was deleted in the same batch.
DEFINE_TESTCASE(replacedoc4, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("bar", 2);
    doc1.add_posting("aardvark", 3);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    doc1.remove_term("bar");
    doc1.add_term("hello");

    did = db.add_document(doc1);
    TEST_EQUAL(did, 2);

    doc1.add_term("world", 1);
    did = db.add_document(doc1);
    TEST_EQUAL(did, 3);

    db.delete_document(2);

    Xapian::Document doc2;
    doc2.add_term("world");
    db.replace_document(2, doc2);

    db.commit();

    // Check that the document exists (no DocNotFoundError).
    doc2 = db.get_document(2);

    TEST_EQUAL(db.get_termfreq("foo"), 2);
    TEST_EQUAL(db.get_termfreq("aardvark"), 2);
    TEST_EQUAL(db.get_termfreq("hello"), 1);
    TEST_EQUAL(db.get_termfreq("world"), 2);

    Xapian::PostingIterator p = db.postlist_begin("foo");
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 1);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 3);
    ++p;
    TEST_EQUAL(p, db.postlist_end("foo"));

    p = db.postlist_begin("world");
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 2);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 3);
    ++p;
    TEST_EQUAL(p, db.postlist_end("world"));
}

// Test replacing a document with itself without modifying postings.
// Regression test for bug in 0.9.9 and earlier - there flint and quartz
// lost all positional information for the document when you did this.
DEFINE_TESTCASE(replacedoc5, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    {
	Xapian::Document doc;
	doc.add_posting("hello", 1);
	doc.add_posting("world", 2);

	Xapian::docid did = db.add_document(doc);
	TEST_EQUAL(did, 1);
	db.commit();
    }

    {
	Xapian::Document doc = db.get_document(1);
	TEST(db.has_positions());
	TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
	TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));
	db.replace_document(1, doc);
	db.commit();

	TEST(db.has_positions());
	TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
	TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));
    }

    // The backends now spot simple cases of replacing the same document and
    // don't do needless work.  Force them to actually do the replacement to
    // make sure that case works.

    {
	Xapian::Document doc;
	doc.add_term("Q2");
	db.add_document(doc);
	db.commit();
    }

    {
	Xapian::Document doc = db.get_document(1);
	TEST(db.has_positions());
	TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
	TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));
	(void)db.get_document(2);
	db.replace_document(1, doc);
	db.commit();

	TEST(db.has_positions());
	TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
	TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));
    }
}

// Test replacing a document while adding values, without changing anything
// else.  Regression test for a bug introduced while implementing lazy update,
// and also covers a few other code paths.
DEFINE_TESTCASE(replacedoc6, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    Xapian::docid did = db.add_document(doc);
    TEST_EQUAL(did, 1);
    db.commit();

    // Add document
    doc = db.get_document(1);
    TEST_EQUAL(doc.get_value(1), "");
    TEST_EQUAL(doc.get_value(2), "");
    doc.add_value(1, "banana1");
    db.replace_document(1, doc);

    doc = db.get_document(1);
    TEST_EQUAL(doc.get_value(1), "banana1");
    TEST_EQUAL(doc.get_value(2), "");
    db.commit();

    doc = db.get_document(1);
    TEST_EQUAL(doc.get_value(1), "banana1");
    TEST_EQUAL(doc.get_value(2), "");
    doc.add_value(2, "banana2");
    db.replace_document(1, doc);

    TEST_EQUAL(doc.get_value(1), "banana1");
    TEST_EQUAL(doc.get_value(2), "banana2");
    db.commit();

    doc = db.get_document(1);
    TEST_EQUAL(doc.get_value(1), "banana1");
    TEST_EQUAL(doc.get_value(2), "banana2");
}

// Test of new feature: WritableDatabase::replace_document and delete_document
// can take a unique termname instead of a document id as of Xapian 0.8.2.
DEFINE_TESTCASE(uniqueterm1, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    for (int n = 1; n <= 20; ++n) {
	Xapian::Document doc;
	string uterm = "U" + str(n % 16);
	doc.add_term(uterm);
	doc.add_term(str(n));
	doc.add_term(str(n ^ 9));
	doc.add_term("all");
	doc.set_data("pass1");
	db.add_document(doc);
    }

    TEST_EQUAL(db.get_doccount(), 20);

    static const Xapian::doccount sizes[20] = {
	19, 17, 16, 15,
	15, 15, 15, 15,
	15, 15, 15, 15,
	15, 15, 15, 15,
	15, 15, 15, 15
    };
    for (int n = 1; n <= 20; ++n) {
	string uterm = "U" + str(n % 16);
	if (uterm == "U2") {
	    db.delete_document(uterm);
	} else {
	    Xapian::Document doc;
	    doc.add_term(uterm);
	    doc.add_term(str(n));
	    doc.add_term(str(n ^ 9));
	    doc.add_term("all");
	    doc.set_data("pass2");
	    db.replace_document(uterm, doc);
	}
	TEST_EQUAL(db.get_doccount(), sizes[n - 1]);
    }

    string uterm = "U571";
    Xapian::Document doc;
    doc.add_term(uterm);
    doc.set_data("pass3");
    db.replace_document(uterm, doc);

    TEST_EQUAL(db.get_doccount(), 16);

    db.delete_document("U2");

    TEST_EQUAL(db.get_doccount(), 16);
}

// tests all document postlists
DEFINE_TESTCASE(allpostlist2, writable) {
    Xapian::WritableDatabase db(get_writable_database("apitest_manydocs"));
    Xapian::PostingIterator i = db.postlist_begin("");
    unsigned int j = 1;
    while (i != db.postlist_end("")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
    }
    TEST_EQUAL(j, 513);

    db.delete_document(1);
    db.delete_document(50);
    db.delete_document(512);

    i = db.postlist_begin("");
    j = 2;
    while (i != db.postlist_end("")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
	if (j == 50) j++;
    }
    TEST_EQUAL(j, 512);

    i = db.postlist_begin("");
    j = 2;
    while (i != db.postlist_end("")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
	if (j == 40) {
	    j += 10;
	    i.skip_to(j);
	    j++;
	}
    }
    TEST_EQUAL(j, 512);
}

static void test_emptyterm2_helper(Xapian::WritableDatabase & db)
{
    // Don't bother with postlist_begin() because allpostlist tests cover that.
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.positionlist_begin(1, ""));
    TEST_EQUAL(db.get_doccount(), db.get_termfreq(""));
    TEST_EQUAL(db.get_doccount() != 0, db.term_exists(""));
    TEST_EQUAL(db.get_doccount(), db.get_collection_freq(""));
}

// tests results of passing an empty term to various methods
// equivalent of emptyterm1 for a writable database
DEFINE_TESTCASE(emptyterm2, writable) {
    {
	Xapian::WritableDatabase db(get_writable_database("apitest_manydocs"));
	TEST_EQUAL(db.get_doccount(), 512);
	test_emptyterm2_helper(db);
	db.delete_document(1);
	TEST_EQUAL(db.get_doccount(), 511);
	test_emptyterm2_helper(db);
	db.delete_document(50);
	TEST_EQUAL(db.get_doccount(), 510);
	test_emptyterm2_helper(db);
	db.delete_document(512);
	TEST_EQUAL(db.get_doccount(), 509);
	test_emptyterm2_helper(db);
    }

    {
	Xapian::WritableDatabase db(get_writable_database("apitest_onedoc"));
	TEST_EQUAL(db.get_doccount(), 1);
	test_emptyterm2_helper(db);
	db.delete_document(1);
	TEST_EQUAL(db.get_doccount(), 0);
	test_emptyterm2_helper(db);
    }

    {
	Xapian::WritableDatabase db(get_writable_database());
	TEST_EQUAL(db.get_doccount(), 0);
	test_emptyterm2_helper(db);
    }
}

// Check that PHRASE/NEAR becomes AND if there's no positional info in the
// database.
DEFINE_TESTCASE(phraseorneartoand1, backend) {
    Xapian::Database db = get_database("phraseorneartoand1",
				       [](Xapian::WritableDatabase& wdb,
					  const string&)
				       {
					   for (int n = 1; n <= 20; ++n) {
					       Xapian::Document doc;
					       doc.add_term(str(n));
					       doc.add_term(str(n ^ 9));
					       doc.add_term("all");
					       doc.set_data("pass1");
					       wdb.add_document(doc);
					   }
				       });

    Xapian::Enquire enquire(db);
    Xapian::MSet mymset;

    static const char * const q1[] = { "all", "1" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_PHRASE, q1, q1 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(2, mymset.size());

    enquire.set_query(Xapian::Query(Xapian::Query::OP_NEAR, q1, q1 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(2, mymset.size());

    static const char * const q2[] = { "1", "2" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_PHRASE, q2, q2 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(0, mymset.size());

    enquire.set_query(Xapian::Query(Xapian::Query::OP_NEAR, q2, q2 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(0, mymset.size());
}

static void
gen_longpositionlist1_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    for (Xapian::termpos n = 1; n <= 2000; ++n) {
	doc.add_posting("fork", n * 3);
	doc.add_posting("knife", n * unsigned(log(double(n + 2))));
	doc.add_posting("spoon", n * n);
	// Exercise positions up to 4 billion.
	Xapian::termpos half_cube = n * n / 2 * n;
	doc.add_posting("chopsticks", half_cube);
	if (sizeof(Xapian::termpos) >= 8) {
	    // Exercise 64-bit positions.
	    doc.add_posting("spork", half_cube * half_cube);
	}
    }
    doc.set_data("cutlery");
    db.add_document(doc);
}

// Check that a large number of position list entries for a particular term
// works - regression test for flint.
DEFINE_TESTCASE(longpositionlist1, backend) {
    Xapian::Database db = get_database("longpositionlist1",
				       gen_longpositionlist1_db);

    Xapian::Document doc = db.get_document(1);

    Xapian::TermIterator t, tend;
    Xapian::PositionIterator p, pend;

    t = doc.termlist_begin();
    tend = doc.termlist_end();

    TEST(t != tend);
    TEST_EQUAL(*t, "chopsticks");
    p = t.positionlist_begin();
    pend = t.positionlist_end();
    for (Xapian::termpos n = 1; n <= 2000; ++n) {
	TEST(p != pend);
	Xapian::termpos half_cube = n * n / 2 * n;
	TEST_EQUAL(*p, half_cube);
	++p;
    }
    TEST(p == pend);

    ++t;
    TEST(t != tend);
    TEST_EQUAL(*t, "fork");
    p = t.positionlist_begin();
    pend = t.positionlist_end();
    for (Xapian::termpos n = 1; n <= 2000; ++n) {
	TEST(p != pend);
	TEST_EQUAL(*p, n * 3);
	++p;
    }
    TEST(p == pend);

    ++t;
    TEST(t != tend);
    TEST_EQUAL(*t, "knife");
    p = t.positionlist_begin();
    pend = t.positionlist_end();
    for (Xapian::termpos n = 1; n <= 2000; ++n) {
	TEST(p != pend);
	TEST_EQUAL(*p, n * unsigned(log(double(n + 2))));
	++p;
    }
    TEST(p == pend);

    ++t;
    TEST(t != tend);
    TEST_EQUAL(*t, "spoon");
    p = t.positionlist_begin();
    pend = t.positionlist_end();
    for (Xapian::termpos n = 1; n <= 2000; ++n) {
	TEST(p != pend);
	TEST_EQUAL(*p, n * n);
	++p;
    }
    TEST(p == pend);

    if (sizeof(Xapian::termpos) >= 8) {
	++t;
	TEST(t != tend);
	TEST_EQUAL(*t, "spork");
	p = t.positionlist_begin();
	pend = t.positionlist_end();
	for (Xapian::termpos n = 1; n <= 2000; ++n) {
	    TEST(p != pend);
	    Xapian::termpos half_cube = n * n / 2 * n;
	    TEST_EQUAL(*p, half_cube * half_cube);
	    ++p;
	}
	TEST(p == pend);
    }

    ++t;
    TEST(t == tend);
}

static void
gen_consistency2_db(Xapian::WritableDatabase& db, const string&)
{
    char buf[2] = "X";

    // Add 5 documents indexed by "test" with wdf 1.
    for (int i = 0; i < 5; ++i) {
	Xapian::Document doc;
	*buf = '0' + i;
	doc.add_value(0, buf);
	doc.add_term("test");
	db.add_document(doc);
    }

    // Add 5 documents indexed by "test" with wdf 2.
    for (int i = 0; i < 5; ++i) {
	Xapian::Document doc;
	*buf = '0' + i;
	doc.add_value(0, buf);
	doc.add_term("test", 2);
	db.add_document(doc);
    }
}

// Regression test for bug#110: Inconsistent sort order between pages with
// set_sort_by_value_then_relevance.
DEFINE_TESTCASE(consistency2, backend) {
    Xapian::Database db = get_database("consistency2", gen_consistency2_db);

    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("test"));

    enq.set_sort_by_value_then_relevance(0, true);

    // 10 results, unpaged.
    Xapian::MSet mset1 = enq.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 10);

    // 10 results, split.
    Xapian::MSet mset2a = enq.get_mset(0, 1);
    TEST_EQUAL(mset2a.size(), 1);
    Xapian::MSet mset2b = enq.get_mset(1, 1);
    TEST_EQUAL(mset2b.size(), 1);
    Xapian::MSet mset2c = enq.get_mset(2, 8);
    TEST_EQUAL(mset2c.size(), 8);

    TEST_EQUAL(*mset1[0], *mset2a[0]);
    TEST_EQUAL(*mset1[1], *mset2b[0]);
    for (Xapian::doccount i = 0; i < 8; ++i) {
	TEST_EQUAL(*mset1[i + 2], *mset2c[i]);
    }
}

DEFINE_TESTCASE(crashrecovery1, chert) {
    // Glass has a single version file per revision, rather than multiple base
    // files, so it simply can't get into the situations we are testing
    // recovery from.
    const string & dbtype = get_dbtype();
    string path = ".";
    path += dbtype;
    path += "/dbw";
    const char * base_ext = ".baseB";

    Xapian::Document doc;
    {
	Xapian::WritableDatabase db = get_writable_database();
	Xapian::Database dbr(get_writable_database_as_database());
	TEST_EQUAL(dbr.get_doccount(), 0);

	// Xapian::Database has full set of baseA, no baseB
	TEST(file_exists(path + "/postlist.baseA"));
	TEST(file_exists(path + "/record.baseA"));
	TEST(file_exists(path + "/termlist.baseA"));
	TEST(!file_exists(path + "/postlist.baseB"));
	TEST(!file_exists(path + "/record.baseB"));
	TEST(!file_exists(path + "/termlist.baseB"));

	db.add_document(doc);
	db.commit();
	TEST(dbr.reopen());
	TEST_EQUAL(dbr.get_doccount(), 1);

	// Xapian::Database has full set of baseB, old baseA
	TEST(file_exists(path + "/postlist.baseA"));
	TEST(file_exists(path + "/record.baseA"));
	TEST(file_exists(path + "/termlist.baseA"));
	TEST(file_exists(path + "/postlist.baseB"));
	TEST(file_exists(path + "/record.baseB"));
	TEST(file_exists(path + "/termlist.baseB"));

	db.add_document(doc);
	db.commit();
	TEST(dbr.reopen());
	TEST_EQUAL(dbr.get_doccount(), 2);

	// Xapian::Database has full set of baseA, old baseB
	TEST(file_exists(path + "/postlist.baseA"));
	TEST(file_exists(path + "/record.baseA"));
	TEST(file_exists(path + "/termlist.baseA"));
	TEST(file_exists(path + "/postlist.baseB"));
	TEST(file_exists(path + "/record.baseB"));
	TEST(file_exists(path + "/termlist.baseB"));

	// Simulate a transaction starting, some of the baseB getting removed,
	// but then the transaction fails.
	unlink((path + "/record" + base_ext).c_str());
	unlink((path + "/termlist" + base_ext).c_str());

	TEST(!dbr.reopen());
	TEST_EQUAL(dbr.get_doccount(), 2);
    }

    Xapian::WritableDatabase db(path, Xapian::DB_OPEN);
    // Xapian::Database has full set of baseA, some old baseB
    TEST(file_exists(path + "/postlist.baseA"));
    TEST(file_exists(path + "/record.baseA"));
    TEST(file_exists(path + "/termlist.baseA"));
    TEST(file_exists(path + "/postlist.baseB"));
    TEST(!file_exists(path + "/record.baseB"));
    TEST(!file_exists(path + "/termlist.baseB"));
    Xapian::Database dbr = Xapian::Database(path);

    db.add_document(doc);
    db.commit();
    TEST(dbr.reopen());
    TEST_EQUAL(dbr.get_doccount(), 3);

    // Xapian::Database has full set of baseB, old baseA
    TEST(file_exists(path + "/postlist.baseA"));
    TEST(file_exists(path + "/record.baseA"));
    TEST(file_exists(path + "/termlist.baseA"));
    TEST(file_exists(path + "/postlist.baseB"));
    TEST(file_exists(path + "/record.baseB"));
    TEST(file_exists(path + "/termlist.baseB"));

    db.add_document(doc);
    db.commit();
    TEST(dbr.reopen());
    TEST_EQUAL(dbr.get_doccount(), 4);
}

// Check that DatabaseError is thrown if the docid counter would wrap.
// Regression test for bug#152.
DEFINE_TESTCASE(nomoredocids1, writable) {
    // The InMemory backend uses a vector for the documents, so trying to add
    // a document with a really large docid will fail because we can't allocate
    // enough memory!
    SKIP_TEST_FOR_BACKEND("inmemory");

    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.set_data("prose");
    doc.add_term("word");

    Xapian::docid max_id = numeric_limits<Xapian::docid>::max();
    db.replace_document(max_id, doc);

    TEST_EXCEPTION(Xapian::DatabaseError, db.add_document(doc));

    // Also test replace_document() by term which will try to add a new
    // document if the term isn't present - that should also fail if the
    // docid counter would wrap.
    TEST_EXCEPTION(Xapian::DatabaseError, db.replace_document("Q42", doc));
}

// Test synonym iterators.
DEFINE_TESTCASE(synonymitor1, writable && synonyms) {
    Xapian::WritableDatabase db = get_writable_database();

    // Test iterators for terms which aren't there.
    TEST(db.synonyms_begin("abc") == db.synonyms_end("abc"));

    // Test iterating the synonym keys when there aren't any.
    TEST(db.synonym_keys_begin() == db.synonym_keys_end());

    db.add_synonym("hello", "howdy");
    db.add_synonym("hello", "hi");
    db.add_synonym("goodbye", "bye");
    db.add_synonym("goodbye", "farewell");

    Xapian::TermIterator t;
    string s;

    // Try these tests twice - once before committing and once after.
    for (int times = 1; times <= 2; ++times) {
	// Test iterators for terms which aren't there.
	TEST(db.synonyms_begin("abc") == db.synonyms_end("abc"));
	TEST(db.synonyms_begin("ghi") == db.synonyms_end("ghi"));
	TEST(db.synonyms_begin("zzzzz") == db.synonyms_end("zzzzz"));

	s = "|";
	t = db.synonyms_begin("hello");
	while (t != db.synonyms_end("hello")) {
	    s += *t++;
	    s += '|';
	}
	TEST_STRINGS_EQUAL(s, "|hi|howdy|");

	s = "|";
	t = db.synonyms_begin("goodbye");
	while (t != db.synonyms_end("goodbye")) {
	    s += *t++;
	    s += '|';
	}
	TEST_STRINGS_EQUAL(s, "|bye|farewell|");

	s = "|";
	t = db.synonym_keys_begin();
	while (t != db.synonym_keys_end()) {
	    s += *t++;
	    s += '|';
	}
	TEST_STRINGS_EQUAL(s, "|goodbye|hello|");

	db.commit();
    }

    // Delete a synonym for "hello" and all synonyms for "goodbye".
    db.remove_synonym("hello", "hi");
    db.clear_synonyms("goodbye");

    // Try these tests twice - once before committing and once after.
    for (int times = 1; times <= 2; ++times) {
	// Test iterators for terms which aren't there.
	TEST(db.synonyms_begin("abc") == db.synonyms_end("abc"));
	TEST(db.synonyms_begin("ghi") == db.synonyms_end("ghi"));
	TEST(db.synonyms_begin("zzzzz") == db.synonyms_end("zzzzz"));

	s = "|";
	t = db.synonyms_begin("hello");
	while (t != db.synonyms_end("hello")) {
	    s += *t++;
	    s += '|';
	}
	TEST_STRINGS_EQUAL(s, "|howdy|");

	TEST(db.synonyms_begin("goodbye") == db.synonyms_end("goodbye"));

	s = "|";
	t = db.synonym_keys_begin();
	while (t != db.synonym_keys_end()) {
	    s += *t++;
	    s += '|';
	}
	TEST_STRINGS_EQUAL(s, "|hello|");

	db.commit();
    }

    Xapian::Database db_multi;
    db_multi.add_database(db);
    db_multi.add_database(get_database("apitest_simpledata"));

    // Test iterators for terms which aren't there.
    TEST(db_multi.synonyms_begin("abc") == db_multi.synonyms_end("abc"));
    TEST(db_multi.synonyms_begin("ghi") == db_multi.synonyms_end("ghi"));
    TEST(db_multi.synonyms_begin("zzzzz") == db_multi.synonyms_end("zzzzz"));

    s = "|";
    t = db_multi.synonyms_begin("hello");
    while (t != db_multi.synonyms_end("hello")) {
	s += *t++;
	s += '|';
    }
    TEST_STRINGS_EQUAL(s, "|howdy|");

    TEST(db_multi.synonyms_begin("goodbye") == db_multi.synonyms_end("goodbye"));

    s = "|";
    t = db_multi.synonym_keys_begin();
    while (t != db_multi.synonym_keys_end()) {
	s += *t++;
	s += '|';
    }
    TEST_STRINGS_EQUAL(s, "|hello|");
}

// Test that adding a document with a really long term gives an error on
// add_document() rather than on commit().
DEFINE_TESTCASE(termtoolong1, writable) {
    // Inmemory doesn't impose a limit.
    SKIP_TEST_FOR_BACKEND("inmemory");

    Xapian::WritableDatabase db = get_writable_database();

    for (size_t i = 246; i <= 290; ++i) {
	tout.str(string());
	tout << "Term length " << i << '\n';
	Xapian::Document doc;
	string term(i, 'X');
	doc.add_term(term);
	try {
	    db.add_document(doc);
	    TEST_AND_EXPLAIN(false, "Expecting exception InvalidArgumentError");
	} catch (const Xapian::InvalidArgumentError &e) {
	    // Check that the max length is correctly expressed in the
	    // exception message - we've got this wrong in two different ways
	    // in the past!
	    tout << e.get_msg() << '\n';
	    TEST(e.get_msg().find("Term too long (> 245)") != string::npos);
	}
    }

    for (size_t j = 240; j <= 245; ++j) {
	tout.str(string());
	tout << "Term length " << j << '\n';
	Xapian::Document doc;
	string term(j, 'X');
	doc.add_term(term);
	db.add_document(doc);
    }

    db.commit();

    size_t limit = endswith(get_dbtype(), "glass") ? 255 : 252;
    {
	// Currently chert and glass escape zero bytes from terms in keys for
	// some tables, so a term with 127 zero bytes won't work for chert, and
	// with 128 zero bytes won't work for glass.
	Xapian::Document doc;
	doc.add_term(string(limit / 2 + 1, '\0'));
	db.add_document(doc);
	try {
	    db.commit();
	    TEST_AND_EXPLAIN(false, "Expecting exception InvalidArgumentError");
	} catch (const Xapian::InvalidArgumentError &e) {
	    // Check that the max length is correctly expressed in the
	    // exception message - we've got this wrong in two different ways
	    // in the past!
	    tout << e.get_msg() << '\n';
	    string target = " is ";
	    target += str(limit);
	    target += " bytes";
	    TEST(e.get_msg().find(target) != string::npos);
	}
    }

    if (get_dbtype().find("chert") != string::npos) {
	XFAIL("Chert fails to clear pending changes after "
	      "InvalidArgumentError - fix too invasive");
    }
    // Try adding a document.  Regression test for a bug fixed in 1.4.15 - in
    // earlier versions the pending changes which caused the
    // InvalidArgumentError were left around and a subsequent commit() on the
    // same WritableDatabase would also fail with InvalidArgumentError.
    Xapian::Document doc;
    doc.add_term("ok");
    db.add_document(doc);
    db.commit();
}

/// Test playing with a postlist
DEFINE_TESTCASE(postlist7, writable) {
    Xapian::WritableDatabase db_w = get_writable_database();

    {
	Xapian::Document doc;
	doc.add_term("foo", 3);
	doc.add_term("zz", 4);
	db_w.replace_document(5, doc);
    }

    Xapian::PostingIterator p;
    p = db_w.postlist_begin("foo");
    TEST(p != db_w.postlist_end("foo"));
    TEST_EQUAL(*p, 5);
    TEST_EQUAL(p.get_wdf(), 3);
    TEST_EQUAL(p.get_doclength(), 7);
    TEST_EQUAL(p.get_unique_terms(), 2);
    ++p;
    TEST(p == db_w.postlist_end("foo"));

    {
	Xapian::Document doc;
	doc.add_term("foo", 1);
	doc.add_term("zz", 1);
	db_w.replace_document(6, doc);
    }

    p = db_w.postlist_begin("foo");
    TEST(p != db_w.postlist_end("foo"));
    TEST_EQUAL(*p, 5);
    TEST_EQUAL(p.get_wdf(), 3);
    TEST_EQUAL(p.get_doclength(), 7);
    TEST_EQUAL(p.get_unique_terms(), 2);
    ++p;
    TEST(p != db_w.postlist_end("foo"));
    TEST_EQUAL(*p, 6);
    TEST_EQUAL(p.get_wdf(), 1);
    TEST_EQUAL(p.get_doclength(), 2);
    TEST_EQUAL(p.get_unique_terms(), 2);
    ++p;
    TEST(p == db_w.postlist_end("foo"));
}

static void
gen_lazytablebug1_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);
    db.commit();

    string synonym(255, 'x');
    char buf[] = " iamafish!!!!!!!!!!";
    for (int i = 33; i < 120; ++i) {
	db.add_synonym(buf, synonym);
	++buf[0];
    }
}

DEFINE_TESTCASE(lazytablebug1, synonyms) {
    Xapian::Database db = get_database("lazytablebug1", gen_lazytablebug1_db);
    for (Xapian::TermIterator t = db.synonym_keys_begin(); t != db.synonym_keys_end(); ++t) {
	tout << *t << '\n';
    }
}

/** Regression test for bug #287 for flint.
 *
 *  Chert also has the same duff code but this testcase doesn't actually
 *  tickle the bug there.
 */
DEFINE_TESTCASE(cursordelbug1, writable && path) {
    static const int terms[] = { 219, 221, 222, 223, 224, 225, 226 };
    static const int copies[] = { 74, 116, 199, 21, 45, 155, 189 };

    Xapian::WritableDatabase db;
    db = get_named_writable_database("cursordelbug1", string());

    for (size_t i = 0; i < sizeof(terms) / sizeof(terms[0]); ++i) {
	Xapian::Document doc;
	doc.add_term("XC" + str(terms[i]));
	doc.add_term("XTabc");
	doc.add_term("XAdef");
	doc.add_term("XRghi");
	doc.add_term("XYabc");
	size_t c = copies[i];
	while (c--) db.add_document(doc);
    }

    db.commit();

    for (size_t i = 0; i < sizeof(terms) / sizeof(terms[0]); ++i) {
	db.delete_document("XC" + str(terms[i]));
    }

    db.commit();

    const string & db_path = get_named_writable_database_path("cursordelbug1");
    TEST_EQUAL(Xapian::Database::check(db_path), 0);
}

/** Helper function for modifyvalues1.
 *
 * Check that the values stored in the database match */
static void
check_vals(const Xapian::Database & db, const map<Xapian::docid, string> & vals)
{
    TEST_EQUAL(db.get_doccount(), vals.size());
    if (vals.empty()) return;
    TEST_REL(vals.rbegin()->first,<=,db.get_lastdocid());
    map<Xapian::docid, string>::const_iterator i;
    for (i = vals.begin(); i != vals.end(); ++i) {
	tout.str(string());
	tout << "Checking value in doc " << i->first << " - should be '" << i->second << "'\n";
	Xapian::Document doc = db.get_document(i->first);
	string dbval = doc.get_value(1);
	TEST_EQUAL(dbval, i->second);
	if (dbval.empty()) {
	    TEST_EQUAL(0, doc.values_count());
	    TEST_EQUAL(doc.values_begin(), doc.values_end());
	} else {
	    TEST_EQUAL(1, doc.values_count());
	    Xapian::ValueIterator valit = doc.values_begin();
	    TEST_NOT_EQUAL(valit, doc.values_end());
	    TEST_EQUAL(dbval, *valit);
	    TEST_EQUAL(1, valit.get_valueno());
	    ++valit;
	    TEST_EQUAL(valit, doc.values_end());
	}
    }
}

/** Regression test for bug in initial streaming values implementation in
 *  chert.
 */
DEFINE_TESTCASE(modifyvalues1, writable) {
    unsigned int seed = 7;
    Xapian::WritableDatabase db = get_writable_database();
    // Note: doccount must be coprime with 13
    const Xapian::doccount doccount = 1000;
    static_assert(doccount % 13 != 0, "doccount divisible by 13");

    map<Xapian::docid, string> vals;

    for (Xapian::doccount num = 1; num <= doccount; ++num) {
	tout.str(string());
	Xapian::Document doc;
	string val = "val" + str(num);
	tout << "Setting val '" << val << "' in doc " << num << "\n";
	doc.add_value(1, val);
	db.add_document(doc);
	vals[num] = val;
    }
    check_vals(db, vals);
    db.commit();
    check_vals(db, vals);

    // Modify one of the values (this is a regression test which failed with
    // the initial implementation of streaming values).
    {
	Xapian::Document doc;
	string val = "newval0";
	tout << "Setting val '" << val << "' in doc 2\n";
	doc.add_value(1, val);
	db.replace_document(2, doc);
	vals[2] = val;
	check_vals(db, vals);
	db.commit();
	check_vals(db, vals);
    }

    // Check that value doesn't get lost when replacing a document with itself.
    {
	tout << "Replacing document 1 with itself\n";
	Xapian::Document doc = db.get_document(1);
	db.replace_document(1, doc);
	check_vals(db, vals);
	db.commit();
	check_vals(db, vals);
    }

    // Check that value doesn't get lost when replacing a document with itself,
    // accessing another document in the meantime.  This is a regression test
    // for a bug in the code which implements lazy updates - this used to
    // forget the values in the document in this situation.
    {
	tout << "Replacing document 1 with itself, after reading doc 2.\n";
	Xapian::Document doc = db.get_document(1);
	db.get_document(2);
	db.replace_document(1, doc);
	check_vals(db, vals);
	db.commit();
	check_vals(db, vals);
    }

    // Do some random modifications: seed random generator, for repeatable
    // results.
    tout << "Setting seed to " << seed << "\n";
    srand(seed);
    for (Xapian::doccount num = 1; num <= doccount * 2; ++num) {
	tout.str(string());
	Xapian::docid did = ((rand() >> 8) % doccount) + 1;
	Xapian::Document doc;
	string val;

	if (num % 5 != 0) {
	    val = "newval" + str(num);
	    tout << "Setting val '" << val << "' in doc " << did << "\n";
	    doc.add_value(1, val);
	} else {
	    tout << "Adding/replacing empty document " << did << "\n";
	}
	db.replace_document(did, doc);
	vals[did] = val;
    }
    check_vals(db, vals);
    db.commit();
    check_vals(db, vals);

    // Delete all the remaining values, in a slightly shuffled order.
    // This is where it's important that doccount is coprime with 13.
    for (Xapian::doccount num = 0; num < doccount * 13; num += 13) {
	tout.str(string());
	Xapian::docid did = (num % doccount) + 1;
	tout << "Clearing val in doc " << did << "\n";
	Xapian::Document doc;
	db.replace_document(did, doc);
	vals[did] = string();
    }
    check_vals(db, vals);
    db.commit();
    check_vals(db, vals);
}

/** Regression test for protocol design bug.
 *
 *  Previously some messages didn't send a reply but could result in an
 *  exception being sent over the link.  That exception would then get
 *  read as a response to the next message instead of its actual response
 *  so we'd be out of step.
 *
 *  This also affected MSG_DELETEDOCUMENTTERM, MSG_CANCEL, MSG_SETMETADATA
 *  and MSG_ADDSPELLING but it's harder to reliably trigger an exception
 *  from any of those.
 *
 *  See #783.  Fixed in 1.4.12.
 */
DEFINE_TESTCASE(protocolbug1, remote && writable) {
    Xapian::WritableDatabase db = get_writable_database("");
    Xapian::Document doc;
    doc.add_term(string(300, 'x'));

    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   db.replace_document(1, doc));
    db.commit();
}

DEFINE_TESTCASE(remotefdleak1, remote && writable) {
    Xapian::WritableDatabase wdb = get_writable_database("");
    TEST_EXCEPTION(Xapian::DatabaseLockError,
		   auto wdb2 = get_writable_database_again());
}
