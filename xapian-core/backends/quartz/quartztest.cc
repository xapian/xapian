/* quartztest.cc: test of the Quartz Database
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
#include "quartz_db_table.h"
#include "quartz_db_entries.h"

#include "autoptr.h"

/// Check the values returned by a table containing key/tag "hello"/"world"
static void check_table_values_hello(const QuartzDbTable & table, string world)
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
static void check_table_values_empty(const QuartzDbTable & table)
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

/// Test making and playing with a QuartzDbTable
static bool test_dbtable1()
{
    {
	QuartzDbTable table0("./test_dbtable1_", true);
	TEST_EXCEPTION(OmOpeningError, table0.open());
    }
    QuartzDbTable table2("./test_dbtable1_", false);
    table2.open();
    QuartzDbTable table1("./test_dbtable1_", true);
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

/// Test making and playing with a QuartzDbEntries
static bool test_dbentries1()
{
    QuartzDbEntries entries;

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
    TEST_EXCEPTION(OmAssertionError, entries.forget_entry(key1));
#endif

    key1.value="foo";
    TEST(!entries.have_entry(key1));
    {
	AutoPtr<QuartzDbTag> tagptr(new QuartzDbTag);
	tagptr->value = "bar";
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST(entries.get_tag(key1) != 0);
    TEST(entries.get_tag(key1)->value == "bar");
    {
	AutoPtr<QuartzDbTag> tagptr(0);
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST(entries.get_tag(key1) == 0);

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    OmSettings settings;
    settings.set("quartz_dir", "foo");

    QuartzDatabase database(settings, false);
    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================

/// The lists of tests to perform
test_desc tests[] = {
    {"quartzdbtable1",		test_dbtable1},
    {"quartzdbentries1",	test_dbentries1},
    {"quartzopen1",		test_open1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
