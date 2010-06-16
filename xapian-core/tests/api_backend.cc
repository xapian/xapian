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

#include <cstring> // For strcmp().

#include "safeunistd.h"

using namespace std;

/// Regression test - lockfile should honour umask, was only user-readable.
DEFINE_TESTCASE(lockfileumask1, flint) {
#if !defined __WIN32__ && !defined __CYGWIN__ && !defined __EMX__
    mode_t old_umask = umask(022);
    try {
	Xapian::WritableDatabase db = get_named_writable_database("lockfileumask1");

	string path;
	const string & dbtype = get_dbtype();
	if (dbtype == "flint") {
	    // Hardcoded path for 1.0.x backport:
	    path = ".flint/dbw__lockfileumask1/flintlock";
	} else {
	    SKIP_TEST("Test only supported for flint backend");
	}

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
    db.flush();
    TEST_EQUAL(db.get_avlength(), 2000000000);
    if (strcmp(get_dbtype(), "inmemory") != 0) {
	// InMemory doesn't support get_writable_database_as_database().
	Xapian::Database dbr = get_writable_database_as_database();
	TEST_EQUAL(dbr.get_avlength(), 2000000000);
    }
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
    db.flush();
    db.replace_document(1, a);
    db.replace_document(1, b);
   
    mset_expect_order(enq.get_mset(0, 2), 2);
   
    return true;
}

DEFINE_TESTCASE(lockfilefd0or1, flint) {
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
DEFINE_TESTCASE(replacedoc7, writable && !inmemory && !remote && !quartz) {
    // The inmemory backend doesn't batch changes, so there's nothing to
    // check there.
    //
    // The remote backend doesn't implement the lazy replacement of documents
    // optimisation currently, and neither does quartz.
    Xapian::WritableDatabase db(get_writable_database());
    Xapian::Document doc;
    doc.set_data("fish");
    doc.add_term("Hlocalhost");
    doc.add_posting("hello", 1);
    doc.add_posting("world", 2);
    doc.add_value(1, "myvalue");
    db.add_document(doc);
    db.flush();

    // We add a second document, and then replace the first document with
    // itself 10000 times.  If the document count for the database reopened
    // read-only is 2, then we triggered an automatic flush.

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

/// Test coverage for DatabaseModifiedError.
DEFINE_TESTCASE(databasemodified1, writable && !inmemory && !remote) {
    // The inmemory backend doesn't support revisions.
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
    db.flush();

    Xapian::Database rodb(get_writable_database_as_database());
    db.add_document(doc);
    db.flush();

    db.add_document(doc);
    db.flush();

    db.add_document(doc);
    try {
	TEST_EQUAL(*rodb.termlist_begin(N - 1), "abc");
	return false;
    } catch (const Xapian::DatabaseModifiedError &) {
    }

    return true;
}

/// Regression test for bug#462 fixed in 1.0.19 and 1.1.5.
DEFINE_TESTCASE(qpmemoryleak1, writable && !inmemory) {
    // Inmemory never throws DatabaseModifiedError.
    Xapian::WritableDatabase wdb(get_writable_database());
    Xapian::Document doc;

    doc.add_term("foo");
    for (int i = 100; i < 120; ++i) {
        doc.add_term(om_tostring(i));
    }

    for (int j = 0; j < 50; ++j) {
        wdb.add_document(doc);
    }
    wdb.flush();

    Xapian::Database database(get_writable_database_as_database());
    Xapian::QueryParser queryparser;
    queryparser.set_database(database);
    TEST_EXCEPTION(Xapian::DatabaseModifiedError,
	for (int k = 0; k < 3; ++k) {
	    wdb.add_document(doc);
	    wdb.flush();
	    (void)queryparser.parse_query("1", queryparser.FLAG_PARTIAL);
	}
    );

    return true;
}

/// Regression test for ticket#464, fixed in 1.1.6 and 1.0.20.
DEFINE_TESTCASE(msize1, writable && !inmemory && !remote) {
    Xapian::WritableDatabase db = get_writable_database();
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

/// Regression test for bug related to ticket#464, fixed in 1.1.6 and 1.0.20.
DEFINE_TESTCASE(msize2, writable && !inmemory && !remote) {
    Xapian::WritableDatabase db = get_writable_database();
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

/// Regression test for bug in decay of XOR, fixed in 1.2.1 and 1.0.21.
DEFINE_TESTCASE(xordecay1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    for (int n = 1; n != 50; ++n) {
	Xapian::Document doc;
	for (int i = 1; i != 50; ++i) {
	    if (n % i == 0)
		doc.add_term("N" + str(i));
	}
	db.add_document(doc);
    }

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
DEFINE_TESTCASE(ordecay1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    make_ordecay_db(db, string());

    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query(Xapian::Query::OP_OR,
				Xapian::Query("N20"),
				Xapian::Query("N21")));

    Xapian::MSet msetall = enq.get_mset(0, db.get_doccount());
    for (unsigned int i = 1; i < msetall.size(); ++i) {
	Xapian::MSet submset = enq.get_mset(0, i);
	TEST(mset_range_is_same(submset, 0, msetall, 0, submset.size()));
    }
    return true;
}

/** Regression test for bug in decay of OR to AND_MAYBE, fixed in 1.2.1 and
 *  1.0.21.
 */
DEFINE_TESTCASE(ordecay2, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    make_ordecay_db(db, string());
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
    return true;
}

static void
make_orcheck_db(Xapian::WritableDatabase &db, const string &)
{
    static const int t1[6] = {2, 4, 6, 8, 10, 0};
    static const int t2[11] = {6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 0};
    static const int t3[11] = {3, 7, 8, 11, 12, 13, 14, 15, 16, 17, 0};

    for (unsigned i = 1; i <= 17; ++i) {
	Xapian::Document doc;
	db.replace_document(i, doc);
    }
    for (const int * p = t1; *p != 0; ++p) {
	Xapian::Document doc(db.get_document(*p));
	doc.add_term("T1");
	db.replace_document(*p, doc);
    }
    for (const int * p = t2; *p != 0; ++p) {
	Xapian::Document doc(db.get_document(*p));
	doc.add_term("T2");
	if (*p < 17) {
	    doc.add_term("T2_lowfreq");
	}
	doc.add_value(2, "1");
	db.replace_document(*p, doc);
    }
    for (const int * p = t3; *p != 0; ++p) {
	Xapian::Document doc(db.get_document(*p));
	doc.add_term("T3");
	if (*p < 17) {
	    doc.add_term("T3_lowfreq");
	}
	doc.add_value(3, "1");
	db.replace_document(*p, doc);
    }
}

/** Regression test for bugs in the check() method of OrPostList. (ticket #485)
 *  Bugs introduced and fixed between 1.2.0 and 1.2.1 (never in a release).
 */
DEFINE_TESTCASE(orcheck1, writable) {
    Xapian::WritableDatabase db = get_writable_database();
    make_orcheck_db(db, string());
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

    return true;
}

/** Regression test for bug fixed in 1.2.1 and 1.0.21.
 *
 *  We failed to mark the Btree as unmodified after cancel().
 */
DEFINE_TESTCASE(failedadd1, flint) {
    // The fix was applied to quartz too, but this testcase doesn't work there
    // because quartz doesn't vet term length until changes are flushed.
    Xapian::WritableDatabase db(get_writable_database());
    Xapian::Document doc;
    doc.add_term("foo");
    db.add_document(doc);
    Xapian::docid did = db.add_document(doc);
    doc.add_term("abc");
    doc.add_term(string(1000, 'm'));
    doc.add_term("xyz");
    TEST_EXCEPTION(Xapian::InvalidArgumentError, db.replace_document(did, doc));
    db.flush();
    TEST_EQUAL(db.get_doccount(), 0);
    TEST_EQUAL(db.get_termfreq("foo"), 0);
    return true;
}
