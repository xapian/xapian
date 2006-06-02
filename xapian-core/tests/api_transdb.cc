/** @file api_transdb.cc
 * @brief tests requiring a database backend supporting transactions
 */
/* Copyright (C) 2006 Olly Betts
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

#include <xapian.h>

#include "api_transdb.h"
#include "apitest.h"
#include "testutils.h"

using namespace std;

#define TESTCASE(S) {#S, test_##S}
#define END_OF_TESTCASES {0, 0}

/// Test incorrect uses of the transaction API lead to errors.
static bool test_badtransaction1()
{
    Xapian::WritableDatabase db(get_writable_database("apitest_simpledata"));

    TEST_EXCEPTION(Xapian::InvalidOperationError, db.commit_transaction());
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.cancel_transaction());

    db.begin_transaction();
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.begin_transaction());
    db.commit_transaction();

    TEST_EXCEPTION(Xapian::InvalidOperationError, db.commit_transaction());
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.cancel_transaction());

    db.begin_transaction();
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.begin_transaction());
    db.cancel_transaction();

    TEST_EXCEPTION(Xapian::InvalidOperationError, db.commit_transaction());
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.cancel_transaction());

    db.begin_transaction();
    db.commit_transaction();

    db.begin_transaction();
    db.cancel_transaction();

    return true;
}

/// Test committing a simple transaction.
static bool test_committransaction1()
{
    Xapian::WritableDatabase db(get_writable_database("apitest_simpledata"));

    Xapian::doccount docs = db.get_doccount();
    db.begin_transaction();
    Xapian::Document doc;
    doc.set_data("testing");
    doc.add_term("befuddlement");
    db.add_document(doc);
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.begin_transaction());
    TEST_EQUAL(db.get_doccount(), docs + 1);
    TEST_EQUAL(db.get_termfreq("befuddlement"), 1);
    db.commit_transaction();
    TEST_EQUAL(db.get_doccount(), docs + 1);
    TEST_EQUAL(db.get_termfreq("befuddlement"), 1);

    return true;
}

/// Test cancelling a simple transaction.
static bool test_canceltransaction1()
{
    Xapian::WritableDatabase db(get_writable_database("apitest_simpledata"));

    Xapian::doccount docs = db.get_doccount();
    db.begin_transaction();
    Xapian::Document doc;
    doc.set_data("testing");
    doc.add_term("befuddlement");
    db.add_document(doc);
    TEST_EXCEPTION(Xapian::InvalidOperationError, db.begin_transaction());
    TEST_EQUAL(db.get_doccount(), docs + 1);
    TEST_EQUAL(db.get_termfreq("befuddlement"), 1);
    db.cancel_transaction();
    TEST_EQUAL(db.get_doccount(), docs);
    TEST_EQUAL(db.get_termfreq("befuddlement"), 0);

    return true;
}

/// Test that begin_transaction() flushes any changes pending before the
//  transaction.
static bool test_canceltransaction2()
{
    Xapian::WritableDatabase db(get_writable_database("apitest_simpledata"));

    Xapian::doccount docs = db.get_doccount();
    Xapian::Document doc0;
    doc0.set_data("pending");
    doc0.add_term("pending_update");
    Xapian::docid docid = db.add_document(doc0);

    db.begin_transaction();
    TEST_EQUAL(db.get_doccount(), docs + 1);
    Xapian::Document doc;
    doc.set_data("testing");
    doc.add_term("befuddlement");
    db.add_document(doc);
    TEST_EQUAL(db.get_doccount(), docs + 2);
    db.cancel_transaction();

    TEST_EQUAL(db.get_doccount(), docs + 1);
    TEST(db.term_exists("pending_update"));
    Xapian::Document doc_out = db.get_document(docid);
    TEST_EQUAL(doc_out.get_data(), "pending");

    return true;
}

/** Test cases requiring a database backend supporting transactions. */
test_desc transactiondb_tests[] = {
    TESTCASE(badtransaction1),
    TESTCASE(committransaction1),
    TESTCASE(canceltransaction1),
    TESTCASE(canceltransaction2),
    END_OF_TESTCASES
};
