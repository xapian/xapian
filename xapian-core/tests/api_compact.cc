/** @file api_compact.cc
 * @brief Tests of xapian-compact.
 */
/* Copyright (C) 2009,2010,2011,2013 Olly Betts
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
#include <fstream>

#include "str.h"
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

DEFINE_TESTCASE(compactnorenumber1, generated) {
    string a = get_database_path("compactnorenumber1a", make_sparse_db,
				 "5-7 24 76 987 1023-1027 9999 !9999");
    string a_uuid;
    {
	Xapian::Database db(a);
	a_uuid = db.get_uuid();
    }
    string b = get_database_path("compactnorenumber1b", make_sparse_db,
				 "1027-1030");
    string c = get_database_path("compactnorenumber1c", make_sparse_db,
				 "1028-1040");
    string d = get_database_path("compactnorenumber1d", make_sparse_db,
				 "3000 999999 !999999");

    string out = get_named_writable_database_path("compactnorenumber1out");

    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(a);
	compact.compact();
    }

    check_sparse_uid_terms(out);

    {
	if (get_dbtype() == "flint") {
	    // Flint creates its uuid file if it doesn't already exist, so we
	    // need a white box test to ensure xapian-compact created it.
	    TEST(file_exists(out + "/uuid"));
	}
	TEST(!dir_exists(out + "/donor"));
	Xapian::Database db(out);
	// xapian-compact should change the UUID of the database, but didn't
	// prior to 1.0.18/1.1.4.
	string out_uuid = db.get_uuid();
	TEST_NOT_EQUAL(a_uuid, out_uuid);
	TEST_EQUAL(out_uuid.size(), 36);
	TEST_NOT_EQUAL(out_uuid, "00000000-0000-0000-0000-000000000000");

	// White box test - ensure that the donor database is removed.
	TEST(!dir_exists(out + "/donor"));
    }

    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(a);
	compact.add_source(c);
	compact.compact();
    }
    check_sparse_uid_terms(out);
    {
	// Check that xapian-compact is producing a consistent database.  Also,
	// regression test - xapian 1.1.4 set lastdocid to 0 in the output
	// database.
	Xapian::Database outdb(out);
	dbcheck(outdb, 24, 9999);
    }

    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(d);
	compact.add_source(a);
	compact.add_source(c);
	compact.compact();
    }
    check_sparse_uid_terms(out);

    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(c);
	compact.add_source(a);
	compact.add_source(d);
	compact.compact();
    }
    check_sparse_uid_terms(out);

    // Should fail.
    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(a);
	compact.add_source(b);
	TEST_EXCEPTION(Xapian::InvalidOperationError, compact.compact());
    }
 
    // Should fail.
    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(b);
	compact.add_source(a);
	TEST_EXCEPTION(Xapian::InvalidOperationError, compact.compact());
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(a);
	compact.add_source(b);
	compact.add_source(d);
	TEST_EXCEPTION(Xapian::InvalidOperationError, compact.compact());
    }
 
    // Should fail.
    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(d);
	compact.add_source(b);
	compact.add_source(a);
	TEST_EXCEPTION(Xapian::InvalidOperationError, compact.compact());
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Compactor compact;
	compact.set_renumber(false);
	compact.set_destdir(out);
	compact.add_source(b);
	compact.add_source(a);
	compact.add_source(d);
	TEST_EXCEPTION(Xapian::InvalidOperationError, compact.compact());
    }

    return true;
}

// Test use of compact to merge two databases.
DEFINE_TESTCASE(compactmerge1, brass || chert || flint) {
    string indbpath = get_database_path("apitest_simpledata");
    string outdbpath = get_named_writable_database_path("compactmerge1out");
    rm_rf(outdbpath);

    Xapian::Compactor compact;
    compact.set_destdir(outdbpath);
    compact.add_source(indbpath);
    compact.add_source(indbpath);
    compact.compact();

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

    db.commit();
}

// Test use of compact on a database which has multiple chunks for a term.
// This is a regression test for ticket #427
DEFINE_TESTCASE(compactmultichunks1, generated) {
    string indbpath = get_database_path("compactmultichunks1in",
					make_multichunk_db, "");
    string outdbpath = get_named_writable_database_path("compactmultichunks1out");
    rm_rf(outdbpath);

    Xapian::Compactor compact;
    compact.set_destdir(outdbpath);
    compact.add_source(indbpath);
    compact.compact();

    Xapian::Database indb(indbpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

// Test compacting from a stub database directory.
DEFINE_TESTCASE(compactstub1, brass || chert || flint) {
    const char * stubpath = ".stub/compactstub1";
    const char * stubpathfile = ".stub/compactstub1/XAPIANDB";
    mkdir(".stub", 0755);
    mkdir(stubpath, 0755);
    ofstream stub(stubpathfile);
    TEST(stub.is_open());
    stub << "auto ../../" << get_database_path("apitest_simpledata") << endl;
    stub << "auto ../../" << get_database_path("apitest_simpledata2") << endl;
    stub.close();

    string outdbpath = get_named_writable_database_path("compactstub1out");
    rm_rf(outdbpath);

    Xapian::Compactor compact;
    compact.set_destdir(outdbpath);
    compact.add_source(stubpath);
    compact.compact();

    Xapian::Database indb(stubpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

// Test compacting from a stub database file.
DEFINE_TESTCASE(compactstub2, brass || chert || flint) {
    const char * stubpath = ".stub/compactstub2";
    mkdir(".stub", 0755);
    ofstream stub(stubpath);
    TEST(stub.is_open());
    stub << "auto ../" << get_database_path("apitest_simpledata") << endl;
    stub << "auto ../" << get_database_path("apitest_simpledata2") << endl;
    stub.close();

    string outdbpath = get_named_writable_database_path("compactstub2out");
    rm_rf(outdbpath);

    Xapian::Compactor compact;
    compact.set_destdir(outdbpath);
    compact.add_source(stubpath);
    compact.compact();

    Xapian::Database indb(stubpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

// Test compacting a stub database file to itself.
DEFINE_TESTCASE(compactstub3, brass || chert || flint) {
    const char * stubpath = ".stub/compactstub3";
    mkdir(".stub", 0755);
    ofstream stub(stubpath);
    TEST(stub.is_open());
    stub << "auto ../" << get_database_path("apitest_simpledata") << endl;
    stub << "auto ../" << get_database_path("apitest_simpledata2") << endl;
    stub.close();

    Xapian::doccount in_docs;
    {
	Xapian::Database indb(stubpath);
	in_docs = indb.get_doccount();
    }

    Xapian::Compactor compact;
    compact.set_destdir(stubpath);
    compact.add_source(stubpath);
    compact.compact();

    Xapian::Database outdb(stubpath);

    TEST_EQUAL(in_docs, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

// Test compacting a stub database directory to itself.
DEFINE_TESTCASE(compactstub4, brass || chert || flint) {
    const char * stubpath = ".stub/compactstub4";
    const char * stubpathfile = ".stub/compactstub4/XAPIANDB";
    mkdir(".stub", 0755);
    mkdir(stubpath, 0755);
    ofstream stub(stubpathfile);
    TEST(stub.is_open());
    stub << "auto ../../" << get_database_path("apitest_simpledata") << endl;
    stub << "auto ../../" << get_database_path("apitest_simpledata2") << endl;
    stub.close();

    Xapian::doccount in_docs;
    {
	Xapian::Database indb(stubpath);
	in_docs = indb.get_doccount();
    }

    Xapian::Compactor compact;
    compact.set_destdir(stubpath);
    compact.add_source(stubpath);
    compact.compact();

    Xapian::Database outdb(stubpath);

    TEST_EQUAL(in_docs, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    return true;
}

static void
make_all_tables(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);
    db.add_spelling("foo");
    db.add_synonym("foobar", "foo");

    db.commit();
}

static void
make_missing_tables(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);

    db.commit();
}

DEFINE_TESTCASE(compactmissingtables1, generated) {
    string a = get_database_path("compactmissingtables1a",
				 make_all_tables);
    string b = get_database_path("compactmissingtables1b",
				 make_missing_tables);

    string out = get_named_writable_database_path("compactmissingtables1out");
    rm_rf(out);

    Xapian::Compactor compact;
    compact.set_destdir(out);
    compact.add_source(a);
    compact.add_source(b);
    compact.compact();

    {
	Xapian::Database db(out);
	TEST_NOT_EQUAL(db.spellings_begin(), db.spellings_end());
	TEST_NOT_EQUAL(db.synonym_keys_begin(), db.synonym_keys_end());
	// FIXME: arrange for input b to not have a termlist table.
//	TEST_EXCEPTION(Xapian::FeatureUnavailableError, db.termlist_begin(1));
    }

    return true;
}

static void
make_all_tables2(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_term("bar");
    db.add_document(doc);
    db.add_spelling("bar");
    db.add_synonym("bar", "baa");
    db.add_synonym("barfoo", "barbar");
    db.add_synonym("foofoo", "barfoo");

    db.commit();
}

/// Adds coverage for merging synonym table.
DEFINE_TESTCASE(compactmergesynonym1, generated) {
    string a = get_database_path("compactmergesynonym1a",
				 make_all_tables);
    string b = get_database_path("compactmergesynonym1b",
				 make_all_tables2);

    string out = get_named_writable_database_path("compactmergesynonym1out");
    rm_rf(out);

    Xapian::Compactor compact;
    compact.set_destdir(out);
    compact.add_source(a);
    compact.add_source(b);
    compact.compact();

    {
	Xapian::Database db(out);

	Xapian::TermIterator i = db.spellings_begin();
	TEST_NOT_EQUAL(i, db.spellings_end());
	TEST_EQUAL(*i, "bar");
	++i;
	TEST_NOT_EQUAL(i, db.spellings_end());
	TEST_EQUAL(*i, "foo");
	++i;
	TEST_EQUAL(i, db.spellings_end());

	i = db.synonym_keys_begin();
	TEST_NOT_EQUAL(i, db.synonym_keys_end());
	TEST_EQUAL(*i, "bar");
	++i;
	TEST_NOT_EQUAL(i, db.synonym_keys_end());
	TEST_EQUAL(*i, "barfoo");
	++i;
	TEST_NOT_EQUAL(i, db.synonym_keys_end());
	TEST_EQUAL(*i, "foobar");
	++i;
	TEST_NOT_EQUAL(i, db.synonym_keys_end());
	TEST_EQUAL(*i, "foofoo");
	++i;
	TEST_EQUAL(i, db.synonym_keys_end());
    }

    return true;
}

DEFINE_TESTCASE(compactempty1, brass || chert) {
    string empty_dbpath = get_database_path(string());
    string outdbpath = get_named_writable_database_path("compactempty1out");
    rm_rf(outdbpath);

    {
	// Compacting an empty database tried to divide by zero in 1.3.0.
	Xapian::Compactor compact;
	compact.set_destdir(outdbpath);
	compact.add_source(empty_dbpath);
	compact.compact();

	Xapian::Database outdb(outdbpath);
	TEST_EQUAL(outdb.get_doccount(), 0);
	dbcheck(outdb, 0, 0);
    }

    {
	// Check compacting two empty databases together.
	Xapian::Compactor compact;
	compact.set_destdir(outdbpath);
	compact.add_source(empty_dbpath);
	compact.add_source(empty_dbpath);
	compact.compact();

	Xapian::Database outdb(outdbpath);
	TEST_EQUAL(outdb.get_doccount(), 0);
	dbcheck(outdb, 0, 0);
    }

    return true;
}

DEFINE_TESTCASE(compactmultipass1, brass || chert) {
    string outdbpath = get_named_writable_database_path("compactmultipass1");
    rm_rf(outdbpath);

    string a = get_database_path("compactnorenumber1a", make_sparse_db,
				 "5-7 24 76 987 1023-1027 9999 !9999");
    string b = get_database_path("compactnorenumber1b", make_sparse_db,
				 "1027-1030");
    string c = get_database_path("compactnorenumber1c", make_sparse_db,
				 "1028-1040");
    string d = get_database_path("compactnorenumber1d", make_sparse_db,
				 "3000 999999 !999999");

    Xapian::Compactor compact;
    compact.set_destdir(outdbpath);
    compact.add_source(a);
    compact.add_source(b);
    compact.add_source(c);
    compact.add_source(d);
    compact.set_multipass(true);
    compact.compact();

    Xapian::Database outdb(outdbpath);
    dbcheck(outdb, 29, 1041);

    return true;
}
