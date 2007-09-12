/* api_wrdb.cc: tests which need a writable backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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
#include <algorithm>
#include <map>
#include <string>

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"

#include <cmath>
#include <list>

using namespace std;

// #######################################################################
// # Tests start here

// test that indexing a term more than once at the same position increases
// the wdf
static bool test_adddoc1()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    return true;
}

// test that removing a posting and removing a term works
static bool test_adddoc2()
{
    Xapian::WritableDatabase db = get_writable_database("");

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
    //TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "foo");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 4);
    TEST_EQUAL(iter2.get_wdf(), 4);
    //TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "fooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    // assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

    iter1++;
    iter2++;
    TEST(iter1 != doc1.termlist_end());
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter1, "gone");
    TEST_EQUAL(*iter2, *iter1);
    TEST_EQUAL(iter1.get_wdf(), 1);
    TEST_EQUAL(iter2.get_wdf(), 1);
    // assertion fails in debug build! TEST_EQUAL(iter1.get_termfreq(), 0);
    TEST_EQUAL(iter2.get_termfreq(), 1);

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
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bar");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "bat");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
    iter2++;
    TEST(iter2 != doc2.termlist_end());
    TEST_EQUAL(*iter2, "foo");
    //TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter1.get_termfreq(), 1);
    //TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter1.get_termfreq(), 2);
    //TEST_EQUAL(iter2.get_termfreq(), 0);

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
    TEST_EQUAL(iter1.get_termfreq(), 1);
    //TEST_EQUAL(iter2.get_termfreq(), 0);
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
    TEST_EQUAL(iter1.get_termfreq(), 2);
    //TEST_EQUAL(iter2.get_termfreq(), 0);

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

    return true;
}

// test that adding lots of documents works, and doesn't leak memory
// REGRESSION FIXED:2003-09-07
static bool test_adddoc3()
{
    Xapian::WritableDatabase db = get_writable_database("");

    for (Xapian::doccount i = 0; i < 2100; ++i) {
	Xapian::Document doc;
	for (Xapian::termcount t = 0; t < 100; ++t) {
	    string term("foo");
	    term += char(t ^ 70 ^ i);
	    doc.add_posting(term, t);
	}
	db.add_document(doc);
    }
    return true;
}

// We want to test that a termlist starting with a 48 character long term works
// OK since this value currently requires special handling in flint for
// historical reasons!)  Also test all other term lengths while we're at it.
static bool test_adddoc4()
{
    Xapian::WritableDatabase db = get_writable_database("");

    for (Xapian::doccount i = 1; i <= 240; ++i) {
	Xapian::Document doc;
	string term(i, 'X');
	doc.add_term(term);
	db.add_document(doc);
    }
    db.add_document(Xapian::Document());
    db.flush();

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

    return true;
}

// tests that database destructors flush if it isn't done explicitly
static bool test_implicitendsession1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc;

    doc.set_data(string("top secret"));
    doc.add_posting("cia", 1);
    doc.add_posting("nsa", 2);
    doc.add_posting("fbi", 3);
    db.add_document(doc);

    return true;
}

// tests that assignment of Xapian::Database and Xapian::WritableDatabase work as expected
static bool test_databaseassign1()
{
    Xapian::WritableDatabase wdb = get_writable_database("");
    Xapian::Database db = get_database("");
    Xapian::Database actually_wdb = wdb;
    Xapian::WritableDatabase w1(wdb);
    w1 = wdb;
    Xapian::Database d1(wdb);
    Xapian::Database d2(actually_wdb);
    d2 = wdb;
    d2 = actually_wdb;
    wdb = wdb; // check assign to itself works
    db = db; // check assign to itself works
    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc1()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    return true;
}

// tests that deletion and updating of documents works as expected
static bool test_deldoc2()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    db.flush();

    db.reopen();

    db.delete_document(1);
    db.delete_document(2);
    db.delete_document(3);

    db.flush();

    db.reopen();

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

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(3));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// another test of deletion of documents, a cut-down version of deldoc2
static bool test_deldoc3()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("one", 1);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);

    db.flush();

    db.reopen();

    db.delete_document(1);

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(1));
    (void)&db; // gcc 2.95 seems to miscompile without this!!! - Olly
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(2));

    // test positionlist_{begin,end}?

    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_termfreq("one"), 0);

    TEST(!db.term_exists("one"));

    TEST_EQUAL(db.get_collection_freq("one"), 0);

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(2));

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    TEST_EQUAL(db.allterms_begin(), db.allterms_end());

    return true;
}

// tests that deletion and updating of (lots of) documents works as expected
static bool test_deldoc4()
{
    Xapian::WritableDatabase db = get_writable_database("");

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
	    db.flush();
	    db.reopen();
	}
    }
    db.flush();
    db.reopen();

    /* delete the documents in a peculiar order */
    for (Xapian::docid i = 0; i < maxdoc / 3; ++i) {
	db.delete_document(maxdoc - i);
	db.delete_document(maxdoc / 3 + i + 1);
	db.delete_document(i + 1);
    }

    db.flush();

    db.reopen();

    TEST_EQUAL(db.postlist_begin("one"), db.postlist_end("one"));
    TEST_EQUAL(db.postlist_begin("two"), db.postlist_end("two"));
    TEST_EQUAL(db.postlist_begin("three"), db.postlist_end("three"));

    for (Xapian::docid i = 1; i <= maxdoc; ++i) {
	// TEST_EXCEPTION writes to tout each time if the test is run
	// in verbose mode and some string stream implementations get
	// very inefficient with large strings, so clear tout on each pass of
	// the loop to speed up the test since the older information isn't
	// interesting anyway.
	tout.str("");
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.termlist_begin(i));
	TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(i));
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

    return true;
}

// Test deleting a document which was added in the same batch.
static bool test_deldoc5()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    db.flush();

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

    return true;
}

// Regression test for bug in quartz and flint, fixed in 1.0.2.
static bool test_deldoc6()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    db.flush();

    db.delete_document(2);
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.delete_document(3));

    db.flush();

    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_document(2));

    return true;
}

static bool test_replacedoc1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone",3);
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
    Xapian::TermIterator tIter = doc3.termlist_begin();
    TEST_EQUAL(*tIter, "bar");
    Xapian::PositionIterator pIter = tIter.positionlist_begin();
    TEST_EQUAL(*pIter, 4);
    ++tIter;
    TEST_EQUAL(*tIter, "foo");
    Xapian::PositionIterator qIter = tIter.positionlist_begin();
    TEST_EQUAL(*qIter, 1);
    ++qIter;
    TEST_EQUAL(*qIter, 5);
    ++tIter;
    TEST_EQUAL(*tIter, "pipco");
    Xapian::PositionIterator rIter = tIter.positionlist_begin();
    TEST_EQUAL(*rIter, 2);
    ++tIter;
    TEST_EQUAL(tIter, doc3.termlist_end());
    return true;
}

// Test of new feature: WritableDatabase::replace_document accepts a docid
// which doesn't yet exist as of Xapian 0.8.2.
static bool test_replacedoc2()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;

    doc1.add_posting("foo", 1);
    doc1.add_posting("foo", 2);
    doc1.add_posting("gone",3);
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
    Xapian::TermIterator tIter = doc3.termlist_begin();
    TEST_EQUAL(*tIter, "bar");
    Xapian::PositionIterator pIter = tIter.positionlist_begin();
    TEST_EQUAL(*pIter, 4);
    ++tIter;
    TEST_EQUAL(*tIter, "foo");
    Xapian::PositionIterator qIter = tIter.positionlist_begin();
    TEST_EQUAL(*qIter, 1);
    ++qIter;
    TEST_EQUAL(*qIter, 5);
    ++tIter;
    TEST_EQUAL(*tIter, "pipco");
    Xapian::PositionIterator rIter = tIter.positionlist_begin();
    TEST_EQUAL(*rIter, 2);
    ++tIter;
    TEST_EQUAL(tIter, doc3.termlist_end());

    did = db.add_document(doc1);
    TEST_EQUAL(did, 31771);
    TEST_EQUAL(db.get_doccount(), 2);

    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.replace_document(0, doc2));

    return true;
}

// Test replacing a document which was added in the same batch.
static bool test_replacedoc3()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    db.flush();

    // Check that the document exists (no DocNotFoundError).
    doc2 = db.get_document(2);

    TEST_EQUAL(db.get_termfreq("foo"), 2);
    TEST_EQUAL(db.get_termfreq("aardvark"), 2);
    TEST_EQUAL(db.get_termfreq("hello"), 1);
    TEST_EQUAL(db.get_termfreq("world"), 2);

    TEST_EQUAL(db.get_doclength(1), 3);
    TEST_EQUAL(db.get_doclength(2), 1);
    TEST_EQUAL(db.get_doclength(3), 4);

    Xapian::PostingIterator p = db.postlist_begin("foo");
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 1);
    TEST_EQUAL(p.get_doclength(), 3);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 3);
    TEST_EQUAL(p.get_doclength(), 4);
    ++p;
    TEST_EQUAL(p, db.postlist_end("foo"));

    p = db.postlist_begin("world");
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 2);
    TEST_EQUAL(p.get_doclength(), 1);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("world"));
    TEST_EQUAL(*p, 3);
    TEST_EQUAL(p.get_doclength(), 4);
    ++p;
    TEST_EQUAL(p, db.postlist_end("world"));

    return true;
}

// Test replacing a document which was deleted in the same batch.
static bool test_replacedoc4()
{
    Xapian::WritableDatabase db = get_writable_database("");

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

    db.flush();

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

    return true;
}

// Test replacing a document with itself without modifying postings.
// Regression test for bug in 0.9.9 and earlier - there flint and quartz
// lose all positional information for the document when you do this.
static bool test_replacedoc5()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc1;
    doc1.add_posting("hello", 1);
    doc1.add_posting("world", 2);

    Xapian::docid did = db.add_document(doc1);
    TEST_EQUAL(did, 1);
    db.flush();

    Xapian::Document doc2 = db.get_document(1);
    TEST(db.has_positions());
    TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
    TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));
    db.replace_document(1, doc2);
    db.flush();

    TEST(db.has_positions());
    TEST(db.positionlist_begin(1, "hello") != db.positionlist_end(1, "hello"));
    TEST(db.positionlist_begin(1, "world") != db.positionlist_end(1, "world"));

    return true;
}

// Test of new feature: WritableDatabase::replace_document and delete_document
// can take a unique termname instead of a document id as of Xapian 0.8.2.
static bool test_uniqueterm1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    for (int n = 1; n <= 20; ++n) {
	Xapian::Document doc;
	string uterm = "U" + om_tostring(n % 16);
	doc.add_term(uterm);
	doc.add_term(om_tostring(n));
	doc.add_term(om_tostring(n ^ 9));
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
	string uterm = "U" + om_tostring(n % 16);
	if (uterm == "U2") {
	    db.delete_document(uterm);
	} else {
	    Xapian::Document doc;
	    doc.add_term(uterm);
	    doc.add_term(om_tostring(n));
	    doc.add_term(om_tostring(n ^ 9));
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

    return true;
}

// tests all document postlists
static bool test_allpostlist2()
{
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

    return true;
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
static bool test_emptyterm2()
{
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
	Xapian::WritableDatabase db(get_writable_database(""));
	TEST_EQUAL(db.get_doccount(), 0);
	test_emptyterm2_helper(db);
    }

    return true;
}

// Check that PHRASE/NEAR becomes AND if there's no positional info in the
// database.
static bool test_phraseorneartoand1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    for (int n = 1; n <= 20; ++n) {
	Xapian::Document doc;
	doc.add_term(om_tostring(n));
	doc.add_term(om_tostring(n ^ 9));
	doc.add_term("all");
	doc.set_data("pass1");
	db.add_document(doc);
    }
    db.flush();

    Xapian::Enquire enquire(db);
    Xapian::MSet mymset;

    const char * q1[] = { "all", "1" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_PHRASE, q1, q1 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(2, mymset.size());

    enquire.set_query(Xapian::Query(Xapian::Query::OP_NEAR, q1, q1 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(2, mymset.size());

    const char * q2[] = { "1", "2" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_PHRASE, q2, q2 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(0, mymset.size());

    enquire.set_query(Xapian::Query(Xapian::Query::OP_NEAR, q2, q2 + 2));
    mymset = enquire.get_mset(0, 10);
    TEST_EQUAL(0, mymset.size());

    return true;
}

// Check that a large number of position list entries for a particular term
// works - regression test for flint.
static bool test_longpositionlist1()
{
    Xapian::WritableDatabase db = get_writable_database("");

    Xapian::Document doc;
    Xapian::termpos n;
    for (n = 1; n <= 2000; ++n) {
	doc.add_posting("fork", n * 3);
	doc.add_posting("knife", n * unsigned(log(double(n + 2))));
	doc.add_posting("spoon", n * n);
    }
    doc.set_data("cutlery");
    Xapian::docid did = db.add_document(doc);
    db.flush();

    doc = db.get_document(did);

    Xapian::TermIterator t, tend;
    Xapian::PositionIterator p, pend;

    t = doc.termlist_begin();
    tend = doc.termlist_end();

    TEST(t != tend);
    TEST_EQUAL(*t, "fork");
    p = t.positionlist_begin();
    pend = t.positionlist_end();
    for (n = 1; n <= 2000; ++n) {
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
    for (n = 1; n <= 2000; ++n) {
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
    for (n = 1; n <= 2000; ++n) {
	TEST(p != pend);
	TEST_EQUAL(*p, n * n);
	++p;
    }
    TEST(p == pend);

    ++t;
    TEST(t == tend);

    return true;
}

// Regression test for bug#110: Inconsistent sort order between pages with
// set_sort_by_value_then_relevance.
bool test_consistency2()
{
    Xapian::WritableDatabase db = get_writable_database("");
    char buf[2] = "X";
    int i = 0;

    // Add 5 documents indexed by "test" with wdf 1.
    for (i = 0; i < 5; ++i) {
	Xapian::Document doc;
	*buf = '0' + i;
	doc.add_value(0, buf);
	doc.add_term("test");
	db.add_document(doc);
    }

    // Add 5 documents indexed by "test" with wdf 2.
    for (i = 0; i < 5; ++i) {
	Xapian::Document doc;
	*buf = '0' + i;
	doc.add_value(0, buf);
	doc.add_term("test", 2);
	db.add_document(doc);
    }

    db.flush();

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
    for (i = 0; i < 8; ++i) {
	TEST_EQUAL(*mset1[i + 2], *mset2c[i]);
    }

    return true;
}

static bool test_crashrecovery1()
{
    const string & dbtype = get_dbtype();
    string path, base_ext;
    if (dbtype == "flint") {
	path = ".flint/dbw";
	base_ext = ".baseB";
    } else if (dbtype == "quartz") {
	path = ".quartz/dbw";
	base_ext = "_baseB";
    } else {
	SKIP_TEST("Test only supported for flint and quartz backends");
    }

    Xapian::Document doc;
    {
	Xapian::WritableDatabase db = get_writable_database("");
	Xapian::Database dbr(path);
	TEST_EQUAL(dbr.get_doccount(), 0);

	// Xapian::Database has full set of baseA, no baseB

	db.add_document(doc);
	db.flush();
	dbr.reopen();
	TEST_EQUAL(dbr.get_doccount(), 1);

	// Xapian::Database has full set of baseB, old baseA

	db.add_document(doc);
	db.flush();
	dbr.reopen();
	TEST_EQUAL(dbr.get_doccount(), 2);

	// Xapian::Database has full set of baseA, old baseB

	// Simulate a transaction starting, some of the baseB getting removed,
	// but then the transaction fails.
	unlink(path + "/record" + base_ext);
	unlink(path + "/termlist" + base_ext);

	dbr.reopen();
	TEST_EQUAL(dbr.get_doccount(), 2);
    }

    Xapian::WritableDatabase db(path, Xapian::DB_OPEN);
    // Xapian::Database has full set of baseA, some old baseB
    Xapian::Database dbr = Xapian::Database(path);

    db.add_document(doc);
    db.flush();
    dbr.reopen();
    TEST_EQUAL(dbr.get_doccount(), 3);

    // Xapian::Database has full set of baseB, old baseA

    db.add_document(doc);
    db.flush();
    dbr.reopen();
    TEST_EQUAL(dbr.get_doccount(), 4);

    return true;
}

// Check that DatabaseError is thrown if the docid counter would wrap.
// Regression test for bug#152.
static bool test_nomoredocids1()
{
    if (get_dbtype() == "inmemory") {
	// The InMemory backend uses a vector for the documents, so trying
	// to add document "-1" will fail because we can't allocate enough
	// memory!
	SKIP_TEST("Test not supported on inmemory backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
    Xapian::Document doc;
    doc.set_data("prose");
    doc.add_term("word");

    db.replace_document(Xapian::docid(-1), doc);

    TEST_EXCEPTION(Xapian::DatabaseError, db.add_document(doc));

    return true;
}

// Test basic spelling correction features.
static bool test_spell1()
{
    string dbpath;
    if (get_dbtype() == "flint") {
	dbpath = ".flint/dbw";
    } else {
	SKIP_TEST("Test only supported for flint backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");

    // Check that the more frequent term is chosen.
    db.add_spelling("hello");
    db.add_spelling("cell", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    db.flush();
    Xapian::Database dbr(dbpath);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(dbr.get_spelling_suggestion("hell"), "cell");

    // Check suggestions for single edit errors to "zig".
    db.add_spelling("zig");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("izg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zgi"), "zig");
    // Substitutions:
    TEST_EQUAL(db.get_spelling_suggestion("sig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zog"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zif"), "zig");
    // Deletions:
    TEST_EQUAL(db.get_spelling_suggestion("ig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zi"), "zig");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("azig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zaig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziag"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziga"), "zig");

    // Check suggestions for single edit errors to "ch".
    db.add_spelling("ch");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("hc"), "ch");
    // Substitutions - we don't handle these for two character words:
    TEST_EQUAL(db.get_spelling_suggestion("qh"), "");
    TEST_EQUAL(db.get_spelling_suggestion("cq"), "");
    // Deletions would leave a single character, and we don't handle those.
    TEST_EQUAL(db.get_spelling_suggestion("c"), "");
    TEST_EQUAL(db.get_spelling_suggestion("h"), "");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("qch"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("cqh"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("chq"), "ch");

    // Check assorted cases:
    TEST_EQUAL(db.get_spelling_suggestion("shello"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("hellot"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("acell"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("acella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helo"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helol"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("clel"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("ecll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");

    // Check that edit distance 3 isn't found by default:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx"), "");
    TEST_EQUAL(db.get_spelling_suggestion("celling"), "");
    TEST_EQUAL(db.get_spelling_suggestion("dellin"), "");

    // Check that edit distance 3 is found if specified:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx", 3), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("celling", 3), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("dellin", 3), "cell");
    return true;
}

// Test spelling correction for Unicode.
static bool test_spell2()
{
    string dbpath;
    if (get_dbtype() == "flint") {
	dbpath = ".flint/dbw";
    } else {
	SKIP_TEST("Test only supported for flint backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");

    // Check that a UTF-8 sequence counts as a single character.
    db.add_spelling("h\xc3\xb6hle");
    db.add_spelling("ascii");
    TEST_EQUAL(db.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(db.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");
    db.flush();
    Xapian::Database dbr(dbpath);
    TEST_EQUAL(dbr.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(dbr.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");

    return true;
}

// Test spelling correction with multi databases
static bool test_spell3()
{
    if (get_dbtype() != "flint") {
	SKIP_TEST("Test only supported for flint backend");
    }

#ifdef __WIN32__
    SKIP_TEST("Test not supported on windows");
#endif
    // FIXME: the following two lines create two writable databases, but at the
    // same path.  The first database is deleted, but kept open, before the
    // second is opened.  This isn't really supported behaviour (in Olly's
    // words: "it'll end in tears if it tries to flush"), and doesn't work on
    // windows (hence the SKIP_TEST for __WIN32__ above) so we need to fix
    // this.  This probably involves making the backendmanager support a "name"
    // parameter for get_writable_database() which specifies the path to create
    // the database at.  This would also make it easier to get a readonly
    // handle on a previously created writable database.
    Xapian::WritableDatabase db1 = get_writable_database("");
    Xapian::WritableDatabase db2 = get_writable_database("");

    db1.add_spelling("hello");
    db1.add_spelling("cell", 2);
    db2.add_spelling("hello", 2);
    db2.add_spelling("helo");

    Xapian::Database db;
    db.add_database(db1);
    db.add_database(db2);

    TEST_EQUAL(db.get_spelling_suggestion("hello"), "");
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "hello");
    TEST_EQUAL(db1.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(db2.get_spelling_suggestion("hell"), "hello");


    // Test spelling iterator
    Xapian::TermIterator i(db1.spellings_begin());
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db1.spellings_end());

    i = db2.spellings_begin();
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db2.spellings_end());

    i = db.spellings_begin();
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 3);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db.spellings_end());

    return true;
}

// Regression test - check that appending works correctly.
static bool test_spell4()
{
    if (get_dbtype() != "flint") {
	SKIP_TEST("Test only supported for flint backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");

    db.add_spelling("check");
    db.add_spelling("pecks", 2);
    db.flush();
    db.add_spelling("becky");
    db.flush();

    TEST_EQUAL(db.get_spelling_suggestion("jeck", 2), "pecks");

    return true;
}

// Regression test - used to segfault with some input values.
static bool test_spell5()
{
    if (get_dbtype() != "flint") {
	SKIP_TEST("Test only supported for flint backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");

    db.add_spelling("一些");
    db.flush();

    TEST_EQUAL(db.get_spelling_suggestion("不", strlen("不")), "一些");

    return true;
}

// Test synonym iterators.
static bool test_synonymitor1()
{
    if (get_dbtype() != "flint") {
	SKIP_TEST("Test only supported for flint backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");

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

    // Try these tests twice - once before flushing and once after.
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

	db.flush();
    }

    // Delete a synonym for "hello" and all synonyms for "goodbye".
    db.remove_synonym("hello", "hi");
    db.clear_synonyms("goodbye");

    // Try these tests twice - once before flushing and once after.
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

	db.flush();
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

    return true;
}

static bool test_matchspy1()
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + om_tostring(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + om_tostring(factor));
		++factors;
	    }
	}

	// Number of factors.
	doc.add_value(0, om_tostring(factors));
	// Units digits.
	doc.add_value(1, om_tostring(c % 10));
	// Constant.
	doc.add_value(2, "fish");
	// Number of digits.
	doc.add_value(3, om_tostring(om_tostring(c).size()));

	db.add_document(doc);
    }

    Xapian::CategorySelectMatchSpy spy;

    spy.add_slot(0);
    spy.add_slot(1);
    spy.add_slot(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    Xapian::MSet mset = enq.get_mset(0, 10, 0, NULL, NULL, &spy);

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
	string str("|");
	map<string, Xapian::doccount>::const_iterator i;
	for (i = cat.begin(); i != cat.end(); ++i) {
	    str += i->first;
	    str += ':';
	    str += om_tostring(i->second);
	    str += '|';
	}
	tout << "value " << v << endl;
	TEST_STRINGS_EQUAL(str, results[v]);
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

static bool test_matchspy2()
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + om_tostring(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + om_tostring(factor));
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

    Xapian::MSet mset = enq.get_mset(0, 10, 0, NULL, NULL, &spy);

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
	string str("|");
	map<string, Xapian::doccount>::const_iterator i;
	for (i = cat.begin(); i != cat.end(); ++i) {
	    if (i->first.size() > 9) {
		double start = Xapian::sortable_unserialise((i->first).substr(0, 9));
		double end = Xapian::sortable_unserialise((i->first).substr(9));
		start = floor(start * 100);
		end = floor(end * 100);
		str += om_tostring(start);
		str += "..";
		str += om_tostring(end);
	    } else {
		double start = Xapian::sortable_unserialise((i->first).substr(0, 9));
		start = floor(start * 100);
		str += om_tostring(start);
	    }
	    str += ':';
	    str += om_tostring(i->second);
	    str += '|';
	}
	tout << "value " << v << endl;
	TEST_STRINGS_EQUAL(str, results[v]);
    }

    return true;
}

static bool test_matchspy3()
{
    if (get_dbtype() == "remotetcp" || get_dbtype() == "remoteprog") {
	SKIP_TEST("Test not supported for remote backend");
    }

    Xapian::WritableDatabase db = get_writable_database("");
    for (int c = 1; c <= 25; ++c) {
	Xapian::Document doc;
	doc.set_data("Document " + om_tostring(c));
	int factors = 0;
	for (int factor = 1; factor <= c; ++factor) {
	    doc.add_term("all");
	    if (c % factor == 0) {
		doc.add_term("XFACT" + om_tostring(factor));
		++factors;
	    }
	}

	// Number of factors.
	doc.add_value(0, om_tostring(factors));
	// Units digits.
	doc.add_value(1, om_tostring(c % 10));
	// Constant.
	doc.add_value(2, "fish");
	// Number of digits.
	doc.add_value(3, om_tostring(om_tostring(c).size()));

	db.add_document(doc);
    }
    
    Xapian::ValueCountMatchSpy spy;

    spy.add_slot(0);
    spy.add_slot(1);
    spy.add_slot(3);

    Xapian::Enquire enq(db);

    enq.set_query(Xapian::Query("all"));

    Xapian::MSet mset = enq.get_mset(0, 10, 0, NULL, NULL, &spy);

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
	    allvals_str += om_tostring(allvals[i].frequency);
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

// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which use a writable backend
test_desc writabledb_tests[] = {
    TESTCASE(adddoc1),
    TESTCASE(adddoc2),
    TESTCASE(adddoc3),
    TESTCASE(adddoc4),
    TESTCASE(implicitendsession1),
    TESTCASE(databaseassign1),
    TESTCASE(deldoc1),
    TESTCASE(deldoc2),
    TESTCASE(deldoc3),
    TESTCASE(deldoc4),
    TESTCASE(deldoc5),
    TESTCASE(deldoc6),
    TESTCASE(replacedoc1),
    TESTCASE(replacedoc2),
    TESTCASE(replacedoc3),
    TESTCASE(replacedoc4),
    TESTCASE(replacedoc5),
    TESTCASE(uniqueterm1),
    TESTCASE(emptyterm2),
    TESTCASE(phraseorneartoand1),
    TESTCASE(longpositionlist1),
    TESTCASE(allpostlist2),
    TESTCASE(consistency2),
    TESTCASE(crashrecovery1),
    TESTCASE(nomoredocids1),
    TESTCASE(spell1),
    TESTCASE(spell2),
    TESTCASE(spell3),
    TESTCASE(spell4),
    TESTCASE(spell5),
    TESTCASE(synonymitor1),
    TESTCASE(matchspy1),
    TESTCASE(matchspy2),
    TESTCASE(matchspy3),
    END_OF_TESTCASES
};
