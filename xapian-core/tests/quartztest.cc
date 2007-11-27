/* quartztest.cc: test of the Quartz backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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

// We have to use the deprecated Quartz::open() method.
#define XAPIAN_DEPRECATED(D) D

#include "safeerrno.h"

#include "unixcmds.h"
#include "testsuite.h"
#include "testutils.h"
#include <xapian/error.h>

#include "quartz_database.h"
#include "quartz_postlist.h"
#include "bcursor.h"
#include "quartz_utils.h"
#include "utils.h" // for mkdir for MSVC

#include <vector>
#include <algorithm>
using namespace std;

#include <sys/types.h>
#include "safesysstat.h"

static string tmpdir;

static void makedir(const string &filename)
{
    if (mkdir(filename, 0700) == -1 && errno != EEXIST) {
	FAIL_TEST("Couldn't create directory `" << filename << "' (" <<
		  strerror(errno) << ")");
    }
}

static void removedir(const string &filename)
{
    rm_rf(filename);
    struct stat buf;
    if (stat(filename, &buf) == 0 || errno != ENOENT) {
	FAIL_TEST("Failed to remove directory `" << filename << "' (" <<
		  strerror(errno) << ")");
    }
}

/// Test creating and opening of quartz databases
static bool test_create1()
{
    const string dbdir = tmpdir + "testdb_create1";
    removedir(dbdir);

    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db;
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048);

    // (7) db exists (create, overwrite)
    db = new QuartzDatabase(dbdir);
    TEST_EQUAL(db->get_doccount(), 0);
    db = 0; // Close the database - Cygwin can't delete a locked file...
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE_OR_OVERWRITE, 2048);
    TEST_EQUAL(db->get_doccount(), 0);
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);
    db->add_document(document_in);
    TEST_EQUAL(db->get_doccount(), 1);
    db->add_document(document_in);
    TEST_EQUAL(db->get_doccount(), 2);

    // (8) db exists with data (create, overwrite)
    db = new QuartzDatabase(dbdir);
    db = 0; // Close the database - Cygwin can't delete a locked file...
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE_OR_OVERWRITE, 2048);
    db->add_document(document_in);
    TEST_EQUAL(db->get_doccount(), 1);

    return true;
}

/** Test adding and deleting a document, and that flushing occurs in a
 *  sensible manner.
 */
static bool test_adddoc1()
{
    const string dbdir = tmpdir + "testdb_adddoc1";
    removedir(dbdir);

    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048);

    TEST_EQUAL(db->get_doccount(), 0);
    TEST_EQUAL(db->get_avlength(), 0);
    Xapian::Document document;
    Xapian::docid did;

    did = db->add_document(document);
    TEST_EQUAL(db->get_doccount(), 1);
    TEST_EQUAL(did, 1);
    TEST_EQUAL(db->get_avlength(), 0);
#if 0
    // FIXME: this test is flawed as the database is entitled to autoflush
    {
	Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_readonly = new QuartzDatabase(dbdir);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
#endif
    db->flush();
    {
	Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_readonly = new QuartzDatabase(dbdir);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    db->delete_document(did);
    TEST_EQUAL(db->get_doccount(), 0);
    TEST_EQUAL(db->get_avlength(), 0);
#if 0
    // FIXME: this test is flawed as the database is entitled to autoflush
    {
	Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_readonly = new QuartzDatabase(dbdir);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
#endif
    db->flush();
    {
	Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_readonly = new QuartzDatabase(dbdir);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    did = db->add_document(document);
    TEST_EQUAL(db->get_doccount(), 1);
    TEST_EQUAL(did, 2);
    TEST_EQUAL(db->get_avlength(), 0);

    db->flush();

    return true;
}

/// Test packing integers into strings
static bool test_packint1()
{
    TEST_EQUAL(pack_uint(0u), string("\0", 1));
    TEST_EQUAL(pack_uint(1u), string("\001", 1));
    TEST_EQUAL(pack_uint(127u), string("\177", 1));
    TEST_EQUAL(pack_uint(128u), string("\200\001", 2));
    TEST_EQUAL(pack_uint(0xffffu), string("\377\377\003", 3));
    TEST_EQUAL(pack_uint(0xffffffffu), string("\377\377\377\377\017", 5));

    return true;
}

/// Test packing integers into strings and unpacking again
static bool test_packint2()
{
    string foo;

    foo += pack_uint(3u);
    foo += pack_uint(12475123u);
    foo += pack_uint(128u);
    foo += pack_uint(0xffffffffu);
    foo += pack_uint(127u);
    foo += pack_uint(0u);
    foo += pack_uint(0xffffffffu);
    foo += pack_uint(0u);
    foo += pack_uint(82134u);

    const char * p = foo.data();
    om_uint32 result;

    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 3u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 12475123u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 128u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffffu);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 127u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffffu);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 82134u);

    return true;
}

/// Test the sort preserving packing operations
static bool test_packint3()
{
    string foo;

    vector<unsigned int> ints;
    vector<string> strings;

    ints.push_back(3u);
    ints.push_back(12475123u);
    ints.push_back(128u);
    ints.push_back(0xffffffffu);
    ints.push_back(127u);
    ints.push_back(256u);
    ints.push_back(254u);
    ints.push_back(255u);
    ints.push_back(0u);
    ints.push_back(0xffffffffu);
    ints.push_back(0u);
    ints.push_back(82134u);

    vector<unsigned int>::const_iterator i;
    vector<string>::const_iterator j;
    for (i = ints.begin(); i != ints.end(); i++) {
	foo += pack_uint_preserving_sort(*i);
	strings.push_back(pack_uint_preserving_sort(*i));
    }

    const char * p = foo.data();
    om_uint32 result;
    i = ints.begin();
    while (p != foo.data() + foo.size()) {
	TEST(i != ints.end());
	TEST(p != 0);
	TEST(unpack_uint_preserving_sort(&p, foo.data() + foo.size(), &result));
	TEST_EQUAL(result, *i);
	i++;
    }
    TEST(p != 0);
    TEST(i == ints.end());

    sort(ints.begin(), ints.end());
    sort(strings.begin(), strings.end());

    for (i = ints.begin(), j = strings.begin();
	 i != ints.end() && j != strings.end();
	 i++, j++) {
	TEST(pack_uint_preserving_sort(*i) == *j);
    }

    return true;
}

/// Test unpacking integers from strings
static bool test_unpackint1()
{
    string foo;
    const char *p;
    om_uint32 result;
    bool success;

    p = foo.data();
    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    foo = string("\000\002\301\001", 4);
    result = 1;
    p = foo.data();

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0);
    TEST_EQUAL(p, reinterpret_cast<const char *>(foo.data() + 1));

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 2);
    TEST_EQUAL(p, foo.data() + 2);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 65 + 128);
    TEST_EQUAL(p, foo.data() + 4);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    foo = string("\377\377\377\377\017\377\377\377\377\020\007\200\200\200\200\200\200\200\000\200\200\200\200\200\200\001\200\200\200\200\200\200", 32);
    result = 1;
    p = foo.data();

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0xffffffff);
    TEST_EQUAL(p, foo.data() + 5);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 10);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 7);
    TEST_EQUAL(p, foo.data() + 11);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 19);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 26);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    return true;
}

/// Test that write locks work
static bool test_writelock1()
{
    const string dbname = tmpdir + "writelock1";
    removedir(dbname);

    Xapian::WritableDatabase writer = Xapian::Quartz::open(dbname, Xapian::DB_CREATE);
    TEST_EXCEPTION(Xapian::DatabaseLockError,
	Xapian::WritableDatabase writer2 = Xapian::Quartz::open(dbname, Xapian::DB_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseLockError,
	Xapian::WritableDatabase writer2 = Xapian::Quartz::open(dbname, Xapian::DB_CREATE_OR_OVERWRITE));
    // Xapian::DB_CREATE would fail with DatabaseCreateError
    TEST_EXCEPTION(Xapian::DatabaseLockError,
	Xapian::WritableDatabase writer2 = Xapian::Quartz::open(dbname, Xapian::DB_CREATE_OR_OPEN));
    return true;
}

/// Test pack_string_preserving_sort() etc
static bool test_packstring1()
{
    string before;
    string after;
    string packed;
    const char *src;
    const char *src_end;

    before = "foo";
    packed = pack_string_preserving_sort(before);
    if (verbose) { tout << "packed = `" << packed << "'\n"; }
    src = packed.data();
    src_end = src + packed.length();
    TEST(unpack_string_preserving_sort(&src, src_end, after));
    TEST(src == src_end);
    TEST(before == after);

    before = "";
    packed = pack_string_preserving_sort(before);
    if (verbose) { tout << "packed = `" << packed << "'\n"; }
    src = packed.data();
    src_end = src + packed.length();
    TEST(unpack_string_preserving_sort(&src, src_end, after));
    TEST(src == src_end);
    TEST(before == after);

    before = "length_8";
    packed = pack_string_preserving_sort(before);
    if (verbose) { tout << "packed = `" << packed << "'\n"; }
    src = packed.data();
    src_end = src + packed.length();
    TEST(unpack_string_preserving_sort(&src, src_end, after));
    TEST(src == src_end);
    TEST(before == after);

    before = "Quite a long string, really";
    packed = pack_string_preserving_sort(before);
    if (verbose) { tout << "packed = `" << packed << "'\n"; }
    src = packed.data();
    src_end = src + packed.length();
    TEST(unpack_string_preserving_sort(&src, src_end, after));
    TEST(src == src_end);
    TEST(before == after);

    //        1234567812345678123456781234567812345678
    before = "Quite a long string - multiple of eight.";
    packed = pack_string_preserving_sort(before);
    if (verbose) { tout << "packed = `" << packed << "'\n"; }
    src = packed.data();
    src_end = src + packed.length();
    TEST(unpack_string_preserving_sort(&src, src_end, after));
    TEST(src == src_end);
    TEST(before == after);

    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================

/// The lists of tests to perform
test_desc tests[] = {
    {"create1",		test_create1},
    {"adddoc1",		test_adddoc1},
    {"packint1",	test_packint1},
    {"packint2",	test_packint2},
    {"packint3",	test_packint3},
    {"unpackint1",	test_unpackint1},
    {"writelock1",	test_writelock1},
    {"packstring1",	test_packstring1},
    {0, 0}
};

int main(int argc, char **argv)
{
    tmpdir = ".quartztmp";
    removedir(tmpdir);
    makedir(tmpdir);
    tmpdir += '/';
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
}
