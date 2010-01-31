/** @file api_compact.cc
 * @brief Tests of xapian-compact.
 */
/* Copyright (C) 2009,2010 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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

#include "api_compact.h"

#include "apitest.h"
#include "dbcheck.h"
#include "testsuite.h"
#include "testutils.h"

#include <xapian.h>

#include <cstdlib>
#include <sys/wait.h>

#include "utils.h"
#include "unixcmds.h"

using namespace std;

static void
make_sparse_db(Xapian::WritableDatabase &db, const string & s)
{
    // Need non-const pointer for strtoul(), but data isn't modified.
    char * p = const_cast<char *>(s.c_str());

    while (*p) {
	bool del = (*p == '!');
	if (del) ++p;
	Xapian::docid first = strtoul(p, &p, 10);
	Xapian::docid last = first;
	if (*p == '-') {
	    last = strtoul(p + 1, &p, 10);
	}
	if (*p && *p != ' ') {
	    tout << p - s.c_str() << endl;
	    FAIL_TEST("Bad sparse db spec (expected space): " << s);
	}
	if (first > last) {
	    FAIL_TEST("Bad sparse db spec (first > last): " << s);
	}

	do {
	    if (del) {
		db.delete_document(first);
	    } else {
		Xapian::Document doc;
		string id = om_tostring(first);
		doc.set_data(id);
		doc.add_term("Q" + om_tostring(first));
		doc.add_term(string(first % 7 + 1, char((first % 26) + 'a')));
		db.replace_document(first, doc);
	    }
	} while (first++ < last);

	if (*p == '\0') break;
	++p;
    }

    db.flush();
}

static void
check_sparse_uid_terms(const string & path)
{
    Xapian::Database db(path);
    Xapian::TermIterator t;
    for (t = db.allterms_begin("Q"); t != db.allterms_end("Q"); ++t) {
	Xapian::docid did = atoi((*t).c_str() + 1);
	Xapian::PostingIterator p = db.postlist_begin(*t);
	TEST_EQUAL(*p, did);
    }
}

DEFINE_TESTCASE(compactnorenumber1, flint) {
    int status;

    string b = get_database_path("compactnorenumber1b", make_sparse_db,
				 "1027-1030");
    b += ' ';
    string d = get_database_path("compactnorenumber1d", make_sparse_db,
				 "3000 999999 !999999");
    d += ' ';

    string cmd = "../bin/xapian-compact >/dev/null 2>&1 --no-renumber ";
    string out = get_named_writable_database_path("compactnorenumber1out");

    rm_rf(out);
    status = system(cmd + b + out);
    TEST_EQUAL(WEXITSTATUS(status), 0);
    check_sparse_uid_terms(out);

    // Should fail for 1.0.x which doesn't support --no-renumber when merging.
    rm_rf(out);
    status = system(cmd + b + d + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);

    return true;
}

// Test use of compact to merge two databases.
DEFINE_TESTCASE(compactmerge1, flint) {
    int status;

    string cmd = "../bin/xapian-compact >/dev/null 2>&1 ";
    string indbpath = get_database_path("apitest_simpledata") + ' ';
    string outdbpath = get_named_writable_database_path("compactmerge1out");
    rm_rf(outdbpath);

    status = system(cmd + indbpath + indbpath + outdbpath);
    TEST_EQUAL(WEXITSTATUS(status), 0);

    Xapian::Database indb(get_database("apitest_simpledata"));
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount() * 2, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

static void
make_multichunk_db(Xapian::WritableDatabase &db, const string &)
{
    int count = 10000;

    Xapian::Document doc;
    doc.add_term("a");
    while (count) {
	db.add_document(doc);
	--count;
    }

    db.flush();
}

// Test use of compact on a database which has multiple chunks for a term.
// This is a regression test for ticket #427
DEFINE_TESTCASE(compactmultichunks1, flint) {
    int status;

    string cmd = "../bin/xapian-compact >/dev/null 2>&1 ";
    string indbpath = get_database_path("compactmultichunks1in",
					make_multichunk_db, "");
    string outdbpath = get_named_writable_database_path("compactmultichunks1out");
    rm_rf(outdbpath);

    status = system(cmd + indbpath + ' ' + outdbpath);
    TEST_EQUAL(WEXITSTATUS(status), 0);

    Xapian::Database indb(indbpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}
