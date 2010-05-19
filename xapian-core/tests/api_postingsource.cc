/* api_postingsource.cc: tests of posting sources
 *
 * Copyright 2008,2009 Olly Betts
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

#include "testutils.h"
#include "str.h"
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

    void next(Xapian::weight wt) {
	(void)wt;
	++did;
	if (did % 2 == 0) ++did;
    }

    void skip_to(Xapian::docid to_did, Xapian::weight wt) {
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
    TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::Query bad(NULL));
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

    return true;
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

    return true;
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

    Xapian::weight get_weight() const {
	return (did % 2) ? 1000 : 0.001;
    }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const { return 0; }

    Xapian::doccount get_termfreq_est() const { return num_docs / 2; }

    Xapian::doccount get_termfreq_max() const { return num_docs; }

    void next(Xapian::weight wt) {
	(void)wt;
	++did;
    }

    void skip_to(Xapian::docid to_did, Xapian::weight wt) {
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

    tout << "max possible weight = " << mset.get_max_possible() << endl;
    TEST(mset.get_max_possible() > 1000);

    enq.set_cutoff(0, 1000.001);
    mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 5, 7, 11, 13, 9);

    tout << "max possible weight = " << mset.get_max_possible() << endl;
    TEST(mset.get_max_possible() > 1000);

    enq.set_query(Xapian::Query(q.OP_SCALE_WEIGHT, Xapian::Query(&src), 0.5));
    mset = enq.get_mset(0, 10);
    TEST(mset.empty());

    TEST_EQUAL(mset.get_max_possible(), 500);

    enq.set_query(Xapian::Query(q.OP_SCALE_WEIGHT, Xapian::Query(&src), 2));
    mset = enq.get_mset(0, 10);
    mset_expect_order(mset, 1, 3, 5, 7, 9, 11, 13, 15, 17);

    TEST_EQUAL(mset.get_max_possible(), 2000);

    return true;
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

    Xapian::weight get_weight() const {
	FAIL_TEST("MyDontAskWeightPostingSource::get_weight() called");
    }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const { return num_docs; }

    Xapian::doccount get_termfreq_est() const { return num_docs; }

    Xapian::doccount get_termfreq_max() const { return num_docs; }

    void next(Xapian::weight wt) {
	(void)wt;
	++did;
    }

    void skip_to(Xapian::docid to_did, Xapian::weight wt) {
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

    tout << "OP_SCALE_WEIGHT 0" << endl;
    enq.set_query(Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, Xapian::Query(&src), 0));

    Xapian::MSet mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 1, 2, 3, 4, 5);

    tout << "OP_FILTER" << endl;
    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);

    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 6, 4, 5, 7);

    tout << "BoolWeight" << endl;
    enq.set_query(Xapian::Query(&src));
    enq.set_weighting_scheme(Xapian::BoolWeight());

    //mset = enq.get_mset(0, 5);
    //mset_expect_order(mset, 1, 2, 3, 4, 5);

    return true;
}

// Check that valueweightsource works correctly.
DEFINE_TESTCASE(valueweightsource1, backend) {
    Xapian::Database db(get_database("apitest_phrase"));
    Xapian::Enquire enq(db);
    Xapian::ValueWeightPostingSource src(11);

    // Should be in descending order of length
    tout << "RAW" << endl;
    enq.set_query(Xapian::Query(&src));
    Xapian::MSet mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 3, 1, 2, 8, 14);

    // In relevance order
    tout << "OP_FILTER" << endl;
    Xapian::Query q(Xapian::Query::OP_FILTER,
		    Xapian::Query("leav"),
		    Xapian::Query(&src));
    enq.set_query(q);
    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 6, 4, 5, 7);

    // Should be in descending order of length
    tout << "OP_FILTER other way" << endl;
    q = Xapian::Query(Xapian::Query::OP_FILTER,
		      Xapian::Query(&src),
		      Xapian::Query("leav"));
    enq.set_query(q);
    mset = enq.get_mset(0, 5);
    mset_expect_order(mset, 8, 14, 9, 13, 7);

    return true;
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

    return true;
}

// Check that valueweightsource skip_to() can stay in the same position.
DEFINE_TESTCASE(valueweightsource3, valuestats && !multi) {
    // FIXME: multi doesn't support iterating valuestreams yet.
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

    return true;
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

    return true;
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

    Xapian::weight get_weight() const {
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

    void next(Xapian::weight) {
	++did;
	set_maxweight(5 - did);
    }

    void skip_to(Xapian::docid to_did, Xapian::weight) {
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

    return true;
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

    return true;
}
