/** @file api_backend.cc
 * @brief Backend-related tests.
 */
/* Copyright (C) 2008,2009 Olly Betts
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

#include "api_backend.h"

#include <xapian.h>

#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"

using namespace std;

/// Regression test - lockfile should honour umask, was only user-readable.
DEFINE_TESTCASE(lockfileumask1, flint || chert) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __EMX__
    mode_t old_umask = umask(022);
    try {
	Xapian::WritableDatabase db = get_named_writable_database("lockfileumask1");

	string path;
	const string & dbtype = get_dbtype();
	if (dbtype == "flint" || dbtype == "chert") {
	    path = get_named_writable_database_path("lockfileumask1");
	    path += "/flintlock";
	} else {
	    SKIP_TEST("Test only supported for flint and chert backends");
	}

	struct stat statbuf;
	TEST(stat(path, &statbuf) == 0);
	TEST_EQUAL(statbuf.st_mode & 0777, 0644);
    } catch (...) {
	umask(old_umask);
	throw;
    }

    umask(old_umask);
#endif

    return true;
}

/// Check that the backend handles total document length > 0xffffffff.
DEFINE_TESTCASE(totaldoclen1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_posting("foo", 1, 2000000000);
    db.add_document(doc);
    db.add_document(doc);
    TEST_EQUAL(db.get_avlength(), 2000000000);
    db.commit();
    TEST_EQUAL(db.get_avlength(), 2000000000);
    if (get_dbtype() != "inmemory") {
	// InMemory doesn't support get_writable_database_as_database().
	Xapian::Database dbr = get_writable_database_as_database();
	TEST_EQUAL(dbr.get_avlength(), 2000000000);
    }
    return true;
}
