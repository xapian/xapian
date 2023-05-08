/** @file
 * @brief Test the user metadata functionality.
 */
/* Copyright (C) 2007,2009,2011 Olly Betts
 * Copyright (C) 2007,2008,2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_metadata.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

#include <string>

using namespace std;

// Test metadata methods for non-writable databases.
DEFINE_TESTCASE(metadata1, metadata) {
    Xapian::Database db = get_database("metadata1",
				       [](Xapian::WritableDatabase& wdb,
					  const string&)
				       {
					   wdb.set_metadata("empty", "");
					   wdb.set_metadata("foo", "bar");
					   wdb.set_metadata(string("", 1),
							    string("", 1));
					   wdb.set_metadata(string("\0fo", 3),
							    string("\0xx", 3));
					   wdb.set_metadata(string("f\0o", 3),
							    string("x\0x", 3));
					   wdb.set_metadata(string("fo\0", 3),
							    string("xx\0", 3));
				       });
    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_metadata("empty"), "");
    TEST_EQUAL(db.get_metadata("unset"), "");
    TEST_EQUAL(db.get_metadata("foo"), "bar");

    // Check for transparent handling of zero bytes.
    TEST_EQUAL(db.get_metadata(string("", 1)), string("", 1));
    TEST_EQUAL(db.get_metadata(string("\0fo", 3)), string("\0xx", 3));
    TEST_EQUAL(db.get_metadata(string("f\0o", 3)), string("x\0x", 3));
    TEST_EQUAL(db.get_metadata(string("fo\0", 3)), string("xx\0", 3));

    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.get_metadata(""));
}

// Basic test of metadata methods.
DEFINE_TESTCASE(metadata2, metadata && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    TEST_EQUAL(db.get_metadata("foo"), "");
    db.set_metadata("foo", "bar");
    TEST_EQUAL(db.get_metadata("foo"), "bar");
    db.set_metadata("foo", "baz");
    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_metadata("foo"), "baz");
    db.set_metadata("foo", "");
    TEST_EQUAL(db.get_metadata("foo"), "");

    TEST_EQUAL(db.get_doccount(), 0);

    // Check for transparent handling of zero bytes.
    db.set_metadata("foo", "value of foo");
    db.set_metadata(string("foo\0bar", 7), string(1, '\0'));
    db.set_metadata(string("foo\0", 4), string("foo\0bar", 7));

    TEST_EQUAL(db.get_metadata("foo"), "value of foo");
    TEST_EQUAL(db.get_metadata(string("foo\0bar", 7)), string(1, '\0'));
    TEST_EQUAL(db.get_metadata(string("foo\0", 4)), string("foo\0bar", 7));

    db.commit();

    TEST_EQUAL(db.get_metadata("foo"), "value of foo");
    TEST_EQUAL(db.get_metadata(string("foo\0bar", 7)), string(1, '\0'));
    TEST_EQUAL(db.get_metadata(string("foo\0", 4)), string("foo\0bar", 7));
}

// Test that metadata gets applied at same time as other changes.
DEFINE_TESTCASE(metadata3, metadata && writable && !inmemory) {
    // get_writable_database_as_database() not implemented for inmemory.
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Database dbr = get_writable_database_as_database();

    TEST_EQUAL(db.get_metadata("foo"), "");
    db.set_metadata("foo", "bar");
    TEST_EQUAL(db.get_metadata("foo"), "bar");
    TEST_EQUAL(dbr.get_metadata("foo"), "");
    db.commit();
    TEST_EQUAL(dbr.get_metadata("foo"), "");
    TEST(dbr.reopen());
    TEST_EQUAL(db.get_metadata("foo"), "bar");
    TEST_EQUAL(dbr.get_metadata("foo"), "bar");
    TEST_EQUAL(dbr.get_doccount(), 0);

    db.add_document(Xapian::Document());
    db.set_metadata("foo", "baz");
    TEST_EQUAL(db.get_doccount(), 1);
    TEST_EQUAL(db.get_metadata("foo"), "baz");
    db.commit();

    TEST_EQUAL(dbr.get_metadata("foo"), "bar");
    TEST(dbr.reopen());
    TEST_EQUAL(dbr.get_metadata("foo"), "baz");

    db.set_metadata("foo", "");
    TEST_EQUAL(db.get_metadata("foo"), "");
    db.commit();
    TEST_EQUAL(dbr.get_metadata("foo"), "baz");
    TEST(dbr.reopen());
    TEST_EQUAL(dbr.get_metadata("foo"), "");

    TEST_EQUAL(db.get_doccount(), 1);
}

// Test the empty metadata keys give an error correctly.
DEFINE_TESTCASE(metadata4, metadata && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.get_metadata(""));
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.set_metadata("", "foo"));
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.get_metadata(""));
}

// Regression test for adding a piece of metadata on its own before adding
// other things.
DEFINE_TESTCASE(metadata5, metadata && writable && !inmemory) {
    // get_writable_database_as_database() not implemented for inmemory.
    Xapian::WritableDatabase db = get_writable_database();

    db.set_metadata("foo", "foo");
    db.commit();

    Xapian::Document doc;
    doc.add_posting("foo", 1);
    db.add_document(doc);

    Xapian::Database dbr(get_writable_database_as_database());
}

// Test metadata iterators.
DEFINE_TESTCASE(metadata6, writable) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that iterator on empty database returns nothing.
    Xapian::TermIterator iter;
    iter = db.metadata_keys_begin();
    TEST_EQUAL(iter, db.metadata_keys_end());

    // FIXME: inmemory doesn't implement metadata iterators yet, except in the
    // trivial case of there being no keys to iterate.
    SKIP_TEST_FOR_BACKEND("inmemory");

    try {
	db.set_metadata("foo", "val");
    } catch (const Xapian::UnimplementedError &) {
	SKIP_TEST("Metadata not supported by this backend");
    }
    db.commit();

    // Check iterator on a database with only metadata items.
    iter = db.metadata_keys_begin();
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo");
    ++iter;
    TEST(iter == db.metadata_keys_end());

    // Check iterator on a database with metadata items and documents.
    Xapian::Document doc;
    doc.add_posting("foo", 1);
    db.add_document(doc);
    db.commit();

    iter = db.metadata_keys_begin();
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo");
    ++iter;
    TEST(iter == db.metadata_keys_end());

    // Check iterator on a database with documents but no metadata.  Also
    // checks that setting metadata to empty stops the iterator returning it.
    db.set_metadata("foo", "");
    db.commit();
    iter = db.metadata_keys_begin();
    TEST(iter == db.metadata_keys_end());

    // Check use of a prefix, and skip_to.
    db.set_metadata("a", "val");
    db.set_metadata("foo", "val");
    db.set_metadata("foo1", "val");
    db.set_metadata("foo2", "val");
    db.set_metadata("z", "val");
    db.commit();

    iter = db.metadata_keys_begin();
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "a");
    ++iter;
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo");
    ++iter;
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo1");
    ++iter;
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo2");
    ++iter;
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "z");
    ++iter;
    TEST(iter == db.metadata_keys_end());

    iter = db.metadata_keys_begin("foo");
    TEST(iter != db.metadata_keys_end("foo"));
    TEST_EQUAL(*iter, "foo");
    ++iter;
    TEST(iter != db.metadata_keys_end("foo"));
    TEST_EQUAL(*iter, "foo1");
    ++iter;
    TEST(iter != db.metadata_keys_end("foo"));
    TEST_EQUAL(*iter, "foo2");
    ++iter;
    TEST(iter == db.metadata_keys_end("foo"));

    iter = db.metadata_keys_begin("foo1");
    TEST(iter != db.metadata_keys_end("foo1"));
    TEST_EQUAL(*iter, "foo1");
    ++iter;
    TEST(iter == db.metadata_keys_end("foo1"));

    iter = db.metadata_keys_begin();
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "a");

    // Skip to "" should move to the first key.
    iter.skip_to("");
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "a");

    // This skip_to should skip the "foo" key.
    iter.skip_to("foo1");
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo1");

    // Check that skipping to the current key works.
    iter.skip_to("foo1");
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo1");

    // Check that skip_to a key before the current one doesn't move forwards.
    iter.skip_to("a");
    TEST(iter != db.metadata_keys_end());
    TEST_REL(*iter, <=, "foo1");

    // Make sure we're back on foo1.
    iter.skip_to("foo1");
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo1");

    // Check that advancing after a skip_to() works correctly.
    ++iter;
    TEST(iter != db.metadata_keys_end());
    TEST_EQUAL(*iter, "foo2");

    // Check that skipping to a key after the last key works.
    iter.skip_to("zoo");
    TEST(iter == db.metadata_keys_end());
}

/// Regression test of reading after writing but not committing.
DEFINE_TESTCASE(writeread1, metadata && writable) {
    Xapian::WritableDatabase db_w = get_writable_database();
    db_w.set_metadata("1", "2");
    string longitem(20000, 'j');
    db_w.set_metadata("2", longitem);

    string readitem = db_w.get_metadata("2");
    TEST_EQUAL(readitem, longitem);
}
