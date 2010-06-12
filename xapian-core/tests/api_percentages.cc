/** @file api_percentages.cc
 * @brief Tests of percentage calculations.
 */
/* Copyright (C) 2008,2009 Lemur Consulting Ltd
 * Copyright (C) 2008,2009,2010 Olly Betts
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
#include "testutils.h"

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

/// Test that a search with a non-existent term doesn't get 100%.
DEFINE_TESTCASE(topercent5, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Query q(Xapian::Query::OP_OR,
		    Xapian::Query("paragraph"), Xapian::Query("xyzzy"));
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    TEST(!mset.empty());
    TEST(mset[0].get_percent() < 100);
    return true;
}
