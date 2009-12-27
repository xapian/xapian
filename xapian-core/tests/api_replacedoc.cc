/* api_replacedoc.cc: tests of document replacing.
 *
 * Copyright 2009 Richard Boulton
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

using namespace std;

#include <xapian.h>
#include "str.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

static string
positions_to_string(Xapian::PositionIterator & it,
		    const Xapian::PositionIterator & end) {
    string result;
    bool need_comma = false;
    while (it != end) {
	if (need_comma)
	    result += ", ";
	result += str(*it);
	need_comma = true;
	++it;
    }
    return result;
}

static string
postlist_to_string(const Xapian::Database & db, const string & tname) {
    string result;
    bool need_comma = false;

    for (Xapian::PostingIterator p = db.postlist_begin(tname);
	 p != db.postlist_end(tname);
	 ++p) {
	if (need_comma)
	    result += ", ";

	Xapian::PositionIterator it(p.positionlist_begin());
	string posrepr = positions_to_string(it, p.positionlist_end());
	if (!posrepr.empty()) {
	    posrepr = ", pos=[" + posrepr + "]";
	}

	result += "(" + str(*p) +
		", doclen=" + str(p.get_doclength()) +
		", wdf=" + str(p.get_wdf()) +
		posrepr + ")";
	need_comma = true;
    }
    return result;
}

static string
docterms_to_string(const Xapian::Database & db, Xapian::docid did) {
    string result;
    bool need_comma = false;

    for (Xapian::TermIterator t = db.termlist_begin(did);
	 t != db.termlist_end(did);
	 ++t) {
	Xapian::PositionIterator it(t.positionlist_begin());
	string posrepr = positions_to_string(it, t.positionlist_end());
	if (!posrepr.empty()) {
	    posrepr = ", pos=[" + posrepr + "]";
	}
	if (need_comma)
	    result += ", ";
	result += "Term(" + *t + ", wdf=" + str(t.get_wdf()) + posrepr + ")";
	need_comma = true;
    }
    return result;
}

static string
docstats_to_string(const Xapian::Database & db, Xapian::docid did) {
    string result;

    result += "len=" + str(db.get_doclength(did));

    return result;
}

static string
termstats_to_string(const Xapian::Database & db, const string & term) {
    string result;

    result += "tf=" + str(db.get_termfreq(term));
    result += ",cf=" + str(db.get_collection_freq(term));

    return result;
}

static string
dbstats_to_string(const Xapian::Database & db) {
    string result;

    result += "dc=" + str(db.get_doccount());
    result += ",al=" + str(db.get_avlength());
    result += ",ld=" + str(db.get_lastdocid());

    return result;
}

// Test that positionlists are updated correctly.
DEFINE_TESTCASE(poslistupdate1, positional && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    doc.add_posting("pos", 2);
    doc.add_posting("pos", 3);
    db.add_document(doc);
    db.flush();

    TEST_EQUAL(docterms_to_string(db, 1), "Term(pos, wdf=2, pos=[2, 3])");

    doc = db.get_document(1);
    doc.add_term("pos2");
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=2, pos=[2, 3]), "
	       "Term(pos2, wdf=1)");

    doc = db.get_document(1);
    doc.add_posting("pos3", 1);
    doc.add_posting("pos3", 5);
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=2, pos=[2, 3]), "
	       "Term(pos2, wdf=1), "
	       "Term(pos3, wdf=2, pos=[1, 5])");

    doc = db.get_document(1);
    doc.remove_term("pos");
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos2, wdf=1), "
	       "Term(pos3, wdf=2, pos=[1, 5])");

    // Regression test: the old positionlist fragment used to be left lying
    // around here.
    Xapian::PositionIterator posit(db.positionlist_begin(1, "pos"));
    string posrepr = positions_to_string(posit, db.positionlist_end(1, "pos"));
    TEST_EQUAL(posrepr, "");

    doc = db.get_document(1);
    doc.remove_term("pos3");
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos2, wdf=1)");

    // Regression test: the old positionlist fragment used to be left lying
    // around here.
    Xapian::PositionIterator posit2(db.positionlist_begin(1, "pos3"));
    string posrepr2 = positions_to_string(posit2, db.positionlist_end(1, "pos3"));
    TEST_EQUAL(posrepr2, "");

    doc = db.get_document(1);
    doc.add_term("pos");
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=1), "
	       "Term(pos2, wdf=1)");

    return true;
}

/** Check that changing the wdf of a term in a document works.
 */
DEFINE_TESTCASE(modtermwdf1, writable) {
    Xapian::WritableDatabase db(get_writable_database());

    // Add a simple document.
    Xapian::Document doc1;
    doc1.add_term("takeaway", 1);
    db.add_document(doc1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=1)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=1, wdf=1)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=1");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=1");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=1,ld=1");

    // Modify the wdf of an existing document, checking stats before flush.
    Xapian::Document doc2;
    doc2.add_term("takeaway", 2);
    db.replace_document(1, doc2);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=2, wdf=2)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=2");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=2");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=2,ld=1");

    // Remove a term, and then put it back again.
    Xapian::Document doc0;
    db.replace_document(1, doc0);
    TEST_EQUAL(docterms_to_string(db, 1), "");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "");
    TEST_EQUAL(docstats_to_string(db, 1), "len=0");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=0,cf=0");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=0,ld=1");
    db.replace_document(1, doc1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=1)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=1, wdf=1)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=1");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=1");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=1,ld=1");

    // Remove a term, and then put it back again without checking stats.
    db.replace_document(1, doc0);
    db.replace_document(1, doc2);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=2, wdf=2)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=2");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=2");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=2,ld=1");

    // Modify a term, and then put it back again without checking stats.
    db.replace_document(1, doc1);
    db.replace_document(1, doc2);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=2, wdf=2)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=2");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=2");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=2,ld=1");

    // Modify the wdf of an existing document, checking stats after flush.
    Xapian::Document doc3;
    doc3.add_term("takeaway", 3);
    db.replace_document(1, doc3);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=3, wdf=3)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=3");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=3");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=3,ld=1");

    // Modify a document taken from the database.
    Xapian::Document doc4(db.get_document(1));
    Xapian::Document doc3a(db.get_document(1)); // need this one later
    doc3a.termlist_count(); // Pull the document termlist into memory.
    doc4.add_term("takeaway", 1);
    db.replace_document(1, doc4);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=4)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=4, wdf=4)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=4");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=4");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=4,ld=1");

    // Add a document which was previously added and then modified.
    doc1.add_term("takeaway", 1);
    db.replace_document(1, doc1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=2)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=2, wdf=2)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=2");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=2");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=2,ld=1");

    // Add back a document which was taken from the database, but never modified.
    db.replace_document(1, doc3a);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=3, wdf=3)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=3");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=3");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=3,ld=1");

    // Add a position to the document.
    Xapian::Document doc5(db.get_document(1));
    doc5.add_posting("takeaway", 1, 2);
    db.replace_document(1, doc5);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[1])");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=5, wdf=5, pos=[1])");
    TEST_EQUAL(docstats_to_string(db, 1), "len=5");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=5");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=5,ld=1");

    // Add a position without changing the wdf.
    Xapian::Document doc5b(db.get_document(1));
    doc5b.add_posting("takeaway", 2, 0);
    db.replace_document(1, doc5b);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[1, 2])");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=5, wdf=5, pos=[1, 2])");
    TEST_EQUAL(docstats_to_string(db, 1), "len=5");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=5");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=5,ld=1");

    // Delete a position without changing the wdf.
    Xapian::Document doc5c;
    doc5c.add_posting("takeaway", 2, 5);
    db.replace_document(1, doc5c);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5, pos=[2])");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=5, wdf=5, pos=[2])");
    TEST_EQUAL(docstats_to_string(db, 1), "len=5");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=5");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=5,ld=1");

    // Delete the other position without changing the wdf.
    Xapian::Document doc5d;
    doc5d.add_term("takeaway", 5);
    db.replace_document(1, doc5d);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=5)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=5, wdf=5)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=5");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=5");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=5,ld=1");

    // Reduce the wdf to 0, but don't delete the term.
    Xapian::Document doc0freq;
    doc0freq.add_term("takeaway", 0);
    db.replace_document(1, doc0freq);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=0)");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "(1, doclen=0, wdf=0)");
    TEST_EQUAL(docstats_to_string(db, 1), "len=0");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=1,cf=0");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=0,ld=1");

    // Reduce the wdf to 0, and delete the term.
    db.replace_document(1, doc0);
    TEST_EQUAL(docterms_to_string(db, 1), "");
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "");
    TEST_EQUAL(docstats_to_string(db, 1), "len=0");
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=0,cf=0");
    TEST_EQUAL(dbstats_to_string(db), "dc=1,al=0,ld=1");

    // Delete the document.
    db.delete_document(1);
    TEST_EXCEPTION(Xapian::DocNotFoundError, docterms_to_string(db, 1));
    TEST_EQUAL(postlist_to_string(db, "takeaway"), "");
    TEST_EXCEPTION(Xapian::DocNotFoundError, docstats_to_string(db, 1));
    TEST_EQUAL(termstats_to_string(db, "takeaway"), "tf=0,cf=0");
    TEST_EQUAL(dbstats_to_string(db), "dc=0,al=0,ld=1");

    return true;
}
