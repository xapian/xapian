/* quartztest.cc: test of the Quartz backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <config.h>
#include "testsuite.h"
#include "testutils.h"
#include <xapian/error.h>

#include "quartz_database.h"
#include "quartz_postlist.h"
#include "bcursor.h"
#include "quartz_utils.h"

#include "autoptr.h"

#include <errno.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static string tmpdir;

/// Get the size of the given file in bytes.
static int get_filesize(const string &filename)
{
    struct stat buf;
    int result = stat(filename, &buf);
    if (result) return -1;
    return buf.st_size;
}

static void makedir(const string &filename)
{
    if (mkdir(filename, 0700) == -1 && errno != EEXIST) {
	FAIL_TEST("Couldn't create directory `" << filename << "' (" <<
		  strerror(errno) << ")");
    }
}

static void removedir(const string &filename)
{
    rmdir(filename);
    struct stat buf;
    if (stat(filename, &buf) == 0 || errno != ENOENT) {
	FAIL_TEST("Failed to remove directory `" << filename << "' (" <<
		  strerror(errno) << ")");
    }
}

static void unlink_table(const string & path)
{
    unlink(path + "DB");
    unlink(path + "baseA");
    unlink(path + "baseB");
    unlink(path + "bitmapA");
    unlink(path + "bitmapB");
}

/// Check the values returned by a table containing key/tag "hello"/"world"
static void check_table_values_hello(Btree & table, const string &world)
{
    string key;
    string tag;

    // Check exact reads
    key = "hello";
    tag = "foo";
    TEST(table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, world);

    key = "jello";
    tag = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "foo");

    key = "bello";
    tag = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "foo");
    
    AutoPtr<Bcursor> cursor(table.cursor_get());

    // Check normal reads
    key = "hello";
    tag = "foo";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "hello");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, world);

    key = "jello";
    tag = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "hello");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, world);

    key = "bello";
    tag = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");
}

/// Check the values returned by a table containing no key/tag pairs
static void check_table_values_empty(Btree & table)
{
    string key;
    string tag;

    // Check exact reads
    key = "hello";
    tag = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "foo");

    key = "jello";
    tag = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "foo");

    key = "bello";
    tag = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "foo");
    
    AutoPtr<Bcursor> cursor(table.cursor_get());
    
    // Check normal reads
    key = "hello";
    tag = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    key = "jello";
    tag = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    key = "bello";
    tag = "foo";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");
}

/// Test making and playing with a Btree
static bool test_disktable1()
{
    unlink_table(tmpdir + "test_disktable1_");
    {
	Btree table0(tmpdir + "test_disktable1_", true);
	TEST_EXCEPTION(Xapian::DatabaseOpeningError, table0.open());
	TEST_EXCEPTION(Xapian::DatabaseOpeningError, table0.open(10));
    }
    Btree rw_table(tmpdir + "test_disktable1_", false);
    rw_table.create(8192);
    rw_table.open();
    Btree ro_table(tmpdir + "test_disktable1_", true);
    ro_table.open();

    quartz_revision_number_t rev1 = ro_table.get_open_revision_number();
    quartz_revision_number_t rev2 = rw_table.get_open_revision_number();

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_EQUAL(rev2, rw_table.get_open_revision_number());
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 0);

    // Check adding no entries

#ifdef MUS_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError,
		   ro_table.commit(ro_table.get_latest_revision_number() + 1));
#endif
    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 0);

    // Check adding some entries
    string key;
    string tag;
    key = "hello";
    tag = "world";
    
#ifdef MUS_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add(key, tag));
#endif
    rw_table.add(key, tag);
    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(ro_table);
    check_table_values_hello(rw_table, "world");

    // Check adding the same entries
#ifdef MUS_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add(key, tag));
#endif
    rw_table.add(key, tag);
    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(ro_table);
    check_table_values_hello(rw_table, "world");

#ifdef MUS_DEBUG
    // Check adding an entry with a null key
    // Can't add a key to a read-only table anyway, empty or not!
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add("", tag));
    // Empty keys aren't allowed (we no longer enforce this so the
    // magic empty key can be set).
    //TEST_EXCEPTION(Xapian::AssertionError, rw_table.add("", tag));
#endif

    // Check changing an entry, to a null tag
    key = "hello";
    tag = "";
#ifdef MUS_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add(key, tag));
#endif
    rw_table.add(key, tag);
    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 1);
    
    // Check getting the entries out again
    check_table_values_empty(ro_table);
    check_table_values_hello(rw_table, "");

    // Check deleting an entry
    key = "hello";
#ifdef MUS_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.del(key));
#endif
    rw_table.del(key);
    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 0);

    // Check the entries in the table
    check_table_values_empty(ro_table);
    check_table_values_empty(rw_table);
    
    // Check find_entry when looking for something between two elements
    key = "hello";
    tag = "world";
    rw_table.add(key, tag);
    key = "whooo";
    tag = "world";
    rw_table.add(key, tag);

    rw_table.commit(rw_table.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, rw_table.get_open_revision_number());
    rev1 = ro_table.get_open_revision_number();
    rev2 = rw_table.get_open_revision_number();
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 2);

    // Check the entries in the table
    check_table_values_empty(ro_table);
    check_table_values_hello(rw_table, "world");
    
    return true;
}

/// Test making and playing with a Btree
static bool test_disktable2()
{
    unlink_table(tmpdir + "test_disktable2_");

    Btree table(tmpdir + "test_disktable2_", false);
    table.create(8192);
    table.open();
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 0);

    string key = "foo";
    string tag = "bar";

    table.add(key, tag);
    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "test_disktable2_DB"), 8192);

    return true;
}

/// Test making and playing with a Btree
static bool test_disktable3()
{
    unlink_table(tmpdir + "test_disktable3_");

    Btree table(tmpdir + "test_disktable3_", false);
    table.create(8192);
    table.open();

    table.commit(table.get_latest_revision_number() + 1);

    string tradeakey;
    string tradekey;
    string tradkey;

    tradkey = "trad";
    tradekey = "trade";
    tradeakey = "tradea";

    string tag;

    {
	tag = string(2200, 'a');
	table.add(tradkey, tag);
	tag = string(3800, 'b');
	table.add(tradekey, tag);
	tag = string(2000, 'c');
	table.add(tradeakey, tag);

	table.commit(table.get_latest_revision_number() + 1);
    }

    {
	AutoPtr<Bcursor> cursor(table.cursor_get());
	TEST(cursor->find_entry(tradekey));
	TEST_EQUAL(cursor->current_key, tradekey);
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag.size(), 3800);

	cursor->next();
	TEST_EQUAL(cursor->current_key, tradeakey);
    }

    {
	tag = string(4000, 'd');
	table.add(tradekey, tag);
	table.commit(table.get_latest_revision_number() + 1);
    }

    {
	AutoPtr<Bcursor> cursor(table.cursor_get());
	TEST(cursor->find_entry(tradekey));
	TEST_EQUAL(cursor->current_key, tradekey);
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag.size(), 4000);

	cursor->next();
	TEST_EQUAL(cursor->current_key, tradeakey);
    }

    return true;
}

/// Test making and playing with a Btree
static bool test_bufftable1()
{
    unlink_table(tmpdir + "test_bufftable1_");
    Btree bufftable1(tmpdir + "test_bufftable1_", false);
    bufftable1.create(8192);
    bufftable1.open();
    Btree disktable1(tmpdir + "test_bufftable1_", true);
    disktable1.open();

    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    string key = "foo1";

    bufftable1.del(key);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    bufftable1.add(key, "");
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    quartz_revision_number_t new_revision =
	    disktable1.get_latest_revision_number() + 1;
    bufftable1.commit(new_revision);
    disktable1.open();
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    bufftable1.add(key, "");
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    bufftable1.del(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    bufftable1.del(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    key = "bar";
    bufftable1.add(key, "");
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    key = "bar2";
    bufftable1.add(key, "");
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    new_revision += 1;
    bufftable1.commit(new_revision);
    disktable1.open();

    TEST_EQUAL(disktable1.get_entry_count(), 2);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    return true;
}

/// Test making and playing with a Btree
static bool test_bufftable2()
{
    unlink_table(tmpdir + "test_bufftable2_");
    quartz_revision_number_t new_revision;
    quartz_revision_number_t old_revision;
    {
	// Open table and add a few documents
	Btree bufftable(tmpdir + "test_bufftable2_", false);
	bufftable.create(8192);
	bufftable.open();
	Btree disktable(tmpdir + "test_bufftable2_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 0);
	TEST_EQUAL(bufftable.get_entry_count(), 0);

	string key;

	key = "foo1";
	bufftable.add(key, "bar1");
	key = "foo2";
	bufftable.add(key, "bar2");
	key = "foo3";
	bufftable.add(key, "bar3");

	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.commit(new_revision);
	disktable.open();

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree bufftable(tmpdir + "test_bufftable2_", false);
	bufftable.open();
	Btree disktable(tmpdir + "test_bufftable2_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	string key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key = "foo";
	AutoPtr<Bcursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo1");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar1");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo2");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo3");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	// Add a new tag
	key = "foo25";
	bufftable.add(key, "bar25");
	old_revision = new_revision;
	new_revision += 1;
	bufftable.commit(new_revision);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Open old revision
	Btree bufftable(tmpdir + "test_bufftable2_", false);
	TEST(bufftable.open(old_revision));
	Btree disktable(tmpdir + "test_bufftable2_", true);
	disktable.open(old_revision);

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	string key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(old_revision, disktable.get_open_revision_number());

	// Add a new tag
	key = "foo26";
	bufftable.add(key, "bar26");
	new_revision += 1;
	bufftable.commit(new_revision);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	// Add another new tag, but don't apply this one.
	key = "foo37";
	bufftable.add(key, "bar37");
	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 5);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree bufftable(tmpdir + "test_bufftable2_", false);
	bufftable.open();
	Btree disktable(tmpdir + "test_bufftable2_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	string key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key = "foo";
	AutoPtr<Bcursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo1");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar1");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo2");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo26");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar26");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo3");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar3");

	cursor->next();
	TEST(cursor->after_end());
    }
    {
	// Check that opening a nonexistant revision returns false (and doesn't
	// throw an exception).
	Btree disktable(tmpdir + "test_bufftable2_", false);
	TEST(!disktable.open(new_revision + 10));
    }

    return true;
}

/// Test making and playing with a Btree
static bool test_bufftable3()
{
    unlink_table(tmpdir + "test_bufftable3_");
    quartz_revision_number_t new_revision;
    {
	// Open table and add a couple of documents
	Btree bufftable(tmpdir + "test_bufftable3_", false);
	bufftable.create(8192);
	bufftable.open();
	Btree disktable(tmpdir + "test_bufftable3_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 0);
	TEST_EQUAL(bufftable.get_entry_count(), 0);

	string key;

	key = "foo1";
	bufftable.add(key, "bar1");

	key = "foo2";
	bufftable.add(key, "bar2");
	bufftable.cancel();

	key = "foo3";
	bufftable.add(key, "bar3");

	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.commit(new_revision);
	disktable.open();

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree bufftable(tmpdir + "test_bufftable3_", false);
	bufftable.open();
	Btree disktable(tmpdir + "test_bufftable3_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 1);
	TEST_EQUAL(bufftable.get_entry_count(), 1);

	string key;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key = "foo";
	AutoPtr<Bcursor> cursor(bufftable.cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo3");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar3");

	cursor->next();
	TEST(cursor->after_end());
    }
    return true;
}

/// Test making and playing with a Btree
static bool test_cursor3()
{
    unlink_table(tmpdir + "test_tableskipto1_");
    quartz_revision_number_t new_revision;
    {
	// Open table and add a couple of documents
	Btree bufftable(tmpdir + "test_cursor3_", false);
	bufftable.create(8192);
	bufftable.open();
	Btree disktable(tmpdir + "test_cursor3_", true);
	disktable.open();

	TEST_EQUAL(disktable.get_entry_count(), 0);
	{
	    string key;

	    key = "A";
	    bufftable.add(key, "A");

	    key = "B";
	    bufftable.add(key, "B");
	}

	{
	    string key;
	    key = "AA";
	    AutoPtr<Bcursor> cursor(bufftable.cursor_get());
	    TEST(!cursor->find_entry(key));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");
	}

	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.commit(new_revision);
	disktable.open();

	{
	    string key;
	    key = "AA";
	    AutoPtr<Bcursor> cursor(bufftable.cursor_get());
	    TEST(!cursor->find_entry(key));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");
	}

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree disktable(tmpdir + "test_cursor3_", false);
	disktable.open();
	TEST_EQUAL(disktable.get_entry_count(), 2);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	{
	    string key;
	    key = "AA";
	    AutoPtr<Bcursor> cursor(disktable.cursor_get());
	    TEST(!cursor->find_entry(key));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");
	}
    }
    return true;
}

/// Test Bcursors
static bool test_cursor1()
{
    unlink_table(tmpdir + "test_cursor1_");

    string key;

    // Open table and put stuff in it.
    Btree bufftable1(tmpdir + "test_cursor1_", false);
    bufftable1.create(8192);
    bufftable1.open();
    Btree disktable1(tmpdir + "test_cursor1_", true);
    disktable1.open();

    key = "foo1";
    bufftable1.add(key, "bar1");
    key = "foo2";
    bufftable1.add(key, "bar2");
    key = "foo3";
    bufftable1.add(key, "bar3");
    quartz_revision_number_t new_revision = disktable1.get_latest_revision_number();
    new_revision += 1;
    bufftable1.commit(new_revision);
    disktable1.open();

    Btree * table = &disktable1;
    int count = 2;

    while (count != 0) {
	key = "foo25";
	AutoPtr<Bcursor> cursor(table->cursor_get());
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "foo2");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo3");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	key = "foo";
	TEST(!cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo1");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar1");

	key = "foo2";
	TEST(cursor->find_entry(key));
	TEST_EQUAL(cursor->current_key, "foo2");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar2");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo3");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar3");

	cursor->next();
	TEST(cursor->after_end());

	table = &bufftable1;
	count -= 1;
    }

    // Test cursors when we have unapplied changes
    key = "foo25";
    bufftable1.add(key, "bar25");

    key = "foo26";
    bufftable1.del(key);
    key = "foo1";
    bufftable1.del(key);

    key = "foo25";
    AutoPtr<Bcursor> cursor(disktable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    key = "foo25";
    cursor.reset(bufftable1.cursor_get());
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    key = "foo26";
    cursor.reset(bufftable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    key = "foo2";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    cursor->next();
    TEST(cursor->after_end());

    key = "foo1";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    new_revision += 1;
    bufftable1.commit(new_revision);
    disktable1.open();

    cursor.reset(bufftable1.cursor_get());
    key = "foo2";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    key = "foo24";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    key = "foo25";
    TEST(cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    key = "foo24";
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    key = "foo25";
    bufftable1.del(key);

    key = "foo25";
    cursor.reset(bufftable1.cursor_get());
    TEST(!cursor->find_entry(key));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    return true;
}

/// Regression test for cursors
static bool test_cursor2()
{
    unlink_table(tmpdir + "test_cursor2_");

    // Open table and put stuff in it.
    Btree bufftable1(tmpdir + "test_cursor2_", false);
    bufftable1.create(8192);
    bufftable1.open();
    Btree disktable1(tmpdir + "test_cursor2_", true);
    disktable1.open();

    string key1 = "a";
    string tag1 = string(2036, '\x00');
    string key2 = "c";
    string tag2 = "bar2";
    string searchkey = "b";

    bufftable1.add(key1, tag1);
    bufftable1.add(key2, tag2);
    quartz_revision_number_t new_revision = disktable1.get_latest_revision_number();
    new_revision += 1;
    bufftable1.commit(new_revision);
    disktable1.open();

    AutoPtr<Bcursor> cursor(disktable1.cursor_get());

    TEST(!cursor->find_entry(searchkey));
    TEST_EQUAL(cursor->current_key, key1);
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, tag1);

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    string dbdir = tmpdir + "testdb_open1";
    removedir(dbdir);
    
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Internal::RefCntPtr<Xapian::Database::Internal> database_0 = new QuartzDatabase(dbdir));

    makedir(dbdir);
    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> database_w =
	    new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048);
    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> database_r = new QuartzDatabase(dbdir);

    return true;
}

/// Test creating and opening of quartz databases
static bool test_create1()
{
    string dbdir = tmpdir + "testdb_create1";
    removedir(dbdir);

    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db;

    // (1) db doesn't exist (no create)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzDatabase(dbdir));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzWritableDatabase(dbdir, Xapian::DB_OPEN, 2048));

    // (2) db doesn't exist, basedir doesn't exist (create)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzDatabase(dbdir));
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048);
    db = 0; // Close the database - Cygwin can't delete a locked file...
    removedir(dbdir);

    makedir(dbdir);

    // (3) db doesn't exist, basedir exists (no create)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzDatabase(dbdir));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzWritableDatabase(dbdir, Xapian::DB_OPEN, 2048));

    // (4) db doesn't exist, basedir exists (create)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   db = new QuartzDatabase(dbdir));
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048);
    db = new QuartzDatabase(dbdir);

    // (5) db exists (create, no overwrite)
    db = new QuartzDatabase(dbdir);
    TEST_EXCEPTION(Xapian::DatabaseCreateError,
		   db = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 2048));
    db = new QuartzDatabase(dbdir);

    // (6) db exists (no create)
    db = new QuartzWritableDatabase(dbdir, Xapian::DB_OPEN, 2048);

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
    string dbdir = tmpdir + "testdb_adddoc1";
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

/** Test adding a document, and checking that it got added correctly.
 */
static bool test_adddoc2()
{
    string dbdir = tmpdir + "testdb_adddoc2";
    removedir(dbdir);

    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);

    Xapian::Document document_in2;
    document_in2.set_data("Foobar falling");
    document_in2.add_posting("foobar", 1);
    document_in2.add_posting("falling", 2);
    {
	Xapian::WritableDatabase database = Xapian::Quartz::open(dbdir, Xapian::DB_CREATE);

	TEST_EQUAL(database.get_doccount(), 0);
	TEST_EQUAL(database.get_avlength(), 0);

	did = database.add_document(document_in);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 3);

	TEST_EQUAL(database.get_termfreq("foobar"), 1);
	TEST_EQUAL(database.get_collection_freq("foobar"), 2);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 0);
	TEST_EQUAL(database.get_collection_freq("falling"), 0);

	Xapian::docid did2 = database.add_document(document_in2);
	TEST_EQUAL(database.get_doccount(), 2);
	TEST_NOT_EQUAL(did, did2);
	TEST_EQUAL(database.get_avlength(), 5.0/2.0);

	TEST_EQUAL(database.get_termfreq("foobar"), 2);
	TEST_EQUAL(database.get_collection_freq("foobar"), 3);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);

	database.delete_document(did);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 2);

	TEST_EQUAL(database.get_termfreq("foobar"), 1);
	TEST_EQUAL(database.get_collection_freq("foobar"), 1);
	TEST_EQUAL(database.get_termfreq("rising"), 0);
	TEST_EQUAL(database.get_collection_freq("rising"), 0);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);

	did = database.add_document(document_in);
	TEST_EQUAL(database.get_doccount(), 2);
	TEST_EQUAL(database.get_avlength(), 5.0/2.0);

	TEST_EQUAL(database.get_termfreq("foobar"), 2);
	TEST_EQUAL(database.get_collection_freq("foobar"), 3);
	TEST_EQUAL(database.get_termfreq("rising"), 1);
	TEST_EQUAL(database.get_collection_freq("rising"), 1);
	TEST_EQUAL(database.get_termfreq("falling"), 1);
	TEST_EQUAL(database.get_collection_freq("falling"), 1);
    }

    {
	Xapian::Database database = Xapian::Quartz::open(dbdir);
	Xapian::Document document_out = database.get_document(did);

	TEST_EQUAL(document_in.get_data(), document_out.get_data());

	{
	    Xapian::ValueIterator i(document_in.values_begin());
	    Xapian::ValueIterator j(document_out.values_begin());
	    for (; i != document_in.values_end(); i++, j++) {
		TEST_NOT_EQUAL(j, document_out.values_end());
		TEST_EQUAL(*i, *j);
		TEST_EQUAL(i.get_valueno(), j.get_valueno());
	    }
	    TEST_EQUAL(j, document_out.values_end());
	}
	{
	    Xapian::TermIterator i(document_in.termlist_begin());
	    Xapian::TermIterator j(document_out.termlist_begin());
	    for (; i != document_in.termlist_end(); i++, j++) {
		TEST_NOT_EQUAL(j, document_out.termlist_end());
		TEST_EQUAL(*i, *j);
		TEST_EQUAL(i.get_wdf(), j.get_wdf());
		// FIXME: MapTermList::get_termfreq asserts in a debug build
		//TEST_NOT_EQUAL(i.get_termfreq(), j.get_termfreq());
		//TEST_EQUAL(0, i.get_termfreq());
		TEST_NOT_EQUAL(0, j.get_termfreq());
		if (*i == "foobar") {
		    // termfreq of foobar is 2
		    TEST_EQUAL(2, j.get_termfreq());
		} else {
		    // termfreq of rising is 1
		    TEST_EQUAL(*i, "rising");
		    TEST_EQUAL(1, j.get_termfreq());
		}
		Xapian::PositionIterator k(i.positionlist_begin());
		Xapian::PositionIterator l(j.positionlist_begin());
		for (; k != i.positionlist_end(); k++, l++) {
		    TEST_NOT_EQUAL(l, j.positionlist_end());
		    TEST_EQUAL(*k, *l);
		}
		TEST_EQUAL(l, j.positionlist_end());
	    }
	    TEST_EQUAL(j, document_out.termlist_end());
	}
    }

    return true;
}

static bool test_adddoc3()
{
    string dbdir = tmpdir + "testdb_adddoc3";
    removedir(dbdir);

    Xapian::docid did;
    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_value(7, "Value7");
    document_in.add_value(13, "Value13");
    document_in.add_posting("foo", 1);
    document_in.add_posting("bar", 2);

    {
	Xapian::WritableDatabase database = Xapian::Quartz::open(dbdir, Xapian::DB_CREATE);

	did = database.add_document(document_in);
	TEST_EQUAL(did, 1);
	TEST_EQUAL(database.get_doccount(), 1);
	TEST_EQUAL(database.get_avlength(), 2);
    }

    {
	Xapian::WritableDatabase database = Xapian::Quartz::open(dbdir, Xapian::DB_OPEN);

	document_in.remove_term("foo");
	document_in.add_posting("baz", 1);

	database.replace_document(1, document_in);

	database.delete_document(1);

	TEST_EQUAL(database.get_doccount(), 0);
	TEST_EQUAL(database.get_avlength(), 0);
	TEST_EQUAL(database.get_termfreq("foo"), 0);
	TEST_EQUAL(database.get_collection_freq("foo"), 0);
	TEST_EQUAL(database.get_termfreq("bar"), 0);
	TEST_EQUAL(database.get_collection_freq("bar"), 0);
	TEST_EQUAL(database.get_termfreq("baz"), 0);
	TEST_EQUAL(database.get_collection_freq("baz"), 0);
    }

    return true;
}

/// Test packing integers into strings
static bool test_packint1()
{
    TEST_EQUAL(pack_uint(0u), string("\000", 1));
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

/// Test playing with a postlist
static bool test_postlist1()
{
    string dbdir = tmpdir + "testdb_postlist1";
    removedir(dbdir);
    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_w = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 8192);

    Btree bufftable(dbdir + "/postlist_", false);
    bufftable.open();
    Btree disktable(dbdir + "/postlist_", true);
    Btree * table = &bufftable;
    Btree positiontable(dbdir + "/position_", false);

    {
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 0);
	TEST_EQUAL(pl2.get_collection_freq(), 0);
	pl2.next(0);
	TEST(pl2.at_end());
    }

#if 0 // FIXME update to use new stuff
    QuartzPostList::add_entry(&bufftable, "foo", 5, 7, 3);
    {
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
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
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
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
#endif

    return true;
}

/// Test playing with a postlist
static bool test_postlist2()
{
    string dbdir = tmpdir + "testdb_postlist2";
    removedir(dbdir);
    Xapian::Internal::RefCntPtr<Xapian::Database::Internal> db_w = new QuartzWritableDatabase(dbdir, Xapian::DB_CREATE, 8192);

    Btree bufftable(tmpdir + "testdb_postlist2/postlist_", false);
    bufftable.open();
    Btree disktable(tmpdir + "testdb_postlist2/postlist_", true);
    disktable.open();
    Btree * table = &bufftable;
    Btree positiontable(tmpdir + "testdb_postlist2/position_", false);

    {
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), 0);
	TEST_EQUAL(pl2.get_collection_freq(), 0);
	pl2.next(0);
	TEST(pl2.at_end());
    }

#if 0 // FIXME update to use new stuff
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
    bufftable.commit(disktable.get_latest_revision_number() + 1);
    disktable.open();

    {
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	TEST_EQUAL(pl2.get_collection_freq(), collfreq);
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while (!pl2.at_end()) {
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
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while (!pl2.at_end()) {
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
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	pl2.next(0);
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while (!pl2.at_end()) {
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
	QuartzPostList pl2(db_w, table, &positiontable, "foo");
	TEST_EQUAL(pl2.get_termfreq(), testdata.size());
	vector<unsigned int>::const_iterator i3 = testdata.begin();

	while (!pl2.at_end()) {
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
#endif

    return true;
}

/// Test overwriting a table.
static bool test_overwrite1()
{
    unlink_table(tmpdir + "testdb_overwrite1_");
    Btree bufftable(tmpdir + "testdb_overwrite1_", false);
    bufftable.create(2048);
    bufftable.open();
    Btree disktable(tmpdir + "testdb_overwrite1_", true);
    disktable.open();

    quartz_revision_number_t new_revision;
    string key;
    string tag;

    for (int i=1; i<=1000; ++i) {
	key = "foo" + om_tostring(i);

	bufftable.add(key, "bar" + om_tostring(i));
    }
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.commit(new_revision);
    disktable.open();

    key = "foo1";
    string key2;
    key2 = "foo999";

    Btree disktable_ro(tmpdir + "testdb_overwrite1_", true);
    disktable_ro.open();
    TEST(disktable_ro.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "bar1");

    bufftable.add(key, "bar2");
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.commit(new_revision);
    disktable.open();
    TEST(disktable_ro.get_exact_entry(key2, tag));
    TEST(disktable_ro.get_exact_entry(key, tag));
    TEST_EQUAL(tag, "bar1");

    bufftable.add(key, "bar3");
    new_revision = disktable.get_latest_revision_number() + 1;
    bufftable.commit(new_revision);
    disktable.open();
    TEST(disktable_ro.get_exact_entry(key2, tag));
    TEST_EXCEPTION(Xapian::DatabaseModifiedError, disktable_ro.get_exact_entry(key, tag));
    //TEST_EQUAL(tag, "bar1");

    return true;
}

#if 0 // FIXME - why isn't this used?
/// overwrite2
static bool test_overwrite2()
{
    string dbname = tmpdir + "overwrite2";
    removedir(dbname);

    Xapian::WritableDatabase writer(Xapian::Quartz::open(dbname, Xapian::DB_CREATE);

    Xapian::Document document_in;
    document_in.set_data("Foobar rising");
    document_in.add_key(7, "Value7");
    document_in.add_key(13, "Value13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);

    Xapian::docid last_doc = 0;

    for (int i=0; i<1000; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    Xapian::Database reader(settings);
    // FIXME: use reader.get_document() when available.

    Xapian::Enquire enquire(reader);

    string doc_out;
    string value_out;

    doc_out = writer.get_document(last_doc).get_data();
    TEST(doc_out == "Foobar rising");
    value_out = writer.get_document(last_doc).get_value(7);
    TEST(value_out == "Value7");

    for (int i=0; i<1000; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    doc_out = "";
    doc_out = writer.get_document(last_doc).get_data();
    TEST(doc_out == "Foobar rising");

    for (int i=0; i<1000; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    value_out = writer.get_document(last_doc).get_value(7);
    TEST(value_out == "Value7");

    for (int i=0; i<1000; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    enquire.set_query(Xapian::Query("falling"));
    enquire.get_mset(1, 10);

    for (int i=0; i<1; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    Xapian::TermIterator ti = reader.termlist_begin(1);
    *ti;
    ti++;

    for (int i=0; i<1; ++i) {
	last_doc = writer.add_document(document_in);
	if (i % 200 == 0) {
	    writer.flush();
	}
    }
    writer.flush();

    Xapian::PostingIterator ki = reader.postlist_begin("falling");
    *ki;

    return true;
}
#endif

/// Test large bitmap files.
static bool test_bitmap1()
{
    const string dbname = tmpdir + "testdb_bitmap1_";
    unlink_table(dbname);
    /* Use a small block size to make it easier to get a large bitmap */
    Btree bufftable(dbname, false);
    bufftable.create(2048);
    bufftable.open();
    Btree disktable(dbname, true);
    disktable.open();

    quartz_revision_number_t new_revision;

    for (int j = 0; j < 100; ++j) {
	for (int i = 1; i <= 1000; ++i) {
	    string key = "foo" + om_tostring(j) + "_" + om_tostring(i);
	    bufftable.add(key, "bar" + om_tostring(i));
	}
	new_revision = disktable.get_latest_revision_number() + 1;
	bufftable.commit(new_revision);
	disktable.open();
    }
    return true;
}

/// Test that write locks work
static bool test_writelock1()
{
    string dbname = tmpdir + "writelock1";
    removedir(dbname);

    Xapian::WritableDatabase writer = Xapian::Quartz::open(dbname, Xapian::DB_CREATE);
    TEST_EXCEPTION(Xapian::DatabaseLockError, 
	Xapian::WritableDatabase writer2 = Xapian::Quartz::open(dbname, Xapian::DB_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseLockError, 
	Xapian::WritableDatabase writer2 = Xapian::Quartz::open(dbname, Xapian::DB_CREATE_OR_OVERWRITE));
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
//
// FIXME: Tests to write:
//
// Check behaviour of values - write same value twice, test reading
// single values which exist and don't exist / have been deleted.

/// The lists of tests to perform
test_desc tests[] = {
    {"disktable1",	test_disktable1},
    {"disktable2",	test_disktable2},
    {"disktable3",	test_disktable3},
    {"bufftable1",	test_bufftable1},
    {"bufftable2",	test_bufftable2},
    {"bufftable3",	test_bufftable3},
    {"cursor1",		test_cursor1},
    {"cursor2",		test_cursor2},
    {"open1",		test_open1},
    {"create1",		test_create1},
    {"adddoc1",		test_adddoc1},
    {"adddoc2",		test_adddoc2},
    {"adddoc3",		test_adddoc3},
    {"packint1",	test_packint1},
    {"packint2",	test_packint2},
    {"packint3",	test_packint3},
    {"unpackint1",	test_unpackint1},
    {"postlist1",	test_postlist1},
    {"postlist2",	test_postlist2},
    {"overwrite1", 	test_overwrite1},
//    {"overwrite2", 	test_overwrite2},
    {"bitmap1", 	test_bitmap1},
    {"writelock1", 	test_writelock1},
    {"packstring1", 	test_packstring1},
    {"cursor3", 	test_cursor3},
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
