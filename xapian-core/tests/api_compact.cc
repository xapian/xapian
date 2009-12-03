/** @file api_compact.cc
 * @brief Tests of xapian-compact.
 */
/* Copyright (C) 2009 Olly Betts
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
#include "testsuite.h"
#include "testutils.h"

#include <xapian.h>

#include "str.h"
#include <cstdlib>
#include <sys/wait.h>

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
		string id = str(first);
		doc.set_data(id);
		doc.add_term("Q" + str(first));
		doc.add_term(string(first % 7 + 1, char((first % 26) + 'a')));
		db.replace_document(first, doc);
	    }
	} while (first++ < last);

	if (*p == '\0') break;
	++p;
    }

    db.commit();
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

inline int system(const string & cmd) { return system(cmd.c_str()); }

DEFINE_TESTCASE(compactnorenumber1, chert || flint) {
    int status;

    string a = get_database_path("compactnorenumber1a", make_sparse_db,
				 "5-7 24 76 987 1023-1027 9999 !9999");
    a += ' ';
    string b = get_database_path("compactnorenumber1b", make_sparse_db,
				 "1027-1030");
    b += ' ';
    string c = get_database_path("compactnorenumber1c", make_sparse_db,
				 "1028-1040");
    c += ' ';
    string d = get_database_path("compactnorenumber1d", make_sparse_db,
				 "3000 999999 !999999");
    d += ' ';

    string cmd = "../bin/xapian-compact >/dev/null 2>&1 --no-renumber ";
    string out = get_named_writable_database_path("compactnorenumber1out");

    rm_rf(out);
    status = system(cmd + a + out);
    TEST_EQUAL(WEXITSTATUS(status), 0);
    check_sparse_uid_terms(out);

    rm_rf(out);
    status = system(cmd + a + c + out);
    TEST_EQUAL(WEXITSTATUS(status), 0);
    check_sparse_uid_terms(out);

    rm_rf(out);
    status = system(cmd + d + a + c + out);
    TEST_EQUAL(WEXITSTATUS(status), 0);
    check_sparse_uid_terms(out);

    rm_rf(out);
    status = system(cmd + c + a + d + out);
    TEST_EQUAL(WEXITSTATUS(status), 0);
    check_sparse_uid_terms(out);

    // Should fail.
    rm_rf(out);
    status = system(cmd + a + b + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);
 
    // Should fail.
    rm_rf(out);
    status = system(cmd + b + a + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);

    // Should fail.
    rm_rf(out);
    status = system(cmd + a + b + d + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);
 
    // Should fail.
    rm_rf(out);
    status = system(cmd + d + b + a + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);

    // Should fail.
    rm_rf(out);
    status = system(cmd + b + a + d + out);
    TEST_NOT_EQUAL(WEXITSTATUS(status), 0);

    return true;
}
