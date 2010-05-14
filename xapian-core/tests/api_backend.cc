/** @file api_backend.cc
 * @brief Backend-related tests.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
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

#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"

#include "safesysstat.h"
#include "safeunistd.h"

using namespace std;

/// Regression test - lockfile should honour umask, was only user-readable.
DEFINE_TESTCASE(lockfileumask1, brass || chert || flint) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __EMX__
    mode_t old_umask = umask(022);
    try {
	Xapian::WritableDatabase db = get_named_writable_database("lockfileumask1");

	string path = get_named_writable_database_path("lockfileumask1");
	path += "/flintlock";

	struct stat statbuf;
	TEST(stat(path, &statbuf) == 0);
	TEST_EQUAL(statbuf.st_mode & 0777, 0644);
    } catch (...) {
	umask(old_umask);
	throw;
    }

    umask(old_umask);
#endif

    return true;
}

/// Check that the backend handles total document length > 0xffffffff.
DEFINE_TESTCASE(totaldoclen1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_posting("foo", 1, 2000000000);
    db.add_document(doc);
    db.add_document(doc);
    TEST_EQUAL(db.get_avlength(), 2000000000);
    db.commit();
    TEST_EQUAL(db.get_avlength(), 2000000000);
    if (get_dbtype() != "inmemory") {
	// InMemory doesn't support get_writable_database_as_database().
	Xapian::Database dbr = get_writable_database_as_database();
	TEST_EQUAL(dbr.get_avlength(), 2000000000);
    }
    return true;
}

DEFINE_TESTCASE(dbstats1, backend) {
    Xapian::Database db = get_database("etext");

    // Use precalculated values to avoid expending CPU cycles to calculate
    // these every time without improving test coverage.
    const Xapian::termcount min_len = 2;
    const Xapian::termcount max_len = 532;
    const Xapian::termcount max_wdf = 22;

    if (get_dbtype().find("chert") != string::npos ||
	get_dbtype().find("brass") != string::npos) {
	// Should be exact for brass and chert as no deletions have happened.
	TEST_EQUAL(db.get_doclength_upper_bound(), max_len);
	TEST_EQUAL(db.get_doclength_lower_bound(), min_len);
    } else {
	// For other backends, we usually give rather loose bounds.
	TEST_REL(db.get_doclength_upper_bound(),>=,max_len);
	TEST_REL(db.get_doclength_lower_bound(),<=,min_len);
    }

    TEST_REL(db.get_wdf_upper_bound("the"),>=,max_wdf);

    return true;
}

/// Check handling of alldocs on an empty database.
DEFINE_TESTCASE(alldocspl3, backend) {
    Xapian::Database db = get_database(string());

    TEST_EQUAL(db.get_termfreq(string()), 0);
    TEST_EQUAL(db.get_collection_freq(string()), 0);
    TEST(db.postlist_begin(string()) == db.postlist_end(string()));

    return true;
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
   
    return true;
}

/// Regression test for chert bug fixed in 1.1.3 (ticket#397).
DEFINE_TESTCASE(doclenaftercommit1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    TEST_EXCEPTION(Xapian::DocNotFoundError, db.get_doclength(1));
    db.replace_document(1, Xapian::Document());
    db.commit();
    TEST_EQUAL(db.get_doclength(1), 0);;
    return true;
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
    return true;
}

DEFINE_TESTCASE(lockfilefd0or1, brass || chert || flint) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __EMX__
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

    return true;
}

struct MyMatchDecider : public Xapian::MatchDecider {
    mutable bool called;
  
    MyMatchDecider() : called(false) { }

    bool operator()(const Xapian::Document &) const {
	called = true;
	return true;
    }
};

/// Test Xapian::MatchDecider with remote backend fails.
DEFINE_TESTCASE(matchdecider4, remote) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));

    MyMatchDecider mdecider, mspyold;
    Xapian::MSet mset;

    TEST_EXCEPTION(Xapian::UnimplementedError,
	mset = enquire.get_mset(0, 10, NULL, &mdecider));
    TEST(!mdecider.called);

    TEST_EXCEPTION(Xapian::UnimplementedError,
	mset = enquire.get_mset(0, 10, 0, NULL, NULL, &mspyold));
    TEST(!mspyold.called);

    TEST_EXCEPTION(Xapian::UnimplementedError,
	mset = enquire.get_mset(0, 10, 0, NULL, &mdecider, &mspyold));
    TEST(!mdecider.called);
    TEST(!mspyold.called);

    return true;
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
    rodb.reopen();

    TEST_EQUAL(rodb.get_doccount(), 2);
    return true;
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
    return true;
}

/** Check that the parent directory for the database doesn't need to be
 *  writable.  Regression test for early brass versions which failed to
 *  append a "/" when generating a temporary filename from the database
 *  directory.
 */
DEFINE_TESTCASE(readonlyparentdir1, brass || chert || flint) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __EMX__
    string path = get_named_writable_database_path("readonlyparentdir1");
    mkdir(path, 0777);
    mkdir(path + "/sub", 0777);
    TEST(chmod(path.c_str(), 0500) == 0);
    Xapian::WritableDatabase db = get_named_writable_database("readonlyparentdir1/sub");
    Xapian::Document doc;
    doc.add_term("hello");
    doc.set_data("some text");
    db.add_document(doc);
    db.commit();
    TEST(chmod(path.c_str(), 0700) == 0);
#endif
    return true;
}

/// Test coverage for DatabaseModifiedError.
DEFINE_TESTCASE(databasemodified1, writable && !inmemory && !brass && !remote) {
    // The inmemory backend doesn't support revisions.
    //
    // The brass backend doesn't reuse blocks currently (FIXME).
    //
    // The remote backend doesn't work as expected here, I think due to
    // test harness issues.
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
	return false;
    } catch (const Xapian::DatabaseModifiedError &) {
    }

    return true;
}

/// Regression test for bug#462 fixed in 1.0.19 and 1.1.5.
DEFINE_TESTCASE(qpmemoryleak1, writable && !inmemory && !brass) {
    // Inmemory never throws DatabaseModifiedError.
    // Brass doesn't reuse blocks currently (FIXME).
    SKIP_TEST_FOR_BACKEND("remoteprog_brass");
    SKIP_TEST_FOR_BACKEND("remotetcp_brass");
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
	for (int k = 0; k < 3; ++k) {
	    wdb.add_document(doc);
	    wdb.commit();
	    (void)queryparser.parse_query("1", queryparser.FLAG_PARTIAL);
	}
    );

    return true;
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
DEFINE_TESTCASE(msize1, writable && !inmemory && !remote) {
    // Generated databases not supported by inmemory or remote.
    Xapian::Database db = get_database("msize1", make_msize1_db);
    Xapian::Enquire enq(db);
    enq.set_sort_by_value(1, false);
    enq.set_collapse_key(0);
    enq.set_query(Xapian::Query("K1"));

    Xapian::MSet mset = enq.get_mset(0, 10, 1000);
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

    Xapian::MSet mset3 = enq.get_mset(0, 60);
    Xapian::doccount lb3 = mset3.get_matches_lower_bound();
    Xapian::doccount ub3 = mset3.get_matches_upper_bound();
    Xapian::doccount est3 = mset3.get_matches_estimated();
    TEST_EQUAL(lb3, ub3);
    TEST_EQUAL(lb3, est3);
    TEST_EQUAL(est, est3);

    return true;
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
DEFINE_TESTCASE(msize2, writable && !inmemory && !remote) {
    // Generated databases not supported by inmemory or remote.
    Xapian::Database db = get_database("msize2", make_msize2_db);
    Xapian::Enquire enq(db);
    enq.set_sort_by_value(1, false);
    enq.set_collapse_key(0);
    enq.set_query(Xapian::Query("K1"));

    Xapian::MSet mset = enq.get_mset(0, 10, 1000);
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

    Xapian::MSet mset3 = enq.get_mset(0, 60);
    Xapian::doccount lb3 = mset3.get_matches_lower_bound();
    Xapian::doccount ub3 = mset3.get_matches_upper_bound();
    Xapian::doccount est3 = mset3.get_matches_estimated();
    TEST_EQUAL(lb3, ub3);
    TEST_EQUAL(lb3, est3);
    TEST_EQUAL(est, est3);

    return true;
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

/// Regression test for bug in decay of XOR, fixed in 1.2.1.
DEFINE_TESTCASE(xordecay1, writable && !inmemory && !remote) {
    // Generated databases not supported by inmemory or remote.
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
    return true;
}
