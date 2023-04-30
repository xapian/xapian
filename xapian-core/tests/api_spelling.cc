/** @file
 * @brief Test the spelling correction suggestion API.
 */
/* Copyright (C) 2007-2023 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#include "api_spelling.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

#include <string>

using namespace std;

// Test add_spelling() and remove_spelling(), which remote dbs support.
DEFINE_TESTCASE(spell0, (spelling || remote) && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    db.add_spelling("hello");
    db.add_spelling("cell", 2);
    db.commit();
    db.add_spelling("zig");
    db.add_spelling("ch");
    db.add_spelling("hello", 2);
    db.remove_spelling("hello", 2);
    db.remove_spelling("cell", 6);
    db.commit();
    db.remove_spelling("hello");
    db.remove_spelling("nonsuch");
    db.remove_spelling("zzzzzzzzz", 1000000);
    db.remove_spelling("aarvark");
    db.remove_spelling("hello");
    db.commit();
    db.remove_spelling("hello");
}

// Test basic spelling correction features.
DEFINE_TESTCASE(spell1, spelling && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that the more frequent term is chosen.
    db.add_spelling("hello");
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.add_spelling("cell", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(dbr.get_spelling_suggestion("hell"), "cell");

    // Check suggestions for single edit errors to "zig".
    db.add_spelling("zig");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("izg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zgi"), "zig");
    // Substitutions:
    TEST_EQUAL(db.get_spelling_suggestion("sig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zog"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zif"), "zig");
    // Deletions:
    TEST_EQUAL(db.get_spelling_suggestion("ig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zi"), "zig");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("azig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zaig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziag"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziga"), "zig");

    // Check suggestions for single edit errors to "ch".
    db.add_spelling("ch");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("hc"), "ch");
    // Substitutions - we don't handle these for two character words:
    TEST_EQUAL(db.get_spelling_suggestion("qh"), "");
    TEST_EQUAL(db.get_spelling_suggestion("cq"), "");
    // Deletions would leave a single character, and we don't handle those.
    TEST_EQUAL(db.get_spelling_suggestion("c"), "");
    TEST_EQUAL(db.get_spelling_suggestion("h"), "");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("qch"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("cqh"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("chq"), "ch");

    // Check assorted cases:
    TEST_EQUAL(db.get_spelling_suggestion("shello"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("hellot"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("acell"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("acella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helo"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helol"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("clel"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("ecll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");

    // Check that edit distance 3 isn't found by default:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx"), "");
    TEST_EQUAL(db.get_spelling_suggestion("celling"), "");
    TEST_EQUAL(db.get_spelling_suggestion("dellin"), "");

    // Check that edit distance 3 is found if specified:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx", 3), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("celling", 3), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("dellin", 3), "cell");

    // Make "hello" more frequent than "cell" (3 vs 2).
    db.add_spelling("hello", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "hello");
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("cello"), "hello");
    db.remove_spelling("hello", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    // Test "over-removing".
    db.remove_spelling("cell", 6);
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.remove_spelling("hello");
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "");

    // Test removing words not in the table.
    db.remove_spelling("nonsuch");
    db.remove_spelling("zzzzzzzzz", 1000000);
    db.remove_spelling("aarvark");

    // Try removing word which was present but no longer is.
    db.remove_spelling("hello");
    db.commit();
    db.remove_spelling("hello");
}

// Test spelling correction for Unicode.
DEFINE_TESTCASE(spell2, spelling && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that a UTF-8 sequence counts as a single character.
    db.add_spelling("h\xc3\xb6hle");
    db.add_spelling("ascii");
    TEST_EQUAL(db.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(db.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(dbr.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(dbr.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");
}

// Test spelling correction with multi databases
DEFINE_TESTCASE(spell3, spelling) {
    Xapian::Database db1 = get_database("spell3a",
					[](Xapian::WritableDatabase& wdb,
					   const string&) {
					    wdb.add_spelling("hello");
					    wdb.add_spelling("cell", 2);
					});
    Xapian::Database db2 = get_database("spell3b",
					[](Xapian::WritableDatabase& wdb,
					   const string&) {
					    wdb.add_spelling("hello", 2);
					    wdb.add_spelling("helo");
					});

    Xapian::Database db;
    db.add_database(db1);
    db.add_database(db2);

    TEST_EQUAL(db.get_spelling_suggestion("hello"), "");
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "hello");
    TEST_EQUAL(db1.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(db2.get_spelling_suggestion("hell"), "hello");

    // Test spelling iterator
    Xapian::TermIterator i(db1.spellings_begin());
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db1.spellings_end());

    i = db2.spellings_begin();
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db2.spellings_end());

    i = db.spellings_begin();
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 3);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db.spellings_end());

    // Regression test for TermIterator::skip_to() bug fixed in 1.4.19.
    i = db.spellings_begin();
    i.skip_to("helo");
    TEST(i != db.spellings_end());
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    i.skip_to("help");
    TEST(i == db.spellings_end());
}

// Regression test - check that appending works correctly.
DEFINE_TESTCASE(spell4, spelling) {
    Xapian::Database db = get_database("spell4",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   wdb.add_spelling("check");
					   wdb.add_spelling("pecks", 2);
					   wdb.commit();
					   wdb.add_spelling("becky");
				       });

    TEST_EQUAL(db.get_spelling_suggestion("jeck", 2), "pecks");
}

// Regression test - used to segfault with some input values.
DEFINE_TESTCASE(spell5, spelling) {
    // Using constexpr instead of a macro fails with MSVC - it's not visible
    // inside the lambda, and we can't explicitly capture it or else the lambda
    // can't be passed as a function pointer.
#define TARGET "\xe4\xb8\x80\xe4\xba\x9b"
    Xapian::Database db = get_database("spell5",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   wdb.add_spelling(TARGET);
				       });

    string s = db.get_spelling_suggestion("\xe4\xb8\x8d", 3);
    TEST_EQUAL(s, TARGET);
#undef TARGET
}

// Test basic spelling correction features.
DEFINE_TESTCASE(spell6, spelling && writable) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that the more frequent term is chosen.
    db.add_spelling("hello", 2);
    db.add_spelling("sell", 3);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "sell");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "sell");
    TEST_EQUAL(dbr.get_spelling_suggestion("hell"), "sell");
}

// Test suggestions when there's an exact match.
DEFINE_TESTCASE(spell7, spelling) {
    Xapian::Database db = get_database("spell7",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   wdb.add_spelling("word", 57);
					   wdb.add_spelling("wrod", 3);
					   wdb.add_spelling("sword", 56);
					   wdb.add_spelling("words", 57);
					   wdb.add_spelling("ward", 58);
				       });

    // Check that the more frequent term is chosen.
    TEST_EQUAL(db.get_spelling_suggestion("ward"), "");
    TEST_EQUAL(db.get_spelling_suggestion("words"), "word");
    TEST_EQUAL(db.get_spelling_suggestion("sword"), "word");
    TEST_EQUAL(db.get_spelling_suggestion("wrod"), "word");
}

/// Regression test - repeated trigrams cancelled in 1.2.5 and earlier.
DEFINE_TESTCASE(spell8, spelling) {
    Xapian::Database db = get_database("spell8",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   wdb.add_spelling("skinking", 2);
					   wdb.add_spelling("stinking", 1);
				       });

    // kin and kin used to cancel out in "skinking".
    TEST_EQUAL(db.get_spelling_suggestion("scimkin", 3), "skinking");
}
