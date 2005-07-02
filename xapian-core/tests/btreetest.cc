/* btreetest.cc: test of the btree manager
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
#include "btreecheck.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include <algorithm>
#include <fstream>
#include <string>

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef HAVE_SSTREAM
# define BTREE_CHECK(DIR, OPTS) BtreeCheck::check(DIR, OPTS, tout)
#else
// FIXME: If we don't have <sstream> and hence ostringstream, we roll our own
// which can't be passed as an ostream so we just turn off the options and send
// the output (only a one line report of any error) to cout
# define BTREE_CHECK(DIR, OPTS) BtreeCheck::check(DIR, 0, cout)
#endif

static string tmpdir;
static string datadir;

static void make_dir(const string & filename)
{
    if (mkdir(filename, 0700) == -1 && errno != EEXIST) {
	tout << "Couldn't create directory `" << filename << "' ("
	     << strerror(errno) << ")";
    }
}

/// Get the size of the given file in bytes.
static int get_filesize(const string &filename)
{
    struct stat buf;
    int result = stat(filename, &buf);
    if (result) return -1;
    return buf.st_size;
}

static int process_lines(Btree & btree, ifstream &f)
{
    int count = 0;
    while (true) {
	string s;
        if (!getline(f, s)) return count;
	if (s.empty()) continue;
	if (s[0] == '+') {
	    string::size_type sp = s.find(' ');
	    btree.add(s.substr(1, min(sp - 1, BTREE_MAX_KEY_LEN)),
		      s.substr(sp + 1));
	    ++count;
	} else if (s[0] == '-') {
	    btree.del(s.substr(1, BTREE_MAX_KEY_LEN));
	    --count;
	} else {
	    throw "No '+' or '-' on line `" + s + "'";
	}
    }
}

static int do_update(const string & btree_dir,
		     const string & datafile,
		     bool full_compact = false)
{
    Btree btree(btree_dir, false);
    btree.open();

    if (full_compact) {
	tout << "Compact mode\n";
	btree.set_full_compaction(true);
    }
    
    int count;
    {
	ifstream f(datafile.c_str());
	TEST_AND_EXPLAIN(f.is_open(), "File " << datafile << " not found");
	count = process_lines(btree, f);
    }

    btree.commit(btree.get_open_revision_number() + 1);

    return count;
}

static void do_create(const string & btree_dir, int block_size = 2048)
{
    if (btree_dir.empty()) return;

    // NetBSD mkdir() doesn't cope with a trailing slash.
    string no_slash = btree_dir;
    if (no_slash[no_slash.size() - 1] == '/')
	no_slash.resize(no_slash.size() - 1);
    rmdir(no_slash);
    make_dir(no_slash);

    Btree dummy(btree_dir, false);
    dummy.create(block_size);
    tout << btree_dir << "/DB created with block size " << block_size << "\n";
}

static void unlink_table(const string & path)
{
    unlink(path + "DB");
    unlink(path + "baseA");
    unlink(path + "baseB");
}

static void check_table_values_hello(Btree & table, const string &world)
{
    string tag;

    // Check exact reads
    tag = "foo";
    TEST(table.get_exact_entry("hello", tag));
    TEST_EQUAL(tag, world);

    tag = "foo";
    TEST(!table.get_exact_entry("jello", tag));
    TEST_EQUAL(tag, "foo");

    tag = "foo";
    TEST(!table.get_exact_entry("bello", tag));
    TEST_EQUAL(tag, "foo");
    
    Bcursor * cursor = table.cursor_get();

    // Check normal reads
    tag = "foo";
    TEST(cursor->find_entry("hello"));
    TEST_EQUAL(cursor->current_key, "hello");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, world);

    tag = "foo";
    TEST(!cursor->find_entry("jello"));
    TEST_EQUAL(cursor->current_key, "hello");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, world);

    tag = "foo";
    TEST(!cursor->find_entry("bello"));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    delete cursor;
}

/// Check the values returned by a table containing no key/tag pairs
static void check_table_values_empty(Btree & table)
{
    string tag;

    // Check exact reads
    tag = "foo";
    TEST(!table.get_exact_entry("hello", tag));
    TEST_EQUAL(tag, "foo");

    tag = "foo";
    TEST(!table.get_exact_entry("jello", tag));
    TEST_EQUAL(tag, "foo");

    tag = "foo";
    TEST(!table.get_exact_entry("bello", tag));
    TEST_EQUAL(tag, "foo");
    
    Bcursor * cursor = table.cursor_get();
    
    // Check normal reads
    tag = "foo";
    TEST(!cursor->find_entry("hello"));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    tag = "foo";
    TEST(!cursor->find_entry("jello"));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    tag = "foo";
    TEST(!cursor->find_entry("bello"));
    TEST_EQUAL(cursor->current_key, "");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "");

    delete cursor;
}

/// Test playing with a btree
static bool test_simple1()
{
    const string path = tmpdir + "/test_simple1_";
    Btree btree(path, true);
    btree.create(8192);
    btree.open();

    string key = "foo";
    {
	Bcursor cursor(&btree);
	int found = cursor.find_entry(key);
	TEST(!found);
    }
    {
	Bcursor cursor(&btree);
	int found = cursor.find_entry(key);
	TEST(!found);
    }

    return true;
}

/// Test inserting and deleting items from a Btree
static bool test_insertdelete1()
{
    const string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    if (!file_exists(datadir + "ord+") || !file_exists(datadir + "ord-"))
	SKIP_TEST("Data files not present");

    unsigned int count = do_update(btree_dir, datadir + "ord+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);
    {
	Btree btree(btree_dir, true);
	btree.open();
	TEST_EQUAL(count, btree.get_entry_count());
    }

    count += do_update(btree_dir, datadir + "ord-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);
    {
	Btree btree(btree_dir, true);
	btree.open();
	TEST_EQUAL(btree.get_entry_count(), 0);
	TEST_EQUAL(count, btree.get_entry_count());
    }

    return true;
}

/// Test sequential addition in a Btree
static bool test_sequent1()
{
    const string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    if (!file_exists(datadir + "ordnum+") || !file_exists(datadir + "ordnum-"))
	SKIP_TEST("Data files not present");

    do_update(btree_dir, datadir + "ord+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    do_update(btree_dir, datadir + "ordnum+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    do_update(btree_dir, datadir + "ord-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);

    do_update(btree_dir, datadir + "ordnum-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);

    Btree btree(btree_dir, true);
    btree.open();
    TEST_EQUAL(btree.get_entry_count(), 0);

    return true;
}

static bool test_emptykey1()
{
    const string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree(btree_dir, false);
	btree.open();

	tout << "Setting tag to jam" << endl;
	btree.add("", "jam");
	btree.commit(btree.get_open_revision_number() + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree(btree_dir, false);
	btree.open();
	TEST_EQUAL(btree.get_entry_count(), 0);

	tout << "Setting tag to marmite" << endl;
	btree.add("", "marmite");
	btree.commit(btree.get_open_revision_number() + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree(btree_dir, false);
	btree.open();
	TEST_EQUAL(btree.get_entry_count(), 0);

	tout << "Deleting tag" << endl;
	btree.del("");
	btree.commit(btree.get_open_revision_number() + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree(btree_dir, false);
	btree.open();
	TEST_EQUAL(btree.get_entry_count(), 0);

	tout << "Setting tag to butter" << endl;
	btree.add("", "butter");
	btree.add("test", "me");
	btree.commit(btree.get_open_revision_number() + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    Btree btree(btree_dir, true);
    btree.open();
    TEST_EQUAL(btree.get_entry_count(), 1);
 
    return true;
}

/// Test making and playing with a Btree
static bool test_table1()
{
    const string tablename = tmpdir + "/test_table1_";
    unlink_table(tablename);
    {
	Btree table0(tablename, true);
	TEST_EXCEPTION(Xapian::DatabaseOpeningError, table0.open());
	TEST_EXCEPTION(Xapian::DatabaseOpeningError, table0.open(10));
    }
    Btree rw_table(tablename, false);
    rw_table.create(8192);
    rw_table.open();
    Btree ro_table(tablename, true);
    ro_table.open();

    quartz_revision_number_t rev1 = ro_table.get_open_revision_number();
    quartz_revision_number_t rev2 = rw_table.get_open_revision_number();

    TEST_EQUAL(rev1, ro_table.get_open_revision_number());
    TEST_EQUAL(rev2, rw_table.get_open_revision_number());
    TEST_EQUAL(ro_table.get_entry_count(), 0);
    TEST_EQUAL(rw_table.get_entry_count(), 0);

    // Check adding no entries

#ifdef XAPIAN_DEBUG
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
#ifdef XAPIAN_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add("hello", "world"));
#endif
    rw_table.add("hello", "world");
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
#ifdef XAPIAN_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add("hello", "world"));
#endif
    rw_table.add("hello", "world");
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

#ifdef XAPIAN_DEBUG
    // Check adding an entry with a null key
    // Can't add a key to a read-only table anyway, empty or not!
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add("", "world"));
    // Empty keys aren't allowed (we no longer enforce this so the
    // magic empty key can be set).
    //TEST_EXCEPTION(Xapian::AssertionError, rw_table.add("", "world"));
#endif

    // Check changing an entry, to a null tag
#ifdef XAPIAN_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.add("hello", ""));
#endif
    rw_table.add("hello", "");
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
#ifdef XAPIAN_DEBUG
    TEST_EXCEPTION(Xapian::AssertionError, ro_table.del("hello"));
#endif
    rw_table.del("hello");
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
    rw_table.add("hello", "world");
    rw_table.add("whooo", "world");

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
static bool test_table2()
{
    const string tablename = tmpdir + "/test_table2_";
    unlink_table(tablename);

    Btree table(tablename, false);
    table.create(8192);
    table.open();
    TEST_EQUAL(get_filesize(tmpdir + "/test_table2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "/test_table2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "/test_table2_DB"), 0);

    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "/test_table2_DB"), 0);

    table.add("foo", "bar");
    table.commit(table.get_latest_revision_number() + 1);
    TEST_EQUAL(get_filesize(tmpdir + "/test_table2_DB"), 8192);

    return true;
}

/// Test making and playing with a Btree
static bool test_table3()
{
    const string tablename = tmpdir + "/test_table3_";
    unlink_table(tablename);

    Btree table(tablename, false);
    table.create(8192);
    table.open();

    table.commit(table.get_latest_revision_number() + 1);

    table.add("trad", string(2200, 'a'));
    table.add("trade", string(3800, 'b'));
    table.add("tradea", string(2000, 'c'));

    table.commit(table.get_latest_revision_number() + 1);

    {
	Bcursor * cursor = table.cursor_get();
	TEST(cursor->find_entry("trade"));
	TEST_EQUAL(cursor->current_key, "trade");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag.size(), 3800);

	cursor->next();
	TEST_EQUAL(cursor->current_key, "tradea");
	delete cursor;
    }

    table.add("trade", string(4000, 'd'));
    table.commit(table.get_latest_revision_number() + 1);

    {
	Bcursor * cursor = table.cursor_get();
	TEST(cursor->find_entry("trade"));
	TEST_EQUAL(cursor->current_key, "trade");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag.size(), 4000);

	cursor->next();
	TEST_EQUAL(cursor->current_key, "tradea");
	delete cursor;
    }

    return true;
}

/// Test making and playing with a Btree
static bool test_table4()
{
    const string tablename = tmpdir + "/test_table4_";
    unlink_table(tablename);
    Btree table_rw(tablename, false);
    table_rw.create(8192);
    table_rw.open();
    Btree table_ro(tablename, true);
    table_ro.open();

    TEST_EQUAL(table_ro.get_entry_count(), 0);
    TEST_EQUAL(table_rw.get_entry_count(), 0);

    table_rw.del("foo1");
    TEST_EQUAL(table_ro.get_entry_count(), 0);
    TEST_EQUAL(table_rw.get_entry_count(), 0);

    table_rw.add("foo1", "");
    TEST_EQUAL(table_ro.get_entry_count(), 0);
    TEST_EQUAL(table_rw.get_entry_count(), 1);

    quartz_revision_number_t new_revision =
	    table_ro.get_latest_revision_number() + 1;
    table_rw.commit(new_revision);
    table_ro.open();
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 1);

    table_rw.add("foo1", "");
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 1);

    table_rw.del("foo1");
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 0);

    table_rw.del("foo1");
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 0);

    table_rw.add("bar", "");
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 1);

    table_rw.add("bar2", "");
    TEST_EQUAL(table_ro.get_entry_count(), 1);
    TEST_EQUAL(table_rw.get_entry_count(), 2);

    new_revision += 1;
    table_rw.commit(new_revision);
    table_ro.open();

    TEST_EQUAL(table_ro.get_entry_count(), 2);
    TEST_EQUAL(table_rw.get_entry_count(), 2);

    return true;
}

/// Test making and playing with a Btree
static bool test_table5()
{
    const string tablename = tmpdir + "/test_table5_";
    unlink_table(tablename);
    quartz_revision_number_t new_revision;
    quartz_revision_number_t old_revision;
    {
	// Open table and add a few documents
	Btree table_rw(tablename, false);
	table_rw.create(8192);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 0);
	TEST_EQUAL(table_rw.get_entry_count(), 0);

	table_rw.add("foo1", "bar1");
	table_rw.add("foo2", "bar2");
	table_rw.add("foo3", "bar3");

	new_revision = table_ro.get_latest_revision_number() + 1;
	table_rw.commit(new_revision);
	table_ro.open();

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree table_rw(tablename, false);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 3);
	TEST_EQUAL(table_rw.get_entry_count(), 3);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());

	Bcursor * cursor = table_rw.cursor_get();
	TEST(!cursor->find_entry("foo"));
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
	table_rw.add("foo25", "bar25");
	old_revision = new_revision;
	new_revision += 1;
	table_rw.commit(new_revision);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 4);
	TEST_EQUAL(table_rw.get_entry_count(), 4);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());
	delete cursor;
    }
    {
	// Open old revision
	Btree table_rw(tablename, false);
	TEST(table_rw.open(old_revision));
	Btree table_ro(tablename, true);
	table_ro.open(old_revision);

	TEST_EQUAL(table_ro.get_entry_count(), 3);
	TEST_EQUAL(table_rw.get_entry_count(), 3);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(old_revision, table_ro.get_open_revision_number());

	// Add a new tag
	table_rw.add("foo26", "bar26");
	new_revision += 1;
	table_rw.commit(new_revision);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 4);
	TEST_EQUAL(table_rw.get_entry_count(), 4);

	// Add another new tag, but don't apply this one.
	table_rw.add("foo37", "bar37");
	TEST_EQUAL(table_ro.get_entry_count(), 4);
	TEST_EQUAL(table_rw.get_entry_count(), 5);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree table_rw(tablename, false);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 4);
	TEST_EQUAL(table_rw.get_entry_count(), 4);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());

	Bcursor * cursor = table_rw.cursor_get();
	TEST(!cursor->find_entry("foo"));
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
	delete cursor;
    }
    {
	// Check that opening a nonexistant revision returns false (and doesn't
	// throw an exception).
	Btree table_ro(tablename, false);
	TEST(!table_ro.open(new_revision + 10));
    }

    return true;
}

/// Test making and playing with a Btree
static bool test_table6()
{
    const string tablename = tmpdir + "/test_table6_";
    unlink_table(tablename);
    quartz_revision_number_t new_revision;
    {
	// Open table and add a couple of documents
	Btree table_rw(tablename, false);
	table_rw.create(8192);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 0);
	TEST_EQUAL(table_rw.get_entry_count(), 0);

	table_rw.add("foo1", "bar1");

	table_rw.add("foo2", "bar2");
	table_rw.cancel();

	table_rw.add("foo3", "bar3");

	new_revision = table_ro.get_latest_revision_number() + 1;
	table_rw.commit(new_revision);
	table_ro.open();

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree table_rw(tablename, false);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 1);
	TEST_EQUAL(table_rw.get_entry_count(), 1);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());

	Bcursor * cursor = table_rw.cursor_get();
	TEST(!cursor->find_entry("foo"));
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
	delete cursor;
    }
    return true;
}

/// Test Bcursors
static bool test_cursor1()
{
    const string tablename = tmpdir + "/test_cursor1_";
    unlink_table(tablename);

    // Open table and put stuff in it.
    Btree table_rw(tablename, false);
    table_rw.create(8192);
    table_rw.open();
    Btree table_ro(tablename, true);
    table_ro.open();

    table_rw.add("foo1", "bar1");
    table_rw.add("foo2", "bar2");
    table_rw.add("foo3", "bar3");
    quartz_revision_number_t new_revision = table_ro.get_latest_revision_number();
    new_revision += 1;
    table_rw.commit(new_revision);
    table_ro.open();

    Btree * table = &table_ro;
    int count = 2;

    while (count != 0) {
	Bcursor * cursor = table->cursor_get();
	TEST(!cursor->find_entry("foo25"));
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

	TEST(!cursor->find_entry("foo"));
	TEST_EQUAL(cursor->current_key, "");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "");

	cursor->next();
	TEST(!cursor->after_end());
	TEST_EQUAL(cursor->current_key, "foo1");
	cursor->read_tag();
	TEST_EQUAL(cursor->current_tag, "bar1");

	TEST(cursor->find_entry("foo2"));
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

	table = &table_rw;
	count -= 1;

	delete cursor;
    }

    // Test cursors when we have unapplied changes
    table_rw.add("foo25", "bar25");

    table_rw.del("foo26");
    table_rw.del("foo1");

    Bcursor * cursor = table_ro.cursor_get();
    TEST(!cursor->find_entry("foo25"));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");
    delete cursor;

    cursor = table_rw.cursor_get();
    TEST(cursor->find_entry("foo25"));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");
    delete cursor;

    cursor = table_rw.cursor_get();
    TEST(!cursor->find_entry("foo26"));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    TEST(cursor->find_entry("foo2"));
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

    TEST(!cursor->find_entry("foo1"));
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
    delete cursor;

    new_revision += 1;
    table_rw.commit(new_revision);
    table_ro.open();

    cursor = table_rw.cursor_get();
    TEST(cursor->find_entry("foo2"));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    TEST(!cursor->find_entry("foo24"));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    TEST(cursor->find_entry("foo25"));
    TEST_EQUAL(cursor->current_key, "foo25");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar25");

    TEST(!cursor->find_entry("foo24"));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    table_rw.del("foo25");
    delete cursor;

    cursor = table_rw.cursor_get();
    TEST(!cursor->find_entry("foo25"));
    TEST_EQUAL(cursor->current_key, "foo2");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar2");

    cursor->next();
    TEST(!cursor->after_end());
    TEST_EQUAL(cursor->current_key, "foo3");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, "bar3");

    delete cursor;

    return true;
}

/// Regression test for cursors
static bool test_cursor2()
{
    const string tablename = tmpdir + "/test_cursor2_";
    unlink_table(tablename);

    // Open table and put stuff in it.
    Btree table_rw(tablename, false);
    table_rw.create(8192);
    table_rw.open();
    Btree table_ro(tablename, true);
    table_ro.open();

    table_rw.add("a", string(2036, '\x00'));
    table_rw.add("c", "bar2");
    quartz_revision_number_t new_revision = table_ro.get_latest_revision_number();
    new_revision += 1;
    table_rw.commit(new_revision);
    table_ro.open();

    Bcursor * cursor = table_ro.cursor_get();

    TEST(!cursor->find_entry("b"));
    TEST_EQUAL(cursor->current_key, "a");
    cursor->read_tag();
    TEST_EQUAL(cursor->current_tag, string(2036, '\x00'));

    delete cursor;

    return true;
}

/// Check the values returned by a table containing key/tag "hello"/"world"
/// Test making and playing with a Btree
static bool test_cursor3()
{
    const string tablename = tmpdir + "/test_cursor3_";
    unlink_table(tablename);
    quartz_revision_number_t new_revision;
    {
	// Open table and add a couple of documents
	Btree table_rw(tablename, false);
	table_rw.create(8192);
	table_rw.open();
	Btree table_ro(tablename, true);
	table_ro.open();

	TEST_EQUAL(table_ro.get_entry_count(), 0);
	table_rw.add("A", "A");
	table_rw.add("B", "B");

	{
	    Bcursor * cursor = table_rw.cursor_get();
	    TEST(!cursor->find_entry("AA"));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");

	    delete cursor;
	}

	new_revision = table_ro.get_latest_revision_number() + 1;
	table_rw.commit(new_revision);
	table_ro.open();

	{
	    Bcursor * cursor = table_rw.cursor_get();
	    TEST(!cursor->find_entry("AA"));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");

	    delete cursor;
	}

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	Btree table_ro(tablename, false);
	table_ro.open();
	TEST_EQUAL(table_ro.get_entry_count(), 2);

	TEST_EQUAL(new_revision, table_ro.get_latest_revision_number());
	TEST_EQUAL(new_revision, table_ro.get_open_revision_number());

	{
	    Bcursor * cursor = table_ro.cursor_get();
	    TEST(!cursor->find_entry("AA"));
	    TEST_EQUAL(cursor->current_key, "A");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "A");

	    cursor->next();
	    TEST(!cursor->after_end());
	    TEST_EQUAL(cursor->current_key, "B");
	    cursor->read_tag();
	    TEST_EQUAL(cursor->current_tag, "B");

	    delete cursor;
	}
    }
    return true;
}

/// Test large bitmap files.
static bool test_bitmap1()
{
    const string tablename = tmpdir + "/test_bitmap1_";
    unlink_table(tablename);
    /* Use a small block size to make it easier to get a large bitmap */
    Btree table_rw(tablename, false);
    table_rw.create(2048);
    table_rw.open();
    Btree table_ro(tablename, true);
    table_ro.open();

    quartz_revision_number_t new_revision;

    for (int j = 0; j < 100; ++j) {
	for (int i = 1; i <= 1000; ++i) {
	    string str_i = om_tostring(i);
	    table_rw.add("foo" + om_tostring(j) + "_" + str_i, "bar" + str_i);
	}
	new_revision = table_ro.get_latest_revision_number() + 1;
	table_rw.commit(new_revision);
	table_ro.open();
    }
    return true;
}

/// Test overwriting a table.
static bool test_overwrite1()
{
    const string tablename = tmpdir + "/test_overwrite1_";
    unlink_table(tablename);
    Btree bufftable(tablename, false);
    bufftable.create(2048);
    bufftable.open();
    Btree disktable(tablename, true);
    disktable.open();

    for (int i = 1; i <= 1000; ++i) {
	bufftable.add("foo" + om_tostring(i), "bar" + om_tostring(i));
    }

    bufftable.commit(disktable.get_latest_revision_number() + 1);
    disktable.open();

    Btree disktable_ro(tablename, true);
    disktable_ro.open();
    string tag;
    TEST(disktable_ro.get_exact_entry("foo1", tag));
    TEST_EQUAL(tag, "bar1");

    bufftable.add("foo1", "bar2");
    bufftable.commit(disktable.get_latest_revision_number() + 1);
    disktable.open();
    TEST(disktable_ro.get_exact_entry("foo999", tag));
    TEST(disktable_ro.get_exact_entry("foo1", tag));
    TEST_EQUAL(tag, "bar1");

    bufftable.add("foo1", "bar3");
    bufftable.commit(disktable.get_latest_revision_number() + 1);
    disktable.open();
    TEST(disktable_ro.get_exact_entry("foo999", tag));
    TEST_EXCEPTION(Xapian::DatabaseModifiedError,
		   disktable_ro.get_exact_entry("foo1", tag));
    //TEST_EQUAL(tag, "bar1");

    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================
//
// The lists of tests to perform
test_desc tests[] = {
    {"simple1",		test_simple1},
    {"insertdelete1",   test_insertdelete1},
    {"sequent1",        test_sequent1},
    {"emptykey1",       test_emptykey1},
    {"table1",		test_table1},
    {"table2",		test_table2},
    {"table3",		test_table3},
    {"table4",		test_table4},
    {"table5",		test_table5},
    {"table6",		test_table6},
    {"cursor1",		test_cursor1},
    {"cursor2",		test_cursor2},
    {"cursor3", 	test_cursor3},
    {"bitmap1", 	test_bitmap1},
    {"overwrite1", 	test_overwrite1},
    {0, 0}
};

int main(int argc, char **argv)
{
    const char * e_tmpdir = getenv("BTREETMP");
    if (e_tmpdir) {
	tmpdir = e_tmpdir;
    } else {
	tmpdir = ".btreetmp";
    }
    rmdir(tmpdir);
    make_dir(tmpdir);
    test_driver::parse_command_line(argc, argv);
    datadir = test_driver::get_srcdir() + "/testdata/btreetest_";
    return test_driver::run(tests);
}
