/** @file api_queryopt.cc
 * @brief Tests of the query optimiser.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
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
#include "utils.h"

#include "apitest.h"
#include <vector>

using namespace std;

class MyDontUsePostingSource : public Xapian::PostingSource {
  public:
    MyDontUsePostingSource() : Xapian::PostingSource() {}
    PostingSource * clone() const { return new MyDontUsePostingSource(); }
    void init(const Xapian::Database &) {
	return;
    }

    Xapian::weight get_weight() const {
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

    void next(Xapian::weight) {
	FAIL_TEST("MyDontUsePostingSource::next() called");
    }

    void skip_to(Xapian::docid, Xapian::weight) {
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
    MyDontUsePostingSource src;
    Xapian::Query qsrc(&src);

    {
	Xapian::Enquire enquire(get_database("etext"));
	Xapian::Query query = Xapian::Query::MatchAll;
	query = Xapian::Query(query.OP_AND_MAYBE, query, qsrc);
	query = Xapian::Query(query.OP_SCALE_WEIGHT, query, 0.0);
	enquire.set_query(query);
	enquire.get_mset(0, 10);
    }

    {
	Xapian::Enquire enquire(get_database("etext"));
	Xapian::Query query = Xapian::Query::MatchAll;
	query = Xapian::Query(query.OP_AND_MAYBE, query, qsrc);
	enquire.set_weighting_scheme(Xapian::BoolWeight());
	enquire.set_query(query);
	enquire.get_mset(0, 10);
    }

    return true;
}

/** Test that ANDMAYBE branches return same documents and lists of matching
 *  terms when their weight factor is 0.
 */
DEFINE_TESTCASE(boolandmaybe2, backend) {
    Xapian::Query query(query.OP_AND_MAYBE,
			Xapian::Query("last"), Xapian::Query("day"));
    Xapian::MSet mset1, mset2, mset3;
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire1(db), enquire2(db), enquire3(db);

    {
	Xapian::Query query1(query.OP_SCALE_WEIGHT, query, 0.0);
	enquire1.set_query(query1);
	mset1 = enquire1.get_mset(0, 1000);
    }

    {
	enquire2.set_query(query);
	enquire2.set_weighting_scheme(Xapian::BoolWeight());
	mset2 = enquire2.get_mset(0, 1000);
    }

    TEST(mset_range_is_same(mset1, 0, mset2, 0, mset1.size()));

    {
	enquire3.set_query(query);
	mset3 = enquire3.get_mset(0, 1000);
    }

    vector<Xapian::docid> docs1, docs2, docs3;
    for (Xapian::doccount i = 0; i != mset1.size(); ++i) {
	docs1.push_back(*mset1[i]);
	docs2.push_back(*mset2[i]);
	docs3.push_back(*mset3[i]);
    }
    sort(docs1.begin(), docs1.end());
    sort(docs3.begin(), docs3.end());
    TEST_EQUAL(docs1, docs3);

    for (Xapian::doccount i = 0; i != mset1.size(); ++i) {
	Xapian::TermIterator ti1(enquire1.get_matching_terms_begin(docs1[i]));
	Xapian::TermIterator tie1(enquire1.get_matching_terms_end(docs1[i]));
	Xapian::TermIterator ti2(enquire2.get_matching_terms_begin(docs2[i]));
	Xapian::TermIterator tie2(enquire2.get_matching_terms_end(docs2[i]));
	Xapian::TermIterator ti3(enquire3.get_matching_terms_begin(docs3[i]));
	Xapian::TermIterator tie3(enquire3.get_matching_terms_end(docs3[i]));

	while(ti1 != tie1) {
	    TEST(ti2 != tie2);
	    TEST(ti3 != tie3);
	    TEST_EQUAL(*ti1, *ti2);
	    TEST_EQUAL(*ti2, *ti3);

	    ++ti1;
	    ++ti2;
	    ++ti3;
	}
	TEST(ti2 == tie2);
	TEST(ti3 == tie3);
    }

    return true;
}
