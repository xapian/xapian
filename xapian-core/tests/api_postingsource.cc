/** @file
 * @brief tests of posting sources
 */
/* Copyright 2008,2009,2011,2015,2016,2019 Olly Betts
 * Copyright 2008,2009 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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

#include "api_postingsource.h"

#include <xapian.h>

#include <string>
#include "safeunistd.h"

#include "str.h"
#include "testutils.h"
#include "apitest.h"

using namespace std;

class MyOddPostingSource : public Xapian::PostingSource {
    Xapian::doccount num_docs;

    Xapian::doccount last_docid;

    Xapian::docid did;

    MyOddPostingSource(Xapian::doccount num_docs_,
		       Xapian::doccount last_docid_)
	: num_docs(num_docs_), last_docid(last_docid_), did(0)
    { }

  public:
    MyOddPostingSource(const Xapian::Database &db)
	: num_docs(db.get_doccount()), last_docid(db.get_lastdocid()), did(0)
    { }

    PostingSource * clone() const { return new MyOddPostingSource(num_docs, last_docid); }

    void init(const Xapian::Database &) { did = 0; }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const { return 0; }

    Xapian::doccount get_termfreq_est() const { return num_docs / 2; }

    Xapian::doccount get_termfreq_max() const { return num_docs; }

    void next(double wt) {
	(void)wt;
	++did;
	if (did % 2 == 0) ++did;
    }

    void skip_to(Xapian::docid to_did, double wt) {
	(void)wt;
	did = to_did;
	if (did % 2 == 0) ++did;
    }

    bool at_end() const {
	// Doesn't work if last_docid is 2^32 - 1.
	return did > last_docid;
    }

    Xapian::docid get_docid() const { return did; }

    string get_description() const { return "MyOddPostingSource"; }
};

DEFINE_TESTCASE(externalsource1, backend && !remote && !multi) {
    // Doesn't work for remote without registering with the server.
    // Doesn't work for multi because it checks the docid in the
    // subdatabase.
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    MyOddPostingSource src(db);

    // Check that passing NULL is rejected as intended.
    Xapian::PostingSource * nullsrc = NULL;
    TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::Query bad(nullsrc));

    enq.set_query(Xapian::Query(&src));

    Xapian::MSet mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 1, 3, 5, 7, 9, 11, 13, 15, 17);

    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);

    mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 5, 7, 11, 13, 9);
}

// Test that trying to use PostingSource with the remote backend throws
// Xapian::UnimplementedError as expected (we need to register the class
// in xapian-tcpsrv/xapian-progsrv for this to work).
DEFINE_TESTCASE(externalsource2, remote) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    MyOddPostingSource src(db);

    enq.set_query(Xapian::Query(&src));

    TEST_EXCEPTION(Xapian::UnimplementedError,
		   Xapian::MSet mset = enq.get_mset(0, 10));

    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);

    TEST_EXCEPTION(Xapian::UnimplementedError,
		   Xapian::MSet mset = enq.get_mset(0, 10));
}

class MyOddWeightingPostingSource : public Xapian::PostingSource {
    Xapian::doccount num_docs;

    Xapian::doccount last_docid;

    Xapian::docid did;

    MyOddWeightingPostingSource(Xapian::doccount num_docs_,
				Xapian::doccount last_docid_)
	: num_docs(num_docs_), last_docid(last_docid_), did(0)
    {
	set_maxweight(1000);
    }

  public:
    MyOddWeightingPostingSource(const Xapian::Database &db)
	: num_docs(db.get_doccount()), last_docid(db.get_lastdocid()), did(0)
    { }

    PostingSource * clone() const {
	return new MyOddWeightingPostingSource(num_docs, last_docid);
    }

    void init(const Xapian::Database &) { did = 0; }

    double get_weight() const {
	return (did % 2) ? 1000 : 0.001;
    }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const { return 0; }

    Xapian::doccount get_termfreq_est() const { return num_docs / 2; }

    Xapian::doccount get_termfreq_max() const { return num_docs; }

    void next(double wt) {
	(void)wt;
	++did;
    }

    void skip_to(Xapian::docid to_did, double wt) {
	(void)wt;
	did = to_did;
    }

    bool at_end() const {
	// Doesn't work if last_docid is 2^32 - 1.
	return did > last_docid;
    }

    Xapian::docid get_docid() const { return did; }

    string get_description() const {
	return "MyOddWeightingPostingSource";
    }
};

// Like externalsource1, except we use the weight to favour odd documents.
DEFINE_TESTCASE(externalsource3, backend && !remote && !multi) {
    // Doesn't work for remote without registering with the server.
    // Doesn't work for multi because it checks the docid in the
    // subdatabase.
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    MyOddWeightingPostingSource src(db);

    enq.set_query(Xapian::Query(&src));

    Xapian::MSet mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 1, 3, 5, 7, 9, 11, 13, 15, 17, 2);

    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);

    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 5, 7, 11, 13, 9);

    tout << "max possible weight = " << mset.get_max_possible() << '\n';
    TEST(mset.get_max_possible() > 1000);

    enq.set_cutoff(0, 1000.001);
    mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 5, 7, 11, 13, 9);

    tout << "max possible weight = " << mset.get_max_possible() << '\n';
    TEST(mset.get_max_possible() > 1000);

    enq.set_query(Xapian::Query(q.OP_SCALE_WEIGHT, Xapian::Query(&src), 0.5));
    mset = enq.get_mset(0, 10);
    TEST(mset.empty());

    TEST_EQUAL(mset.get_max_possible(), 500);

    enq.set_query(Xapian::Query(q.OP_SCALE_WEIGHT, Xapian::Query(&src), 2));
    mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 1, 3, 5, 7, 9, 11, 13, 15, 17);

    TEST_EQUAL(mset.get_max_possible(), 2000);
}

class MyDontAskWeightPostingSource : public Xapian::PostingSource {
    Xapian::doccount num_docs;

    Xapian::doccount last_docid;

    Xapian::docid did;

    MyDontAskWeightPostingSource(Xapian::doccount num_docs_,
				 Xapian::doccount last_docid_)
	: num_docs(num_docs_), last_docid(last_docid_), did(0)
    { }

  public:
    MyDontAskWeightPostingSource() : Xapian::PostingSource() {}

    PostingSource * clone() const { return new MyDontAskWeightPostingSource(num_docs, last_docid); }

    void init(const Xapian::Database &db) {
	num_docs = db.get_doccount();
	last_docid = db.get_lastdocid();
	did = 0;
    }

    double get_weight() const {
	FAIL_TEST("MyDontAskWeightPostingSource::get_weight() called");
    }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const { return num_docs; }

    Xapian::doccount get_termfreq_est() const { return num_docs; }

    Xapian::doccount get_termfreq_max() const { return num_docs; }

    void next(double wt) {
	(void)wt;
	++did;
    }

    void skip_to(Xapian::docid to_did, double wt) {
	(void)wt;
	did = to_did;
    }

    bool at_end() const {
	// Doesn't work if last_docid is 2^32 - 1.
	return did > last_docid;
    }

    Xapian::docid get_docid() const { return did; }

    string get_description() const {
	return "MyDontAskWeightPostingSource";
    }
};

// Check that boolean use doesn't call get_weight().
DEFINE_TESTCASE(externalsource4, backend && !remote) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    MyDontAskWeightPostingSource src;

    tout << "OP_SCALE_WEIGHT 0\n";
    enq.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, Xapian::Query(&src), 0));

    Xapian::MSet mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 1, 2, 3, 4, 5);

    tout << "OP_FILTER\n";
    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);

    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 6, 4, 5, 7);

    tout << "BoolWeight\n";
    enq.set_query(Xapian::Query(&src));
    enq.set_weighting_scheme(Xapian::BoolWeight());

    // mset = enq.get_mset(0, 5);
    // mset_expect_order(mset, 1, 2, 3, 4, 5);
}

// Check that valueweightsource works correctly.
DEFINE_TESTCASE(valueweightsource1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::ValueWeightPostingSource src(11);

    // Should be in descending order of length
    tout << "RAW\n";
    enq.set_query(Xapian::Query(&src));
    Xapian::MSet mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 3, 1, 2, 8, 14);

    // In relevance order
    tout << "OP_FILTER\n";
    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);
    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 6, 4, 5, 7);

    // Should be in descending order of length
    tout << "OP_FILTER other way\n";
    q = Xapian::Query(Xapian::Query::OP_FILTER,
		      Xapian::Query(&src),
		      Xapian::Query("leav"));
    enq.set_query(q);
    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 14, 9, 13, 7);
}

// Check that valueweightsource gives the correct bounds for those databases
// which support value statistics.
DEFINE_TESTCASE(valueweightsource2, valuestats) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::ValueWeightPostingSource src(11);
    src.init(db);
    TEST_EQUAL(src.get_termfreq_min(), 17);
    TEST_EQUAL(src.get_termfreq_est(), 17);
    TEST_EQUAL(src.get_termfreq_max(), 17);
    TEST_EQUAL(src.get_maxweight(), 135);
}

// Check that valueweightsource skip_to() can stay in the same position.
DEFINE_TESTCASE(valueweightsource3, valuestats) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::ValueWeightPostingSource src(11);
    src.init(db);
    TEST(!src.at_end());
    src.skip_to(8, 0.0);
    TEST(!src.at_end());
    TEST_EQUAL(src.get_docid(), 8);
    src.skip_to(8, 0.0);
    TEST(!src.at_end());
    TEST_EQUAL(src.get_docid(), 8);
}

// Check that fixedweightsource works correctly.
DEFINE_TESTCASE(fixedweightsource1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    double wt = 5.6;

    {
	Xapian::FixedWeightPostingSource src(wt);

	// Should be in increasing order of docid.
	enq.set_query(Xapian::Query(&src));
	Xapian::MSet mset = enq.get_mset(0, 5);
	mset_expect_order(mset, 1, 2, 3, 4, 5);

	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	    TEST_EQUAL(i.get_weight(), wt);
	}
    }

    // Do some direct tests, to check the skip_to() and check() methods work.
    {
	// Check next and skip_to().
	Xapian::FixedWeightPostingSource src(wt);
	src.init(db);

	src.next(1.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 1);
	src.next(1.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 2);
	src.skip_to(5, 1.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);
	src.next(wt * 2);
	TEST(src.at_end());
    }
    {
	// Check check() as the first operation, followed by next.
	Xapian::FixedWeightPostingSource src(wt);
	src.init(db);

	TEST_EQUAL(src.check(5, 1.0), true);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);
	src.next(1.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 6);
    }
    {
	// Check check() as the first operation, followed by skip_to().
	Xapian::FixedWeightPostingSource src(wt);
	src.init(db);

	TEST_EQUAL(src.check(5, 1.0), true);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 5);
	src.skip_to(6, 1.0);
	TEST(!src.at_end());
	TEST_EQUAL(src.get_docid(), 6);
	src.skip_to(7, wt * 2);
	TEST(src.at_end());
    }
}

// A posting source which changes the maximum weight.
class ChangeMaxweightPostingSource : public Xapian::PostingSource {
    Xapian::docid did;

    // Maximum docid that get_weight() should be called for.
    Xapian::docid maxid_accessed;

  public:
    ChangeMaxweightPostingSource(Xapian::docid maxid_accessed_)
	    : did(0), maxid_accessed(maxid_accessed_) { }

    void init(const Xapian::Database &) { did = 0; }

    double get_weight() const {
	if (did > maxid_accessed) {
	    FAIL_TEST("ChangeMaxweightPostingSource::get_weight() called "
		      "for docid " + str(did) + ", max id accessed "
		      "should be " + str(maxid_accessed));
	}
	return 5 - did;
    }

    Xapian::doccount get_termfreq_min() const { return 4; }
    Xapian::doccount get_termfreq_est() const { return 4; }
    Xapian::doccount get_termfreq_max() const { return 4; }

    void next(double) {
	++did;
	set_maxweight(5 - did);
    }

    void skip_to(Xapian::docid to_did, double) {
	did = to_did;
	set_maxweight(5 - did);
    }

    bool at_end() const { return did >= 5; }
    Xapian::docid get_docid() const { return did; }
    string get_description() const { return "ChangeMaxweightPostingSource"; }
};

// Test a posting source with a variable maxweight.
DEFINE_TESTCASE(changemaxweightsource1, backend && !remote && !multi) {
    // The ChangeMaxweightPostingSource doesn't work with multi or remote.
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);

    {
	ChangeMaxweightPostingSource src1(5);
	Xapian::FixedWeightPostingSource src2(2.5);

	Xapian::Query q(Xapian::Query::OP_AND,
			Xapian::Query(&src1), Xapian::Query(&src2));
	enq.set_query(q);
	// Set descending docid order so that the matcher isn't able to
	// terminate early after 4 documents just because weight == maxweight.
	enq.set_docid_order(enq.DESCENDING);

	Xapian::MSet mset = enq.get_mset(0, 4);
	TEST(src1.at_end());
	mset_expect_order(mset, 1, 2, 3, 4);
	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	    TEST_EQUAL_DOUBLE(i.get_weight(), 7.5 - *i);
	}
    }

    {
	ChangeMaxweightPostingSource src1(3);
	Xapian::FixedWeightPostingSource src2(2.5);

	Xapian::Query q(Xapian::Query::OP_AND,
			Xapian::Query(&src1), Xapian::Query(&src2));
	enq.set_query(q);

	Xapian::MSet mset = enq.get_mset(0, 2);
	TEST(!src1.at_end());
	TEST_EQUAL(src1.get_docid(), 3);
	TEST_EQUAL_DOUBLE(src1.get_maxweight(), 2.0);
	mset_expect_order(mset, 1, 2);
	for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	    TEST_EQUAL_DOUBLE(i.get_weight(), 7.5 - *i);
	}
    }
}

// Test using a valueweightpostingsource which has no entries.
DEFINE_TESTCASE(emptyvalwtsource1, backend && !remote && !multi) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);

    Xapian::ValueWeightPostingSource src2(11); // A non-empty slot.
    Xapian::ValueWeightPostingSource src3(100); // An empty slot.
    Xapian::Query q1("leav");
    Xapian::Query q2(&src2);
    Xapian::Query q3(&src3);
    Xapian::Query q(Xapian::Query::OP_OR, Xapian::Query(Xapian::Query::OP_AND_MAYBE, q1, q2), q3);

    // Perform search without ORring with the posting source.
    Xapian::doccount size1;
    {
	enq.set_query(q1);
	Xapian::MSet mset = enq.get_mset(0, 10);
	TEST_REL(mset.get_max_possible(), >, 0.0);
	size1 = mset.size();
	TEST_REL(size1, >, 0);
    }

    // Perform a search with just the non-empty posting source, checking it
    // returns something.
    {
	enq.set_query(q2);
	Xapian::MSet mset = enq.get_mset(0, 10);
	TEST_REL(mset.get_max_possible(), >, 0.0);
	TEST_REL(mset.size(), >, 0);
    }

    // Perform a search with just the empty posting source, checking it returns
    // nothing.
    {
	enq.set_query(q3);
	Xapian::MSet mset = enq.get_mset(0, 10);

	// get_max_possible() returns 0 here for backends which track the upper
	// bound on value slot entries, MAX_DBL for backends which don't.
	// Either is valid.
	TEST_REL(mset.get_max_possible(), >=, 0.0);

	TEST_EQUAL(mset.size(), 0);
    }

    // Perform a search with the posting source ORred with the normal query.
    // This is a regression test - it used to return nothing.
    {
	enq.set_query(q);
	Xapian::MSet mset = enq.get_mset(0, 10);
	TEST_REL(mset.get_max_possible(), >, 0.0);
	TEST_REL(mset.size(), >, 0.0);
	TEST_EQUAL(mset.size(), size1);
    }
}

class SlowDecreasingValueWeightPostingSource
    : public Xapian::DecreasingValueWeightPostingSource {
  public:
    int & count;

    SlowDecreasingValueWeightPostingSource(int & count_)
	: Xapian::DecreasingValueWeightPostingSource(0), count(count_) { }

    SlowDecreasingValueWeightPostingSource * clone() const
    {
	return new SlowDecreasingValueWeightPostingSource(count);
    }

    void next(double min_wt) {
	sleep(1);
	++count;
	return Xapian::DecreasingValueWeightPostingSource::next(min_wt);
    }
};

static void
make_matchtimelimit1_db(Xapian::WritableDatabase &db, const string &)
{
    for (int wt = 20; wt > 0; --wt) {
	Xapian::Document doc;
	doc.add_value(0, Xapian::sortable_serialise(double(wt)));
	db.add_document(doc);
    }
}

// FIXME: This doesn't run for remote databases (we'd need to register
// SlowDecreasingValueWeightPostingSource on the remote).
DEFINE_TESTCASE(matchtimelimit1, backend && !remote)
{
#ifndef HAVE_TIMER_CREATE
    SKIP_TEST("Enquire::set_time_limit() not implemented for this platform");
#endif
    Xapian::Database db = get_database("matchtimelimit1",
				       make_matchtimelimit1_db);

    int count = 0;
    SlowDecreasingValueWeightPostingSource src(count);
    src.init(db);
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(&src));

    enquire.set_time_limit(1.5);

    Xapian::MSet mset = enquire.get_mset(0, 1, 1000);
    TEST_EQUAL(mset.size(), 1);
    TEST_EQUAL(count, 2);
}

class CheckBoundsPostingSource
    : public Xapian::DecreasingValueWeightPostingSource {
  public:
    Xapian::doccount& doclen_lb;

    Xapian::doccount& doclen_ub;

    CheckBoundsPostingSource(Xapian::doccount& doclen_lb_,
			     Xapian::doccount& doclen_ub_)
	: Xapian::DecreasingValueWeightPostingSource(0),
	  doclen_lb(doclen_lb_),
	  doclen_ub(doclen_ub_) { }

    CheckBoundsPostingSource * clone() const
    {
	return new CheckBoundsPostingSource(doclen_lb, doclen_ub);
    }

    void init(const Xapian::Database& database) {
	doclen_lb = database.get_doclength_lower_bound();
	doclen_ub = database.get_doclength_upper_bound();
	Xapian::DecreasingValueWeightPostingSource::init(database);
    }
};

// Test that doclength bounds are correct.
// Regression test for bug fixed in 1.2.25 and 1.4.1.
DEFINE_TESTCASE(postingsourcebounds1, backend && !remote)
{
    Xapian::Database db = get_database("apitest_simpledata");

    Xapian::doccount doclen_lb = 0, doclen_ub = 0;
    CheckBoundsPostingSource ps(doclen_lb, doclen_ub);

    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(&ps));

    Xapian::MSet mset = enquire.get_mset(0, 1);

    TEST_EQUAL(doclen_lb, db.get_doclength_lower_bound());
    TEST_EQUAL(doclen_ub, db.get_doclength_upper_bound());
}

// PostingSource which really just counts the clone() calls.
// Never actually matches anything, but pretends it might.
class CloneTestPostingSource : public Xapian::PostingSource {
    int& clone_count;

  public:
    CloneTestPostingSource(int& clone_count_)
	: clone_count(clone_count_)
    { }

    PostingSource * clone() const {
	++clone_count;
	return new CloneTestPostingSource(clone_count);
    }

    void init(const Xapian::Database&) { }

    Xapian::doccount get_termfreq_min() const { return 0; }

    Xapian::doccount get_termfreq_est() const { return 1; }

    Xapian::doccount get_termfreq_max() const { return 2; }

    void next(double) { }

    void skip_to(Xapian::docid, double) { }

    bool at_end() const {
	return true;
    }

    Xapian::docid get_docid() const { return 0; }

    string get_description() const { return "CloneTestPostingSource"; }
};

/// Test cloning of initial object, which regressed in 1.3.5.
DEFINE_TESTCASE(postingsourceclone1, !backend)
{
    // This fails with 1.3.5-1.4.0 inclusive.
    {
	int clones = 0;
	CloneTestPostingSource ps(clones);
	TEST_EQUAL(clones, 0);
	Xapian::Query q(&ps);
	TEST_EQUAL(clones, 1);
    }

    // Check that clone() isn't needlessly called if reference counting has
    // been turned on for the PostingSource.
    {
	int clones = 0;
	CloneTestPostingSource* ps = new CloneTestPostingSource(clones);
	TEST_EQUAL(clones, 0);
	Xapian::Query q(ps->release());
	TEST_EQUAL(clones, 0);
    }
}

class OnlyTheFirstPostingSource : public Xapian::PostingSource {
    Xapian::doccount last_docid;

    Xapian::docid did;

    bool allow_clone;

  public:
    static Xapian::doccount shard_index;

    explicit
    OnlyTheFirstPostingSource(bool allow_clone_) : allow_clone(allow_clone_) {}

    PostingSource* clone() const {
	return allow_clone ? new OnlyTheFirstPostingSource(true) : nullptr;
    }

    void init(const Xapian::Database& db) {
	did = 0;
	if (shard_index == 0) {
	    last_docid = db.get_lastdocid();
	} else {
	    last_docid = 0;
	}
	++shard_index;
    }

    Xapian::doccount get_termfreq_min() const { return 0; }

    Xapian::doccount get_termfreq_est() const { return last_docid / 2; }

    Xapian::doccount get_termfreq_max() const { return last_docid; }

    void next(double wt) {
	(void)wt;
	++did;
	if (did > last_docid) did = 0;
    }

    void skip_to(Xapian::docid to_did, double wt) {
	(void)wt;
	did = to_did;
	if (did > last_docid) did = 0;
    }

    bool at_end() const {
	return did == 0;
    }

    Xapian::docid get_docid() const { return did; }

    string get_description() const { return "OnlyTheFirstPostingSource"; }
};

Xapian::doccount OnlyTheFirstPostingSource::shard_index;

DEFINE_TESTCASE(postingsourceshardindex1, multi && !remote) {
    Xapian::Database db = get_database("apitest_simpledata");

    OnlyTheFirstPostingSource::shard_index = 0;

    Xapian::Enquire enquire(db);
    {
	auto ps = new OnlyTheFirstPostingSource(true);
	enquire.set_query(Xapian::Query(ps->release()));

	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 1, 3, 5);
    }

    {
	/* Regression test for bug fixed in 1.4.12 - we should get an exception
	 * if we use a PostingSource that doesn't support clone() with a multi
	 * DB.
	 */
	auto ps = new OnlyTheFirstPostingSource(false);
	enquire.set_query(Xapian::Query(ps->release()));

	TEST_EXCEPTION(Xapian::InvalidOperationError,
		       auto m = enquire.get_mset(0, 10));
    }
}

/// PostingSource subclass for injecting tf bounds and estimate.
class EstimatePS : public Xapian::PostingSource {
    Xapian::doccount lb, est, ub;

  public:
    EstimatePS(Xapian::doccount lb_,
	       Xapian::doccount est_,
	       Xapian::doccount ub_)
	: lb(lb_), est(est_), ub(ub_)
    { }

    PostingSource * clone() const { return new EstimatePS(lb, est, ub); }

    void init(const Xapian::Database &) { }

    Xapian::doccount get_termfreq_min() const { return lb; }

    Xapian::doccount get_termfreq_est() const { return est; }

    Xapian::doccount get_termfreq_max() const { return ub; }

    void next(double) {
	FAIL_TEST("EstimatePS::next() shouldn't be called");
    }

    void skip_to(Xapian::docid, double) {
	FAIL_TEST("EstimatePS::skip_to() shouldn't be called");
    }

    bool at_end() const {
	return false;
    }

    Xapian::docid get_docid() const {
	FAIL_TEST("EstimatePS::get_docid() shouldn't be called");
    }

    string get_description() const { return "EstimatePS"; }
};

/// Check estimate is rounded to suitable number of S.F. - new in 1.4.3.
DEFINE_TESTCASE(estimaterounding1, backend && !multi && !remote) {
    Xapian::Database db = get_database("etext");
    Xapian::Enquire enquire(db);
    static const struct { Xapian::doccount lb, est, ub, exp; } testcases[] = {
	// Test rounding down.
	{411, 424, 439, 420},
	{1, 312, 439, 300},
	// Test rounding up.
	{411, 426, 439, 430},
	{123, 351, 439, 400},
	// Rounding based on estimate size if smaller than range size.
	{1, 12, 439, 10},
	// Round "5" away from the nearer bound.
	{1, 15, 439, 20},
	{1, 350, 439, 300},
	// Check we round up if rounding down would be out of range.
	{411, 416, 439, 420},
	{411, 412, 439, 420},
	// Check we round down if rounding up would be out of range.
	{111, 133, 138, 130},
	{111, 137, 138, 130},
	// Check we don't round if either way would be out of range.
	{411, 415, 419, 415},
	// Leave small estimates alone.
	{1, 6, 439, 6},
    };
    for (auto& t : testcases) {
	EstimatePS ps(t.lb, t.est, t.ub);
	enquire.set_query(Xapian::Query(&ps));
	Xapian::MSet mset = enquire.get_mset(0, 0);
	// MSet::get_description() includes bounds and raw estimate.
	tout << mset.get_description() << '\n';
	TEST_EQUAL(mset.get_matches_estimated(), t.exp);
    }
}
