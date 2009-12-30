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
#include <map>

using namespace std;

#include <xapian.h>
#include "str.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

static string
positions_to_string(Xapian::PositionIterator & it,
		    const Xapian::PositionIterator & end,
		    Xapian::termcount * count = NULL) {
    string result;
    bool need_comma = false;
    Xapian::termcount c = 0;
    while (it != end) {
	if (need_comma)
	    result += ", ";
	result += str(*it);
	need_comma = true;
	++it;
	++c;
    }
    if (count) {
	*count = c;
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

/** Check consistency of database and statistics.
 */
static void
dbcheck(const Xapian::Database & db,
	Xapian::doccount expected_doccount,
	Xapian::docid expected_lastdocid) {
    TEST_EQUAL(db.get_doccount(), expected_doccount);
    TEST_EQUAL(db.get_lastdocid(), expected_lastdocid);

    // Note - may not be a very big type, but we're only expecting to use this
    // for small databases, so should be fine.
    unsigned long totlen = 0;

    // A map from term to a representation of the posting list for that term.
    // We build this up from the documents, and then check it against the
    // equivalent built up from the posting lists.
    map<string, string> posting_reprs;

    for (Xapian::PostingIterator dociter = db.postlist_begin(string());
	 dociter != db.postlist_end(string());
	 ++dociter) {
	Xapian::docid did = *dociter;
	TEST_EQUAL(dociter.get_wdf(), 1);
	Xapian::Document doc(db.get_document(did));
	Xapian::termcount doclen(db.get_doclength(did));
	totlen += doclen;

	Xapian::termcount found_termcount = 0;
	Xapian::termcount wdf_sum = 0;
	Xapian::TermIterator t, t2;
	for (t = doc.termlist_begin(), t2 = db.termlist_begin(did);
	     t != doc.termlist_end();
	     ++t, ++t2) {
	    TEST(t2 != db.termlist_end(did));

	    ++ found_termcount;
	    wdf_sum += t.get_wdf();

	    TEST_EQUAL(*t, *t2);
	    TEST_EQUAL(t.get_wdf(), t2.get_wdf());
	    TEST_EQUAL(db.get_termfreq(*t), t.get_termfreq());
	    TEST_EQUAL(db.get_termfreq(*t), t2.get_termfreq());

	    // Check the position lists are equal.
	    Xapian::termcount tc1, tc2;
	    Xapian::PositionIterator it1(t.positionlist_begin());
	    string posrepr = positions_to_string(it1, t.positionlist_end(), &tc1);
	    Xapian::PositionIterator it2(t2.positionlist_begin());
	    string posrepr2 = positions_to_string(it2, t2.positionlist_end(), &tc2);
	    TEST_EQUAL(posrepr, posrepr2);
	    TEST_EQUAL(tc1, tc2);
	    try {
	    	TEST_EQUAL(tc1, t.positionlist_count());
	    } catch (Xapian::UnimplementedError) {
		// positionlist_count() isn't implemented for remote databases.
	    }

	    // Make a representation of the posting.
	    if (!posrepr.empty()) {
		posrepr = ",[" + posrepr + "]";
	    }
	    string posting_repr = "(" + str(did) + "," +
		    str(t.get_wdf()) + "/" + str(doclen) +
		    posrepr + ")";

	    // Append the representation to the list for the term.
	    map<string, string>::iterator i = posting_reprs.find(*t);
	    if (i == posting_reprs.end()) {
		posting_reprs[*t] = posting_repr;
	    } else {
		i->second += "," + posting_repr;
	    }
	}
	TEST(t2 == db.termlist_end(did));
	Xapian::termcount expected_termcount = doc.termlist_count();
	TEST_EQUAL(expected_termcount, found_termcount);
	TEST_EQUAL(doclen, wdf_sum);
    }

    Xapian::TermIterator t;
    map<string, string>::const_iterator i;
    for (t = db.allterms_begin(), i = posting_reprs.begin();
	 t != db.allterms_end();
	 ++t, ++i) {
	TEST(db.term_exists(*t));
	TEST(i != posting_reprs.end());
	TEST_EQUAL(i->first, *t);

	Xapian::doccount tf_count = 0;
	Xapian::termcount cf_count = 0;
	string posting_repr;
	bool need_comma = false;
	for (Xapian::PostingIterator p = db.postlist_begin(*t);
	     p != db.postlist_end(*t);
	     ++p) {
	    if (need_comma) {
		posting_repr += ",";
	    }

	    ++tf_count;
	    cf_count += p.get_wdf();

	    Xapian::PositionIterator it(p.positionlist_begin());
	    string posrepr = positions_to_string(it, p.positionlist_end());
	    if (!posrepr.empty()) {
		posrepr = ",[" + posrepr + "]";
	    }
	    posting_repr += "(" + str(*p) + "," +
		    str(p.get_wdf()) + "/" + str(p.get_doclength()) +
		    posrepr + ")";
	    need_comma = true;
	}

	TEST_EQUAL(posting_repr, i->second);
	TEST_EQUAL(tf_count, t.get_termfreq());
	TEST_EQUAL(tf_count, db.get_termfreq(*t));
	TEST_EQUAL(cf_count, db.get_collection_freq(*t));
    }
    TEST(i == posting_reprs.end());

    if (expected_doccount == 0) {
	TEST_EQUAL(0, db.get_avlength());
    } else {
	TEST_EQUAL_DOUBLE(double(totlen) / expected_doccount,
			  db.get_avlength());
    }
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

    // Modify the wdf of an existing document, checking stats before flush.
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

    // Remove a term, flush, then put it back, remove it, and put it back.
    // This is to test the handling of items in the change cache.
    db.replace_document(1, doc0);
    db.flush();
    db.replace_document(1, doc2);
    db.replace_document(1, doc0);
    db.replace_document(1, doc2);
    db.flush();
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

    // Modify the wdf of an existing document, checking stats after flush.
    Xapian::Document doc3(basic_doc());
    doc3.add_term("takeaway", 3);
    db.replace_document(1, doc3);
    db.flush();
    dbcheck(db, 1, 1);
    TEST_EQUAL(docterms_to_string(db, 1), "Term(takeaway, wdf=3)" + bdt);

    // Change a document, without changing its length.
    Xapian::Document doc3_diff(basic_doc());
    doc3_diff.add_term("takeaways", 3);
    db.replace_document(1, doc3_diff);
    db.flush();
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
    TEST_EQUAL(dbstats_to_string(db), "dc=0,al=0,ld=1");

    return true;
}
