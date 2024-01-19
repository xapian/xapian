/** @file
 * @brief Backend-related tests.
 */
/* Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015,2016,2018,2019 Olly Betts
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

#include "api_backend.h"

#define XAPIAN_DEPRECATED(X) X
#include <xapian.h>

#include "backendmanager.h"
#include "errno_to_string.h"
#include "filetests.h"
#include "str.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"

#include "apitest.h"

#include "safefcntl.h"
#include "safesysstat.h"
#include "safeunistd.h"
#ifdef HAVE_SOCKETPAIR
# include "safesyssocket.h"
# include <signal.h>
# include "safesyswait.h"
#endif

#include <cerrno>
#include <fstream>
#include <iterator>

using namespace std;

/// Regression test - lockfile should honour umask, was only user-readable.
DEFINE_TESTCASE(lockfileumask1, chert || glass) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __OS2__
    mode_t old_umask = umask(022);
    try {
	Xapian::WritableDatabase db = get_named_writable_database("lockfileumask1");

	string path = get_named_writable_database_path("lockfileumask1");
	path += "/flintlock";

	struct stat statbuf;
	TEST(stat(path.c_str(), &statbuf) == 0);
	TEST_EQUAL(statbuf.st_mode & 0777, 0644);
    } catch (...) {
	umask(old_umask);
	throw;
    }

    umask(old_umask);
#endif
}

/// Check that the backend handles total document length > 0xffffffff.
DEFINE_TESTCASE(totaldoclen1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_posting("foo", 1, 2000000000);
    db.add_document(doc);
    Xapian::Document doc2;
    doc2.add_posting("bar", 1, 2000000000);
    db.add_document(doc2);
    TEST_EQUAL(db.get_avlength(), 2000000000);
    TEST_EQUAL(db.get_total_length(), 4000000000ull);
    db.commit();
    TEST_EQUAL(db.get_avlength(), 2000000000);
    TEST_EQUAL(db.get_total_length(), 4000000000ull);
    for (int i = 0; i != 20; ++i) {
	Xapian::Document doc3;
	doc3.add_posting("count" + str(i), 1, 2000000000);
	db.add_document(doc3);
    }
    TEST_EQUAL(db.get_avlength(), 2000000000);
    TEST_EQUAL(db.get_total_length(), 44000000000ull);
    db.commit();
    TEST_EQUAL(db.get_avlength(), 2000000000);
    TEST_EQUAL(db.get_total_length(), 44000000000ull);
    if (get_dbtype() != "inmemory") {
	// InMemory doesn't support get_writable_database_as_database().
	Xapian::Database dbr = get_writable_database_as_database();
	TEST_EQUAL(dbr.get_avlength(), 2000000000);
	TEST_EQUAL(dbr.get_total_length(), 44000000000ull);
    }
}

// Check that exceeding 32bit in combined database doesn't cause a problem
// when using 64bit docids.
DEFINE_TESTCASE(exceed32bitcombineddb1, writable) {
    // Test case is for 64-bit Xapian::docid.
    // FIXME: Though we should check that the overflow is handled gracefully
    // for 32-bit...
    if (sizeof(Xapian::docid) == 4) return;

    // The InMemory backend uses a vector for the documents, so trying to add
    // a document with the maximum docid is likely to fail because we can't
    // allocate enough memory!
    SKIP_TEST_FOR_BACKEND("inmemory");

    Xapian::WritableDatabase db1 = get_writable_database();
    Xapian::Document doc;
    doc.set_data("prose");
    doc.add_term("word");
    Xapian::docid max_32bit_id = 0xffffffff;
    db1.replace_document(max_32bit_id, doc);
    db1.commit();

    Xapian::Database db2 = get_writable_database_as_database();

    Xapian::Database db;
    db.add_database(db1);
    db.add_database(db2);

    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query::MatchAll);
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    TEST_EQUAL(2, mymset.size());

    // We can't usefully check the shard docid if the testharness backend is
    // multi.
    bool multi = (db1.size() > 1);
    for (Xapian::MSetIterator i = mymset.begin(); i != mymset.end(); ++i) {
	doc = i.get_document();
	if (!multi)
	    TEST_EQUAL(doc.get_docid(), max_32bit_id);
	TEST_EQUAL(doc.get_data(), "prose");
    }
}

DEFINE_TESTCASE(dbstats1, backend) {
    Xapian::Database db = get_database("etext");

    // Use precalculated values to avoid expending CPU cycles to calculate
    // these every time without improving test coverage.
    const Xapian::termcount min_len = 2;
    const Xapian::termcount max_len = 532;
    const Xapian::termcount max_wdf = 22;

    if (get_dbtype() != "inmemory") {
	// Should be exact as no deletions have happened.
	TEST_EQUAL(db.get_doclength_upper_bound(), max_len);
	TEST_EQUAL(db.get_doclength_lower_bound(), min_len);
    } else {
	// For inmemory, we usually give rather loose bounds.
	TEST_REL(db.get_doclength_upper_bound(),>=,max_len);
	TEST_REL(db.get_doclength_lower_bound(),<=,min_len);
    }

    if (get_dbtype() != "inmemory" &&
	get_dbtype().find("remote") == string::npos) {
	TEST_EQUAL(db.get_wdf_upper_bound("the"), max_wdf);
    } else {
	// For inmemory and remote backends, we usually give rather loose
	// bounds (remote matches use tighter bounds, but querying the
	// wdf bound gives a looser one).
	TEST_REL(db.get_wdf_upper_bound("the"),>=,max_wdf);
    }

    // This failed with an assertion during development between 1.3.1 and
    // 1.3.2.
    TEST_EQUAL(db.get_wdf_upper_bound(""), 0);
}

// Check stats with a single document.  In a multi-database situation, this
// gave 0 for get-_doclength_lower_bound() in 1.3.2.
DEFINE_TESTCASE(dbstats2, backend) {
    Xapian::Database db = get_database("apitest_onedoc");

    // Use precalculated values to avoid expending CPU cycles to calculate
    // these every time without improving test coverage.
    const Xapian::termcount min_len = 15;
    const Xapian::termcount max_len = 15;
    const Xapian::termcount max_wdf = 7;

    if (get_dbtype() != "inmemory") {
	// Should be exact as no deletions have happened.
	TEST_EQUAL(db.get_doclength_upper_bound(), max_len);
	TEST_EQUAL(db.get_doclength_lower_bound(), min_len);
    } else {
	// For inmemory, we usually give rather loose bounds.
	TEST_REL(db.get_doclength_upper_bound(),>=,max_len);
	TEST_REL(db.get_doclength_lower_bound(),<=,min_len);
    }

    if (get_dbtype() != "inmemory" &&
	get_dbtype().find("remote") == string::npos) {
	TEST_EQUAL(db.get_wdf_upper_bound("word"), max_wdf);
    } else {
	// For inmemory and remote backends, we usually give rather loose
	// bounds (remote matches use tighter bounds, but querying the
	// wdf bound gives a looser one).
	TEST_REL(db.get_wdf_upper_bound("word"),>=,max_wdf);
    }

    TEST_EQUAL(db.get_wdf_upper_bound(""), 0);
}

/// Check handling of alldocs on an empty database.
DEFINE_TESTCASE(alldocspl3, backend) {
    Xapian::Database db = get_database(string());

    TEST_EQUAL(db.get_termfreq(string()), 0);
    TEST_EQUAL(db.get_collection_freq(string()), 0);
    TEST(db.postlist_begin(string()) == db.postlist_end(string()));
}

/// Regression test for bug#392 in ModifiedPostList iteration, fixed in 1.0.15.
DEFINE_TESTCASE(modifiedpostlist1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document a, b;
    Xapian::Enquire enq(db);

    a.add_term("T");
    enq.set_query(Xapian::Query("T"));

    db.replace_document(2, a);
    db.commit();
    db.replace_document(1, a);
    db.replace_document(1, b);

    mset_expect_order(enq.get_mset(0, 2), 2);
}

/// Regression test for chert bug fixed in 1.1.3 (ticket#397).
DEFINE_TESTCASE(doclenaftercommit1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_unique_terms(1));
    db.replace_document(1, Xapian::Document());
    db.commit();
    TEST_EQUAL(db.get_doclength(1), 0);
    TEST_EQUAL(db.get_unique_terms(1), 0);
}

DEFINE_TESTCASE(valuesaftercommit1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_value(0, "value");
    db.replace_document(2, doc);
    db.commit();
    db.replace_document(1, doc);
    db.replace_document(3, doc);
    TEST_EQUAL(db.get_document(3).get_value(0), "value");
    db.commit();
    TEST_EQUAL(db.get_document(3).get_value(0), "value");
}

DEFINE_TESTCASE(lockfilefd0or1, chert || glass) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __OS2__
    int old_stdin = dup(0);
    int old_stdout = dup(1);
    try {
	// With fd 0 available.
	close(0);
	{
	    Xapian::WritableDatabase db = get_writable_database();
	    TEST_EXCEPTION(Xapian::DatabaseLockError,
			   (void)get_writable_database_again());
	}
	// With fd 0 and fd 1 available.
	close(1);
	{
	    Xapian::WritableDatabase db = get_writable_database();
	    TEST_EXCEPTION(Xapian::DatabaseLockError,
			   (void)get_writable_database_again());
	}
	// With fd 1 available.
	dup2(old_stdin, 0);
	{
	    Xapian::WritableDatabase db = get_writable_database();
	    TEST_EXCEPTION(Xapian::DatabaseLockError,
			   (void)get_writable_database_again());
	}
    } catch (...) {
	dup2(old_stdin, 0);
	dup2(old_stdout, 1);
	close(old_stdin);
	close(old_stdout);
	throw;
    }

    dup2(old_stdout, 1);
    close(old_stdin);
    close(old_stdout);
#endif
}

/// Regression test for bug fixed in 1.2.13 and 1.3.1.
DEFINE_TESTCASE(lockfilealreadyopen1, chert || glass) {
    // Ensure database has been created.
    (void)get_named_writable_database("lockfilealreadyopen1");
    string path = get_named_writable_database_path("lockfilealreadyopen1");
    int fd = ::open((path + "/flintlock").c_str(), O_RDONLY);
    TEST(fd != -1);
    try {
	Xapian::WritableDatabase db(path, Xapian::DB_CREATE_OR_OPEN);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::WritableDatabase db2(path, Xapian::DB_CREATE_OR_OPEN)
	);
    } catch (...) {
	close(fd);
	throw;
    }
    close(fd);
}

/// Feature tests for Database::locked().
DEFINE_TESTCASE(testlock1, chert || glass) {
    Xapian::Database rdb;
    TEST(!rdb.locked());
    {
	Xapian::WritableDatabase db = get_named_writable_database("testlock1");
	TEST(db.locked());
	Xapian::Database db_as_database = db;
	TEST(db_as_database.locked());
	TEST(!rdb.locked());
	rdb = get_writable_database_as_database();
	TEST(db.locked());
	TEST(db_as_database.locked());
	try {
	    TEST(rdb.locked());
	} catch (const Xapian::FeatureUnavailableError&) {
	    SKIP_TEST("Database::locked() not supported on this platform");
	}
	db_as_database = rdb;
	TEST(db.locked());
	TEST(db_as_database.locked());
	TEST(rdb.locked());
	db_as_database.close();
	TEST(db.locked());
	TEST(rdb.locked());
	// After close(), locked() should either work as if close() hadn't been
	// called or throw Xapian::DatabaseClosedError.
	try {
	    TEST(db_as_database.locked());
	} catch (const Xapian::DatabaseClosedError&) {
	}
	db.close();
	TEST(!rdb.locked());
	try {
	    TEST(!db_as_database.locked());
	} catch (const Xapian::DatabaseClosedError&) {
	}
    }
    TEST(!rdb.locked());
}

/** Test that locked() returns false for backends which don't support update.
 *
 *  Regression test for bug fixed in 1.4.6.
 */
DEFINE_TESTCASE(testlock2, backend && !writable) {
    Xapian::Database db = get_database("apitest_simpledata");
    TEST(!db.locked());
    db.close();
    TEST(!db.locked());
}

/** Test locked() on inmemory Database objects.
 *
 *  An inmemory Database is always actually a WritableDatabase viewed as a
 *  Database, so it should always report being locked for writing, unless
 *  close() has been called.
 *
 *  Regression test for bug fixed in 1.4.14 - earlier versions always returned
 *  false for an inmemory Database here.
 *
 *  Regression test for bug fixed in 1.4.15 - false should be returned after
 *  close() has been called.
 */
DEFINE_TESTCASE(testlock3, inmemory) {
    Xapian::Database db = get_database("apitest_simpledata");
    TEST(db.locked());
    db.close();
    TEST(!db.locked());
}

/// Test locked() on closed WritableDatabase.
DEFINE_TESTCASE(testlock4, chert || glass) {
    Xapian::Database db = get_writable_database("apitest_simpledata");
    // Even if we don't have a way to test the lock on the current platform,
    // this should know the database is locked because this object holds the
    // lock.
    TEST(db.locked());
    db.close();
    try {
	TEST(!db.locked());
    } catch (const Xapian::FeatureUnavailableError&) {
	SKIP_TEST("Database::locked() not supported on this platform");
    }
}

class CheckMatchDecider : public Xapian::MatchDecider {
    mutable bool called;

  public:
    CheckMatchDecider() : called(false) { }

    bool operator()(const Xapian::Document &) const {
	called = true;
	return true;
    }

    bool was_called() const { return called; }
};

/// Test Xapian::MatchDecider with remote backend fails.
DEFINE_TESTCASE(matchdecider4, remote) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));

    CheckMatchDecider mdecider;
    Xapian::MSet mset;

    TEST_EXCEPTION(Xapian::UnimplementedError,
	mset = enquire.get_mset(0, 10, NULL, &mdecider));
    TEST(!mdecider.was_called());
}

/** Check that replacing an unmodified document doesn't increase the automatic
 *  flush counter.  Regression test for bug fixed in 1.1.4/1.0.18.
 */
DEFINE_TESTCASE(replacedoc7, writable && !inmemory && !remote) {
    // The inmemory backend doesn't batch changes, so there's nothing to
    // check there.
    //
    // The remote backend doesn't implement the lazy replacement of documents
    // optimisation currently.
    Xapian::WritableDatabase db(get_writable_database());
    Xapian::Document doc;
    doc.set_data("fish");
    doc.add_term("Hlocalhost");
    doc.add_posting("hello", 1);
    doc.add_posting("world", 2);
    doc.add_value(1, "myvalue");
    db.add_document(doc);
    db.commit();

    // We add a second document, and then replace the first document with
    // itself 10000 times.  If the document count for the database reopened
    // read-only is 2, then we triggered an automatic commit.

    doc.add_term("XREV2");
    db.add_document(doc);

    for (int i = 0; i < 10000; ++i) {
	doc = db.get_document(1);
	db.replace_document(1, doc);
    }

    Xapian::Database rodb(get_writable_database_as_database());
    TEST_EQUAL(rodb.get_doccount(), 1);

    db.flush();
    TEST(rodb.reopen());

    TEST_EQUAL(rodb.get_doccount(), 2);
}

/** Check that replacing a document deleted since the last flush works.
 *  Prior to 1.1.4/1.0.18, this failed to update the collection frequency and
 *  wdf, and caused an assertion failure when assertions were enabled.
 */
DEFINE_TESTCASE(replacedoc8, writable) {
    Xapian::WritableDatabase db(get_writable_database());
    {
	Xapian::Document doc;
	doc.set_data("fish");
	doc.add_term("takeaway");
	db.add_document(doc);
    }
    db.delete_document(1);
    {
	Xapian::Document doc;
	doc.set_data("chips");
	doc.add_term("takeaway", 2);
	db.replace_document(1, doc);
    }
    db.flush();
    TEST_EQUAL(db.get_collection_freq("takeaway"), 2);
    Xapian::PostingIterator p = db.postlist_begin("takeaway");
    TEST(p != db.postlist_end("takeaway"));
    TEST_EQUAL(p.get_wdf(), 2);
}

/** Check that replacing a document after clear_terms() still deletes old
 *  positional data.  Regression test for bug introduced and fixed in
 *  development prior to 1.5.0.
 */
DEFINE_TESTCASE(replacedoc9, writable) {
    Xapian::WritableDatabase db(get_named_writable_database("replacedoc9"));
    {
	Xapian::Document doc;
	doc.set_data("food");
	doc.add_posting("falafel", 1);
	db.add_document(doc);
    }
    db.commit();
    Xapian::Document doc = db.get_document(1);
    doc.clear_terms();
    doc.add_term("falafel");
    db.replace_document(1, doc);
    db.commit();

    // The positions should have been removed, but the bug meant they weren't.
    TEST_EQUAL(db.positionlist_begin(1, "falafel"),
	       db.positionlist_end(1, "falafel"));
}

/// Test coverage for DatabaseModifiedError.
DEFINE_TESTCASE(databasemodified1, writable && !inmemory && !multi) {
    // The inmemory backend doesn't support revisions.
    //
    // With multi, DatabaseModifiedError doesn't trigger as easily.
    Xapian::WritableDatabase db(get_writable_database());
    Xapian::Document doc;
    doc.set_data("cargo");
    doc.add_term("abc");
    doc.add_term("def");
    doc.add_term("ghi");
    const int N = 500;
    for (int i = 0; i < N; ++i) {
	db.add_document(doc);
    }
    db.commit();

    Xapian::Database rodb(get_writable_database_as_database());
    db.add_document(doc);
    db.commit();

    db.add_document(doc);
    db.commit();

    db.add_document(doc);
    try {
	TEST_EQUAL(*rodb.termlist_begin(N - 1), "abc");
	FAIL_TEST("Expected DatabaseModifiedError wasn't thrown");
    } catch (const Xapian::DatabaseModifiedError &) {
    }

    try {
	Xapian::Enquire enq(rodb);
	enq.set_query(Xapian::Query("abc"));
	Xapian::MSet mset = enq.get_mset(0, 10);
	FAIL_TEST("Expected DatabaseModifiedError wasn't thrown");
    } catch (const Xapian::DatabaseModifiedError &) {
    }
}

/// Regression test for bug#462 fixed in 1.0.19 and 1.1.5.
DEFINE_TESTCASE(qpmemoryleak1, writable && !inmemory) {
    // Inmemory never throws DatabaseModifiedError.
    Xapian::WritableDatabase wdb(get_writable_database());
    Xapian::Document doc;

    doc.add_term("foo");
    for (int i = 100; i < 120; ++i) {
	doc.add_term(str(i));
    }

    for (int j = 0; j < 50; ++j) {
	wdb.add_document(doc);
    }
    wdb.commit();

    Xapian::Database database(get_writable_database_as_database());
    Xapian::QueryParser queryparser;
    queryparser.set_database(database);
    TEST_EXCEPTION(Xapian::DatabaseModifiedError,
	for (int k = 0; k < 1000; ++k) {
	    wdb.add_document(doc);
	    wdb.commit();
	    (void)queryparser.parse_query("1", queryparser.FLAG_PARTIAL);
	}
	SKIP_TEST("didn't manage to trigger DatabaseModifiedError");
    );
}

static void
make_msize1_db(Xapian::WritableDatabase &db, const string &)
{
    const char * value0 =
	"ABBCDEFGHIJKLMMNOPQQRSTTUUVVWXYZZaabcdefghhijjkllmnopqrsttuvwxyz";
    const char * value1 =
	"EMLEMMMMMMMNMMLMELEDNLEDMLMLDMLMLMLMEDGFHPOPBAHJIQJNGRKCGF";
    while (*value0) {
	Xapian::Document doc;
	doc.add_value(0, string(1, *value0++));
	if (*value1) {
	    doc.add_value(1, string(1, *value1++));
	    doc.add_term("K1");
	}
	db.add_document(doc);
    }
}

/// Regression test for ticket#464, fixed in 1.1.6 and 1.0.20.
DEFINE_TESTCASE(msize1, backend) {
    Xapian::Database db = get_database("msize1", make_msize1_db);
    Xapian::Enquire enq(db);
    enq.set_sort_by_value(1, false);
    enq.set_collapse_key(0);
    enq.set_query(Xapian::Query("K1"));

    Xapian::MSet mset = enq.get_mset(0, 60);
    Xapian::doccount lb = mset.get_matches_lower_bound();
    Xapian::doccount ub = mset.get_matches_upper_bound();
    Xapian::doccount est = mset.get_matches_estimated();
    TEST_EQUAL(lb, ub);
    TEST_EQUAL(lb, est);

    Xapian::MSet mset2 = enq.get_mset(50, 10, 1000);
    Xapian::doccount lb2 = mset2.get_matches_lower_bound();
    Xapian::doccount ub2 = mset2.get_matches_upper_bound();
    Xapian::doccount est2 = mset2.get_matches_estimated();
    TEST_EQUAL(lb2, ub2);
    TEST_EQUAL(lb2, est2);
    TEST_EQUAL(est, est2);

    Xapian::MSet mset3 = enq.get_mset(0, 10, 1000);
    Xapian::doccount lb3 = mset3.get_matches_lower_bound();
    Xapian::doccount ub3 = mset3.get_matches_upper_bound();
    Xapian::doccount est3 = mset3.get_matches_estimated();
    TEST_EQUAL(lb3, ub3);
    TEST_EQUAL(lb3, est3);
    TEST_EQUAL(est, est3);
}

static void
make_msize2_db(Xapian::WritableDatabase &db, const string &)
{
    const char * value0 = "AAABCDEEFGHIIJJKLLMNNOOPPQQRSTTUVWXYZ";
    const char * value1 = "MLEMNMLMLMEDEDEMLEMLMLMLPOAHGF";
    while (*value0) {
	Xapian::Document doc;
	doc.add_value(0, string(1, *value0++));
	if (*value1) {
	    doc.add_value(1, string(1, *value1++));
	    doc.add_term("K1");
	}
	db.add_document(doc);
    }
}

/// Regression test for bug related to ticket#464, fixed in 1.1.6 and 1.0.20.
DEFINE_TESTCASE(msize2, backend) {
    Xapian::Database db = get_database("msize2", make_msize2_db);
    Xapian::Enquire enq(db);
    enq.set_sort_by_value(1, false);
    enq.set_collapse_key(0);
    enq.set_query(Xapian::Query("K1"));

    Xapian::MSet mset = enq.get_mset(0, 60);
    Xapian::doccount lb = mset.get_matches_lower_bound();
    Xapian::doccount ub = mset.get_matches_upper_bound();
    Xapian::doccount est = mset.get_matches_estimated();
    TEST_EQUAL(lb, ub);
    TEST_EQUAL(lb, est);

    Xapian::MSet mset2 = enq.get_mset(50, 10, 1000);
    Xapian::doccount lb2 = mset2.get_matches_lower_bound();
    Xapian::doccount ub2 = mset2.get_matches_upper_bound();
    Xapian::doccount est2 = mset2.get_matches_estimated();
    TEST_EQUAL(lb2, ub2);
    TEST_EQUAL(lb2, est2);
    TEST_EQUAL(est, est2);

    Xapian::MSet mset3 = enq.get_mset(0, 10, 1000);
    Xapian::doccount lb3 = mset3.get_matches_lower_bound();
    Xapian::doccount ub3 = mset3.get_matches_upper_bound();
    Xapian::doccount est3 = mset3.get_matches_estimated();
    TEST_EQUAL(lb3, ub3);
    TEST_EQUAL(lb3, est3);
    TEST_EQUAL(est, est3);
}

static void
make_xordecay1_db(Xapian::WritableDatabase &db, const string &)
{
    for (int n = 1; n != 50; ++n) {
	Xapian::Document doc;
	for (int i = 1; i != 50; ++i) {
	    if (n % i == 0)
		doc.add_term("N" + str(i));
	}
	db.add_document(doc);
    }
}

/// Regression test for bug in decay of XOR, fixed in 1.2.1 and 1.0.21.
DEFINE_TESTCASE(xordecay1, backend) {
    Xapian::Database db = get_database("xordecay1", make_xordecay1_db);
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query(Xapian::Query::OP_XOR,
				Xapian::Query("N10"),
				Xapian::Query(Xapian::Query::OP_OR,
					      Xapian::Query("N2"),
					      Xapian::Query("N3"))));
    Xapian::MSet mset1 = enq.get_mset(0, 1);
    Xapian::MSet msetall = enq.get_mset(0, db.get_doccount());

    TEST(mset_range_is_same(mset1, 0, msetall, 0, mset1.size()));
}

static void
make_ordecay_db(Xapian::WritableDatabase &db, const string &)
{
    const char * p = "VJ=QC]LUNTaARLI;715RR^];A4O=P4ZG<2CS4EM^^VS[A6QENR";
    for (int d = 0; p[d]; ++d) {
	int l = int(p[d] - '0');
	Xapian::Document doc;
	for (int n = 1; n < l; ++n) {
	    doc.add_term("N" + str(n));
	    if (n % (d + 1) == 0) {
		doc.add_term("M" + str(n));
	    }
	}
	db.add_document(doc);
    }
}

/// Regression test for bug in decay of OR to AND, fixed in 1.2.1 and 1.0.21.
DEFINE_TESTCASE(ordecay1, backend) {
    Xapian::Database db = get_database("ordecay", make_ordecay_db);
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query(Xapian::Query::OP_OR,
				Xapian::Query("N20"),
				Xapian::Query("N21")));

    Xapian::MSet msetall = enq.get_mset(0, db.get_doccount());
    for (unsigned int i = 1; i < msetall.size(); ++i) {
	Xapian::MSet submset = enq.get_mset(0, i);
	TEST(mset_range_is_same(submset, 0, msetall, 0, submset.size()));
    }
}

/** Regression test for bug in decay of OR to AND_MAYBE, fixed in 1.2.1 and
 *  1.0.21.
 */
DEFINE_TESTCASE(ordecay2, backend) {
    Xapian::Database db = get_database("ordecay", make_ordecay_db);
    Xapian::Enquire enq(db);
    std::vector<Xapian::Query> q;
    q.push_back(Xapian::Query("M20"));
    q.push_back(Xapian::Query("N21"));
    q.push_back(Xapian::Query("N22"));
    enq.set_query(Xapian::Query(Xapian::Query::OP_OR,
				Xapian::Query("N25"),
				Xapian::Query(Xapian::Query::OP_AND,
					      q.begin(),
					      q.end())));

    Xapian::MSet msetall = enq.get_mset(0, db.get_doccount());
    for (unsigned int i = 1; i < msetall.size(); ++i) {
	Xapian::MSet submset = enq.get_mset(0, i);
	TEST(mset_range_is_same(submset, 0, msetall, 0, submset.size()));
    }
}

static void
make_orcheck_db(Xapian::WritableDatabase &db, const string &)
{
    static const unsigned t1[] = {2, 4, 6, 8, 10};
    static const unsigned t2[] = {6, 7, 8, 11, 12, 13, 14, 15, 16, 17};
    static const unsigned t3[] = {3, 7, 8, 11, 12, 13, 14, 15, 16, 17};

    for (unsigned i = 1; i <= 17; ++i) {
	Xapian::Document doc;
	db.replace_document(i, doc);
    }
    for (unsigned i : t1) {
	Xapian::Document doc(db.get_document(i));
	doc.add_term("T1");
	db.replace_document(i, doc);
    }
    for (unsigned i : t2) {
	Xapian::Document doc(db.get_document(i));
	doc.add_term("T2");
	if (i < 17) {
	    doc.add_term("T2_lowfreq");
	}
	doc.add_value(2, "1");
	db.replace_document(i, doc);
    }
    for (unsigned i : t3) {
	Xapian::Document doc(db.get_document(i));
	doc.add_term("T3");
	if (i < 17) {
	    doc.add_term("T3_lowfreq");
	}
	doc.add_value(3, "1");
	db.replace_document(i, doc);
    }
}

/** Regression test for bugs in the check() method of OrPostList. (ticket #485)
 *  Bugs introduced and fixed between 1.2.0 and 1.2.1 (never in a release).
 */
DEFINE_TESTCASE(orcheck1, backend) {
    Xapian::Database db = get_database("orcheck1", make_orcheck_db);
    Xapian::Enquire enq(db);
    Xapian::Query q1("T1");
    Xapian::Query q2("T2");
    Xapian::Query q2l("T2_lowfreq");
    Xapian::Query q3("T3");
    Xapian::Query q3l("T3_lowfreq");
    Xapian::Query v2(Xapian::Query::OP_VALUE_RANGE, 2, "0", "2");
    Xapian::Query v3(Xapian::Query::OP_VALUE_RANGE, 3, "0", "2");

    tout << "Checking q2 OR q3\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_AND, q1,
				Xapian::Query(Xapian::Query::OP_OR, q2, q3)));
    mset_expect_order(enq.get_mset(0, db.get_doccount()), 8, 6);

    tout << "Checking q2l OR q3\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_AND, q1,
				Xapian::Query(Xapian::Query::OP_OR, q2l, q3)));
    mset_expect_order(enq.get_mset(0, db.get_doccount()), 8, 6);

    tout << "Checking q2 OR q3l\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_AND, q1,
				Xapian::Query(Xapian::Query::OP_OR, q2, q3l)));
    mset_expect_order(enq.get_mset(0, db.get_doccount()), 8, 6);

    tout << "Checking v2 OR q3\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_AND, q1,
				Xapian::Query(Xapian::Query::OP_OR, v2, q3)));
    mset_expect_order(enq.get_mset(0, db.get_doccount()), 8, 6);

    tout << "Checking q2 OR v3\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_AND, q1,
				Xapian::Query(Xapian::Query::OP_OR, q2, v3)));
    // Order of results in this one is different, because v3 gives no weight,
    // both documents are in q2, and document 8 has a higher length.
    mset_expect_order(enq.get_mset(0, db.get_doccount()), 6, 8);

}

/** Regression test for bug fixed in 1.2.1 and 1.0.21.
 *
 *  We failed to mark the Btree as unmodified after cancel().
 */
DEFINE_TESTCASE(failedreplace1, chert || glass) {
    Xapian::WritableDatabase db(get_writable_database());
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);
    Xapian::docid did = db.add_document(doc);
    doc.add_term("abc");
    doc.add_term(string(1000, 'm'));
    doc.add_term("xyz");
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.replace_document(did, doc));
    db.commit();
    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_termfreq("foo"), 0);
}

DEFINE_TESTCASE(failedreplace2, chert || glass) {
    Xapian::WritableDatabase db(get_writable_database("apitest_simpledata"));
    db.commit();
    Xapian::doccount db_size = db.get_doccount();
    Xapian::Document doc;
    doc.set_data("wibble");
    doc.add_term("foo");
    doc.add_value(0, "seven");
    db.add_document(doc);
    Xapian::docid did = db.add_document(doc);
    doc.add_term("abc");
    doc.add_term(string(1000, 'm'));
    doc.add_term("xyz");
    doc.add_value(0, "six");
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.replace_document(did, doc));
    db.commit();
    TEST_EQUAL(db.get_doccount(), db_size);
    TEST_EQUAL(db.get_termfreq("foo"), 0);
}

/// Coverage for SelectPostList::skip_to().
DEFINE_TESTCASE(phrase3, positional) {
    Xapian::Database db = get_database("apitest_phrase");

    static const char * const phrase_words[] = { "phrase", "near" };
    Xapian::Query q(Xapian::Query::OP_NEAR, phrase_words, phrase_words + 2, 12);
    q = Xapian::Query(Xapian::Query::OP_AND_MAYBE, Xapian::Query("pad"), q);

    Xapian::Enquire enquire(db);
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 5);

}

/// Check that get_mset(<large number>, 10) doesn't exhaust memory needlessly.
// Regression test for fix in 1.2.4.
DEFINE_TESTCASE(msetfirst2, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;
    // Before the fix, this tried to allocate too much memory.
    mset = enquire.get_mset(0xfffffff0, 1);
    TEST_EQUAL(mset.get_firstitem(), 0xfffffff0);
    // Check that the number of documents gets clamped too.
    mset = enquire.get_mset(1, 0xfffffff0);
    TEST_EQUAL(mset.get_firstitem(), 1);
    // Another regression test - MatchNothing used to give an MSet with
    // get_firstitem() returning 0.
    enquire.set_query(Xapian::Query::MatchNothing);
    mset = enquire.get_mset(1, 1);
    TEST_EQUAL(mset.get_firstitem(), 1);
}

DEFINE_TESTCASE(bm25weight2, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("the"));
    enquire.set_weighting_scheme(Xapian::BM25Weight(0, 0, 0, 0, 1));
    Xapian::MSet mset = enquire.get_mset(0, 100);
    TEST_REL(mset.size(),>=,2);
    double weight0 = mset[0].get_weight();
    for (Xapian::doccount i = 1; i != mset.size(); ++i) {
	TEST_EQUAL(weight0, mset[i].get_weight());
    }
}

DEFINE_TESTCASE(unigramlmweight2, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("the"));
    enquire.set_weighting_scheme(Xapian::LMWeight());
    Xapian::MSet mset = enquire.get_mset(0, 100);
    TEST_REL(mset.size(),>=,2);
}

DEFINE_TESTCASE(tradweight2, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("the"));
    enquire.set_weighting_scheme(Xapian::TradWeight(0));
    Xapian::MSet mset = enquire.get_mset(0, 100);
    TEST_REL(mset.size(),>=,2);
    double weight0 = mset[0].get_weight();
    for (Xapian::doccount i = 1; i != mset.size(); ++i) {
	TEST_EQUAL(weight0, mset[i].get_weight());
    }
}

// Regression test for bug fix in 1.2.9.
DEFINE_TESTCASE(emptydb1, backend) {
    Xapian::Database db(get_database(string()));
    static const Xapian::Query::op ops[] = {
	Xapian::Query::OP_AND,
	Xapian::Query::OP_OR,
	Xapian::Query::OP_AND_NOT,
	Xapian::Query::OP_XOR,
	Xapian::Query::OP_AND_MAYBE,
	Xapian::Query::OP_FILTER,
	Xapian::Query::OP_NEAR,
	Xapian::Query::OP_PHRASE,
	Xapian::Query::OP_ELITE_SET,
	Xapian::Query::OP_SYNONYM,
	Xapian::Query::OP_MAX
    };
    for (Xapian::Query::op op : ops) {
	tout << op << '\n';
	Xapian::Enquire enquire(db);
	Xapian::Query query(op, Xapian::Query("a"), Xapian::Query("b"));
	enquire.set_query(query);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST_EQUAL(mset.get_matches_estimated(), 0);
	TEST_EQUAL(mset.get_matches_upper_bound(), 0);
	TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    }
}

/** Test operators which should allow more than two arguments.
 *
 *  Regression test for bug with OP_FILTER fixed in 1.4.15, and also for bugs
 *  with deleting the PostList which is currently set as the QueryOptimiser's
 *  hint fixed in 1.4.15.
 */
DEFINE_TESTCASE(multiargop1, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    static const struct { unsigned hits; Xapian::Query::op op; } tests[] = {
	{ 0, Xapian::Query::OP_AND },
	{ 6, Xapian::Query::OP_OR },
	{ 0, Xapian::Query::OP_AND_NOT },
	{ 5, Xapian::Query::OP_XOR },
	{ 2, Xapian::Query::OP_AND_MAYBE },
	{ 0, Xapian::Query::OP_FILTER },
	{ 0, Xapian::Query::OP_NEAR },
	{ 0, Xapian::Query::OP_PHRASE },
	{ 6, Xapian::Query::OP_ELITE_SET },
	{ 6, Xapian::Query::OP_SYNONYM },
	{ 6, Xapian::Query::OP_MAX }
    };
    static const char* terms[] = {"two", "all", "paragraph", "banana"};
    Xapian::Enquire enquire(db);
    for (auto& test : tests) {
	Xapian::Query::op op = test.op;
	Xapian::doccount hits = test.hits;
	tout << op << " should give " << hits << " hits\n";
	Xapian::Query query(op, terms, terms + 4);
	enquire.set_query(query);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST_EQUAL(mset.get_matches_estimated(), hits);
	TEST_EQUAL(mset.get_matches_upper_bound(), hits);
	TEST_EQUAL(mset.get_matches_lower_bound(), hits);
    }
}

/// Test error opening non-existent stub databases.
// Regression test for bug fixed in 1.3.1 and 1.2.11.
DEFINE_TESTCASE(stubdb7, !backend) {
    TEST_EXCEPTION(Xapian::DatabaseNotFoundError,
	    Xapian::Database("nosuchdirectory", Xapian::DB_BACKEND_STUB));
    TEST_EXCEPTION(Xapian::DatabaseNotFoundError,
	    Xapian::WritableDatabase("nosuchdirectory",
		Xapian::DB_OPEN|Xapian::DB_BACKEND_STUB));
}

/// Test which checks the weights are as expected.
//  This runs for multi_* too, so serves to check that we get the same weights
//  with multiple databases as without.
DEFINE_TESTCASE(msetweights1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("paragraph"),
		    Xapian::Query("word"));
    enq.set_query(q);
    // 5 documents match, and the 4th and 5th have the same weight, so ask for
    // 4 as that's a good test that we get the right one in this case.
    Xapian::MSet mset = enq.get_mset(0, 4);

    static const struct { Xapian::docid did; double wt; } expected[] = {
	{ 2, 1.2058248004573934864 },
	{ 4, 0.81127876655507624726 },
	{ 1, 0.17309550762546158098 },
	{ 3, 0.14609528172558261527 }
    };

    TEST_EQUAL(mset.size(), sizeof(expected) / sizeof(expected[0]));
    for (Xapian::doccount i = 0; i < mset.size(); ++i) {
	TEST_EQUAL(*mset[i], expected[i].did);
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), expected[i].wt);
    }

    // Now test a query which matches only even docids, so in the multi case
    // one subdatabase doesn't match.
    enq.set_query(Xapian::Query("one"));
    mset = enq.get_mset(0, 3);

    static const struct { Xapian::docid did; double wt; } expected2[] = {
	{ 6, 0.73354729848273669823 },
	{ 2, 0.45626501034348893038 }
    };

    TEST_EQUAL(mset.size(), sizeof(expected2) / sizeof(expected2[0]));
    for (Xapian::doccount i = 0; i < mset.size(); ++i) {
	TEST_EQUAL(*mset[i], expected2[i].did);
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), expected2[i].wt);
    }
}

DEFINE_TESTCASE(itorskiptofromend1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    Xapian::TermIterator t = db.termlist_begin(1);
    t.skip_to("zzzzz");
    TEST(t == db.termlist_end(1));
    // This worked in 1.2.x but segfaulted in 1.3.1.
    t.skip_to("zzzzzz");

    Xapian::PostingIterator p = db.postlist_begin("one");
    p.skip_to(99999);
    TEST(p == db.postlist_end("one"));
    // This segfaulted prior to 1.3.2.
    p.skip_to(999999);

    Xapian::PositionIterator i = db.positionlist_begin(6, "one");
    i.skip_to(99999);
    TEST(i == db.positionlist_end(6, "one"));
    // This segfaulted prior to 1.3.2.
    i.skip_to(999999);

    Xapian::ValueIterator v = db.valuestream_begin(1);
    v.skip_to(99999);
    TEST(v == db.valuestream_end(1));
    // These segfaulted prior to 1.3.2.
    v.skip_to(999999);
    v.check(9999999);
}

/// Check handling of invalid block sizes.
// Regression test for bug fixed in 1.2.17 and 1.3.2 - the size gets fixed
// but the uncorrected size was passed to the base file.  Also, abort() was
// called on 0.
DEFINE_TESTCASE(blocksize1, chert || glass) {
    string db_dir = "." + get_dbtype();
    mkdir(db_dir.c_str(), 0755);
    db_dir += "/db__blocksize1";
    int flags;
    if (get_dbtype() == "chert") {
	flags = Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT;
    } else {
	flags = Xapian::DB_CREATE|Xapian::DB_BACKEND_GLASS;
    }
    static const unsigned bad_sizes[] = {
	65537, 8000, 2000, 1024, 16, 7, 3, 1, 0
    };
    for (size_t i = 0; i < sizeof(bad_sizes) / sizeof(bad_sizes[0]); ++i) {
	size_t block_size = bad_sizes[i];
	rm_rf(db_dir);
	Xapian::WritableDatabase db(db_dir, flags, block_size);
	Xapian::Document doc;
	doc.add_term("XYZ");
	doc.set_data("foo");
	db.add_document(doc);
	db.commit();
    }
}

/// Feature test for Xapian::DB_NO_TERMLIST.
DEFINE_TESTCASE(notermlist1, glass) {
    string db_dir = "." + get_dbtype();
    mkdir(db_dir.c_str(), 0755);
    db_dir += "/db__notermlist1";
    int flags = Xapian::DB_CREATE|Xapian::DB_NO_TERMLIST;
    if (get_dbtype() == "chert") {
	flags |= Xapian::DB_BACKEND_CHERT;
    } else {
	flags |= Xapian::DB_BACKEND_GLASS;
    }
    rm_rf(db_dir);
    Xapian::WritableDatabase db(db_dir, flags);
    Xapian::Document doc;
    doc.add_term("hello");
    doc.add_value(42, "answer");
    db.add_document(doc);
    db.commit();
    TEST(!file_exists(db_dir + "/termlist.glass"));
    TEST_EXCEPTION(Xapian::FeatureUnavailableError, db.termlist_begin(1));
}

/// Regression test for bug starting a new glass freelist block.
DEFINE_TESTCASE(newfreelistblock1, writable) {
    Xapian::Document doc;
    doc.add_term("foo");
    for (int i = 100; i < 120; ++i) {
	doc.add_term(str(i));
    }

    Xapian::WritableDatabase wdb(get_writable_database());
    for (int j = 0; j < 50; ++j) {
	wdb.add_document(doc);
    }
    wdb.commit();

    for (int k = 0; k < 1000; ++k) {
	wdb.add_document(doc);
	wdb.commit();
    }
}

/** Check that the parent directory for the database doesn't need to be
 *  writable.  Regression test for early versions on the glass new btree
 *  branch which failed to append a "/" when generating a temporary filename
 *  from the database directory.
 */
DEFINE_TESTCASE(readonlyparentdir1, chert || glass) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __OS2__
    string path = get_named_writable_database_path("readonlyparentdir1");
    // Fix permissions if the previous test was killed.
    (void)chmod(path.c_str(), 0700);
    mkdir(path.c_str(), 0777);
    mkdir((path + "/sub").c_str(), 0777);
    Xapian::WritableDatabase db = get_named_writable_database("readonlyparentdir1/sub");
    TEST(chmod(path.c_str(), 0500) == 0);
    try {
	Xapian::Document doc;
	doc.add_term("hello");
	doc.set_data("some text");
	db.add_document(doc);
	db.commit();
    } catch (...) {
	// Attempt to fix the permissions, otherwise things like "rm -rf" on
	// the source tree will fail.
	(void)chmod(path.c_str(), 0700);
	throw;
    }
    TEST(chmod(path.c_str(), 0700) == 0);
#endif
}

static void
make_phrasebug1_db(Xapian::WritableDatabase &db, const string &)
{
    Xapian::Document doc;
    doc.add_posting("hurricane", 199881);
    doc.add_posting("hurricane", 203084);
    doc.add_posting("katrina", 199882);
    doc.add_posting("katrina", 202473);
    doc.add_posting("katrina", 203085);
    db.add_document(doc);
}

/// Regression test for ticket#653, fixed in 1.3.2 and 1.2.19.
DEFINE_TESTCASE(phrasebug1, positional) {
    Xapian::Database db = get_database("phrasebug1", make_phrasebug1_db);
    static const char * const qterms[] = { "katrina", "hurricane" };
    Xapian::Enquire e(db);
    Xapian::Query q(Xapian::Query::OP_PHRASE, qterms, qterms + 2, 5);
    e.set_query(q);
    Xapian::MSet mset = e.get_mset(0, 100);
    TEST_EQUAL(mset.size(), 0);
    static const char * const qterms2[] = { "hurricane", "katrina" };
    Xapian::Query q2(Xapian::Query::OP_PHRASE, qterms2, qterms2 + 2, 5);
    e.set_query(q2);
    mset = e.get_mset(0, 100);
    TEST_EQUAL(mset.size(), 1);
}

/// Feature test for Xapian::DB_RETRY_LOCK
DEFINE_TESTCASE(retrylock1, writable && path) {
    // FIXME: Can't see an easy way to test this for remote databases - the
    // harness doesn't seem to provide a suitable way to reopen a remote.
#if defined HAVE_FORK && defined HAVE_SOCKETPAIR
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, PF_UNSPEC, fds) < 0) {
	FAIL_TEST("socketpair() failed");
    }
    if (fds[1] >= FD_SETSIZE)
	SKIP_TEST("socketpair() gave fd >= FD_SETSIZE");
    if (fcntl(fds[1], F_SETFL, O_NONBLOCK) < 0)
	FAIL_TEST("fcntl() failed to set O_NONBLOCK");
    pid_t child = fork();
    if (child == -1)
	FAIL_TEST("fork() failed");
    if (child == 0) {
	// Wait for signal that parent has opened the database.
	char ch;
	while (read(fds[0], &ch, 1) < 0) { }

	try {
	    Xapian::WritableDatabase db2(get_named_writable_database_path("retrylock1"),
					 Xapian::DB_OPEN|Xapian::DB_RETRY_LOCK);
	    if (write(fds[0], "y", 1)) { }
	} catch (const Xapian::DatabaseLockError &) {
	    if (write(fds[0], "l", 1)) { }
	} catch (const Xapian::Error &e) {
	    const string & m = e.get_description();
	    if (write(fds[0], m.data(), m.size())) { }
	} catch (...) {
	    if (write(fds[0], "o", 1)) { }
	}
	_exit(0);
    }

    close(fds[0]);

    Xapian::WritableDatabase db = get_named_writable_database("retrylock1");
    if (write(fds[1], "", 1) != 1)
	FAIL_TEST("Failed to signal to child process");

    char result[256];
    int r = read(fds[1], result, sizeof(result));
    if (r == -1) {
	if (errno == EAGAIN) {
	    // Good.
	    result[0] = 'y';
	} else {
	    // Error.
	    tout << "errno=" << errno << ": " << errno_to_string(errno) << '\n';
	    result[0] = 'e';
	}
	r = 1;
    } else if (r >= 1) {
	if (result[0] == 'y') {
	    // Child process managed to also get write lock!
	    result[0] = '!';
	}
    } else {
	// EOF.
	result[0] = 'z';
	r = 1;
    }

    try {
	db.close();
    } catch (...) {
	kill(child, SIGKILL);
	int status;
	while (waitpid(child, &status, 0) < 0) {
	    if (errno != EINTR) break;
	}
	throw;
    }

    if (result[0] == 'y') {
retry:
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fds[1], &fdset);
	int sr = select(fds[1] + 1, &fdset, NULL, NULL, &tv);
	if (sr == 0) {
	    // Timed out.
	    result[0] = 'T';
	    r = 1;
	} else if (sr == -1) {
	    if (errno == EINTR || errno == EAGAIN)
		goto retry;
	    tout << "select() failed with errno=" << errno << ": "
		 << errno_to_string(errno) << '\n';
	    result[0] = 'S';
	    r = 1;
	} else {
	    r = read(fds[1], result, sizeof(result));
	    if (r == -1) {
		// Error.
		tout << "read() failed with errno=" << errno << ": "
		     << errno_to_string(errno) << '\n';
		result[0] = 'R';
		r = 1;
	    } else if (r == 0) {
		// EOF.
		result[0] = 'Z';
		r = 1;
	    }
	}
    }

    close(fds[1]);

    kill(child, SIGKILL);
    int status;
    while (waitpid(child, &status, 0) < 0) {
	if (errno != EINTR) break;
    }

    tout << string(result, r) << '\n';
    TEST_EQUAL(result[0], 'y');
#endif
}

// Opening a WritableDatabase with low fds available - it should avoid them.
DEFINE_TESTCASE(dbfilefd012, writable && !remote) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __OS2__
    int oldfds[3];
    for (int i = 0; i < 3; ++i) {
	oldfds[i] = dup(i);
    }
    try {
	for (int j = 0; j < 3; ++j) {
	    close(j);
	    TEST_REL(lseek(j, 0, SEEK_CUR), <, 0);
	    TEST_EQUAL(errno, EBADF);
	}

	Xapian::WritableDatabase db = get_writable_database();

	// Check we didn't use any of those low fds for tables, as that risks
	// data corruption if some other code in the same process tries to
	// write to them (see #651).
	for (int fd = 0; fd < 3; ++fd) {
	    // Check that the fd is still closed, or isn't open O_RDWR (the
	    // lock file gets opened O_WRONLY), or it's a pipe (if we're using
	    // a child process to hold a non-OFD fcntl lock).
	    int flags = fcntl(fd, F_GETFL);
	    if (flags == -1) {
		TEST_EQUAL(errno, EBADF);
	    } else if ((flags & O_ACCMODE) != O_RDWR) {
		// OK.
	    } else {
		struct stat sb;
		TEST_NOT_EQUAL(fstat(fd, &sb), -1);
#ifdef S_ISSOCK
		TEST(S_ISSOCK(sb.st_mode));
#else
		// If we can't check it is a socket, at least check it is not a
		// regular file.
		TEST(!S_ISREG(sb.st_mode));
#endif
	    }
	}
    } catch (...) {
	for (int j = 0; j < 3; ++j) {
	    dup2(oldfds[j], j);
	    close(oldfds[j]);
	}
	throw;
    }

    for (int j = 0; j < 3; ++j) {
	dup2(oldfds[j], j);
	close(oldfds[j]);
    }
#endif
}

/// Regression test for #675, fixed in 1.3.3 and 1.2.21.
DEFINE_TESTCASE(cursorbug1, writable && path) {
    Xapian::WritableDatabase wdb = get_writable_database();
    Xapian::Database db = get_writable_database_as_database();
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query::MatchAll);
    Xapian::MSet mset;
    // The original problem triggers for chert and glass on repeat==7.
    for (int repeat = 0; repeat < 10; ++repeat) {
	tout.str(string());
	tout << "iteration #" << repeat << '\n';

	const int ITEMS = 10;
	int free_id = db.get_doccount();
	int offset = max(free_id, ITEMS * 2) - (ITEMS * 2);
	int limit = offset + (ITEMS * 2);

	mset = enq.get_mset(offset, limit);
	for (Xapian::MSetIterator m1 = mset.begin(); m1 != mset.end(); ++m1) {
	    (void)m1.get_document().get_value(0);
	}

	for (int i = free_id; i <= free_id + ITEMS; ++i) {
	    Xapian::Document doc;
	    const string & id = str(i);
	    string qterm = "Q" + id;
	    doc.add_value(0, id);
	    doc.add_boolean_term(qterm);
	    wdb.replace_document(qterm, doc);
	}
	wdb.commit();

	db.reopen();
	mset = enq.get_mset(offset, limit);
	for (Xapian::MSetIterator m2 = mset.begin(); m2 != mset.end(); ++m2) {
	    (void)m2.get_document().get_value(0);
	}
    }
}

// Regression test for #674, fixed in 1.2.21 and 1.3.3.
DEFINE_TESTCASE(sortvalue2, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    db.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query::MatchAll);
    enq.set_sort_by_value(0, false);
    Xapian::MSet mset = enq.get_mset(0, 50);

    // Check all results are in key order - the bug was that they were sorted
    // by docid instead with multiple remote databases.
    string old_key;
    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	string key = db.get_document(*i).get_value(0);
	TEST(old_key <= key);
	swap(old_key, key);
    }
}

/// Check behaviour of Enquire::get_query().
DEFINE_TESTCASE(enquiregetquery1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    TEST_EQUAL(enq.get_query().get_description(), "Query()");
}

DEFINE_TESTCASE(embedded1, singlefile) {
    // In reality you should align the embedded database to a multiple of
    // database block size, but any offset is meant to work.
    off_t offset = 1234;

    Xapian::Database db = get_database("apitest_simpledata");
    const string & db_path = get_database_path("apitest_simpledata");
    const string & tmp_path = db_path + "-embedded";
    ofstream out(tmp_path, fstream::trunc|fstream::binary);
    out.seekp(offset);
    out << ifstream(db_path, fstream::binary).rdbuf();
    out.close();

    {
	int fd = open(tmp_path.c_str(), O_RDONLY|O_BINARY);
	lseek(fd, offset, SEEK_SET);
	Xapian::Database db_embedded(fd);
	TEST_EQUAL(db.get_doccount(), db_embedded.get_doccount());
    }

    {
	int fd = open(tmp_path.c_str(), O_RDONLY|O_BINARY);
	lseek(fd, offset, SEEK_SET);
	size_t check_errors =
	    Xapian::Database::check(fd, Xapian::DBCHECK_SHOW_STATS, &tout);
	TEST_EQUAL(check_errors, 0);
    }
}

/// Regression test for bug fixed in 1.3.7.
DEFINE_TESTCASE(exactxor1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);

    static const char * const words[4] = {
	"blank", "test", "paragraph", "banana"
    };
    Xapian::Query q(Xapian::Query::OP_XOR, words, words + 4);
    enq.set_query(q);
    enq.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::MSet mset = enq.get_mset(0, 0);
    // A reversed conditional gave us 5 in this case.
    TEST_EQUAL(mset.get_matches_upper_bound(), 6);
    // Test improved lower bound in 1.3.7 (earlier versions gave 0).
    TEST_EQUAL(mset.get_matches_lower_bound(), 2);

    static const char * const words2[4] = {
	"queri", "test", "paragraph", "word"
    };
    Xapian::Query q2(Xapian::Query::OP_XOR, words2, words2 + 4);
    enq.set_query(q2);
    enq.set_weighting_scheme(Xapian::BoolWeight());
    mset = enq.get_mset(0, 0);
    // A reversed conditional gave us 6 in this case.
    TEST_EQUAL(mset.get_matches_upper_bound(), 5);
    // Test improved lower bound in 1.3.7 (earlier versions gave 0).
    TEST_EQUAL(mset.get_matches_lower_bound(), 1);
}

/// Feature test for Database::get_revision().
DEFINE_TESTCASE(getrevision1, chert || glass) {
    Xapian::WritableDatabase db = get_writable_database();
    TEST_EQUAL(db.get_revision(), 0);
    db.commit();
    TEST_EQUAL(db.get_revision(), 0);
    Xapian::Document doc;
    doc.add_term("hello");
    db.add_document(doc);
    TEST_EQUAL(db.get_revision(), 0);
    db.commit();
    TEST_EQUAL(db.get_revision(), 1);
    db.commit();
    TEST_EQUAL(db.get_revision(), 1);
    db.add_document(doc);
    db.commit();
    TEST_EQUAL(db.get_revision(), 2);
}

/// Check get_revision() on an empty database reports 0.  (Since 1.5.0)
DEFINE_TESTCASE(getrevision2, !backend) {
    Xapian::Database db;
    TEST_EQUAL(db.get_revision(), 0);
    Xapian::Database wdb;
    TEST_EQUAL(wdb.get_revision(), 0);
}

/// Feature test for DOC_ASSUME_VALID.
DEFINE_TESTCASE(getdocumentlazy1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Document doc_lazy = db.get_document(2, Xapian::DOC_ASSUME_VALID);
    Xapian::Document doc = db.get_document(2);
    TEST_EQUAL(doc.get_data(), doc_lazy.get_data());
    TEST_EQUAL(doc.get_value(0), doc_lazy.get_value(0));
}

/// Feature test for DOC_ASSUME_VALID for a docid that doesn't actually exist.
DEFINE_TESTCASE(getdocumentlazy2, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Document doc;
    try {
	doc = db.get_document(db.get_lastdocid() + 1, Xapian::DOC_ASSUME_VALID);
    } catch (const Xapian::DocNotFoundError&) {
	// DOC_ASSUME_VALID is really just a hint, so ignoring is OK (the
	// remote backend currently does).
    }
    TEST(doc.get_data().empty());
    TEST_EXCEPTION(Xapian::DocNotFoundError,
	doc = db.get_document(db.get_lastdocid() + 1);
    );
}

static void
gen_uniqterms_gt_doclen_db(Xapian::WritableDatabase& db, const string&)
{
    Xapian::Document doc;
    doc.add_term("foo");
    doc.add_boolean_term("bar");
    db.add_document(doc);
    Xapian::Document doc2;
    doc2.add_posting("foo", 0, 2);
    doc2.add_term("foo2");
    doc2.add_boolean_term("baz");
    doc2.add_boolean_term("baz2");
    db.add_document(doc2);
}

DEFINE_TESTCASE(getuniqueterms1, backend) {
    Xapian::Database db =
	get_database("uniqterms_gt_doclen", gen_uniqterms_gt_doclen_db);

    auto unique1 = db.get_unique_terms(1);
    TEST_REL(unique1, <=, db.get_doclength(1));
    TEST_REL(unique1, <, db.get_document(1).termlist_count());
    // Ideally it'd be equal to 1, and in this case it is, but the current
    // backends can't always efficiently ensure an exact answer.
    TEST_REL(unique1, >=, 1);

    auto unique2 = db.get_unique_terms(2);
    TEST_REL(unique2, <=, db.get_doclength(2));
    TEST_REL(unique2, <, db.get_document(2).termlist_count());
    // Ideally it'd be equal to 2, but the current backends can't always
    // efficiently ensure an exact answer and here it is actually 3.
    TEST_REL(unique2, >=, 2);
}

/** Regression test for bug fixed in 1.4.6.
 *
 *  OP_NEAR would think a term without positional information occurred at
 *  position 1 if it had the lowest term frequency amongst the OP_NEAR's
 *  subqueries.
 */
DEFINE_TESTCASE(nopositionbug1, backend) {
    Xapian::Database db =
	get_database("uniqterms_gt_doclen", gen_uniqterms_gt_doclen_db);

    // Test both orders.
    static const char* const terms1[] = { "foo", "baz" };
    static const char* const terms2[] = { "baz", "foo" };

    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query(Xapian::Query::OP_NEAR,
				begin(terms1), end(terms1), 10));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);

    enq.set_query(Xapian::Query(Xapian::Query::OP_NEAR,
				begin(terms2), end(terms2), 10));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);

    enq.set_query(Xapian::Query(Xapian::Query::OP_PHRASE,
				begin(terms1), end(terms1), 10));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);

    enq.set_query(Xapian::Query(Xapian::Query::OP_PHRASE,
				begin(terms2), end(terms2), 10));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);

    // Exercise exact phrase case too.
    enq.set_query(Xapian::Query(Xapian::Query::OP_PHRASE,
				begin(terms1), end(terms1), 2));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);

    enq.set_query(Xapian::Query(Xapian::Query::OP_PHRASE,
				begin(terms2), end(terms2), 2));
    TEST_EQUAL(enq.get_mset(0, 5).size(), 0);
}

/** Regression test for bug with get_mset(0, 0, N) (N > 0).
 *
 *  Fixed in 1.5.0 and 1.4.6.
 */
DEFINE_TESTCASE(checkatleast4, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("paragraph"));
    // This used to cause access to an element in an empty vector.
    Xapian::MSet mset = enq.get_mset(0, 0, 4);
    TEST_EQUAL(mset.size(), 0);
}

/// Regression test for glass freelist leak fixed in 1.4.6 and 1.5.0.
DEFINE_TESTCASE(freelistleak1, check) {
    auto path = get_database_path("freelistleak1",
				  [](Xapian::WritableDatabase& wdb,
				     const string&)
				  {
				      wdb.set_metadata("foo", "bar");
				      wdb.commit();
				      Xapian::Document doc;
				      doc.add_term("baz");
				      wdb.add_document(doc);
				  });
    size_t check_errors =
	Xapian::Database::check(path, Xapian::DBCHECK_SHOW_STATS, &tout);
    TEST_EQUAL(check_errors, 0);
}

/// Regression test for split position handling - broken in 1.4.8.
DEFINE_TESTCASE(splitpostings1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    // Add postings to create a split internally.
    for (Xapian::termpos pos = 0; pos <= 100; pos += 10) {
	doc.add_posting("foo", pos);
    }
    for (Xapian::termpos pos = 5; pos <= 100; pos += 20) {
	doc.add_posting("foo", pos);
    }
    db.add_document(doc);
    db.commit();

    Xapian::termpos expect = 0;
    Xapian::termpos pos = 0;
    for (auto p = db.positionlist_begin(1, "foo");
	 p != db.positionlist_end(1, "foo"); ++p) {
	TEST_REL(expect, <=, 100);
	pos = *p;
	TEST_EQUAL(pos, expect);
	expect += 5;
	if (expect % 20 == 15) expect += 5;
    }
    TEST_EQUAL(pos, 100);
}

/// Feature tests for Database::size().
DEFINE_TESTCASE(multidb1, backend) {
    Xapian::Database db;
    TEST_EQUAL(db.size(), 0);
    Xapian::Database db2 = get_database("apitest_simpledata");
    TEST(db2.size() != 0);
    db.add_database(db2);
    TEST_EQUAL(db.size(), db2.size());
    db.add_database(db2);
    // Regression test for bug introduced and fixed in git master before 1.5.0.
    // Adding a multi database to an empty database incorrectly worked just
    // like assigning the database object.  The list of shards is now copied
    // instead.
    TEST_EQUAL(db.size(), db2.size() * 2);
    db.add_database(Xapian::Database());
    TEST_EQUAL(db.size(), db2.size() * 2);
}

// Test that all the terms returned exist.
DEFINE_TESTCASE(allterms7, backend) {
    Xapian::Database db = get_database("etext");
    for (auto i = db.allterms_begin(); i != db.allterms_end(); ++i) {
	string term = *i;
	TEST(db.get_termfreq(term) > 0);
	TEST(db.postlist_begin(term) != db.postlist_end(term));
    }
}

/* Test searching for non-existent terms returns zero results.
 *
 * Regression test for GlassTable::readahead_key() throwing "Key too long"
 * error if passed an oversized key.
 */
DEFINE_TESTCASE(nosuchterm, backend) {
    Xapian::Database db = get_database("apitest_simpledata");
    Xapian::Enquire enquire{db};
    // Test up to a length longer than any backend supports.
    const unsigned MAX_LEN = 300;
    string term;
    term.reserve(MAX_LEN);
    while (term.size() < MAX_LEN) {
	term += 'x';
	enquire.set_query(Xapian::Query(term));
	TEST_EQUAL(enquire.get_mset(0, 10).size(), 0);
    }
}

// Test exception for check() on remote via stub.
DEFINE_TESTCASE(unsupportedcheck1, path) {
    mkdir(".stub", 0755);
    const char* stubpath = ".stub/unsupportedcheck1";
    ofstream out(stubpath);
    TEST(out.is_open());
    out << "remote :" << BackendManager::get_xapian_progsrv_command()
	<< ' ' << get_database_path("apitest_simpledata") << '\n';
    out.close();

    TEST_EXCEPTION(Xapian::UnimplementedError,
		   Xapian::Database::check(stubpath));
}

// Test exception for check() on inmemory via stub.
DEFINE_TESTCASE(unsupportedcheck2, inmemory) {
    mkdir(".stub", 0755);
    const char* stubpath = ".stub/unsupportedcheck2";
    ofstream out(stubpath);
    TEST(out.is_open());
    out << "inmemory\n";
    out.close();

    TEST_EXCEPTION(Xapian::UnimplementedError,
		   Xapian::Database::check(stubpath));
}

// Test exception for passing empty filename to check().
DEFINE_TESTCASE(unsupportedcheck3, !backend) {
    // Regression test, exception was DatabaseOpeningError with description:
    // Failed to rewind file descriptor -1 (Bad file descriptor)
    try {
	Xapian::Database::check(string());
    } catch (const Xapian::DatabaseOpeningError& e) {
	string enoent_msg = errno_to_string(ENOENT);
	TEST_EQUAL(e.get_error_string(), enoent_msg);
    }
}

// Test handling of corrupt DB with out of range values in the version file.
// Regression test for #824, fixed in 1.4.25.
DEFINE_TESTCASE(corruptglass1, glass) {
    string db_path =
	test_driver::get_srcdir() + "/testdata/glass_corrupt_db1";

    for (int n = '1'; n <= '3'; ++n) {
	db_path.back() = n;
	TEST_EXCEPTION(Xapian::DatabaseCorruptError,
		       Xapian::Database db(db_path));

	TEST_EXCEPTION(Xapian::DatabaseCorruptError,
		       Xapian::Database::check(db_path));
    }
}
