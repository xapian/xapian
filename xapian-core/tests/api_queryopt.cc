/** @file
 * @brief Tests of the query optimiser.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
 * Copyright (C) 2019 Olly Betts
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

#include "api_queryopt.h"

#include <xapian.h>

#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"
#include <vector>

using namespace std;

class MyDontUsePostingSource : public Xapian::PostingSource {
  public:
    MyDontUsePostingSource() : Xapian::PostingSource() {}
    PostingSource* clone() const { return new MyDontUsePostingSource(); }
    void init(const Xapian::Database&) { }

    double get_weight() const {
	FAIL_TEST("MyDontUsePostingSource::get_weight() called");
    }

    // These bounds could be better, but that's not important here.
    Xapian::doccount get_termfreq_min() const {
	FAIL_TEST("MyDontUsePostingSource::get_termfreq_min() called");
    }
    Xapian::doccount get_termfreq_est() const {
	FAIL_TEST("MyDontUsePostingSource::get_termfreq_est() called");
    }
    Xapian::doccount get_termfreq_max() const {
	FAIL_TEST("MyDontUsePostingSource::get_termfreq_max() called");
    }

    void next(double) {
	FAIL_TEST("MyDontUsePostingSource::next() called");
    }

    void skip_to(Xapian::docid, double) {
	FAIL_TEST("MyDontUsePostingSource::skip_to() called");
    }

    bool at_end() const {
	FAIL_TEST("MyDontUsePostingSource::at_end() called");
    }

    Xapian::docid get_docid() const {
	FAIL_TEST("MyDontUsePostingSource::get_docid() called");
    }

    string get_description() const {
	return "MyDontUsePostingSource";
    }
};

/// Test that ANDMAYBE branches are ignored if their weight factor is 0.
DEFINE_TESTCASE(boolandmaybe1, backend && !remote) {
    Xapian::Enquire enquire(get_database("etext"));

    MyDontUsePostingSource src;
    Xapian::Query query(Xapian::Query::OP_AND_MAYBE,
			Xapian::Query::MatchAll,
			Xapian::Query(&src));

    enquire.set_query(0.0 * query);
    enquire.get_mset(0, 10);

    enquire.set_weighting_scheme(Xapian::BoolWeight());
    enquire.set_query(query);
    enquire.get_mset(0, 10);
}

/** Test that ANDMAYBE branches return same documents and lists of matching
 *  terms when their weight factor is 0.
 */
DEFINE_TESTCASE(boolandmaybe2, backend) {
    Xapian::Query query(Xapian::Query::OP_AND_MAYBE,
			Xapian::Query("last"), Xapian::Query("day"));
    Xapian::MSet mset1, mset2, mset3;
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire1(db), enquire2(db), enquire3(db);

    enquire1.set_query(0 * query);
    mset1 = enquire1.get_mset(0, 1000);

    enquire2.set_query(query);
    enquire2.set_weighting_scheme(Xapian::BoolWeight());
    mset2 = enquire2.get_mset(0, 1000);

    enquire3.set_query(query);
    mset3 = enquire3.get_mset(0, 1000);

    TEST_EQUAL(mset1.size(), mset2.size());
    TEST_EQUAL(mset1.size(), mset3.size());

    vector<Xapian::docid> docids;
    for (Xapian::doccount i = 0; i != mset3.size(); ++i) {
	docids.push_back(*mset3[i]);
    }
    sort(docids.begin(), docids.end());

    for (Xapian::doccount i = 0; i != mset1.size(); ++i) {
	TEST_EQUAL(*mset1[i], *mset2[i]);
	TEST_EQUAL(*mset1[i], docids[i]);
    }

    for (Xapian::docid did : docids) {
	Xapian::TermIterator ti1(enquire1.get_matching_terms_begin(did));
	Xapian::TermIterator ti2(enquire2.get_matching_terms_begin(did));
	Xapian::TermIterator ti3(enquire3.get_matching_terms_begin(did));

	while (ti1 != enquire1.get_matching_terms_end(did)) {
	    TEST(ti2 != enquire2.get_matching_terms_end(did));
	    TEST(ti3 != enquire3.get_matching_terms_end(did));
	    TEST_EQUAL(*ti1, *ti2);
	    TEST_EQUAL(*ti2, *ti3);

	    ++ti1;
	    ++ti2;
	    ++ti3;
	}
	TEST(ti2 == enquire2.get_matching_terms_end(did));
	TEST(ti3 == enquire3.get_matching_terms_end(did));
    }
}
