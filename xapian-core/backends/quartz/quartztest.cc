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
#include "quartz_postlist.h"
#include "quartz_table.h"
#include "quartz_table_entries.h"
#include "quartz_utils.h"

#include "database_builder.h"

#include "om/autoptr.h"

#include "unistd.h"
#include <vector>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static std::string tmpdir;

/// Get the size of the given file in bytes.
static int get_filesize(std::string filename)
{
    struct stat buf;
    int result = stat(filename.c_str(), &buf);
    if (result) return -1;
    return buf.st_size;
}

static void deletedir(std::string filename)
{
    system(("rm -fr " + filename).c_str());
}

static void makedir(std::string filename)
{
    system(("mkdir " + filename).c_str());
}

/// Check the values returned by a table containing key/tag "hello"/"world"
static void check_table_values_hello(QuartzDiskTable & table,
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
    
    AutoPtr<QuartzCursor> cursor(table.cursor_get());
#ifdef MUS_DEBUG
    key.value = "";
    TEST_EXCEPTION(OmAssertionError, cursor->find_entry(key));
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "hello");
    TEST_EQUAL(cursor->current_tag.value, world);

    key.value = "jello";
    tag.value = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "hello");
    TEST_EQUAL(cursor->current_tag.value, world);

    key.value = "bello";
    tag.value = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "");
    TEST_EQUAL(cursor->current_tag.value, "");
}

/// Check the values returned by a table containing no key/tag pairs
static void check_table_values_empty(QuartzDiskTable & table)
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
    
    AutoPtr<QuartzCursor> cursor(table.cursor_get());
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, cursor->find_entry(key));
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "");
    TEST_EQUAL(cursor->current_tag.value, "");

    key.value = "jello";
    tag.value = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "");
    TEST_EQUAL(cursor->current_tag.value, "");

    key.value = "bello";
    tag.value = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "");
    TEST_EQUAL(cursor->current_tag.value, "");
}

static void unlink_table(const std::string & path)
{
    unlink((path + "DB").c_str());
    unlink((path + "baseA").c_str());
    unlink((path + "baseB").c_str());
    unlink((path + "bitmapA").c_str());
    unlink((path + "bitmapB").c_str());
}

/// Test making and playing with a QuartzDiskTable
static bool test_disktable1()
{
    unlink_table(tmpdir + "test_disktable1_");
    {
	QuartzDiskTable table0(tmpdir + "test_disktable1_", true, 0);
	TEST_EXCEPTION(OmOpeningError, table0.open());
	TEST_EXCEPTION(OmOpeningError, table0.open(10));
    }
    QuartzDiskTable table2(tmpdir + "test_disktable1_", false, 8192);
    table2.open();
    QuartzDiskTable table1(tmpdir + "test_disktable1_", true, 0);
    table1.open();


    quartz_revision_number_t rev1 = table1.get_open_revision_number();
    quartz_revision_number_t rev2 = table2.get_open_revision_number();

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_EQUAL(rev2, table2.get_open_revision_number());
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check adding no entries
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.apply(table1.get_latest_revision_number() + 1));
    table2.apply(table2.get_latest_revision_number() + 1);

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
    
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entry(key, &tag));
    table2.set_entry(key, &tag);
    table2.apply(table2.get_latest_revision_number() + 1);

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
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entry(key, &tag));
    table2.set_entry(key, &tag);
    table2.apply(table2.get_latest_revision_number() + 1);

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
#ifdef MUS_DEBUG
    TEST_EXCEPTION(OmAssertionError, table1.set_entry(key, &tag));
    TEST_EXCEPTION(OmAssertionError, table2.set_entry(key, &tag));
#else
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entry(key, &tag));
#endif

    // Check changing an entry, to a null tag
    key.value = "hello";
    tag.value = "";
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entry(key, &tag));
    table2.set_entry(key, &tag);
    table2.apply(table2.get_latest_revision_number() + 1);

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
    key.value = "hello";
    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entry(key, 0));
    table2.set_entry(key, 0);
    table2.apply(table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check the entries in the table
    check_table_values_empty(table1);
    check_table_values_empty(table2);
    
    // Check find_entry when looking for something between two elements
    key.value = "hello";
    tag.value = "world";
    table2.set_entry(key, &tag);
    key.value = "whooo";
    tag.value = "world";
    table2.set_entry(key, &tag);

    table2.apply(table2.get_latest_revision_number() + 1);

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

/// Test making and playing with a QuartzDiskTable
static bool test_disktable2()
{
    unlink_table(tmpdir + "test_disktable2_");

    QuartzDiskTable table(tmpdir + "test_disktable2_", false, 8192);
    table.open();
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.apply(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.apply(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.apply(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    QuartzDbKey key;
    key.value = "foo";
    QuartzDbTag tag;
    tag.value = "bar";

    table.set_entry(key, &tag);
    table.apply(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 8192);

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
    unlink_table(tmpdir + "test_bufftable1_");
    QuartzDiskTable disktable1(tmpdir + "test_bufftable1_", false, 8192);
    disktable1.open();
    QuartzBufferedTable bufftable1(&disktable1);

    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbKey key;
    key.value = "foo1";

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbTag * tag = bufftable1.get_or_make_tag(key);
    TEST_NOT_EQUAL(tag, 0);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    quartz_revision_number_t new_revision =
	    disktable1.get_latest_revision_number() + 1;
    bufftable1.apply(new_revision);
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

    new_revision += 1;
    bufftable1.apply(new_revision);

    TEST_EQUAL(disktable1.get_entry_count(), 2);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    return true;
}

/// Test making and playing with a QuartzBufferedTable
static bool test_bufftable2()
{
    unlink_table(tmpdir + "test_bufftable2_");
    quartz_revision_number_t new_revision;
    quartz_revision_number_t old_revision;
    {
	// Open table and add a few documents
	QuartzDiskTable disktable(tmpdir + "test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 0);
	TEST_EQUAL(bufftable.get_entry_count(), 0);

	QuartzDbKey key;

	key.value = "foo1";
	bufftable.get_or_make_tag(key)->value = "bar1";
	key.value = "foo2";
	bufftable.get_or_make_tag(key)->value = "bar2";
	key.value = "foo3";
	bufftable.get_or_make_tag(key)->value = "bar3";

	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.apply(new_revision);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	QuartzDiskTable disktable(tmpdir + "test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	QuartzDbKey key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key.value = "foo";
	AutoPtr<QuartzCursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "");
	TEST_EQUAL(cursor->current_tag.value, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo1");
	TEST_EQUAL(cursor->current_tag.value, "bar1");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo2");
	TEST_EQUAL(cursor->current_tag.value, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo3");
	TEST_EQUAL(cursor->current_tag.value, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	// Add a new tag
	key.value = "foo25";
	bufftable.get_or_make_tag(key)->value = "bar25";
	old_revision = new_revision;
	new_revision += 1;
	bufftable.apply(new_revision);

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Open old revision
	QuartzDiskTable disktable(tmpdir + "test_bufftable2_", false, 8192);
	TEST(disktable.open(old_revision));
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	QuartzDbKey key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(old_revision, disktable.get_open_revision_number());

	// Add a new tag
	key.value = "foo26";
	bufftable.get_or_make_tag(key)->value = "bar26";
	new_revision += 1;
	bufftable.apply(new_revision);

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	// Add another new tag, but don't apply this one.
	key.value = "foo37";
	bufftable.get_or_make_tag(key)->value = "bar37";
	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 5);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	QuartzDiskTable disktable(tmpdir + "test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	QuartzDbKey key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key.value = "foo";
	AutoPtr<QuartzCursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "");
	TEST_EQUAL(cursor->current_tag.value, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo1");
	TEST_EQUAL(cursor->current_tag.value, "bar1");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo2");
	TEST_EQUAL(cursor->current_tag.value, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo26");
	TEST_EQUAL(cursor->current_tag.value, "bar26");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo3");
	TEST_EQUAL(cursor->current_tag.value, "bar3");

	cursor->next();
	TEST(cursor->after_end());
    }
    {
	// Check that opening a nonexistant revision returns false (but doesn't
	// throw an exception).
	QuartzDiskTable disktable(tmpdir + "test_bufftable2_", false, 8192);
	TEST(!disktable.open(new_revision + 10));
    }

    return true;
}

/// Test making and playing with a QuartzBufferedTable
static bool test_bufftable3()
{
    unlink_table(tmpdir + "test_bufftable3_");
    quartz_revision_number_t new_revision;
    {
	// Open table and add a couple of documents
	QuartzDiskTable disktable(tmpdir + "test_bufftable3_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 0);
	TEST_EQUAL(bufftable.get_entry_count(), 0);

	QuartzDbKey key;

	key.value = "foo1";
	bufftable.get_or_make_tag(key)->value = "bar1";
	bufftable.write();

	key.value = "foo2";
	bufftable.get_or_make_tag(key)->value = "bar2";
	bufftable.cancel();

	key.value = "foo3";
	bufftable.get_or_make_tag(key)->value = "bar3";

	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.apply(new_revision);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	QuartzDiskTable disktable(tmpdir + "test_bufftable3_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 1);
	TEST_EQUAL(bufftable.get_entry_count(), 1);

	QuartzDbKey key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key.value = "foo";
	AutoPtr<QuartzCursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "");
	TEST_EQUAL(cursor->current_tag.value, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo3");
	TEST_EQUAL(cursor->current_tag.value, "bar3");

	cursor->next();
	TEST(cursor->after_end());
    }
    return true;
}

/// Test QuartzCursors
static bool test_cursor1()
{
    unlink_table(tmpdir + "test_cursor1_");

    QuartzDbKey key;

    // Open table and put stuff in it.
    QuartzDiskTable disktable1(tmpdir + "test_cursor1_", false, 8192);
    disktable1.open();
    QuartzBufferedTable bufftable1(&disktable1);

    key.value = "foo1";
    bufftable1.get_or_make_tag(key)->value = "bar1";
    key.value = "foo2";
    bufftable1.get_or_make_tag(key)->value = "bar2";
    key.value = "foo3";
    bufftable1.get_or_make_tag(key)->value = "bar3";
    quartz_revision_number_t new_revision = disktable1.get_latest_revision_number();
    new_revision += 1;
    bufftable1.apply(new_revision);

    QuartzTable * table = &disktable1;
    int count = 2;

    while(count != 0) {
	key.value = "foo25";
	AutoPtr<QuartzCursor> cursor(table->cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "foo2");
	TEST_EQUAL(cursor->current_tag.value, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo3");
	TEST_EQUAL(cursor->current_tag.value, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	key.value = "foo";
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "");
	TEST_EQUAL(cursor->current_tag.value, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo1");
	TEST_EQUAL(cursor->current_tag.value, "bar1");

	key.value = "foo2";
	TEST(cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key.value, "foo2");
	TEST_EQUAL(cursor->current_tag.value, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key.value, "foo3");
	TEST_EQUAL(cursor->current_tag.value, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	table = &bufftable1;
	count -= 1;
    }

    // Test cursors when we have unapplied changes
    key.value = "foo25";
    bufftable1.get_or_make_tag(key)->value = "bar25";

    key.value = "foo26";
    bufftable1.delete_tag(key);
    key.value = "foo1";
    bufftable1.delete_tag(key);

    key.value = "foo25";
    AutoPtr<QuartzCursor> cursor(disktable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    key.value = "foo25";
    cursor.reset(bufftable1.cursor_get());
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo25");
    TEST_EQUAL(cursor->current_tag.value, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    key.value = "foo26";
    cursor.reset(bufftable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo25");
    TEST_EQUAL(cursor->current_tag.value, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    key.value = "foo2";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo25");
    TEST_EQUAL(cursor->current_tag.value, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    cursor->next();
    TEST(cursor->after_end());

    key.value = "foo1";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "");
    TEST_EQUAL(cursor->current_tag.value, "");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo25");
    TEST_EQUAL(cursor->current_tag.value, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    new_revision += 1;
    bufftable1.apply(new_revision);

    cursor.reset(bufftable1.cursor_get());
    key.value = "foo2";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    key.value = "foo24";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    key.value = "foo25";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo25");
    TEST_EQUAL(cursor->current_tag.value, "bar25");

    key.value = "foo24";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    key.value = "foo25";
    bufftable1.delete_tag(key);

    key.value = "foo25";
    cursor.reset(bufftable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, "foo2");
    TEST_EQUAL(cursor->current_tag.value, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key.value, "foo3");
    TEST_EQUAL(cursor->current_tag.value, "bar3");

    return true;
}

/// Regression test for cursors
static bool test_cursor2()
{
    unlink_table(tmpdir + "test_cursor2_");

    QuartzDbKey key;

    // Open table and put stuff in it.
    QuartzDiskTable disktable1(tmpdir + "test_cursor2_", false, 8192);
    disktable1.open();
    QuartzBufferedTable bufftable1(&disktable1);

    std::string key1 = "a";
    std::string tag1 = std::string(2036, '\x00');
    std::string key2 = "c";
    std::string tag2 = "bar2";
    std::string searchkey = "b";

    key.value = key1;
    bufftable1.get_or_make_tag(key)->value = tag1;
    key.value = key2;
    bufftable1.get_or_make_tag(key)->value = tag2;
    quartz_revision_number_t new_revision = disktable1.get_latest_revision_number();
    new_revision += 1;
    bufftable1.apply(new_revision);

    AutoPtr<QuartzCursor> cursor(disktable1.cursor_get());

    key.value = searchkey;
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key.value, key1);
    TEST_EQUAL(cursor->current_tag.value, tag1);

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    OmSettings settings;
    deletedir(tmpdir + "testdb_open1");
    settings.set("quartz_dir", tmpdir + "testdb_open1");
    settings.set("quartz_logfile", tmpdir + "log_open1");
    settings.set("backend", "quartz");

    TEST_EXCEPTION(OmOpeningError,
		   RefCntPtr<Database> database_0 =
		   DatabaseBuilder::create(settings, true));

    makedir(tmpdir + "testdb_open1");
    RefCntPtr<Database> database_w =
	    DatabaseBuilder::create(settings, false);
    RefCntPtr<Database> database_r =
	    DatabaseBuilder::create(settings, true);
    return true;
}

/** Test adding and deleting a document, and that flushing occurs in a
 *  sensible manner.
 */
static bool test_adddoc1()
{
    OmSettings settings;
    deletedir(tmpdir + "testdb_adddoc1");
    makedir(tmpdir + "testdb_adddoc1");
    settings.set("quartz_dir", tmpdir + "testdb_adddoc1");
    settings.set("quartz_logfile", tmpdir + "log_adddoc1");
    settings.set("backend", "quartz");

    RefCntPtr<Database> database = DatabaseBuilder::create(settings, false);

    database->begin_session(0);
    TEST_EQUAL(database->get_doccount(), 0);
    TEST_EQUAL(database->get_avlength(), 0);
    OmDocumentContents document;
    om_docid did;

    did = database->add_document(document);
    TEST_EQUAL(database->get_doccount(), 1);
    TEST_EQUAL(did, 1);
    TEST_EQUAL(database->get_avlength(), 0);
    settings.set("quartz_logfile", tmpdir + "log_adddoc1_ro");
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
    database->flush();
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    database->delete_document(did);
    TEST_EQUAL(database->get_doccount(), 0);
    TEST_EQUAL(database->get_avlength(), 0);
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
    database->flush();
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    did = database->add_document(document);
    TEST_EQUAL(database->get_doccount(), 1);
    TEST_EQUAL(did, 2);
    TEST_EQUAL(database->get_avlength(), 0);

    database->flush();
    database->end_session();

    return true;
}

/** Test adding a document, and checking that it got added correctly.
 */
static bool test_adddoc2()
{
    OmSettings settings;
    deletedir(tmpdir + "testdb_adddoc2");
    makedir(tmpdir + "testdb_adddoc2");
    settings.set("quartz_dir", tmpdir + "testdb_adddoc2");
    settings.set("quartz_logfile", tmpdir + "log_adddoc2");
    settings.set("backend", "quartz");

    om_docid did;
    OmDocumentContents document_in;
    document_in.data.value = "Foobar rising";
    document_in.keys[7] = OmKey("Key7");
    document_in.keys[13] = OmKey("Key13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);

    OmDocumentContents document_in2;
    document_in2.data.value = "Foobar falling";
    document_in2.add_posting("foobar", 1);
    document_in2.add_posting("falling", 2);
    {
	RefCntPtr<Database> database = DatabaseBuilder::create(settings, false);

	TEST_EQUAL(database->get_doccount(), 0);
	TEST_EQUAL(database->get_avlength(), 0);

	did = database->add_document(document_in);
	TEST_EQUAL(database->get_doccount(), 1);
	TEST_EQUAL(database->get_avlength(), 3);

	TEST_EQUAL(database->get_termfreq("foobar"), 1);
	TEST_EQUAL(database->get_collection_freq("foobar"), 2);
	TEST_EQUAL(database->get_termfreq("rising"), 1);
	TEST_EQUAL(database->get_collection_freq("rising"), 1);
	TEST_EQUAL(database->get_termfreq("falling"), 0);
	TEST_EQUAL(database->get_collection_freq("falling"), 0);

	om_docid did2 = database->add_document(document_in2);
	TEST_EQUAL(database->get_doccount(), 2);
	TEST_NOT_EQUAL(did, did2);
	TEST_EQUAL(database->get_avlength(), 5.0/2.0);

	TEST_EQUAL(database->get_termfreq("foobar"), 2);
	TEST_EQUAL(database->get_collection_freq("foobar"), 3);
	TEST_EQUAL(database->get_termfreq("rising"), 1);
	TEST_EQUAL(database->get_collection_freq("rising"), 1);
	TEST_EQUAL(database->get_termfreq("falling"), 1);
	TEST_EQUAL(database->get_collection_freq("falling"), 1);

	database->delete_document(did);
	TEST_EQUAL(database->get_doccount(), 1);
	TEST_EQUAL(database->get_avlength(), 2);

	TEST_EQUAL(database->get_termfreq("foobar"), 1);
	TEST_EQUAL(database->get_collection_freq("foobar"), 1);
	TEST_EQUAL(database->get_termfreq("rising"), 0);
	TEST_EQUAL(database->get_collection_freq("rising"), 0);
	TEST_EQUAL(database->get_termfreq("falling"), 1);
	TEST_EQUAL(database->get_collection_freq("falling"), 1);

	did = database->add_document(document_in);
	TEST_EQUAL(database->get_doccount(), 2);
	TEST_EQUAL(database->get_avlength(), 5.0/2.0);

	TEST_EQUAL(database->get_termfreq("foobar"), 2);
	TEST_EQUAL(database->get_collection_freq("foobar"), 3);
	TEST_EQUAL(database->get_termfreq("rising"), 1);
	TEST_EQUAL(database->get_collection_freq("rising"), 1);
	TEST_EQUAL(database->get_termfreq("falling"), 1);
	TEST_EQUAL(database->get_collection_freq("falling"), 1);
    }

    {
	settings.set("quartz_logfile", tmpdir + "log_adddoc2_ro");
	RefCntPtr<Database> database = DatabaseBuilder::create(settings, true);
	OmDocumentContents document_out = database->get_document(did);

	TEST_EQUAL(document_in.data.value, document_out.data.value);
	TEST_EQUAL(document_in.keys.size(), document_out.keys.size());
	TEST_EQUAL(document_in.terms.size(), document_out.terms.size());

	{
	    OmDocumentContents::document_keys::const_iterator i,j;
	    for (i = document_in.keys.begin(), j = document_out.keys.begin();
		 i != document_in.keys.end();
		 i++, j++) {
		TEST_EQUAL(i->first, j->first);
		TEST_EQUAL(i->second.value, j->second.value);
	    }
	}
	{
	    OmDocumentContents::document_terms::const_iterator i,j;
	    for (i = document_in.terms.begin(), j = document_out.terms.begin();
		 i != document_in.terms.end();
		 i++, j++) {
		TEST_EQUAL(i->first, j->first);
		TEST_EQUAL(i->second.tname, j->second.tname);
		TEST_EQUAL(i->second.wdf, j->second.wdf);
		TEST_NOT_EQUAL(i->second.termfreq, j->second.termfreq);
		TEST_EQUAL(0, i->second.termfreq);
		TEST_NOT_EQUAL(0, j->second.termfreq);
		if (i->second.tname == "foobar") {
		    // termfreq of foobar is 2
		    TEST_EQUAL(2, j->second.termfreq);
		} else {
		    // termfreq of rising is 1
		    TEST_EQUAL(i->second.tname, "rising");
		    TEST_EQUAL(1, j->second.termfreq);
		}
		TEST_EQUAL(i->second.positions.size(),
			   j->second.positions.size());
		OmDocumentTerm::term_positions::const_iterator k,l;
		for (k = i->second.positions.begin(),
		     l = j->second.positions.begin();
		     k != i->second.positions.end();
		     k++, l++) {
		    TEST_EQUAL(*k, *l);
		}
	    }
	}
    }

    return true;
}

/// Test packing integers into strings
static bool test_packint1()
{
    TEST_EQUAL(pack_uint(0u), std::string("\000", 1));
    TEST_EQUAL(pack_uint(1u), std::string("\001", 1));
    TEST_EQUAL(pack_uint(127u), std::string("\177", 1));
    TEST_EQUAL(pack_uint(128u), std::string("\200\001", 2));
    TEST_EQUAL(pack_uint(0xffffu), std::string("\377\377\003", 3));
    TEST_EQUAL(pack_uint(0xffffffffu), std::string("\377\377\377\377\017", 5));

    return true;
}

/// Test packing integers into strings and unpacking again
static bool test_packint2()
{
    std::string foo;

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
    std::string foo;

    std::vector<unsigned int> ints;
    std::vector<std::string> strings;

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

    std::vector<unsigned int>::const_iterator i;
    std::vector<std::string>::const_iterator j;
    for (i = ints.begin();
	 i != ints.end();
	 i++) {
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
    std::string foo;
    const char *p;
    om_uint32 result;
    bool success;
    
    p = foo.data();
    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    foo = std::string("\000\002\301\001", 4);
    result = 1;
    p = foo.data();

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0);
    TEST_EQUAL((void *)p, (void *)(foo.data() + 1));

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

    foo = std::string("\377\377\377\377\017\377\377\377\377\020\007\200\200\200\200\200\200\200\000\200\200\200\200\200\200\001\200\200\200\200\200\200", 32);
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

/// Test playing with a btree
static bool test_btree1()
{
    std::string path = tmpdir + "test_btree1_";
    Btree_create(path.c_str(), 8192);
    struct Btree * btree = Btree_open_to_read(path.c_str());

    std::string key = "foo";
    {
	AutoPtr<Bcursor> cursor = Bcursor_create(btree);
	int found = cursor->find_key(reinterpret_cast<const byte *>(key.data()),
				     key.size());
	TEST(!found);
    }
    {
	AutoPtr<Bcursor> cursor = Bcursor_create(btree);
	int found = cursor->find_key(reinterpret_cast<const byte *>(key.data()),
				     key.size());
	TEST(!found);
    }
    
    Btree_quit(btree);

    return true;
}


/// Test playing with a postlist
static bool test_postlist1()
{
    OmSettings settings;
    deletedir(tmpdir + "testdb_postlist1");
    makedir(tmpdir + "testdb_postlist1");
    settings.set("quartz_dir", tmpdir + "testdb_postlist1");
    settings.set("quartz_logfile", tmpdir + "log_postlist1");
    settings.set("backend", "quartz");
    RefCntPtr<Database> database_w = DatabaseBuilder::create(settings, false);

    QuartzDiskTable disktable(tmpdir + "testdb_postlist1/postlist_", false, 8192);
    disktable.open();
    QuartzBufferedTable bufftable(&disktable);
    QuartzTable * table = &bufftable;
    QuartzDiskTable positiontable(tmpdir + "testdb_postlist1/position_", false, 8192);

    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 0);
	TEST_EQUAL(pl2.get_collection_freq(), 0);
	pl2.next(0);
	TEST(pl2.at_end());
    }

    QuartzPostList::add_entry(&bufftable, "foo", 5, 7, 3);
    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 1);
	TEST_EQUAL(pl2.get_collection_freq(), 7);
	pl2.next(0);
	TEST(!pl2.at_end());
	TEST_EQUAL(pl2.get_docid(), 5);
	TEST_EQUAL(pl2.get_doclength(), 3.0);
	TEST_EQUAL(pl2.get_wdf(), 7);
	pl2.next(0);
	TEST(pl2.at_end());
    }

    QuartzPostList::add_entry(&bufftable, "foo", 6, 1, 2);
    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 2);
	TEST_EQUAL(pl2.get_collection_freq(), 8);
	pl2.next(0);
	TEST(!pl2.at_end());
	TEST_EQUAL(pl2.get_docid(), 5);
	TEST_EQUAL(pl2.get_doclength(), 3.0);
	TEST_EQUAL(pl2.get_wdf(), 7);
	pl2.next(0);
	TEST(!pl2.at_end());
	TEST_EQUAL(pl2.get_docid(), 6);
	TEST_EQUAL(pl2.get_doclength(), 2.0);
	TEST_EQUAL(pl2.get_wdf(), 1);
	pl2.next(0);
	TEST(pl2.at_end());
    }

    return true;
}

/// Test playing with a postlist
static bool test_postlist2()
{
    OmSettings settings;
    deletedir(tmpdir + "testdb_postlist2");
    makedir(tmpdir + "testdb_postlist2");
    settings.set("quartz_dir", tmpdir + "testdb_postlist2");
    settings.set("quartz_logfile", tmpdir + "log_postlist2");
    settings.set("backend", "quartz");
    RefCntPtr<Database> database_w = DatabaseBuilder::create(settings, false);

    QuartzDiskTable disktable(tmpdir + "testdb_postlist2/postlist_", false, 8192);
    disktable.open();
    QuartzBufferedTable bufftable(&disktable);
    QuartzTable * table = &bufftable;
    QuartzDiskTable positiontable(tmpdir + "testdb_postlist2/position_", false, 8192);

    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 0);
	TEST_EQUAL(pl2.get_collection_freq(), 0);
	pl2.next(0);
	TEST(pl2.at_end());
    }


    vector<unsigned int> testdata;
    unsigned int pos = 0;
    srand(17);
    for (unsigned int i = 10000; i > 0; i--) {
	pos += (unsigned int)(10.0*rand()/(RAND_MAX+1.0)) + 1;
	testdata.push_back(pos);
    }
    unsigned int collfreq = 0;
    for (vector<unsigned int>::const_iterator i2 = testdata.begin();
	 i2 != testdata.end(); i2++) {
	QuartzPostList::add_entry(&bufftable, "foo",
				  *i2, (*i2) % 5 + 1, (*i2) % 7 + 1);
	collfreq += (*i2) % 5 + 1;
    }
    bufftable.apply(disktable.get_latest_revision_number() + 1);

    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	TEST_EQUAL(pl2.get_collection_freq(), collfreq);
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while(!pl2.at_end()) {
	    TEST_EQUAL(pl2.get_docid(), *i3);
	    TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
	    TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);

	    pl2.next(0);
	    i3++;
	}
	TEST(i3 == testdata.end());
	TEST(pl2.at_end());
    }
    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while(!pl2.at_end()) {
	    TEST_EQUAL(pl2.get_docid(), *i3);
	    TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
	    TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);

	    pl2.skip_to(*i3 - 1, 0);
	    TEST_EQUAL(pl2.get_docid(), *i3);
	    TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
	    TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);

	    pl2.skip_to(*i3, 0);
	    TEST_EQUAL(pl2.get_docid(), *i3);
	    TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
	    TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);

	    i3++;
	    if (i3 == testdata.end()) {
		pl2.skip_to(1000000000, 0);
	    } else {
		pl2.skip_to(*i3, 0);
	    }
	}
	TEST(i3 == testdata.end());
	TEST(pl2.at_end());
    }
    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while(!pl2.at_end()) {
	    TEST_EQUAL(pl2.get_docid(), *i3);
	    TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
	    TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);

	    pl2.skip_to(*i3 + 1, 0);
	    i3++;
	}
	TEST(i3 == testdata.end());
	TEST(pl2.at_end());
    }
    {
	QuartzPostList pl2(database_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while(!pl2.at_end()) {
	    while (i3 != testdata.end()) {
		if ((unsigned int)(10.0*rand()/(RAND_MAX+1.0)) == 1) break;
		i3++;
	    }
	    if (i3 == testdata.end()) {
		break;
	    }
	    pl2.skip_to(*i3 + 1, 0);
	    i3++;
	    if (i3 == testdata.end()) {
		TEST(pl2.at_end());
		break;
	    } else {
		TEST_EQUAL(pl2.get_docid(), *i3);
		TEST_EQUAL(pl2.get_wdf(), (*i3) % 5 + 1);
		TEST_EQUAL(pl2.get_doclength(), (*i3) % 7 + 1);
	    }
	}
	TEST(i3 == testdata.end());
    }

    return true;
}

/// Test playing with a positionlist, testing skip_to in particular.
static bool test_positionlist1()
{
    unlink_table(tmpdir + "testdb_positionlist1_");
    QuartzDiskTable disktable(tmpdir + "testdb_positionlist1_", false, 8192);
    disktable.open();
    QuartzBufferedTable bufftable(&disktable);

    OmDocumentTerm::term_positions positions;

    positions.push_back(5);
    positions.push_back(8);
    positions.push_back(10);
    positions.push_back(12);
    QuartzPositionList::set_positionlist(&bufftable, 1, "foo", positions);

    QuartzPositionList pl;

    TEST_EXCEPTION(OmDocNotFoundError, pl.read_data(&bufftable, 1, "foobar"));
    TEST_EXCEPTION(OmDocNotFoundError, pl.read_data(&bufftable, 2, "foo"));
    pl.read_data(&bufftable, 1, "foo");
    TEST_EQUAL(pl.get_size(), 4);

    pl.next();
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_size(), 4);
    TEST_EQUAL(pl.get_position(), 5);

    pl.next();
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 8);

    pl.next();
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 10);

    pl.next();
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 12);

    pl.next();
    TEST(pl.at_end());

    pl.read_data(&bufftable, 1, "foo");
    TEST_EQUAL(pl.get_size(), 4);

    pl.skip_to(5);
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_size(), 4);
    TEST_EQUAL(pl.get_position(), 5);

    pl.skip_to(9);
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 10);

    pl.next();
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 12);

    pl.skip_to(12);
    TEST(!pl.at_end());
    TEST_EQUAL(pl.get_position(), 12);

    pl.skip_to(13);
    TEST(pl.at_end());

    return true;
}

/// Test playing with a positionlist, testing skip_to in particular.
static bool test_overwrite1()
{
    unlink_table(tmpdir + "testdb_overwrite1_");
    QuartzDiskTable disktable(tmpdir + "testdb_overwrite1_", false, 2048);
    disktable.open();
    QuartzBufferedTable bufftable(&disktable);


    quartz_revision_number_t new_revision;
    QuartzDbKey key;
    QuartzDbTag tag;

    for (int i=1; i<=1000; ++i) {
	key.value = "foo" + om_tostring(i);

	bufftable.get_or_make_tag(key)->value = "bar" + om_tostring(i);
    }
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.apply(new_revision);

    key.value = "foo1";
    QuartzDbKey key2;
    key2.value = "foo999";

    QuartzDiskTable disktable_ro(tmpdir + "testdb_overwrite1_", true, 2048);
    disktable_ro.open();
    TEST(disktable_ro.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "bar1");

    bufftable.get_or_make_tag(key)->value = "bar2";
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.apply(new_revision);
    TEST(disktable_ro.get_exact_entry(key2, tag));
    TEST(disktable_ro.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "bar1");

    bufftable.get_or_make_tag(key)->value = "bar3";
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.apply(new_revision);
    TEST(disktable_ro.get_exact_entry(key2, tag));
    TEST_EXCEPTION(OmDatabaseModifiedError, disktable_ro.get_exact_entry(key, tag));
    //TEST_EQUAL(tag.value, "bar1");

    return true;
}

/// Test large bitmap files.
static bool test_bitmap1()
{
    const std::string dbname = tmpdir + "testdb_bitmap1_";
    unlink_table(dbname);
    /* Use a small block size to make it easier to get a large bitmap */
    QuartzDiskTable disktable(dbname, false, 256);
    disktable.open();
    QuartzBufferedTable bufftable(&disktable);


    quartz_revision_number_t new_revision;
    QuartzDbKey key;
    QuartzDbTag tag;

    for (int j=0; j<100; ++j) {
	for (int i=1; i<=1000; ++i) {
	    key.value = "foo" + om_tostring(j) + "_" + om_tostring(i);
	    bufftable.get_or_make_tag(key)->value = "bar" + om_tostring(i);
	}
	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.apply(new_revision);
    }
    return true;
}


// ================================
// ========= END OF TESTS =========
// ================================
//
// Tests to write:
//
// Check behaviour of attributes - write same attribute twice, test reading
// single attributes which exist and don't exist / have been deleted.

/// The lists of tests to perform
test_desc tests[] = {
    {"quartzdisktable1",	test_disktable1},
    {"quartzdisktable2",	test_disktable2},
    {"quartztableentries1",	test_tableentries1},
    {"quartzbufftable1",	test_bufftable1},
    {"quartzbufftable2",	test_bufftable2},
    {"quartzbufftable3",	test_bufftable3},
    {"quartzcursor1",		test_cursor1},
    {"quartzcursor2",		test_cursor2},
    {"quartzopen1",		test_open1},
    {"quartzadddoc1",		test_adddoc1},
    {"quartzadddoc2",		test_adddoc2},
    {"quartzpackint1",		test_packint1},
    {"quartzpackint2",		test_packint2},
    {"quartzpackint3",		test_packint3},
    {"quartzunpackint1",	test_unpackint1},
    {"quartzbtree1",		test_btree1},
    {"quartzpostlist1",		test_postlist1},
    {"quartzpostlist2",		test_postlist2},
    {"quartzpositionlist1",	test_positionlist1},
    {"quartzoverwrite1", 	test_overwrite1},
    {"quartzbitmap1", 		test_bitmap1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    tmpdir = ".quartztmp/";
    deletedir(tmpdir);
    makedir(tmpdir);
    return test_driver::main(argc, argv, tests);
}
