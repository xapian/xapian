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
docterms_to_string(const Xapian::Database & db, Xapian::docid did) {
    string result;
    bool need_comma = false;

    for (Xapian::TermIterator t = db.termlist_begin(did);
	 t != db.termlist_end(did);
	 ++t) {
	Xapian::PositionIterator it(t.positionlist_begin());
	string posrepr = positions_to_string(it, t.positionlist_end());
	if (need_comma)
	    result += ", ";
	result += "Term(" + *t + ", wdf=" + str(t.get_wdf()) + ", pos=[" +
		posrepr + "])";
	need_comma = true;
    }
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
	       "Term(pos2, wdf=1, pos=[])");

    doc = db.get_document(1);
    doc.add_posting("pos3", 1);
    doc.add_posting("pos3", 5);
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos, wdf=2, pos=[2, 3]), "
	       "Term(pos2, wdf=1, pos=[]), "
	       "Term(pos3, wdf=2, pos=[1, 5])");

    doc = db.get_document(1);
    doc.remove_term("pos");
    db.replace_document(1, doc);
    db.flush();
    TEST_EQUAL(docterms_to_string(db, 1),
	       "Term(pos2, wdf=1, pos=[]), "
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
	       "Term(pos2, wdf=1, pos=[])");

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
	       "Term(pos, wdf=1, pos=[]), "
	       "Term(pos2, wdf=1, pos=[])");

    return true;
}
