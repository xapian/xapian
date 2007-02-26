/* api_wrdb.cc: tests which need a writable backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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
#include <string>

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"
#include "api_wrdb.h"

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

    TEST_EQUAL(db.term_exists("one"), false);
    TEST_EQUAL(db.term_exists("two"), false);
    TEST_EQUAL(db.term_exists("three"), false);

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

    TEST_EQUAL(db.term_exists("one"), false);

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

    TEST_EQUAL(db.term_exists("one"), false);
    TEST_EQUAL(db.term_exists("two"), false);
    TEST_EQUAL(db.term_exists("three"), false);

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

    Xapian::PostingIterator p;
    try {
	p = db.postlist_begin("foo");
    } catch (const Xapian::UnimplementedError & e) {
	SKIP_TEST("Database::postlist_begin not implemented");
    }
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 1);
    ++p;
    TEST_NOT_EQUAL(p, db.postlist_end("foo"));
    TEST_EQUAL(*p, 3);
    ++p;
    TEST_EQUAL(p, db.postlist_end("foo"));

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

    Xapian::PostingIterator p;
    try {
	p = db.postlist_begin("foo");
    } catch (const Xapian::UnimplementedError & e) {
	SKIP_TEST("Database::postlist_begin not implemented");
    }
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

    Xapian::PostingIterator p;
    try {
	p = db.postlist_begin("foo");
    } catch (const Xapian::UnimplementedError & e) {
	SKIP_TEST("Database::postlist_begin not implemented");
    }
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
	    try {
		db.replace_document(uterm, doc);
	    } catch (const Xapian::UnimplementedError & e) {
		SKIP_TEST("WritableDatabase::replace_document(TERM) not implemented");
	    }
	}
    }

    TEST_EQUAL(db.get_doccount(), 15);

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
    Xapian::PostingIterator i;
    try {
	i = db.postlist_begin("");
    } catch (const Xapian::UnimplementedError & e) {
	SKIP_TEST("WritableDatabase::replace_document(TERM) not implemented");
    }
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
	try {
	    (void)db.postlist_begin("");
	} catch (const Xapian::UnimplementedError & e) {
	    SKIP_TEST("Database::postlist_begin not implemented");
	}
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

// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which use a writable backend
test_desc writabledb_tests[] = {
    {"adddoc1",		   test_adddoc1},
    {"adddoc2",		   test_adddoc2},
    {"adddoc3",		   test_adddoc3},
    {"implicitendsession1",test_implicitendsession1},
    {"databaseassign1",	   test_databaseassign1},
    {"deldoc1",		   test_deldoc1},
    {"deldoc2",		   test_deldoc2},
    {"deldoc3",		   test_deldoc3},
    {"deldoc4",		   test_deldoc4},
    {"deldoc5",		   test_deldoc5},
    {"replacedoc1",	   test_replacedoc1},
    {"replacedoc2",	   test_replacedoc2},
    {"replacedoc3",	   test_replacedoc3},
    {"replacedoc4",	   test_replacedoc4},
    {"replacedoc5",	   test_replacedoc5},
    {"uniqueterm1",	   test_uniqueterm1},
    {"emptyterm2",	   test_emptyterm2},
    {"phraseorneartoand1", test_phraseorneartoand1},
    {"longpositionlist1",  test_longpositionlist1},
    {"allpostlist2",	   test_allpostlist2},
    {"consistency2",	   test_consistency2},
    {0, 0}
};
