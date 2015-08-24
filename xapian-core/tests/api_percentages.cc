/** @file api_percentages.cc
 * @brief Tests of percentage calculations.
 */
/* Copyright (C) 2008,2009 Lemur Consulting Ltd
 * Copyright (C) 2008,2009,2010,2012 Olly Betts
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

#include "api_percentages.h"

#include <xapian.h>

#include "apitest.h"
#include "backendmanager_local.h"
#include "str.h"
#include "testutils.h"

#include <cfloat>

using namespace std;

// Test that percentages reported are the same regardless of which part of the
// mset is returned, for sort-by-value search.  Regression test for bug#216 in
// 1.0.10 and earlier with returned percentages.
DEFINE_TESTCASE(consistency3, backend) {
    Xapian::Database db(get_database("apitest_sortconsist"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("foo"));
    enquire.set_sort_by_value(1, 0);
    Xapian::doccount lots = 3;
    Xapian::MSet bigmset = enquire.get_mset(0, lots);
    TEST_EQUAL(bigmset.size(), lots);
    for (Xapian::doccount start = 0; start < lots; ++start) {
	tout << *bigmset[start] << ":" << bigmset[start].get_weight() << ":"
	     << bigmset[start].get_percent() << "%" << endl;
	for (Xapian::doccount size = 0; size < lots - start; ++size) {
	    Xapian::MSet mset = enquire.get_mset(start, size);
	    if (mset.size()) {
		TEST_EQUAL(start + mset.size(),
			   min(start + size, bigmset.size()));
	    } else if (size) {
		TEST(start >= bigmset.size());
	    }
	    for (Xapian::doccount i = 0; i < mset.size(); ++i) {
		TEST_EQUAL(*mset[i], *bigmset[start + i]);
		TEST_EQUAL_DOUBLE(mset[i].get_weight(),
				  bigmset[start + i].get_weight());
		TEST_EQUAL_DOUBLE(mset[i].get_percent(),
				  bigmset[start + i].get_percent());
	    }
	}
    }
    return true;
}

class MyPostingSource : public Xapian::PostingSource {
    vector<pair<Xapian::docid, Xapian::weight> > weights;
    vector<pair<Xapian::docid, Xapian::weight> >::const_iterator i;
    bool started;

    MyPostingSource(const vector<pair<Xapian::docid, Xapian::weight> > &weights_,
		    Xapian::weight max_wt)
	: weights(weights_), started(false)
    {
	set_maxweight(max_wt);
    }

  public:
    MyPostingSource() : started(false) { }

    PostingSource * clone() const
    {
	return new MyPostingSource(weights, get_maxweight());
    }

    void append_docweight(Xapian::docid did, Xapian::weight wt) {
	weights.push_back(make_pair(did, wt));
	if (wt > get_maxweight()) set_maxweight(wt);
    }

    void init(const Xapian::Database &) { started = false; }

    Xapian::weight get_weight() const { return i->second; }

    Xapian::doccount get_termfreq_min() const { return weights.size(); }
    Xapian::doccount get_termfreq_est() const { return weights.size(); }
    Xapian::doccount get_termfreq_max() const { return weights.size(); }

    void next(Xapian::weight /*wt*/) {
	if (!started) {
	    i = weights.begin();
	    started = true;
	} else {
	    ++i;
	}
    }

    bool at_end() const {
	return (i == weights.end());
    }

    Xapian::docid get_docid() const { return i->first; }

    string get_description() const {
	return "MyPostingSource";
    }
};


/// Test for rounding errors in percentage weight calculations and cutoffs.
DEFINE_TESTCASE(pctcutoff4, backend && !remote && !multi) {
    // Find the number of DBL_EPSILONs to subtract which result in the
    // percentage of the second hit being 49% instead of 50%.
    int epsilons = 0;
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    while (true) {
	MyPostingSource source;
	source.append_docweight(1, 100);
	source.append_docweight(2, 50 - epsilons * DBL_EPSILON);
	enquire.set_query(Xapian::Query(&source));
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST_EQUAL(mset.size(), 2);
	if (mset[1].get_percent() != 50) break;
	++epsilons;
    }

    // Make a set of document weights including ones on either side of the
    // 49% / 50% boundary.
    MyPostingSource source;
    source.append_docweight(1, 100);
    source.append_docweight(2, 50);
    source.append_docweight(3, 50 - (epsilons - 1) * DBL_EPSILON);
    source.append_docweight(4, 50 - epsilons * DBL_EPSILON);
    source.append_docweight(5, 25);

    enquire.set_query(Xapian::Query(&source));
    Xapian::MSet mset1 = enquire.get_mset(0, 10);
    TEST_EQUAL(mset1.size(), 5);
    TEST_EQUAL(mset1[2].get_percent(), 50);
    TEST_EQUAL(mset1[3].get_percent(), 49);

    // Use various different percentage cutoffs, and check that the values
    // returned are as expected.
    int percent = 100;
    for (Xapian::MSetIterator i = mset1.begin(); i != mset1.end(); ++i) {
	int new_percent = mset1.convert_to_percent(i);
	tout << "mset1 item = " << i.get_percent() << "%\n";
	if (new_percent != percent) {
	    enquire.set_cutoff(percent);
	    Xapian::MSet mset2 = enquire.get_mset(0, 10);
	    tout << "cutoff = " << percent << "%, "
		    "mset size = " << mset2.size() << "\n";
	    TEST_EQUAL(mset2.size(), i.get_rank());
	    percent = new_percent;
	}
    }

    return true;
}

/// Check we throw for a percentage cutoff while sorting primarily by value.
DEFINE_TESTCASE(pctcutoff5, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("test"));
    enquire.set_cutoff(42);
    Xapian::MSet mset;

    enquire.set_sort_by_value(0, false);
    TEST_EXCEPTION(Xapian::UnimplementedError, mset = enquire.get_mset(0, 10));

    enquire.set_sort_by_value(0, true);
    TEST_EXCEPTION(Xapian::UnimplementedError, mset = enquire.get_mset(0, 10));

    enquire.set_sort_by_value_then_relevance(0, false);
    TEST_EXCEPTION(Xapian::UnimplementedError, mset = enquire.get_mset(0, 10));

    enquire.set_sort_by_value_then_relevance(0, true);
    TEST_EXCEPTION(Xapian::UnimplementedError, mset = enquire.get_mset(0, 10));

    return true;
}

// Regression test for bug fixed in 1.0.14.
DEFINE_TESTCASE(topercent3, remote) {
    BackendManagerLocal local_manager;
    local_manager.set_datadir(test_driver::get_srcdir() + "/testdata/");
    Xapian::Database db;
    db.add_database(get_database("apitest_simpledata"));
    db.add_database(local_manager.get_database("apitest_simpledata"));

    Xapian::Enquire enquire(db);
    enquire.set_sort_by_value(1, false);

    const char * terms[] = { "paragraph", "banana" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, terms, terms + 2));

    Xapian::MSet mset = enquire.get_mset(0, 20);

    Xapian::MSetIterator i;
    for (i = mset.begin(); i != mset.end(); ++i) {
	// We should never achieve 100%.
	TEST_REL(i.get_percent(),<,100);
    }

    return true;
}

// Regression test for bug introduced temporarily by the "percent without
// termlist" patch.
DEFINE_TESTCASE(topercent4, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));

    Xapian::Query query(Xapian::Query::OP_FILTER,
			Xapian::Query("paragraph"),
			Xapian::Query("queri"));
    query = Xapian::Query(Xapian::Query::OP_XOR,
			  query, Xapian::Query("rubbish"));

    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 10);

    // We should get 50% not 33%.
    TEST(!mset.empty());
    TEST_EQUAL(mset[0].get_percent(), 50);

    return true;
}

/// Test that a search with a non-existent term doesn't get 100%.
DEFINE_TESTCASE(topercent5, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("paragraph"), Xapian::Query("xyzzy"));
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    TEST(mset[0].get_percent() < 100);
    // It would be odd if the non-existent term was worth more, but in 1.0.x
    // the top hit got 4% in this testcase.  In 1.2.x it gets 50%, which is
    // better, but >50% would be more natural.
    TEST(mset[0].get_percent() >= 50);
    return true;
}

/// Test that OP_FILTER doesn't affect percentages.
//  Regression test for bug#590 fixed in 1.3.1 and 1.2.10.
DEFINE_TESTCASE(topercent6, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("rubbish"), Xapian::Query("letter"));
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    TEST(mset[0].get_percent() < 100);

    q = Xapian::Query(q.OP_FILTER, q, Xapian::Query("this"));
    enquire.set_query(q);
    Xapian::MSet mset2 = enquire.get_mset(0, 10);
    TEST(!mset2.empty());
    TEST_EQUAL(mset[0].get_percent(), mset2[0].get_percent());
    return true;
}

static void
make_topercent7_db(Xapian::WritableDatabase &db, const string &)
{
    for (int i = 1; i <= 6; ++i) {
	Xapian::Document d;
	d.set_data(str(i));
	d.add_term("boom", 2 + (i - 4)*(i - 2));
	if (i != 5)
	    d.add_boolean_term("XCAT122");
	db.add_document(d);
    }
    db.commit();
}

/// Test that a term with wdf always = 0 gets counted.
//  Regression test for bug introduced in 1.2.10 by the original fix for #590,
//  and fixed in 1.2.13 (and in trunk before 1.3.1 was released).
DEFINE_TESTCASE(topercent7, generated) {
    Xapian::Database db(get_database("topercent7", make_topercent7_db));

    Xapian::Query q;
    q = Xapian::Query(q.OP_OR, Xapian::Query("tomb"), Xapian::Query("boom"));
    q = Xapian::Query(q.OP_AND, q, Xapian::Query("XCAT122"));

    Xapian::Enquire enq(db);
    enq.set_query(q);
    Xapian::MSet m = enq.get_mset(0, 10);
    TEST(!m.empty());
    TEST_REL(m[0].get_percent(),>,60);
    return true;
}
