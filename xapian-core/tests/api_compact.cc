/** @file
 * @brief Tests of Database::compact()
 */
/* Copyright (C) 2009,2010,2011,2012,2013,2015,2016,2017,2018,2019 Olly Betts
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

#include <xapian.h>

#include "apitest.h"
#include "dbcheck.h"
#include "filetests.h"
#include "msvcignoreinvalidparam.h"
#include "str.h"
#include "testsuite.h"
#include "testutils.h"

#include <cerrno>
#include <cstdlib>
#include <fstream>

#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#include "safeunistd.h"

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
	    tout << p - s.c_str() << '\n';
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

// With multi the docids in the shards change the behaviour.
DEFINE_TESTCASE(compactnorenumber1, compact && !multi) {
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

    string out = get_compaction_output_path("compactnorenumber1out");

    rm_rf(out);
    {
	Xapian::Database db(a);
	db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER);
    }

    check_sparse_uid_terms(out);

    {
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
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(c));
	db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER);
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
	Xapian::Database db;
	db.add_database(Xapian::Database(d));
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(c));
	db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER);
    }
    check_sparse_uid_terms(out);

    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(c));
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(d));
	db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER);
    }
    check_sparse_uid_terms(out);

    // Should fail.
    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(b));
	TEST_EXCEPTION(Xapian::InvalidOperationError,
	    db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER)
	);
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(b));
	db.add_database(Xapian::Database(a));
	TEST_EXCEPTION(Xapian::InvalidOperationError,
	    db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER)
	);
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(b));
	db.add_database(Xapian::Database(d));
	TEST_EXCEPTION(Xapian::InvalidOperationError,
	    db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER)
	);
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(d));
	db.add_database(Xapian::Database(b));
	db.add_database(Xapian::Database(a));
	TEST_EXCEPTION(Xapian::InvalidOperationError,
	    db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER)
	);
    }

    // Should fail.
    rm_rf(out);
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(b));
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(d));
	TEST_EXCEPTION(Xapian::InvalidOperationError,
	    db.compact(out, Xapian::DBCOMPACT_NO_RENUMBER)
	);
    }
}

// Test use of compact to merge two databases.
DEFINE_TESTCASE(compactmerge1, compact) {
    string indbpath = get_database_path("apitest_simpledata");
    string outdbpath = get_compaction_output_path("compactmerge1out");
    rm_rf(outdbpath);

    bool singlefile = startswith(get_dbtype(), "singlefile_");
    {
	Xapian::Database db;
	db.add_database(Xapian::Database(indbpath));
	db.add_database(Xapian::Database(indbpath));
	if (singlefile) {
	    db.compact(outdbpath, Xapian::DBCOMPACT_SINGLE_FILE);
	} else {
	    db.compact(outdbpath);
	}
    }

    Xapian::Database indb(get_database("apitest_simpledata"));
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount() * 2, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());

    if (singlefile) {
	// Check we actually got a single file out.
	TEST(file_exists(outdbpath));
	TEST_EQUAL(Xapian::Database::check(outdbpath, 0, &tout), 0);
    } else if (indb.size() > 1) {
	// Can't check tables for a sharded DB.
	TEST_EQUAL(Xapian::Database::check(outdbpath, 0, &tout), 0);
    } else {
	// Check we got a directory out, not a file.
	TEST(dir_exists(outdbpath));
	static const char* const suffixes[] = {
	    "", "/postlist", "/termlist.", nullptr
	};
	for (auto s : suffixes) {
	    string suffix;
	    if (s) {
		suffix = s;
	    } else {
		if (get_dbtype() == "chert") {
		    suffix = "/record.DB";
		} else {
		    suffix = "/docdata." + get_dbtype();
		}
	    }
	    tout.str(string());
	    tout << "Trying suffix '" << suffix << "'\n";
	    string arg = outdbpath;
	    arg += suffix;
	    TEST_EQUAL(Xapian::Database::check(arg, 0, &tout), 0);
	}
    }
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
DEFINE_TESTCASE(compactmultichunks1, compact) {
    string indbpath = get_database_path("compactmultichunks1in",
					make_multichunk_db, "");
    string outdbpath = get_compaction_output_path("compactmultichunks1out");
    rm_rf(outdbpath);

    {
	Xapian::Database db(indbpath);
	db.compact(outdbpath);
    }

    Xapian::Database indb(indbpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Test compacting from a stub database directory.
DEFINE_TESTCASE(compactstub1, compact) {
    const char * stubpath = ".stub/compactstub1";
    const char * stubpathfile = ".stub/compactstub1/XAPIANDB";
    mkdir(".stub", 0755);
    mkdir(stubpath, 0755);
    ofstream stub(stubpathfile);
    TEST(stub.is_open());
    stub << "auto ../../" << get_database_path("apitest_simpledata") << '\n';
    stub << "auto ../../" << get_database_path("apitest_simpledata2") << '\n';
    stub.close();

    string outdbpath = get_compaction_output_path("compactstub1out");
    rm_rf(outdbpath);

    {
	Xapian::Database db(stubpath);
	db.compact(outdbpath);
    }

    Xapian::Database indb(stubpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Test compacting from a stub database file.
DEFINE_TESTCASE(compactstub2, compact) {
    const char * stubpath = ".stub/compactstub2";
    mkdir(".stub", 0755);
    ofstream stub(stubpath);
    TEST(stub.is_open());
    stub << "auto ../" << get_database_path("apitest_simpledata") << '\n';
    stub << "auto ../" << get_database_path("apitest_simpledata2") << '\n';
    stub.close();

    string outdbpath = get_compaction_output_path("compactstub2out");
    rm_rf(outdbpath);

    {
	Xapian::Database db(stubpath);
	db.compact(outdbpath);
    }

    Xapian::Database indb(stubpath);
    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Test compacting a stub database file to itself.
DEFINE_TESTCASE(compactstub3, compact) {
    const char * stubpath = ".stub/compactstub3";
    mkdir(".stub", 0755);
    ofstream stub(stubpath);
    TEST(stub.is_open());
    stub << "auto ../" << get_database_path("apitest_simpledata") << '\n';
    stub << "auto ../" << get_database_path("apitest_simpledata2") << '\n';
    stub.close();

    Xapian::doccount in_docs;
    {
	Xapian::Database indb(stubpath);
	in_docs = indb.get_doccount();
	indb.compact(stubpath);
    }

    Xapian::Database outdb(stubpath);

    TEST_EQUAL(in_docs, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Test compacting a stub database directory to itself.
DEFINE_TESTCASE(compactstub4, compact) {
    const char * stubpath = ".stub/compactstub4";
    const char * stubpathfile = ".stub/compactstub4/XAPIANDB";
    mkdir(".stub", 0755);
    mkdir(stubpath, 0755);
    ofstream stub(stubpathfile);
    TEST(stub.is_open());
    stub << "auto ../../" << get_database_path("apitest_simpledata") << '\n';
    stub << "auto ../../" << get_database_path("apitest_simpledata2") << '\n';
    stub.close();

    Xapian::doccount in_docs;
    {
	Xapian::Database indb(stubpath);
	in_docs = indb.get_doccount();
	indb.compact(stubpath);
    }

    Xapian::Database outdb(stubpath);

    TEST_EQUAL(in_docs, outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

static void
make_all_tables(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);
    db.add_spelling("foo");
    db.add_synonym("bar", "pub");
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

DEFINE_TESTCASE(compactmissingtables1, compact) {
    string a = get_database_path("compactmissingtables1a",
				 make_all_tables);
    string b = get_database_path("compactmissingtables1b",
				 make_missing_tables);

    string out = get_compaction_output_path("compactmissingtables1out");
    rm_rf(out);

    {
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(b));
	db.compact(out);
    }

    {
	Xapian::Database db(out);
	TEST_NOT_EQUAL(db.spellings_begin(), db.spellings_end());
	TEST_NOT_EQUAL(db.synonym_keys_begin(), db.synonym_keys_end());
	// FIXME: arrange for input b to not have a termlist table.
//	TEST_EXCEPTION(Xapian::FeatureUnavailableError, db.termlist_begin(1));
    }
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
DEFINE_TESTCASE(compactmergesynonym1, compact) {
    string a = get_database_path("compactmergesynonym1a",
				 make_all_tables);
    string b = get_database_path("compactmergesynonym1b",
				 make_all_tables2);

    string out = get_compaction_output_path("compactmergesynonym1out");
    rm_rf(out);

    {
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(b));
	db.compact(out);
    }

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
}

DEFINE_TESTCASE(compactempty1, compact) {
    string empty_dbpath = get_database_path(string());
    string outdbpath = get_compaction_output_path("compactempty1out");
    rm_rf(outdbpath);

    {
	// Compacting an empty database tried to divide by zero in 1.3.0.
	Xapian::Database db;
	db.add_database(Xapian::Database(empty_dbpath));
	db.compact(outdbpath);

	Xapian::Database outdb(outdbpath);
	TEST_EQUAL(outdb.get_doccount(), 0);
	dbcheck(outdb, 0, 0);
    }

    {
	// Check compacting two empty databases together.
	Xapian::Database db;
	db.add_database(Xapian::Database(empty_dbpath));
	db.add_database(Xapian::Database(empty_dbpath));
	db.compact(outdbpath);

	Xapian::Database outdb(outdbpath);
	TEST_EQUAL(outdb.get_doccount(), 0);
	dbcheck(outdb, 0, 0);
    }
}

DEFINE_TESTCASE(compactmultipass1, compact) {
    string outdbpath = get_compaction_output_path("compactmultipass1");
    rm_rf(outdbpath);

    string a = get_database_path("compactnorenumber1a", make_sparse_db,
				 "5-7 24 76 987 1023-1027 9999 !9999");
    string b = get_database_path("compactnorenumber1b", make_sparse_db,
				 "1027-1030");
    string c = get_database_path("compactnorenumber1c", make_sparse_db,
				 "1028-1040");
    string d = get_database_path("compactnorenumber1d", make_sparse_db,
				 "3000 999999 !999999");

    {
	Xapian::Database db;
	db.add_database(Xapian::Database(a));
	db.add_database(Xapian::Database(b));
	db.add_database(Xapian::Database(c));
	db.add_database(Xapian::Database(d));
	db.compact(outdbpath, Xapian::DBCOMPACT_MULTIPASS);
    }

    Xapian::Database outdb(outdbpath);
    dbcheck(outdb, 29, 1041);
}

// Test compacting to an fd.
// Chert doesn't support single file databases.
DEFINE_TESTCASE(compacttofd1, compact && !chert) {
    Xapian::Database indb(get_database("apitest_simpledata"));
    string outdbpath = get_compaction_output_path("compacttofd1out");
    rm_rf(outdbpath);

    int fd = open(outdbpath.c_str(), O_CREAT|O_RDWR|O_BINARY, 0666);
    TEST(fd != -1);
    indb.compact(fd);

    // Confirm that the fd was closed by Xapian.  Set errno first to workaround
    // a bug in Wine's msvcrt.dll which fails to set errno in this case:
    // https://bugs.winehq.org/show_bug.cgi?id=43902
    errno = EBADF;
    {
	MSVCIgnoreInvalidParameter invalid_fd_in_close_is_expected;
	TEST(close(fd) == -1);
	TEST_EQUAL(errno, EBADF);
    }

    Xapian::Database outdb(outdbpath);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Test compacting to an fd at at offset.
// Chert doesn't support single file databases.
DEFINE_TESTCASE(compacttofd2, compact && !chert) {
    Xapian::Database indb(get_database("apitest_simpledata"));
    string outdbpath = get_compaction_output_path("compacttofd2out");
    rm_rf(outdbpath);

    int fd = open(outdbpath.c_str(), O_CREAT|O_RDWR|O_BINARY, 0666);
    TEST(fd != -1);
    TEST(lseek(fd, 8192, SEEK_SET) == 8192);
    indb.compact(fd);

    // Confirm that the fd was closed by Xapian.  Set errno first to workaround
    // a bug in Wine's msvcrt.dll which fails to set errno in this case:
    // https://bugs.winehq.org/show_bug.cgi?id=43902
    errno = EBADF;
    {
	MSVCIgnoreInvalidParameter invalid_fd_in_close_is_expected;
	TEST(close(fd) == -1);
	TEST_EQUAL(errno, EBADF);
    }

    fd = open(outdbpath.c_str(), O_RDONLY|O_BINARY);
    TEST(fd != -1);

    // Test that the database wasn't just written to the start of the file.
    char buf[8192];
    size_t n = sizeof(buf);
    while (n) {
	ssize_t c = read(fd, buf, n);
	TEST(c > 0);
	for (const char * p = buf; p != buf + c; ++p) {
	    TEST(*p == 0);
	}
	n -= c;
    }

    TEST(lseek(fd, 8192, SEEK_SET) == 8192);
    Xapian::Database outdb(fd);

    TEST_EQUAL(indb.get_doccount(), outdb.get_doccount());
    dbcheck(outdb, outdb.get_doccount(), outdb.get_doccount());
}

// Regression test for bug fixed in 1.3.5.  If you compact a WritableDatabase
// with uncommitted changes, you get an inconsistent output.
//
// Chert doesn't support single file databases.
DEFINE_TESTCASE(compactsingle1, compact && writable && !chert) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_term("foo");
    doc.add_term("bar");
    doc.add_term("baz");
    db.add_document(doc);
    // Include a zero-length document as a regression test for a
    // Database::check() bug fixed in 1.4.7 (and introduced in 1.4.6).  Test it
    // here so we also have test coverage for compaction for such a document.
    Xapian::Document doc2;
    doc2.add_boolean_term("Kfoo");
    db.add_document(doc2);
    // Also test a completely empty document.
    db.add_document(Xapian::Document());

    string output = get_compaction_output_path("compactsingle1-out");
    // In 1.3.4, we would hang if the output file already existed, so check
    // that works.
    touch(output);

    TEST_EXCEPTION(Xapian::InvalidOperationError,
	db.compact(output, Xapian::DBCOMPACT_SINGLE_FILE));

    // Check the file wasn't removed by the failed attempt.
    TEST(file_exists(output));

    db.commit();
    db.compact(output, Xapian::DBCOMPACT_SINGLE_FILE);
    db.close();

    TEST_EQUAL(Xapian::Database::check(output, 0, &tout), 0);

    TEST_EQUAL(Xapian::Database(output).get_doccount(), 3);
}

// Regression test for bug fixed in 1.4.6.  Same as above, except not with
// a single file database!
DEFINE_TESTCASE(compact1, compact && writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_term("foo");
    doc.add_term("bar");
    doc.add_term("baz");
    db.add_document(doc);
    // Include a zero-length document as a regression test for a
    // Database::check() bug fixed in 1.4.7 (and introduced in 1.4.6).  Test it
    // here so we also have test coverage for compaction for such a document.
    Xapian::Document doc2;
    doc2.add_boolean_term("Kfoo");
    db.add_document(doc2);
    // Also test a completely empty document.
    db.add_document(Xapian::Document());

    string output = get_compaction_output_path("compact1-out");
    rm_rf(output);

    TEST_EXCEPTION(Xapian::InvalidOperationError,
	db.compact(output));

    db.commit();
    db.compact(output);
    db.close();

    TEST_EQUAL(Xapian::Database::check(output, 0, &tout), 0);

    TEST_EQUAL(Xapian::Database(output).get_doccount(), 3);
}
