/* api_db.cc: tests which need a backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2015,2016 Olly Betts
 * Copyright 2006,2007,2008,2009 Lemur Consulting Ltd
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

#include "api_db.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "safenetdb.h" // For gai_strerror().
#include "safesysstat.h" // For mkdir().
#include "safeunistd.h" // For sleep().

#include <xapian.h>

#include "backendmanager.h"
#include "backendmanager_local.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"

#include "apitest.h"

using namespace std;

static Xapian::Query
query(const string &t)
{
    return Xapian::Query(Xapian::Stem("english")(t));
}

// #######################################################################
// # Tests start here

// tests Xapian::Database::get_termfreq() and Xapian::Database::term_exists()
DEFINE_TESTCASE(termstats, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST(!db.term_exists("corn"));
    TEST_EQUAL(db.get_termfreq("corn"), 0);
    TEST(db.term_exists("banana"));
    TEST_EQUAL(db.get_termfreq("banana"), 1);
    TEST(db.term_exists("paragraph"));
    TEST_EQUAL(db.get_termfreq("paragraph"), 5);

    return true;
}

// Check that stub databases work.
DEFINE_TESTCASE(stubdb1, backend && !inmemory && !remote) {
    // Only works for backends which have a path.
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb1";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "auto ../" << get_database_path("apitest_simpledata") << endl;
    out.close();

    {
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }
    {
	Xapian::Database db(dbpath);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }

    return true;
}

// Check that stub databases work remotely.
DEFINE_TESTCASE(stubdb2, backend && !inmemory && !remote) {
    // Only works for backends which have a path.
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb2";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "remote :" << BackendManager::get_xapian_progsrv_command()
	<< ' ' << get_database_path("apitest_simpledata") << endl;
    out.close();

    {
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }
    {
	Xapian::Database db(dbpath);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }

    out.open(dbpath);
    TEST(out.is_open());
    out << "remote" << endl;
    out.close();

    // Quietly ignored prior to 1.4.1.
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB)
    );

    // Quietly ignored prior to 1.4.1.
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::WritableDatabase db(dbpath, Xapian::DB_BACKEND_STUB)
    );

    out.open(dbpath);
    TEST(out.is_open());
    out << "remote foo" << endl;
    out.close();

    // Quietly ignored prior to 1.4.1.
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB)
    );

    // Quietly ignored prior to 1.4.1.
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::WritableDatabase db(dbpath, Xapian::DB_BACKEND_STUB)
    );

    out.open(dbpath);
    TEST(out.is_open());
    out << "remote [::1]:80" << endl;
    out.close();

    try {
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB);
    } catch (const Xapian::NetworkError& e) {
	// 1.4.0 threw (Linux):
	//  NetworkError: Couldn't resolve host [ (context: remote:tcp([:0)) (No address associated with hostname)
	// 1.4.1 throws (because we don't actually support IPv6 yet) on Linux (EAI_ADDRFAMILY):
	//  NetworkError: Couldn't resolve host ::1 (context: remote:tcp(::1:80)) (nodename nor servname provided, or not known)
	// or on OS X (EAI_NONAME):
	//  NetworkError: Couldn't resolve host ::1 (context: remote:tcp(::1:80)) (Address family for hostname not supported)
	// So we test the message instead of the error string for portability.
	TEST(e.get_msg().find("host ::1") != string::npos);
    }

    try {
	Xapian::WritableDatabase db(dbpath, Xapian::DB_BACKEND_STUB);
    } catch (const Xapian::NetworkError& e) {
	// 1.4.0 threw (Linux):
	//  NetworkError: Couldn't resolve host [ (context: remote:tcp([:0)) (No address associated with hostname)
	// 1.4.1 throws (because we don't actually support IPv6 yet) on Linux (EAI_ADDRFAMILY):
	//  NetworkError: Couldn't resolve host ::1 (context: remote:tcp(::1:80)) (nodename nor servname provided, or not known)
	// or on OS X (EAI_NONAME):
	//  NetworkError: Couldn't resolve host ::1 (context: remote:tcp(::1:80)) (Address family for hostname not supported)
	// So we test the message instead of the error string for portability.
	TEST(e.get_msg().find("host ::1") != string::npos);
    }

    out.open(dbpath);
    TEST(out.is_open());
    // Invalid - the port number is required.
    out << "remote [::1]" << endl;
    out.close();

    // 1.4.0 threw:
    // NetworkError: Couldn't resolve host [ (context: remote:tcp([:0)) (No address associated with hostname)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB);
    );

    // 1.4.0 threw:
    // NetworkError: Couldn't resolve host [ (context: remote:tcp([:0)) (No address associated with hostname)
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::WritableDatabase db(dbpath, Xapian::DB_BACKEND_STUB);
    );

    return true;
}

// Regression test - bad entries were ignored after a good entry prior to 1.0.8.
DEFINE_TESTCASE(stubdb3, backend && !inmemory && !remote) {
    // Only works for backends which have a path.
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb3";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "auto ../" << get_database_path("apitest_simpledata") << "\n"
	   "bad line here\n";
    out.close();

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB));

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath));

    return true;
}

// Test a stub database with just a bad entry.
DEFINE_TESTCASE(stubdb4, backend && !inmemory && !remote) {
    // Only works for backends which have a path.
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb4";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "bad line here\n";
    out.close();

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB));

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath));

    return true;
}

// Test a stub database with a bad entry with no spaces (prior to 1.1.0 this
// was deliberately allowed, though not documented.
DEFINE_TESTCASE(stubdb5, backend && !inmemory && !remote) {
    // Only works for backends which have a path.
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb5";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "bad\n"
	   "auto ../" << get_database_path("apitest_simpledata") << endl;
    out.close();

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB));

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Database db(dbpath));

    return true;
}

// Test a stub database with an inmemory database (new feature in 1.1.0).
DEFINE_TESTCASE(stubdb6, inmemory) {
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb6";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "inmemory\n";
    out.close();

    // Read-only tests:
    {
	Xapian::Database db(dbpath, Xapian::DB_BACKEND_STUB);
	TEST_EQUAL(db.get_doccount(), 0);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST(mset.empty());
    }
    {
	Xapian::Database db(dbpath);
	TEST_EQUAL(db.get_doccount(), 0);
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST(mset.empty());
    }

    // Writable tests:
    {
	Xapian::WritableDatabase db(dbpath,
		Xapian::DB_OPEN|Xapian::DB_BACKEND_STUB);
	TEST_EQUAL(db.get_doccount(), 0);
	db.add_document(Xapian::Document());
	TEST_EQUAL(db.get_doccount(), 1);
    }
    {
	Xapian::WritableDatabase db(dbpath,
		Xapian::DB_OPEN|Xapian::DB_BACKEND_STUB);
	TEST_EQUAL(db.get_doccount(), 0);
	db.add_document(Xapian::Document());
	TEST_EQUAL(db.get_doccount(), 1);
    }

    return true;
}

/// Test error running Database::check() on a stub database.
// Regression test - in 1.4.3 and earlier this threw
// Xapian::DatabaseError.
DEFINE_TESTCASE(stubdb8, inmemory) {
    mkdir(".stub", 0755);
    const char * dbpath = ".stub/stubdb8";
    ofstream out(dbpath);
    TEST(out.is_open());
    out << "inmemory\n";
    out.close();

    try {
	Xapian::Database::check(dbpath);
	FAIL_TEST("Managed to check inmemory stub");
    } catch (const Xapian::DatabaseOpeningError & e) {
	// Check the message is appropriate.
	TEST_STRINGS_EQUAL(e.get_msg(),
			   "File is not a Xapian database or database table");
    }
    return true;
}

#if 0 // the "force error" mechanism is no longer in place...
class MyErrorHandler : public Xapian::ErrorHandler {
    public:
	int count;

	bool handle_error(Xapian::Error & error) {
	    ++count;
	    tout << "Error handling caught: " << error.get_description()
		 << ", count is now " << count << "\n";
	    return true;
	}

	MyErrorHandler() : count (0) {}
};

// tests error handler in multimatch().
DEFINE_TESTCASE(multierrhandler1, backend) {
    MyErrorHandler myhandler;

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Database mydb3(get_database("apitest_simpledata2"));
    int errcount = 1;
    for (int testcount = 0; testcount < 14; testcount ++) {
	tout << "testcount=" << testcount << "\n";
	Xapian::Database mydb4(get_database("-e", "apitest_termorder"));
	Xapian::Database mydb5(get_network_database("apitest_termorder", 1));
	Xapian::Database mydb6(get_database("-e2", "apitest_termorder"));
	Xapian::Database mydb7(get_database("-e3", "apitest_simpledata"));

	Xapian::Database dbs;
	switch (testcount) {
	    case 0:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		break;
	    case 1:
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 2:
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		break;
	    case 3:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		sleep(1);
		break;
	    case 4:
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		sleep(1);
		break;
	    case 5:
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		sleep(1);
		break;
	    case 6:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		break;
	    case 7:
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 8:
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		break;
	    case 9:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		break;
	    case 10:
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 11:
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		break;
	    case 12:
		dbs.add_database(mydb2);
		dbs.add_database(mydb6);
		dbs.add_database(mydb7);
		break;
	    case 13:
		dbs.add_database(mydb2);
		dbs.add_database(mydb7);
		dbs.add_database(mydb6);
		break;
	}
	tout << "db=" << dbs << "\n";
	Xapian::Enquire enquire(dbs, &myhandler);

	// make a query
	Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
	enquire.set_weighting_scheme(Xapian::BoolWeight());
	enquire.set_query(myquery);

	tout << "query=" << myquery << "\n";
	// retrieve the top ten results
	Xapian::MSet mymset = enquire.get_mset(0, 10);

	switch (testcount) {
	    case 0: case 3: case 6: case 9:
		mset_expect_order(mymset, 2, 4, 10);
		break;
	    case 1: case 4: case 7: case 10:
		mset_expect_order(mymset, 3, 5, 11);
		break;
	    case 2: case 5: case 8: case 11:
		mset_expect_order(mymset, 1, 6, 12);
		break;
	    case 12:
	    case 13:
		mset_expect_order(mymset, 4, 10);
		errcount += 1;
		break;
	}
	TEST_EQUAL(myhandler.count, errcount);
	errcount += 1;
    }

    return true;
}
#endif

class GrepMatchDecider : public Xapian::MatchDecider {
    string needle;
  public:
    explicit GrepMatchDecider(const string& needle_)
	: needle(needle_) {}

    bool operator()(const Xapian::Document &doc) const {
	// Note that this is not recommended usage of get_data()
	return doc.get_data().find(needle) != string::npos;
    }
};

// Test Xapian::MatchDecider functor.
DEFINE_TESTCASE(matchdecider1, backend && !remote) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("this"));

    GrepMatchDecider myfunctor("This is");

    Xapian::MSet mymset = enquire.get_mset(0, 100, 0, &myfunctor);

    vector<bool> docid_checked(db.get_lastdocid());

    // Check that we get the expected number of matches, and that they
    // satisfy the condition.
    Xapian::MSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    TEST_EQUAL(mymset.size(), 3);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 3);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 3);
    TEST_EQUAL(mymset.get_matches_estimated(), 3);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 3);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 3);
    TEST_EQUAL(mymset.get_uncollapsed_matches_estimated(), 3);
    for ( ; i != mymset.end(); ++i) {
	const Xapian::Document doc(i.get_document());
	TEST(myfunctor(doc));
	docid_checked[*i] = true;
    }

    // Check that there are some documents which aren't accepted by the match
    // decider.
    mymset = enquire.get_mset(0, 100);
    TEST(mymset.size() > 3);

    // Check that the bounds are appropriate even if we don't ask for any
    // actual matches.
    mymset = enquire.get_mset(0, 0, 0, &myfunctor);
    TEST_EQUAL(mymset.size(), 0);
    TEST_EQUAL(mymset.get_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_matches_upper_bound(), 6);
    TEST_REL(mymset.get_matches_estimated(),>,0);
    TEST_REL(mymset.get_matches_estimated(),<=,6);
    TEST_EQUAL(mymset.get_uncollapsed_matches_lower_bound(), 0);
    TEST_EQUAL(mymset.get_uncollapsed_matches_upper_bound(), 6);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),>,0);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),<=,6);

    // Check that the bounds are appropriate if we ask for only one hit.
    // (Regression test - until SVN 10256, we didn't reduce the lower_bound
    // appropriately, and returned 6 here.)
    mymset = enquire.get_mset(0, 1, 0, &myfunctor);
    TEST_EQUAL(mymset.size(), 1);
    TEST_REL(mymset.get_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_matches_estimated(),>,0);
    TEST_REL(mymset.get_matches_estimated(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),>,0);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),<=,6);

    // Check that the other documents don't satisfy the condition.
    for (Xapian::docid did = 1; did < docid_checked.size(); ++did) {
	if (!docid_checked[did]) {
	    TEST(!myfunctor(db.get_document(did)));
	}
    }

    // Check that the bounds are appropriate if a collapse key is used.
    // Use a value which is never set so we don't actually discard anything.
    enquire.set_collapse_key(99);
    mymset = enquire.get_mset(0, 1, 0, &myfunctor);
    TEST_EQUAL(mymset.size(), 1);
    TEST_REL(mymset.get_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_matches_estimated(),>,0);
    TEST_REL(mymset.get_matches_estimated(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),>,0);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),<=,6);

    // Check that the bounds are appropriate if a percentage cutoff is in
    // use.  Set a 1% threshold so we don't actually discard anything.
    enquire.set_collapse_key(Xapian::BAD_VALUENO);
    enquire.set_cutoff(1);
    mymset = enquire.get_mset(0, 1, 0, &myfunctor);
    TEST_EQUAL(mymset.size(), 1);
    TEST_REL(mymset.get_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_matches_estimated(),>,0);
    TEST_REL(mymset.get_matches_estimated(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),>,0);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),<=,6);

    // And now with both a collapse key and percentage cutoff.
    enquire.set_collapse_key(99);
    mymset = enquire.get_mset(0, 1, 0, &myfunctor);
    TEST_EQUAL(mymset.size(), 1);
    TEST_REL(mymset.get_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_matches_estimated(),>,0);
    TEST_REL(mymset.get_matches_estimated(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),>=,1);
    TEST_REL(mymset.get_uncollapsed_matches_lower_bound(),<=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),>=,3);
    TEST_REL(mymset.get_uncollapsed_matches_upper_bound(),<=,6);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),>,0);
    TEST_REL(mymset.get_uncollapsed_matches_estimated(),<=,6);

    return true;
}

// Test Xapian::MatchDecider functor used as a match spy.
DEFINE_TESTCASE(matchdecider2, backend && !remote) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("this"));

    GrepMatchDecider myfunctor("This is");

    Xapian::MSet mymset = enquire.get_mset(0, 100, 0, NULL, &myfunctor);

    vector<bool> docid_checked(db.get_lastdocid());

    // Check that we get the expected number of matches, and that they
    // satisfy the condition.
    Xapian::MSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    TEST_EQUAL(mymset.size(), 3);
    for ( ; i != mymset.end(); ++i) {
	const Xapian::Document doc(i.get_document());
	TEST(myfunctor(doc));
	docid_checked[*i] = true;
    }

    // Check that the other documents don't satisfy the condition.
    for (Xapian::docid did = 1; did < docid_checked.size(); ++did) {
	if (!docid_checked[did]) {
	    TEST(!myfunctor(db.get_document(did)));
	}
    }

    return true;
}

// Regression test for lower bound using functor, sorting and collapsing.
DEFINE_TESTCASE(matchdecider3, backend && !remote) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(""));
    enquire.set_collapse_key(12);
    enquire.set_sort_by_value(11, true);

    GrepMatchDecider myfunctor("We produce");

    Xapian::MSet mset1 = enquire.get_mset(0, 2, 0, NULL, &myfunctor);
    Xapian::MSet mset2 = enquire.get_mset(0, 1000, 0, NULL, &myfunctor);

    // mset2 should contain all the hits, so the statistics should be exact.
    TEST_EQUAL(mset2.get_matches_estimated(), mset2.size());
    TEST_EQUAL(mset2.get_matches_lower_bound(), mset2.get_matches_estimated());
    TEST_EQUAL(mset2.get_matches_estimated(), mset2.get_matches_upper_bound());

    TEST_REL(mset2.get_uncollapsed_matches_lower_bound(),<=,mset2.get_uncollapsed_matches_estimated());
    TEST_REL(mset2.get_uncollapsed_matches_estimated(),<=,mset2.get_uncollapsed_matches_upper_bound());

    // Check that the lower bound in mset1 is not greater than the known
    // number of hits.  This failed until revision 10811.
    TEST_REL(mset1.get_matches_lower_bound(),<=,mset2.size());

    // Check that the bounds for mset1 make sense.
    TEST_REL(mset1.get_matches_lower_bound(),<=,mset1.get_matches_estimated());
    TEST_REL(mset1.get_matches_estimated(),<=,mset1.get_matches_upper_bound());
    TEST_REL(mset1.size(),<=,mset1.get_matches_upper_bound());

    TEST_REL(mset1.get_uncollapsed_matches_lower_bound(),<=,mset1.get_uncollapsed_matches_estimated());
    TEST_REL(mset1.get_uncollapsed_matches_estimated(),<=,mset1.get_uncollapsed_matches_upper_bound());

    // The uncollapsed match would match all documents but the one the
    // matchdecider rejects.
    TEST_REL(mset1.get_uncollapsed_matches_upper_bound(),>=,db.get_doccount() - 1);
    TEST_REL(mset1.get_uncollapsed_matches_upper_bound(),<=,db.get_doccount());
    TEST_REL(mset2.get_uncollapsed_matches_upper_bound(),>=,db.get_doccount() - 1);
    TEST_REL(mset2.get_uncollapsed_matches_upper_bound(),<=,db.get_doccount());

    return true;
}

// tests that mset iterators on msets compare correctly.
DEFINE_TESTCASE(msetiterator1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 2);

    Xapian::MSetIterator j;
    j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    Xapian::MSetIterator n = mymset.begin();
    Xapian::MSetIterator o = mymset.begin();
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    TEST_EQUAL(j, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    TEST_NOT_EQUAL(k, o);
    o++;
    TEST_EQUAL(k, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, mymset.begin());
    TEST_EQUAL(n, mymset.end());

    return true;
}

// tests that mset iterators on empty msets compare equal.
DEFINE_TESTCASE(msetiterator2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 0);

    Xapian::MSetIterator j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests that begin().get_document() works when first != 0
DEFINE_TESTCASE(msetiterator3, backend) {
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(2, 10);

    TEST(!mymset.empty());
    Xapian::Document doc(mymset.begin().get_document());
    TEST(!doc.get_data().empty());

    return true;
}

// tests that eset iterators on empty esets compare equal.
DEFINE_TESTCASE(esetiterator1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(2, myrset);
    Xapian::ESetIterator j;
    j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    Xapian::ESetIterator n = myeset.begin();

    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, myeset.begin());
    TEST_EQUAL(n, myeset.end());

    return true;
}

// tests that eset iterators on empty esets compare equal.
DEFINE_TESTCASE(esetiterator2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(0, myrset);
    Xapian::ESetIterator j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests the collapse-on-key
DEFINE_TESTCASE(collapsekey1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    Xapian::doccount mymsize1 = mymset1.size();

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 100);

	TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
			 "Had no fewer items when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value.empty());
	    values[value] = *i;
	}
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately.
DEFINE_TESTCASE(collapsekey2, backend) {
    SKIP_TEST("Don't have a suitable database currently");
    // FIXME: this needs an appropriate database creating, but that's quite
    // subtle to do it seems.
    Xapian::Enquire enquire(get_database("apitest_simpledata2"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mset1 = enquire.get_mset(0, 1);

    // Test that if no duplicates are found, then the upper bound remains
    // unchanged and the lower bound drops.
    {
	enquire.set_query(Xapian::Query("this"));
	Xapian::valueno value_no = 3;
	enquire.set_collapse_key(value_no);
	Xapian::MSet mset = enquire.get_mset(0, 1);

	TEST_REL(mset.get_matches_lower_bound(),<,mset1.get_matches_lower_bound());
	TEST_EQUAL(mset.get_matches_upper_bound(), mset1.get_matches_upper_bound());
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately.
DEFINE_TESTCASE(collapsekey3, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 3);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST_AND_EXPLAIN(mymset1.get_matches_lower_bound() > mymset.get_matches_lower_bound(),
			 "Lower bound was not lower when performing collapse: don't know whether it worked.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() > mymset.get_matches_upper_bound(),
			 "Upper bound was not lower when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value.empty());
	    values[value] = *i;
	}
    }

    // Test that if the collapse value is always empty, then the upper bound
    // remains unchanged, and the lower bound is the same or lower (it can be
    // lower because the matcher counts the number of documents with empty
    // collapse keys, but may have rejected a document because its weight is
    // too low for the proto-MSet before it even looks at its collapse key).
    {
	Xapian::valueno value_no = 1000;
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST(mymset.get_matches_lower_bound() <= mymset1.get_matches_lower_bound());
	TEST_EQUAL(mymset.get_matches_upper_bound(), mymset1.get_matches_upper_bound());

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value.empty());
	    values[value] = *i;
	}
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately even when no results are requested.
DEFINE_TESTCASE(collapsekey4, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 0);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 0);

	TEST_AND_EXPLAIN(mymset.get_matches_lower_bound() == 1,
			 "Lower bound was not 1 when performing collapse but not asking for any results.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() == mymset.get_matches_upper_bound(),
			 "Upper bound was changed when performing collapse but not asking for any results.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value.empty());
	    values[value] = *i;
	}
    }

    return true;
}

// test for keepalives
DEFINE_TESTCASE(keepalive1, remote) {
    Xapian::Database db(get_remote_database("apitest_simpledata", 5000));

    /* Test that keep-alives work */
    for (int i = 0; i < 10; ++i) {
	sleep(2);
	db.keep_alive();
    }
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    enquire.get_mset(0, 10);

    /* Test that things break without keepalives */
    sleep(10);
    enquire.set_query(Xapian::Query("word"));
    TEST_EXCEPTION(Xapian::NetworkError,
		   enquire.get_mset(0, 10));

    return true;
}

// test that iterating through all terms in a database works.
DEFINE_TESTCASE(allterms1, backend) {
    Xapian::Database db(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();
    TEST(ati != db.allterms_end());
    TEST_EQUAL(*ati, "one");
    TEST_EQUAL(ati.get_termfreq(), 1);

    Xapian::TermIterator ati2 = ati;

    ati++;
    TEST(ati != db.allterms_end());
    if (verbose) {
	tout << "*ati = '" << *ati << "'\n";
	tout << "*ati.length = '" << (*ati).length() << "'\n";
	tout << "*ati == \"one\" = " << (*ati == "one") << "\n";
	tout << "*ati[3] = " << ((*ati)[3]) << "\n";
	tout << "*ati = '" << *ati << "'\n";
    }
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "one");
    TEST(ati2.get_termfreq() == 1);
#endif

    ++ati;
#if 0
    ++ati2;
#endif
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "three");
    TEST(ati2.get_termfreq() == 3);
#endif

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}

// test that iterating through all terms in two databases works.
DEFINE_TESTCASE(allterms2, backend) {
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));
    Xapian::TermIterator ati = db.allterms_begin();

    TEST(ati != db.allterms_end());
    TEST(*ati == "five");
    TEST(ati.get_termfreq() == 2);
    ati++;

    TEST(ati != db.allterms_end());
    TEST(*ati == "four");
    TEST(ati.get_termfreq() == 1);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    ++ati;
    TEST(ati != db.allterms_end());
    TEST(*ati == "six");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}

// test that skip_to sets at_end (regression test)
DEFINE_TESTCASE(allterms3, backend) {
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();

    ati.skip_to(string("zzzzzz"));
    TEST(ati == db.allterms_end());

    return true;
}

// test that next ignores extra entries due to long posting lists being
// chunked (regression test for quartz)
DEFINE_TESTCASE(allterms4, backend) {
    // apitest_allterms4 contains 682 documents each containing just the word
    // "foo".  682 was the magic number which started to cause Quartz problems.
    Xapian::Database db = get_database("apitest_allterms4");

    Xapian::TermIterator i = db.allterms_begin();
    TEST(i != db.allterms_end());
    TEST(*i == "foo");
    TEST(i.get_termfreq() == 682);
    ++i;
    TEST(i == db.allterms_end());

    return true;
}

// test that skip_to with an exact match sets the current term (regression test
// for quartz)
DEFINE_TESTCASE(allterms5, backend) {
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();
    ati.skip_to("three");
    TEST(ati != db.allterms_end());
    TEST_EQUAL(*ati, "three");

    return true;
}

// test allterms iterators with prefixes
DEFINE_TESTCASE(allterms6, backend) {
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));

    Xapian::TermIterator ati = db.allterms_begin("three");
    TEST(ati != db.allterms_end("three"));
    TEST_EQUAL(*ati, "three");
    ati.skip_to("three");
    TEST(ati != db.allterms_end("three"));
    TEST_EQUAL(*ati, "three");
    ati++;
    TEST(ati == db.allterms_end("three"));

    ati = db.allterms_begin("thre");
    TEST(ati != db.allterms_end("thre"));
    TEST_EQUAL(*ati, "three");
    ati.skip_to("three");
    TEST(ati != db.allterms_end("thre"));
    TEST_EQUAL(*ati, "three");
    ati++;
    TEST(ati == db.allterms_end("thre"));

    ati = db.allterms_begin("f");
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "five");
    TEST(ati != db.allterms_end("f"));
    ati.skip_to("three");
    TEST(ati == db.allterms_end("f"));

    ati = db.allterms_begin("f");
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "five");
    ati++;
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "four");
    ati++;
    TEST(ati == db.allterms_end("f"));

    ati = db.allterms_begin("absent");
    TEST(ati == db.allterms_end("absent"));

    return true;
}

// test that searching for a term with a special characters in it works
DEFINE_TESTCASE(specialterms1, backend) {
    Xapian::Enquire enquire(get_database("apitest_space"));
    Xapian::MSet mymset;
    Xapian::doccount count;
    Xapian::MSetIterator m;
    Xapian::Stem stemmer("english");

    enquire.set_query(stemmer("new\nline"));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 0; value_no < 7; ++value_no) {
	string value = mymset.begin().get_document().get_value(value_no);
	TEST_NOT_EQUAL(value, "");
	if (value_no == 0) {
	    TEST(value.size() > 263);
	    TEST_EQUAL(static_cast<unsigned char>(value[262]), 255);
	    for (int k = 0; k < 256; ++k) {
		TEST_EQUAL(static_cast<unsigned char>(value[k + 7]), k);
	    }
	}
    }

    enquire.set_query(stemmer(string("big\0zero", 8)));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    return true;
}

// test that terms with a special characters in appear correctly when iterating
// allterms
DEFINE_TESTCASE(specialterms2, backend) {
    Xapian::Database db(get_database("apitest_space"));

    // Check the terms are all as expected (after stemming) and that allterms
    // copes with iterating over them.
    Xapian::TermIterator t;
    t = db.allterms_begin();
    TEST_EQUAL(*t, "back\\slash"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, string("big\0zero", 8)); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "new\nlin"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "one\x01on"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "space man"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "tab\tbi"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "tu\x02tu"); ++t; TEST_EQUAL(t, db.allterms_end());

    // Now check that skip_to exactly a term containing a zero byte works.
    // This is a regression test for flint and quartz - an Assert() used to
    // fire in debug builds (the Assert was wrong - the actual code handled
    // this OK).
    t = db.allterms_begin();
    t.skip_to(string("big\0zero", 8));
    TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, string("big\0zero", 8));

    return true;
}

// test that rsets behave correctly with multiDBs
DEFINE_TESTCASE(rsetmultidb2, backend && !multi) {
    Xapian::Database mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    Xapian::Database mydb2(get_database("apitest_rset"));
    mydb2.add_database(get_database("apitest_simpledata2"));

    Xapian::Enquire enquire1(mydb1);
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery = query("is");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    Xapian::RSet myrset1;
    Xapian::RSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    Xapian::MSet mymset1a = enquire1.get_mset(0, 10);
    Xapian::MSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    Xapian::MSet mymset2a = enquire2.get_mset(0, 10);
    Xapian::MSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    mset_expect_order(mymset1a, 4, 3);
    mset_expect_order(mymset1b, 4, 3);
    mset_expect_order(mymset2a, 2, 5);
    mset_expect_order(mymset2b, 2, 5);

    TEST(mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2));
    TEST(mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2));
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

// tests an expand across multiple databases
DEFINE_TESTCASE(multiexpand1, backend && !multi) {
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make simple equivalent rsets, with a document from each database in each.
    Xapian::RSet rset1;
    Xapian::RSet rset2;
    rset1.add_document(1);
    rset1.add_document(7);
    rset2.add_document(1);
    rset2.add_document(2);

    // Retrieve all the ESet results in each of the three setups:

    // This is the single database one.
    Xapian::ESet eset1 = enquire1.get_eset(1000, rset1);

    // This is the multi database with approximation
    Xapian::ESet eset2 = enquire2.get_eset(1000, rset2);

    // This is the multi database without approximation
    Xapian::ESet eset3 = enquire2.get_eset(1000, rset2, Xapian::Enquire::USE_EXACT_TERMFREQ);

    TEST_EQUAL(eset1.size(), eset3.size());

    Xapian::ESetIterator i = eset1.begin();
    Xapian::ESetIterator j = eset3.begin();
    while (i != eset1.end() && j != eset3.end()) {
	TEST_EQUAL(*i, *j);
	TEST_EQUAL(i.get_weight(), j.get_weight());
	++i;
	++j;
    }
    TEST(i == eset1.end());
    TEST(j == eset3.end());

    bool eset1_eq_eset2 = true;
    i = eset1.begin();
    j = eset2.begin();
    while (i != eset1.end() && j != eset2.end()) {
	if (i.get_weight() != j.get_weight()) {
	    eset1_eq_eset2 = false;
	    break;
	}
	++i;
	++j;
    }
    TEST(!eset1_eq_eset2);

    return true;
}

// tests that opening a non-existent postlist returns an empty list
DEFINE_TESTCASE(postlist1, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.postlist_begin("rosebud"), db.postlist_end("rosebud"));

    string s = "let_us_see_if_we_can_break_it_with_a_really_really_long_term.";
    for (int i = 0; i < 8; ++i) {
	s += s;
	TEST_EQUAL(db.postlist_begin(s), db.postlist_end(s));
    }

    // A regression test (no, really!)
    TEST_NOT_EQUAL(db.postlist_begin("a"), db.postlist_end("a"));

    return true;
}

// tests that a Xapian::PostingIterator works as an STL iterator
DEFINE_TESTCASE(postlist2, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p;
    p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    TEST(p.get_description() != "PostingIterator()");

    // test operator= creates a copy which compares equal
    Xapian::PostingIterator p_copy = p;
    TEST_EQUAL(p, p_copy);

    TEST(p_copy.get_description() != "PostingIterator()");

    // test copy constructor creates a copy which compares equal
    Xapian::PostingIterator p_clone(p);
    TEST_EQUAL(p, p_clone);

    TEST(p_clone.get_description() != "PostingIterator()");

    vector<Xapian::docid> v(p, pend);

    p = db.postlist_begin("this");
    pend = db.postlist_end("this");
    vector<Xapian::docid>::const_iterator i;
    for (i = v.begin(); i != v.end(); ++i) {
	TEST_NOT_EQUAL(p, pend);
	TEST_EQUAL(*i, *p);
	p++;
    }
    TEST_EQUAL(p, pend);

    TEST_STRINGS_EQUAL(p.get_description(), "PostingIterator()");
    TEST_STRINGS_EQUAL(pend.get_description(), "PostingIterator()");

    return true;
}

// tests that a Xapian::PostingIterator still works when the DB is deleted
DEFINE_TESTCASE(postlist3, backend) {
    Xapian::PostingIterator u;
    {
	Xapian::Database db_temp(get_database("apitest_simpledata"));
	u = db_temp.postlist_begin("this");
    }

    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    while (p != pend) {
	TEST_EQUAL(*p, *u);
	p++;
	u++;
    }
    return true;
}

// tests skip_to
DEFINE_TESTCASE(postlist4, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    i.skip_to(1);
    i.skip_to(999999999);
    TEST(i == db.postlist_end("this"));
    return true;
}

// tests long postlists
DEFINE_TESTCASE(postlist5, backend) {
    Xapian::Database db(get_database("apitest_manydocs"));
    // Allow for databases which don't support length
    if (db.get_avlength() != 1)
	TEST_EQUAL_DOUBLE(db.get_avlength(), 4);
    Xapian::PostingIterator i = db.postlist_begin("this");
    unsigned int j = 1;
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
    }
    TEST_EQUAL(j, 513);
    return true;
}

// tests document length in postlists
DEFINE_TESTCASE(postlist6, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    TEST(i != db.postlist_end("this"));
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(i.get_doclength(), db.get_doclength(*i));
	TEST_EQUAL(i.get_unique_terms(), db.get_unique_terms(*i));
	TEST_REL(i.get_wdf(),<=,i.get_doclength());
	TEST_REL(1,<=,i.get_unique_terms());
	// The next two aren't necessarily true if there are terms with wdf=0
	// in the document, but that isn't the case here.
	TEST_REL(i.get_unique_terms(),<=,i.get_doclength());
	TEST_REL(i.get_wdf() + i.get_unique_terms() - 1,<=,i.get_doclength());
	++i;
    }
    return true;
}

// tests collection frequency
DEFINE_TESTCASE(collfreq1, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.get_collection_freq("this"), 11);
    TEST_EQUAL(db.get_collection_freq("first"), 1);
    TEST_EQUAL(db.get_collection_freq("last"), 0);
    TEST_EQUAL(db.get_collection_freq("word"), 9);

    Xapian::Database db1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Database db2(get_database("apitest_simpledata"));
    db2.add_database(get_database("apitest_simpledata2"));

    TEST_EQUAL(db1.get_collection_freq("this"), 15);
    TEST_EQUAL(db1.get_collection_freq("first"), 1);
    TEST_EQUAL(db1.get_collection_freq("last"), 0);
    TEST_EQUAL(db1.get_collection_freq("word"), 11);
    TEST_EQUAL(db2.get_collection_freq("this"), 15);
    TEST_EQUAL(db2.get_collection_freq("first"), 1);
    TEST_EQUAL(db2.get_collection_freq("last"), 0);
    TEST_EQUAL(db2.get_collection_freq("word"), 11);

    return true;
}

// Regression test for split msets being incorrect when sorting
DEFINE_TESTCASE(sortvalue1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    for (int pass = 1; pass <= 2; ++pass) {
	for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	    tout << "Sorting on value " << value_no << endl;
	    enquire.set_sort_by_value(value_no, true);
	    Xapian::MSet allbset = enquire.get_mset(0, 100);
	    Xapian::MSet partbset1 = enquire.get_mset(0, 3);
	    Xapian::MSet partbset2 = enquire.get_mset(3, 97);
	    TEST_EQUAL(allbset.size(), partbset1.size() + partbset2.size());

	    bool ok = true;
	    int n = 0;
	    Xapian::MSetIterator i, j;
	    j = allbset.begin();
	    for (i = partbset1.begin(); i != partbset1.end(); ++i) {
		tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		TEST(j != allbset.end());
		if (*i != *j) ok = false;
		++j;
		++n;
	    }
	    tout << "===\n";
	    for (i = partbset2.begin(); i != partbset2.end(); ++i) {
		tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		TEST(j != allbset.end());
		if (*i != *j) ok = false;
		++j;
		++n;
	    }
	    TEST(j == allbset.end());
	    if (!ok)
		FAIL_TEST("Split msets aren't consistent with unsplit");
	}
	enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    }

    return true;
}

// consistency check match - vary mset size and check results agree.
// consistency1 will run on the remote backend, but it's particularly slow
// with that, and testing it there doesn't actually improve the test
// coverage really.
DEFINE_TESTCASE(consistency1, backend && !remote) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, Xapian::Query("the"), Xapian::Query("sky")));
    Xapian::doccount lots = 214;
    Xapian::MSet bigmset = enquire.get_mset(0, lots);
    TEST_EQUAL(bigmset.size(), lots);
    try {
	for (Xapian::doccount start = 0; start < lots; ++start) {
	    for (Xapian::doccount size = 0; size < lots - start; ++size) {
		Xapian::MSet mset = enquire.get_mset(start, size);
		if (mset.size()) {
		    TEST_EQUAL(start + mset.size(),
			       min(start + size, bigmset.size()));
		} else if (size) {
//		tout << start << mset.size() << bigmset.size() << endl;
		    TEST(start >= bigmset.size());
		}
		for (Xapian::doccount i = 0; i < mset.size(); ++i) {
		    TEST_EQUAL(*mset[i], *bigmset[start + i]);
		    TEST_EQUAL_DOUBLE(mset[i].get_weight(),
				      bigmset[start + i].get_weight());
		}
	    }
	}
    } catch (const Xapian::NetworkTimeoutError &) {
	// consistency1 is a long test - may timeout with the remote backend...
	SKIP_TEST("Test taking too long");
    }
    return true;
}

// tests that specifying a nonexistent input file throws an exception.
DEFINE_TESTCASE(chertdatabaseopeningerror1, chert) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
    mkdir(".chert", 0755);

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	    Xapian::Database(".chert/nosuchdirectory",
		Xapian::DB_BACKEND_CHERT));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	    Xapian::WritableDatabase(".chert/nosuchdirectory",
		Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT));

    mkdir(".chert/emptydirectory", 0700);
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	    Xapian::Database(".chert/emptydirectory",
		Xapian::DB_BACKEND_CHERT));

    touch(".chert/somefile");
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	    Xapian::Database(".chert/somefile",
		Xapian::DB_BACKEND_CHERT));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	    Xapian::WritableDatabase(".chert/somefile",
		Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT));
    TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::WritableDatabase(".chert/somefile",
		Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT));
    TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::WritableDatabase(".chert/somefile",
		Xapian::DB_CREATE_OR_OPEN|Xapian::DB_BACKEND_CHERT));
    TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::WritableDatabase(".chert/somefile",
		Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_CHERT));
#endif

    return true;
}

/// Test opening of a chert database
DEFINE_TESTCASE(chertdatabaseopen1, chert) {
#ifdef XAPIAN_HAS_CHERT_BACKEND
    const string dbdir = ".chert/test_chertdatabaseopen1";
    mkdir(".chert", 0755);

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::WritableDatabase(dbdir,
		Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT));
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_CREATE_OR_OPEN|Xapian::DB_BACKEND_CHERT);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::WritableDatabase(dbdir,
		Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_CHERT));
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_CHERT);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::WritableDatabase(dbdir,
		Xapian::DB_CREATE_OR_OPEN|Xapian::DB_BACKEND_CHERT));
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }

    {
	TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::WritableDatabase(dbdir,
		Xapian::DB_CREATE|Xapian::DB_BACKEND_CHERT));
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_CREATE_OR_OVERWRITE|Xapian::DB_BACKEND_CHERT);
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }

    {
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_CREATE_OR_OPEN|Xapian::DB_BACKEND_CHERT);
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }

    {
	Xapian::WritableDatabase wdb(dbdir,
		Xapian::DB_OPEN|Xapian::DB_BACKEND_CHERT);
	Xapian::Database(dbdir, Xapian::DB_BACKEND_CHERT);
    }
#endif

    return true;
}

// feature test for Enquire:
// set_sort_by_value
// set_sort_by_value_then_relevance
// set_sort_by_relevance_then_value
// Prior to 1.2.17 and 1.3.2, order8 and order9 were swapped, and
// set_sort_by_relevance_then_value was buggy, so this testcase now serves as
// a regression test for that bug.
DEFINE_TESTCASE(sortrel1, backend) {
    Xapian::Enquire enquire(get_database("apitest_sortrel"));
    enquire.set_sort_by_value(1, true);
    enquire.set_query(Xapian::Query("woman"));

    const Xapian::docid order1[] = { 1,2,3,4,5,6,7,8,9 };
    const Xapian::docid order2[] = { 2,1,3,6,5,4,7,9,8 };
    const Xapian::docid order3[] = { 3,2,1,6,5,4,9,8,7 };
    const Xapian::docid order4[] = { 7,8,9,4,5,6,1,2,3 };
    const Xapian::docid order5[] = { 9,8,7,6,5,4,3,2,1 };
    const Xapian::docid order6[] = { 7,9,8,6,5,4,2,1,3 };
    const Xapian::docid order7[] = { 7,9,8,6,5,4,2,1,3 };
    const Xapian::docid order8[] = { 2,6,7,1,5,9,3,4,8 };
    const Xapian::docid order9[] = { 7,6,2,9,5,1,8,4,3 };

    Xapian::MSet mset;
    size_t i;

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order1) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order1) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order1[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, true);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order2) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order2) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order2[i]);
    }

    enquire.set_sort_by_value(1, true);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order1) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order1) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order1[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, true);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order2) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order2) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order2[i]);
    }

    enquire.set_sort_by_value(1, true);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order3) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order3) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order3[i]);
    }

    enquire.set_sort_by_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order4) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order4) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order4[i]);
    }

    enquire.set_sort_by_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order5) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order5) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order5[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order6) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order6) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order6[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order7) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order7) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order7[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, true);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order8) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order8) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order8[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, true);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order8) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order8) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order8[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order9) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order9) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order9[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order9) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order9) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order9[i]);
    }

    return true;
}

// Test network stats and local stats give the same results.
DEFINE_TESTCASE(netstats1, remote) {
    BackendManagerLocal local_manager;
    local_manager.set_datadir(test_driver::get_srcdir() + "/testdata/");

    const char * words[] = { "paragraph", "word" };
    Xapian::Query query(Xapian::Query::OP_OR, words, words + 2);
    const size_t MSET_SIZE = 10;

    Xapian::RSet rset;
    rset.add_document(4);
    rset.add_document(9);

    Xapian::MSet mset_alllocal;
    {
	Xapian::Database db;
	db.add_database(local_manager.get_database("apitest_simpledata"));
	db.add_database(local_manager.get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	mset_alllocal = enq.get_mset(0, MSET_SIZE, &rset);
    }

    {
	Xapian::Database db;
	db.add_database(local_manager.get_database("apitest_simpledata"));
	db.add_database(get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset.get_matches_lower_bound(), mset_alllocal.get_matches_lower_bound());
	TEST_EQUAL(mset.get_matches_upper_bound(), mset_alllocal.get_matches_upper_bound());
	TEST_EQUAL(mset.get_matches_estimated(), mset_alllocal.get_matches_estimated());
	TEST_EQUAL(mset.get_max_attained(), mset_alllocal.get_max_attained());
	TEST_EQUAL(mset.size(), mset_alllocal.size());
	TEST(mset_range_is_same(mset, 0, mset_alllocal, 0, mset.size()));
    }

    {
	Xapian::Database db;
	db.add_database(get_database("apitest_simpledata"));
	db.add_database(local_manager.get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset.get_matches_lower_bound(), mset_alllocal.get_matches_lower_bound());
	TEST_EQUAL(mset.get_matches_upper_bound(), mset_alllocal.get_matches_upper_bound());
	TEST_EQUAL(mset.get_matches_estimated(), mset_alllocal.get_matches_estimated());
	TEST_EQUAL(mset.get_max_attained(), mset_alllocal.get_max_attained());
	TEST_EQUAL(mset.size(), mset_alllocal.size());
	TEST(mset_range_is_same(mset, 0, mset_alllocal, 0, mset.size()));
    }

    {
	Xapian::Database db;
	db.add_database(get_database("apitest_simpledata"));
	db.add_database(get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset.get_matches_lower_bound(), mset_alllocal.get_matches_lower_bound());
	TEST_EQUAL(mset.get_matches_upper_bound(), mset_alllocal.get_matches_upper_bound());
	TEST_EQUAL(mset.get_matches_estimated(), mset_alllocal.get_matches_estimated());
	TEST_EQUAL(mset.get_max_attained(), mset_alllocal.get_max_attained());
	TEST_EQUAL(mset.size(), mset_alllocal.size());
	TEST(mset_range_is_same(mset, 0, mset_alllocal, 0, mset.size()));
    }

    return true;
}

// Coordinate matching - scores 1 for each matching term
class MyWeight : public Xapian::Weight {
    double scale_factor;

  public:
    MyWeight * clone() const {
	return new MyWeight;
    }
    void init(double factor) {
	scale_factor = factor;
    }
    MyWeight() { }
    ~MyWeight() { }
    std::string name() const { return "MyWeight"; }
    string serialise() const { return string(); }
    MyWeight * unserialise(const string &) const { return new MyWeight; }
    double get_sumpart(Xapian::termcount, Xapian::termcount, Xapian::termcount) const {
	return scale_factor;
    }
    double get_maxpart() const { return scale_factor; }

    double get_sumextra(Xapian::termcount, Xapian::termcount) const { return 0; }
    double get_maxextra() const { return 0; }
};

// tests user weighting scheme.
// Would work with remote if we registered the weighting scheme.
// FIXME: do this so we also test that functionality...
DEFINE_TESTCASE(userweight1, backend && !remote) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(MyWeight());
    const char * query[] = { "this", "line", "paragraph", "rubbish" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, query,
				    query + sizeof(query) / sizeof(query[0])));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    // MyWeight scores 1 for each matching term, so the weight should equal
    // the number of matching terms.
    for (Xapian::MSetIterator i = mymset1.begin(); i != mymset1.end(); ++i) {
	Xapian::termcount matching_terms = 0;
	Xapian::TermIterator t = enquire.get_matching_terms_begin(i);
	while (t != enquire.get_matching_terms_end(i)) {
	    ++matching_terms;
	    ++t;
	}
	TEST_EQUAL(i.get_weight(), matching_terms);
    }

    return true;
}

// tests MatchAll queries
// This is a regression test, which failed with assertion failures in
// revision 9094.  Also check that the results aren't ranked by relevance
// (regression test for bug fixed in 1.0.9).
DEFINE_TESTCASE(matchall1, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query::MatchAll);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.get_matches_lower_bound(), db.get_doccount());
    TEST_EQUAL(mset.get_uncollapsed_matches_lower_bound(), db.get_doccount());

    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
				    Xapian::Query("nosuchterm"),
				    Xapian::Query::MatchAll));
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.get_matches_lower_bound(), db.get_doccount());
    TEST_EQUAL(mset.get_uncollapsed_matches_lower_bound(), db.get_doccount());

    // Check that the results aren't ranked by relevance (fixed in 1.0.9).
    TEST(mset.size() > 1);
    TEST_EQUAL(mset[mset.size() - 1].get_weight(), 0);
    TEST_EQUAL(*mset[0], 1);
    TEST_EQUAL(*mset[mset.size() - 1], mset.size());

    return true;
}

// Test using a ValueSetMatchDecider
DEFINE_TESTCASE(valuesetmatchdecider2, backend && !remote) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("leav"));

    Xapian::ValueSetMatchDecider vsmd1(1, true);
    vsmd1.add_value("n");
    Xapian::ValueSetMatchDecider vsmd2(1, false);
    vsmd2.add_value("n");

    Xapian::MSet mymset = enq.get_mset(0, 20);
    mset_expect_order(mymset, 8, 6, 4, 5, 7, 10, 12, 11, 13, 9, 14);
    mymset = enq.get_mset(0, 20, 0, NULL, &vsmd1);
    mset_expect_order(mymset, 6, 12);
    mymset = enq.get_mset(0, 20, 0, NULL, &vsmd2);
    mset_expect_order(mymset, 8, 4, 5, 7, 10, 11, 13, 9, 14);

    return true;
}
