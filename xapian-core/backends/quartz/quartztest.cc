/* quartztest.cc: test of the Quartz backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include "testsuite.h"
#include "testutils.h"
#include "om/omerror.h"

#include "quartz_database.h"
#include "quartz_table.h"
#include "quartz_table_entries.h"
#include "quartz_utils.h"

#include "om/autoptr.h"

#include "unistd.h"

/// Check the values returned by a table containing key/tag "hello"/"world"
static void check_table_values_hello(const QuartzDiskTable & table,
				     std::string world)
{
    QuartzDbKey key;
    QuartzDbTag tag;

    // Check exact reads
    key.value = "hello";
    tag.value = "foo";
    TEST(table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, world);

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "hello");
    TEST_EQUAL(tag.value, world);

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "hello");
    TEST_EQUAL(tag.value, world);

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "foo");
#endif
}

/// Check the values returned by a table containing no key/tag pairs
static void check_table_values_empty(const QuartzDiskTable & table)
{
    QuartzDbKey key;
    QuartzDbTag tag;

    // Check exact reads
    key.value = "hello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "foo");
#endif
}

/// Test making and playing with a QuartzDiskTable
static bool test_disktable1()
{
    unlink("./test_dbtable1_data_1");
    unlink("./test_dbtable1_data_2");
    {
	QuartzDiskTable table0("./test_dbtable1_", true);
	TEST_EXCEPTION(OmOpeningError, table0.open());
    }
    QuartzDiskTable table2("./test_dbtable1_", false);
    table2.open();
    QuartzDiskTable table1("./test_dbtable1_", true);
    table1.open();


    QuartzRevisionNumber rev1 = table1.get_open_revision_number();
    QuartzRevisionNumber rev2 = table2.get_open_revision_number();

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_EQUAL(rev2, table2.get_open_revision_number());
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    std::map<QuartzDbKey, QuartzDbTag *> newentries;

    // Check adding no entries
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check adding some entries
    QuartzDbKey key;
    QuartzDbTag tag;
    key.value = "hello";
    tag.value = "world";
    newentries[key] = &tag;
    
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");

    // Check adding the same entries
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");


    // Check adding an entry with a null key
    key.value = "";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
#ifdef MUS_DEBUG
    TEST_EXCEPTION(OmAssertionError, table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment()));
#endif

    // Check changing an entry, to a null tag
    newentries.clear();
    key.value = "hello";
    tag.value = "";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);
    
    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "");

    // Check deleting an entry
    newentries.clear();
    key.value = "hello";
    newentries[key] = 0;
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check the entries in the table
    check_table_values_empty(table1);
    check_table_values_empty(table2);
    
    // Check get_nearest_entry when looking for something between two elements
    newentries.clear();
    key.value = "hello";
    tag.value = "world";
    newentries[key] = &tag;
    key.value = "whooo";
    tag.value = "world";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries,
			table1.get_latest_revision_number().increment()));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number().increment());

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 2);

    // Check the entries in the table
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");
    
    return true;
}

/// Test making and playing with a QuartzTableEntries
static bool test_tableentries1()
{
    QuartzTableEntries entries;

    QuartzDbKey key1;

#ifdef MUS_DEBUG
    key1.value="";
    TEST_EXCEPTION(OmAssertionError, entries.have_entry(key1));
    {
	AutoPtr<QuartzDbTag> tagptr(new QuartzDbTag);
	tagptr->value = "bar";
	TEST_EXCEPTION(OmAssertionError, entries.set_tag(key1, tagptr));
    }
    TEST_EXCEPTION(OmAssertionError, entries.have_entry(key1));
    TEST_EXCEPTION(OmAssertionError, entries.get_tag(key1));
#endif

    key1.value="foo";
    TEST(!entries.have_entry(key1));
    {
	AutoPtr<QuartzDbTag> tagptr(new QuartzDbTag);
	tagptr->value = "bar";
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST_NOT_EQUAL(entries.get_tag(key1), 0);
    TEST_EQUAL(entries.get_tag(key1)->value, "bar");
    {
	AutoPtr<QuartzDbTag> tagptr(0);
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST_EQUAL(entries.get_tag(key1), 0);

    return true;
}

/// Test making and playing with a QuartzBufferedTable
static bool test_bufftable1()
{
    unlink("./test_bufftable1_data_1");
    unlink("./test_bufftable1_data_2");
    QuartzDiskTable disktable1("./test_bufftable1_", false);
    QuartzBufferedTable bufftable1(&disktable1);
    disktable1.open();

    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbKey key;
    key.value = "foo1";

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    TEST_EQUAL((void *)bufftable1.get_tag(key), 0);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbTag * tag = bufftable1.get_or_make_tag(key);
    TEST_NOT_EQUAL(tag, 0);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    QuartzRevisionNumber new_revision = disktable1.get_latest_revision_number();
    new_revision.increment();
    TEST(bufftable1.apply(new_revision));
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    key.value = "bar";
    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    key.value = "bar2";
    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    new_revision.increment();
    TEST(bufftable1.apply(new_revision));

    TEST_EQUAL(disktable1.get_entry_count(), 2);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    OmSettings settings;
    system("rm -fr .testdb_open1");
    settings.set("quartz_dir", ".testdb_open1");

    TEST_EXCEPTION(OmOpeningError, QuartzDatabase database_0(settings));
    system("mkdir .testdb_open1");
    QuartzWritableDatabase database_w(settings);
    QuartzDatabase database_r(settings);
    return true;
}

/** Test adding and deleting a document, and that flushing occurs in a
 *  sensible manner.
 */
static bool test_adddoc1()
{
    OmSettings settings;
    system("rm -fr .testdb_adddoc1");
    system("mkdir .testdb_adddoc1");
    settings.set("quartz_dir", ".testdb_adddoc1");
    settings.set("quartz_logfile", "log");
    QuartzWritableDatabase database(settings);

    database.begin_session(0);
    TEST_EQUAL(database.get_doccount(), 0);
    OmDocumentContents document;
    om_docid did;

    did = database.add_document(document);
    TEST_EQUAL(database.get_doccount(), 1);
    settings.set("quartz_logfile", "log_ro");
    {
	QuartzDatabase db_readonly(settings);
	TEST_EQUAL(db_readonly.get_doccount(), 0);
    }
    database.flush();
    {
	QuartzDatabase db_readonly(settings);
	TEST_EQUAL(db_readonly.get_doccount(), 1);
    }

    database.delete_document(did);
    TEST_EQUAL(database.get_doccount(), 0);
    {
	QuartzDatabase db_readonly(settings);
	TEST_EQUAL(db_readonly.get_doccount(), 1);
    }
    database.flush();
    {
	QuartzDatabase db_readonly(settings);
	TEST_EQUAL(db_readonly.get_doccount(), 0);
    }
    database.flush();
    database.end_session();

    return true;
}

/** Test adding a document, and checking that it got added correctly.
 */
static bool test_adddoc2()
{
    OmSettings settings;
    system("rm -fr .testdb_adddoc2");
    system("mkdir .testdb_adddoc2");
    settings.set("quartz_dir", ".testdb_adddoc2");
    settings.set("quartz_logfile", "log");

    om_docid did;
    OmDocumentContents document_in;
    {
	QuartzWritableDatabase database(settings);
	TEST_EQUAL(database.get_doccount(), 0);
	did = database.add_document(document_in);
	TEST_EQUAL(database.get_doccount(), 1);
    }

    {
	settings.set("quartz_logfile", "log_ro");
	QuartzDatabase database(settings);
	OmDocumentContents document_out = database.get_document(did);

	TEST_EQUAL(document_in.data.value, document_out.data.value);
	TEST(document_in.keys.size() == document_out.keys.size());
	TEST(document_in.terms.size() == document_out.terms.size());
    }

    return true;
}

/// Test packing integers into strings
static bool test_packint1()
{
    TEST_EQUAL(pack_uint32(0), std::string("\000", 1));
    TEST_EQUAL(pack_uint32(1), std::string("\001", 1));
    TEST_EQUAL(pack_uint32(127), std::string("\177", 1));
    TEST_EQUAL(pack_uint32(128), std::string("\200\001", 2));
    TEST_EQUAL(pack_uint32(0xffff), std::string("\377\377\003", 3));
    TEST_EQUAL(pack_uint32(0xffffffff), std::string("\377\377\377\377\017", 5));

    return true;
}

/// Test packing integers into strings and unpacking again
static bool test_packint2()
{
    std::string foo;

    foo += pack_uint32(3);
    foo += pack_uint32(12475123);
    foo += pack_uint32(128);
    foo += pack_uint32(0xffffffff);
    foo += pack_uint32(127);
    foo += pack_uint32(0);
    foo += pack_uint32(0xffffffff);
    foo += pack_uint32(0);
    foo += pack_uint32(82134);

    const char * p = foo.data();
    om_uint32 result;

    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 3);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 12475123);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 128);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffff);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 127);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffff);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0);
    TEST(unpack_uint32(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 82134);

    return true;
}

/// Test unpacking integers from strings
static bool test_unpackint1()
{
    std::string foo;
    const char *p;
    om_uint32 result;
    bool success;
    
    p = foo.data();
    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data());

    foo = std::string("\000\002\301\001", 4);
    result = 1;
    p = foo.data();

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0);
    TEST_EQUAL((void *)p, (void *)(foo.data() + 1));

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 2);
    TEST_EQUAL(p, foo.data() + 2);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 65 + 128);
    TEST_EQUAL(p, foo.data() + 4);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 4);

    foo = std::string("\377\377\377\377\017\377\377\377\377\020\007\200\200\200\200\200\200\200\000\200\200\200\200\200\200\001\200\200\200\200\200\200", 32);
    result = 1;
    p = foo.data();

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0xffffffff);
    TEST_EQUAL(p, foo.data() + 5);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 10);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 7);
    TEST_EQUAL(p, foo.data() + 11);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 19);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 26);

    success = unpack_uint32(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 32);

    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================

/// The lists of tests to perform
test_desc tests[] = {
    {"quartzdisktable1",	test_disktable1},
    {"quartztableentries1",	test_tableentries1},
    {"quartzbufftable1",	test_bufftable1},
    {"quartzopen1",		test_open1},
    {"quartzadddoc1",		test_adddoc1},
    {"quartzadddoc2",		test_adddoc2},
    {"quartzpackint1",		test_packint1},
    {"quartzpackint2",		test_packint2},
    {"quartzunpackint1",	test_unpackint1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
