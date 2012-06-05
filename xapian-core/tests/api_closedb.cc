/** @file api_closedb.cc
 * @brief Tests of closing databases.
 */
/* Copyright 2008,2009 Lemur Consulting Ltd
 * Copyright 2009,2012 Olly Betts
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

#include "api_closedb.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

#define COUNT_CLOSEDEXC(CODE) \
    try { CODE; } catch (const Xapian::DatabaseError &) { ++closedexc_count; }

#define IF_NOT_CLOSEDEXC(CODE) \
    do { \
	hadexc = false; \
	try { \
	    CODE; \
	} catch (const Xapian::DatabaseError &) { \
	    ++closedexc_count; \
	    hadexc = true; \
	} \
    } while (false); if (hadexc)

// Iterators used by closedb1.
struct closedb1_iterators {
    Xapian::Database db;
    Xapian::Document doc1;
    Xapian::PostingIterator pl1;
    Xapian::PostingIterator pl2;
    Xapian::PostingIterator plend;

    void setup(Xapian::Database db_) {
	db = db_;

	// Set up the iterators for the test.
	pl1 = db.postlist_begin("paragraph");
	pl2 = db.postlist_begin("paragraph");
	++pl2;
	plend = db.postlist_end("paragraph");
    }

    int perform() {
	int closedexc_count = 0;
	bool hadexc;

	// Getting a document may throw closed.
	IF_NOT_CLOSEDEXC(doc1 = db.get_document(1)) {
	    COUNT_CLOSEDEXC(TEST_EQUAL(doc1.get_data().substr(0, 33),
				       "This is a test document used with"));
	    COUNT_CLOSEDEXC(doc1.termlist_begin());
	}

	// Causing the database to access its files raises the "database
	// closed" error.
	COUNT_CLOSEDEXC(db.postlist_begin("paragraph"));
	COUNT_CLOSEDEXC(db.get_document(1).get_value(1));
	COUNT_CLOSEDEXC(db.termlist_begin(1));
	COUNT_CLOSEDEXC(db.positionlist_begin(1, "paragraph"));
	COUNT_CLOSEDEXC(db.allterms_begin());
	COUNT_CLOSEDEXC(db.allterms_begin("p"));
	COUNT_CLOSEDEXC(db.get_termfreq("paragraph"));
	COUNT_CLOSEDEXC(db.get_collection_freq("paragraph"));
	COUNT_CLOSEDEXC(db.term_exists("paragraph"));
	try {
	    COUNT_CLOSEDEXC(db.get_value_freq(1));
	    COUNT_CLOSEDEXC(db.get_value_lower_bound(1));
	    COUNT_CLOSEDEXC(db.get_value_upper_bound(1));
	} catch (const Xapian::UnimplementedError &) { /* Not implemented for flint */ }
	COUNT_CLOSEDEXC(db.valuestream_begin(1));
	COUNT_CLOSEDEXC(db.get_doclength(1));

	// Reopen raises the "database closed" error.
	COUNT_CLOSEDEXC(db.reopen());

	TEST_NOT_EQUAL(pl1, plend);

	COUNT_CLOSEDEXC(db.postlist_begin("paragraph"));

	COUNT_CLOSEDEXC(TEST_EQUAL(*pl1, 1));
	COUNT_CLOSEDEXC(TEST_EQUAL(pl1.get_doclength(), 28));

	// Advancing the iterator may or may not raise an error, but if it
	// doesn't it must return the correct answers.
	bool advanced = false;
	try {
	    ++pl1;
	    advanced = true;
	} catch (const Xapian::DatabaseError &) {}

	if (advanced) {
	    COUNT_CLOSEDEXC(TEST_EQUAL(*pl1, 2));
	    COUNT_CLOSEDEXC(TEST_EQUAL(pl1.get_doclength(), 81));
	}

	return closedexc_count;
    }
};

// Test for closing a database
DEFINE_TESTCASE(closedb1, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    closedb1_iterators iters;

    // Run the test, checking that we get no "closed" exceptions.
    iters.setup(db);
    int closedexc_count = iters.perform();
    TEST_EQUAL(closedexc_count, 0);

    // Setup for the next test.
    iters.setup(db);

    // Close the database.
    db.close();

    // Reopening a closed database should always raise DatabaseError.
    TEST_EXCEPTION(Xapian::DatabaseError, db.reopen());

    // Run the test again, checking that we get some "closed" exceptions.
    closedexc_count = iters.perform();
    TEST_NOT_EQUAL(closedexc_count, 0);

    // get_description() shouldn't throw an exception.  Actually do something
    // with the description, in case this method is marked as "pure" in the
    // future.
    TEST(!db.get_description().empty());

    // Calling close repeatedly is okay.
    db.close();

    return true;
}

// Test closing a writable database, and that it drops the lock.
DEFINE_TESTCASE(closedb2, writable && !inmemory && !remote) {
    Xapian::WritableDatabase dbw1(get_named_writable_database("apitest_closedb2"));
    TEST_EXCEPTION(Xapian::DatabaseLockError,
		   Xapian::WritableDatabase db(get_named_writable_database_path("apitest_closedb2"),
					       Xapian::DB_OPEN));
    dbw1.close();
    Xapian::WritableDatabase dbw2 = get_named_writable_database("apitest_closedb2");
    TEST_EXCEPTION(Xapian::DatabaseError, dbw1.postlist_begin("paragraph"));
    TEST_EQUAL(dbw2.postlist_begin("paragraph"), dbw2.postlist_end("paragraph"));

    return true;
}

/// Check API methods which might either work or throw an exception.
DEFINE_TESTCASE(closedb3, backend) {
    Xapian::Database db(get_database("etext"));
    const string & uuid = db.get_uuid();
    db.close();
    try {
	TEST_EQUAL(db.get_uuid(), uuid);
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST(db.has_positions());
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST_EQUAL(db.get_doccount(), 566);
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST_EQUAL(db.get_lastdocid(), 566);
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST_REL(db.get_doclength_lower_bound(), <, db.get_avlength());
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST_REL(db.get_doclength_upper_bound(), >, db.get_avlength());
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	TEST(db.get_wdf_upper_bound("king"));
    } catch (const Xapian::DatabaseError &) {
    }
    try {
	// For non-remote databases, keep_alive() is a no-op anyway.
	db.keep_alive();
    } catch (const Xapian::DatabaseError &) {
    }
    return true;
}

/// Regression test for bug fixed in 1.1.4 - close() should implicitly commit().
DEFINE_TESTCASE(closedb4, writable && !inmemory) {
    Xapian::WritableDatabase wdb(get_writable_database());
    wdb.add_document(Xapian::Document());
    TEST_EQUAL(wdb.get_doccount(), 1);
    wdb.close();
    Xapian::Database db(get_writable_database_as_database());
    TEST_EQUAL(db.get_doccount(), 1);
    return true;
}

/// If a transaction is active, close() shouldn't implicitly commit().
DEFINE_TESTCASE(closedb5, transactions && !remote) {
    // FIXME: Fails with the remote backend, but I suspect it may be a test
    // harness issue.
    {
	Xapian::WritableDatabase wdb = get_writable_database();
	wdb.begin_transaction();
	wdb.add_document(Xapian::Document());
	TEST_EQUAL(wdb.get_doccount(), 1);
	wdb.close();
	Xapian::Database db = get_writable_database_as_database();
	TEST_EQUAL(db.get_doccount(), 0);
    }

    {
	// Same test but for an unflushed transaction.
	Xapian::WritableDatabase wdb = get_writable_database();
	wdb.begin_transaction(false);
	wdb.add_document(Xapian::Document());
	TEST_EQUAL(wdb.get_doccount(), 1);
	wdb.close();
	Xapian::Database db = get_writable_database_as_database();
	TEST_EQUAL(db.get_doccount(), 0);
    }
    return true;
}

/// Database::keep_alive() should fail after close() for a remote database.
DEFINE_TESTCASE(closedb6, remote) {
    Xapian::Database db(get_database("etext"));
    db.close();

    try {
	db.keep_alive();
	return false;
    } catch (const Xapian::DatabaseError &) {
    }
    return true;
}

// Test WritableDatabase methods.
DEFINE_TESTCASE(closedb7, writable) {
    Xapian::WritableDatabase db(get_writable_database());
    db.add_document(Xapian::Document());
    db.close();

    // Since we can't make any changes which need to be committed, db.commit()
    // is a no-op, and so doesn't have to fail.  Similarly we may be able to
    // call db.begin_transaction(), but we can't make any changes inside that
    // transaction.
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.add_document(Xapian::Document()));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.delete_document(1));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.replace_document(1, Xapian::Document()));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.replace_document(2, Xapian::Document()));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.replace_document("Qi", Xapian::Document()));

    return true;
}

// Test spelling related methods.
DEFINE_TESTCASE(closedb8, writable && spelling) {
    Xapian::WritableDatabase db(get_writable_database());
    db.add_spelling("pneumatic");
    db.add_spelling("pneumonia");
    db.close();

    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.add_spelling("penmanship"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.remove_spelling("pneumatic"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.get_spelling_suggestion("newmonia"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.spellings_begin());

    return true;
}

// Test synonym related methods.
DEFINE_TESTCASE(closedb9, writable && synonyms) {
    Xapian::WritableDatabase db(get_writable_database());
    db.add_synonym("color", "colour");
    db.add_synonym("honor", "honour");
    db.close();

    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.add_synonym("behavior", "behaviour"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.remove_synonym("honor", "honour"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.clear_synonyms("honor"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.synonyms_begin("color"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.synonym_keys_begin());

    return true;
}

// Test metadata related methods.
DEFINE_TESTCASE(closedb10, writable && metadata) {
    Xapian::WritableDatabase db(get_writable_database());
    db.set_metadata("foo", "FOO");
    db.set_metadata("bar", "BAR");
    db.close();

    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.set_metadata("test", "TEST"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.get_metadata("foo"));
    TEST_EXCEPTION(Xapian::DatabaseError,
		   db.get_metadata("foo"));
    try {
	TEST_EXCEPTION(Xapian::DatabaseError, db.metadata_keys_begin());
    } catch (const Xapian::UnimplementedError &) {
	/* Not implemented for inmemory */
    }

    return true;
}
