/** @file api_closedb.cc
 * @brief Tests of closing databases.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#include "api_closedb.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

// Test for closing a database
DEFINE_TESTCASE(closedb1, backend && !inmemory) {
    Xapian::Database db(get_database("apitest_simpledata"));
    TEST_NOT_EQUAL(db.postlist_begin("paragraph"), db.postlist_end("paragraph"));
    db.close();

    // Counting the documents raises the "database closed" error.
    TEST_EXCEPTION(Xapian::DatabaseError, db.postlist_begin("paragraph"));

    // Reopen raises the "database closed" error.
    TEST_EXCEPTION(Xapian::DatabaseError, db.reopen());

    // Calling close repeatedly is okay.
    db.close();

    return true;
}

// Test for closing an inmemory database
DEFINE_TESTCASE(closedb2, backend && inmemory) {
    Xapian::Database db(get_database("apitest_simpledata"));
    TEST_NOT_EQUAL(db.postlist_begin("paragraph"), db.postlist_end("paragraph"));
    db.close();

    // Inmemory database doesn't currently raise an error, but does release all
    // memory used, so behaves as if database is empty.
    TEST_EQUAL(db.postlist_begin("paragraph"), db.postlist_end("paragraph"));

    // Reopen has no effect for inmemory, and doesn't raise an exception.
    db.reopen();

    // Calling close repeatedly is okay.
    db.close();

    return true;
}

// Test closing a writable database, and that it drops the lock.
DEFINE_TESTCASE(closedb3, backend && writable && !remote && !inmemory) {
    Xapian::WritableDatabase dbw1(get_named_writable_database("apitest_closedb2"));
    TEST_EXCEPTION(Xapian::DatabaseLockError,
		   Xapian::WritableDatabase(get_named_writable_database_path("apitest_closedb2"),
					    Xapian::DB_OPEN));
    dbw1.close();
    Xapian::WritableDatabase dbw2 = get_named_writable_database("apitest_closedb2");
    TEST_EXCEPTION(Xapian::DatabaseError, dbw1.postlist_begin("paragraph"));
    TEST_EQUAL(dbw2.postlist_begin("paragraph"), dbw2.postlist_end("paragraph"));

    return true;
}
