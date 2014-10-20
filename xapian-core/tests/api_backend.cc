/** @file api_backend.cc
 * @brief Backend-related tests.
 */
/* Copyright (C) 2008,2009,2010,2011,2013 Olly Betts
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

#include "str.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"
#include "utils.h"

#include "apitest.h"

#include "safefcntl.h"
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

/// Regression test for bug fixed in 1.2.13 and 1.3.1.
DEFINE_TESTCASE(lockfilealreadyopen1, brass || chert) {
    string path = get_named_writable_database_path("lockfilealreadyopen1");
    int fd = ::open((path + "/flintlock").c_str(), O_RDONLY);
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

    try {
	Xapian::Enquire enq(rodb);
	enq.set_query(Xapian::Query("abc"));
	Xapian::MSet mset = enq.get_mset(0, 10);
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
DEFINE_TESTCASE(msize1, generated) {
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
DEFINE_TESTCASE(msize2, generated) {
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

/// Regression test for bug in decay of XOR, fixed in 1.2.1 and 1.0.21.
DEFINE_TESTCASE(xordecay1, generated) {
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
DEFINE_TESTCASE(ordecay1, generated) {
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
    return true;
}

/** Regression test for bug in decay of OR to AND_MAYBE, fixed in 1.2.1 and
 *  1.0.21.
 */
DEFINE_TESTCASE(ordecay2, generated) {
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
DEFINE_TESTCASE(orcheck1, generated) {
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

    return true;
}

/** Regression test for bug fixed in 1.2.1 and 1.0.21.
 *
 *  We failed to mark the Btree as unmodified after cancel().
 */
DEFINE_TESTCASE(failedreplace1, brass || chert || flint) {
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
    return true;
}

DEFINE_TESTCASE(failedreplace2, brass || chert || flint) {
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
    return true;
}

/// Coverage for SelectPostList::skip_to().
DEFINE_TESTCASE(phrase3, positional) {
    Xapian::Database db = get_database("apitest_phrase");

    const char * phrase_words[] = { "phrase", "near" };
    Xapian::Query q(Xapian::Query::OP_NEAR, phrase_words, phrase_words + 2, 12);
    q = Xapian::Query(Xapian::Query::OP_AND_MAYBE, Xapian::Query("pad"), q);

    Xapian::Enquire enquire(db);
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 5);

    return true;
}

/// Check that get_mset(<large number>, 10) doesn't exhaust memory needlessly.
// Regression test for fix in 1.2.4.
DEFINE_TESTCASE(msetfirst2, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("paragraph"));
    Xapian::MSet mset;
    // Before the fix, this tried to allocated too much memory.
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
    return true;
}

DEFINE_TESTCASE(bm25weight2, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("the"));
    enquire.set_weighting_scheme(Xapian::BM25Weight(0, 0, 0, 0, 1));
    Xapian::MSet mset = enquire.get_mset(0, 100);
    TEST_REL(mset.size(),>=,2);
    Xapian::weight weight0 = mset[0].get_weight();
    for (size_t i = 1; i != mset.size(); ++i) {
	TEST_EQUAL(weight0, mset[i].get_weight());
    }
    return true;
}

DEFINE_TESTCASE(tradweight2, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("the"));
    enquire.set_weighting_scheme(Xapian::TradWeight(0));
    Xapian::MSet mset = enquire.get_mset(0, 100);
    TEST_REL(mset.size(),>=,2);
    Xapian::weight weight0 = mset[0].get_weight();
    for (size_t i = 1; i != mset.size(); ++i) {
	TEST_EQUAL(weight0, mset[i].get_weight());
    }
    return true;
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
	Xapian::Query::OP_ELITE_SET
    };
    const Xapian::Query::op * p;
    for (p = ops; p - ops != sizeof(ops) / sizeof(*ops); ++p) {
	tout << *p << endl;
	Xapian::Enquire enquire(db);
	Xapian::Query query(*p, Xapian::Query("a"), Xapian::Query("b"));
	enquire.set_query(query);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST_EQUAL(mset.get_matches_estimated(), 0);
	TEST_EQUAL(mset.get_matches_upper_bound(), 0);
	TEST_EQUAL(mset.get_matches_lower_bound(), 0);
    }
    return true;
}

/// Test error opening non-existent stub databases.
// Regression test for bug fixed in 1.3.1 and 1.2.11.
DEFINE_TESTCASE(stubdb7, !backend) {
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Auto::open_stub("nosuchdirectory"));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Auto::open_stub("nosuchdirectory", Xapian::DB_OPEN));
    return true;
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
    for (size_t i = 0; i < mset.size(); ++i) {
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
    for (size_t i = 0; i < mset.size(); ++i) {
	TEST_EQUAL(*mset[i], expected2[i].did);
	TEST_EQUAL_DOUBLE(mset[i].get_weight(), expected2[i].wt);
    }

    return true;
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

    return true;
}

/// Check handling of invalid block sizes.
// Regression test for bug fixed in 1.2.17 and 1.3.2 - the size gets fixed
// but the uncorrected size was passed to the base file.  Also, abort() was
// called on 0.
DEFINE_TESTCASE(blocksize1, brass || chert || flint) {
    string db_dir = "." + get_dbtype();
    mkdir(db_dir.c_str(), 0755);
    db_dir += "/db__blocksize1";
    static const unsigned bad_sizes[] = {
	65537, 8000, 2000, 1024, 16, 7, 3, 1, 0
    };
    for (size_t i = 0; i < sizeof(bad_sizes) / sizeof(bad_sizes[0]); ++i) {
	size_t block_size = bad_sizes[i];
	rm_rf(db_dir);
	Xapian::WritableDatabase db;
	if (get_dbtype() == "chert") {
#ifdef XAPIAN_HAS_CHERT_BACKEND
	    db = Xapian::Chert::open(db_dir, Xapian::DB_CREATE, block_size);
#else
	    SKIP_TEST("chert backend disabled");
#endif
	} else if (get_dbtype() == "flint") {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	    db = Xapian::Flint::open(db_dir, Xapian::DB_CREATE, block_size);
#else
	    SKIP_TEST("flint backend disabled");
#endif
	} else {
#ifdef XAPIAN_HAS_BRASS_BACKEND
	    db = Xapian::Brass::open(db_dir, Xapian::DB_CREATE, block_size);
#else
	    SKIP_TEST("brass backend disabled");
#endif
	}
	Xapian::Document doc;
	doc.add_term("XYZ");
	doc.set_data("foo");
	db.add_document(doc);
	db.commit();
    }
    return true;
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
DEFINE_TESTCASE(phrasebug1, generated && positional) {
    Xapian::Database db = get_database("phrasebug1", make_phrasebug1_db);
    const char * qterms[] = { "katrina", "hurricane" };
    Xapian::Enquire e(db);
    Xapian::Query q(q.OP_PHRASE, qterms, qterms + 2, 5);
    e.set_query(q);
    Xapian::MSet mset = e.get_mset(0, 100);
    TEST_EQUAL(mset.size(), 0);
    const char * qterms2[] = { "hurricane", "katrina" };
    Xapian::Query q2(q.OP_PHRASE, qterms2, qterms2 + 2, 5);
    e.set_query(q2);
    mset = e.get_mset(0, 100);
    TEST_EQUAL(mset.size(), 1);
    return true;
}
