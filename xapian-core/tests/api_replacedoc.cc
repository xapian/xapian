/* api_replacedoc.cc: tests of document replacing.
 *
 * Copyright 2009 Richard Boulton
 * Copyright 2015,2016 Olly Betts
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

#include "api_replacedoc.h"

#include <string>
#include <map>

using namespace std;

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"
#include "dbcheck.h"

// Test that positionlists are updated correctly.
DEFINE_TESTCASE(poslistupdate1, positional && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    doc.add_posting("pos", 2);
    doc.add_posting("pos", 3);
    db.add_document(doc);
    db.commit();

    TEST_EQUAL(docterms_to_string(db, 1), "Term(pos, wdf=2, pos=[2, 3])");

    doc = db.get_document(1);
    doc.add_term("pos2");
    db.replace_document(1, doc);
    db.commit();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=2, pos=[2, 3]), "
	       "Term(pos2, wdf=1)");

    doc = db.get_document(1);
    doc.add_posting("pos3", 1);
    doc.add_posting("pos3", 5);
    db.replace_document(1, doc);
    db.commit();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=2, pos=[2, 3]), "
	       "Term(pos2, wdf=1), "
	       "Term(pos3, wdf=2, pos=[1, 5])");

    doc = db.get_document(1);
    doc.remove_term("pos");
    db.replace_document(1, doc);
    db.commit();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos2, wdf=1), "
	       "Term(pos3, wdf=2, pos=[1, 5])");

    // Regression test: the old positionlist fragment used to be left lying
    // around here.
    Xapian::PositionIterator posit(db.positionlist_begin(1, "pos"));
    TEST(posit == db.positionlist_end(1, "pos"));

    doc = db.get_document(1);
    doc.remove_term("pos3");
    db.replace_document(1, doc);
    db.commit();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos2, wdf=1)");

    // Regression test: the old positionlist fragment used to be left lying
    // around here.
    Xapian::PositionIterator posit2(db.positionlist_begin(1, "pos3"));
    TEST(posit2 == db.positionlist_end(1, "pos3"));

    doc = db.get_document(1);
    doc.add_term("pos");
    db.replace_document(1, doc);
    db.commit();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=1), "
	       "Term(pos2, wdf=1)");

    return true;
}

static Xapian::Document
basic_doc() {
    Xapian::Document doc;
    doc.add_term("z0", 0);
    doc.add_term("z1", 1);
    return doc;
}

static string
basic_docterms() {
    return ", Term(z0, wdf=0), Term(z1, wdf=1)";
}

/** Check that changing the wdf of a term in a document works.
 */
DEFINE_TESTCASE(modtermwdf1, writable) {
    Xapian::WritableDatabase db(get_writable_database());

    string bdt(basic_docterms());

    // Add a simple document.
    Xapian::Document doc1(basic_doc());
    doc1.add_term("takeaway", 1);
    db.add_document(doc1);

    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=1)" + bdt);

    // Modify the wdf of an existing document, checking stats before commit.
    Xapian::Document doc2(basic_doc());
    doc2.add_term("takeaway", 2);
    db.replace_document(1, doc2);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)" + bdt);

    // Remove a term, and then put it back again.
    Xapian::Document doc0(basic_doc());
    db.replace_document(1, doc0);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), bdt.substr(2));
    db.replace_document(1, doc1);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=1)" + bdt);

    // Remove a term, commit, then put it back, remove it, and put it back.
    // This is to test the handling of items in the change cache.
    db.replace_document(1, doc0);
    db.commit();
    db.replace_document(1, doc2);
    db.replace_document(1, doc0);
    db.replace_document(1, doc2);
    db.commit();
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)" + bdt);

    // Remove a term, and then put it back again without checking stats.
    db.replace_document(1, doc0);
    db.replace_document(1, doc2);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)" + bdt);

    // Modify a term, and then put it back again without checking stats.
    db.replace_document(1, doc1);
    db.replace_document(1, doc2);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)" + bdt);

    // Modify the wdf of an existing document, checking stats after commit.
    Xapian::Document doc3(basic_doc());
    doc3.add_term("takeaway", 3);
    db.replace_document(1, doc3);
    db.commit();
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)" + bdt);

    // Change a document, without changing its length.
    Xapian::Document doc3_diff(basic_doc());
    doc3_diff.add_term("takeaways", 3);
    db.replace_document(1, doc3_diff);
    db.commit();
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaways, wdf=3)" + bdt);

    // Put it back.
    db.replace_document(1, doc3);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)" + bdt);

    // Modify a document taken from the database.
    Xapian::Document doc4(db.get_document(1));
    Xapian::Document doc3a(db.get_document(1)); // need this one later
    doc3a.termlist_count(); // Pull the document termlist into memory.
    doc4.add_term("takeaway", 1);
    db.replace_document(1, doc4);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=4)" + bdt);

    // Add a document which was previously added and then modified.
    doc1.add_term("takeaway", 1);
    db.replace_document(1, doc1);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)" + bdt);

    // Add back a document which was taken from the database, but never modified.
    db.replace_document(1, doc3a);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)" + bdt);

    // Add a position to the document.
    Xapian::Document doc5(db.get_document(1));
    doc5.add_posting("takeaway", 1, 2);
    db.replace_document(1, doc5);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[1])" + bdt);

    // Add a position without changing the wdf.
    Xapian::Document doc5b(db.get_document(1));
    doc5b.add_posting("takeaway", 2, 0);
    db.replace_document(1, doc5b);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[1, 2])" + bdt);

    // Delete a position without changing the wdf.
    Xapian::Document doc5c(basic_doc());
    doc5c.add_posting("takeaway", 2, 5);
    db.replace_document(1, doc5c);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[2])" + bdt);

    // Delete the other position without changing the wdf.
    Xapian::Document doc5d(basic_doc());
    doc5d.add_term("takeaway", 5);
    db.replace_document(1, doc5d);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5)" + bdt);

    // Reduce the wdf to 0, but don't delete the term.
    Xapian::Document doc0freq(basic_doc());
    doc0freq.add_term("takeaway", 0);
    db.replace_document(1, doc0freq);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=0)" + bdt);

    // Reduce the wdf to 0, and delete the term.
    db.replace_document(1, doc0);
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), bdt.substr(2));

    // Delete the document.
    db.delete_document(1);
    dbcheck(db, 0, 1);
    TEST_EXCEPTION(Xapian::DocNotFoundError, docterms_to_string(db, 1));
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "");
    TEST_EXCEPTION(Xapian::DocNotFoundError, docstats_to_string(db, 1));
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=0,cf=0");
    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_avlength(), 0);
    TEST_EQUAL(db.get_lastdocid(), 1);

    return true;
}
