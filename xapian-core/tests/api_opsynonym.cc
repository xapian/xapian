/* api_opsynonym.cc: tests which need a backend
 *
 * Copyright 2009 Olly Betts
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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

#include "api_opsynonym.h"

#include <map>
#include <vector>

#include <xapian.h>

#include "backendmanager.h"
#include "backendmanager_local.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

// Check a synonym search
DEFINE_TESTCASE(synonym1, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::doccount lots = 214;

    // Make a list of lists of subqueries, which are going to be joined
    // together as a synonym.
    vector<vector<Xapian::Query> > subqueries_list;

    vector<Xapian::Query> subqueries;
    subqueries.push_back(Xapian::Query("date"));
    subqueries_list.push_back(subqueries);

    // Two terms, which co-occur in some documents.
    subqueries.clear();
    subqueries.push_back(Xapian::Query("sky"));
    subqueries.push_back(Xapian::Query("date"));
    subqueries_list.push_back(subqueries);

    // Two terms which are entirely disjoint, and where the maximum weight
    // doesn't occur in the first or second match.
    subqueries.clear();
    subqueries.push_back(Xapian::Query("gutenberg"));
    subqueries.push_back(Xapian::Query("blockhead"));
    subqueries_list.push_back(subqueries);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_OR,
				       Xapian::Query("sky"),
				       Xapian::Query("glove")));
    subqueries_list.push_back(subqueries);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("sky"));
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query("stein"));
    subqueries.push_back(Xapian::Query("ally"));
    subqueries_list.push_back(subqueries);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("attitud"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_PHRASE,
				       Xapian::Query("german"),
				       Xapian::Query("adventur")));
    subqueries_list.push_back(subqueries);

    for (vector<vector<Xapian::Query> >::const_iterator
	 qlist = subqueries_list.begin();
	 qlist != subqueries_list.end(); ++qlist)
    {
	// Run two queries, one joining the subqueries with OR and one joining them
	// with SYNONYM.
	Xapian::Enquire enquire(db);

	// Do the search with OR
	Xapian::Query orquery(Xapian::Query(Xapian::Query::OP_OR, qlist->begin(), qlist->end()));
	enquire.set_query(orquery);
	Xapian::MSet ormset = enquire.get_mset(0, lots);

	// Do the search with synonym, getting all the results.
	Xapian::Query synquery(Xapian::Query::OP_SYNONYM, qlist->begin(), qlist->end());
	enquire.set_query(synquery);
	Xapian::MSet mset = enquire.get_mset(0, lots);

	// Check that the queries return some results.
	TEST_NOT_EQUAL(mset.size(), 0);
	// Check that the queries return the same number of results.
	TEST_EQUAL(mset.size(), ormset.size());
	map<Xapian::docid, Xapian::weight> values_or;
	map<Xapian::docid, Xapian::weight> values_synonym;
	for (Xapian::doccount i = 0; i < mset.size(); ++i) {
	    values_or[*ormset[i]] = ormset[i].get_weight();
	    values_synonym[*mset[i]] = mset[i].get_weight();
	}
	TEST_EQUAL(values_or.size(), values_synonym.size());

	/* Check that the most of the weights for items in the "or" mset are
	 * different from those in the "synonym" mset. */
	int same_weight = 0;
	int different_weight = 0;
	for (map<Xapian::docid, Xapian::weight>::const_iterator
	     j = values_or.begin();
	     j != values_or.end(); ++j)
	{
	    Xapian::docid did = j->first;
	    // Check that all the results in the or tree make it to the synonym tree.
	    TEST(values_synonym.find(did) != values_synonym.end());
	    if (values_or[did] == values_synonym[did]) {
		same_weight += 1;
	    } else {
		different_weight += 1;
	    }
	}
	if (qlist->size() == 1) {
	    // Had a single term - check that all the weights were the same.
	    TEST_EQUAL(different_weight, 0);
	    TEST_NOT_EQUAL(same_weight, 0);
	} else {
	    // Check that most of the weights differ.
	    TEST_NOT_EQUAL(different_weight, 0);
	    TEST_REL(same_weight, <, different_weight);
	}

	// Do the search with synonym, but just get the top result.
	// (Regression test - the OR subquery in the synonym postlist tree used
	// to shortcut incorrectly, and return the wrong result here).
	Xapian::MSet mset_top = enquire.get_mset(0, 1);
	TEST_EQUAL(mset_top.size(), 1);
	TEST(mset_range_is_same(mset_top, 0, mset, 0, 1));
    }
    return true;
}

// Regression test - test a synonym search with a MultiAndPostlist.
DEFINE_TESTCASE(synonym2, backend) {
    Xapian::Query query;
    vector<Xapian::Query> subqueries;
    subqueries.push_back(Xapian::Query("file"));
    subqueries.push_back(Xapian::Query("the"));
    subqueries.push_back(Xapian::Query("next"));
    subqueries.push_back(Xapian::Query("reader"));
    query = Xapian::Query(Xapian::Query::OP_AND, subqueries.begin(), subqueries.end());
    subqueries.clear();
    subqueries.push_back(query);
    subqueries.push_back(Xapian::Query("gutenberg"));
    query = Xapian::Query(Xapian::Query::OP_SYNONYM, subqueries.begin(), subqueries.end());

    tout << query.get_description() << endl;

    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    tout << mset.get_description() << endl;

    // Regression test that OP_SCALE_WEIGHT works with OP_SYNONYM
    double maxposs = mset.get_max_possible();
    query = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 10.0);
    enquire.set_query(query);
    mset = enquire.get_mset(0, 10);
    double maxposs2 = mset.get_max_possible();

    TEST_EQUAL_DOUBLE(maxposs * 10.0, maxposs2);

    return true;
}

// Test a synonym search which has had its weight scaled to 0.
DEFINE_TESTCASE(synonym3, backend) {
    Xapian::Query query = Xapian::Query(Xapian::Query::OP_SYNONYM,
					Xapian::Query("sky"),
					Xapian::Query("date"));

    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset_orig = enquire.get_mset(0, db.get_doccount());

    tout << query.get_description() << endl;
    tout << mset_orig.get_description() << endl;

    // Test that OP_SCALE_WEIGHT with a factor of 0.0 works with OP_SYNONYM
    // (this has a special codepath to avoid doing the synonym calculation).
    query = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 0.0);
    enquire.set_query(query);
    Xapian::MSet mset_zero = enquire.get_mset(0, db.get_doccount());

    tout << query.get_description() << endl;
    tout << mset_zero.get_description() << endl;

    // Check that the queries return some results.
    TEST_NOT_EQUAL(mset_zero.size(), 0);
    // Check that the queries return the same document IDs, and the the zero
    // one has zero weight.
    TEST_EQUAL(mset_zero.size(), mset_orig.size());

    map<Xapian::docid, Xapian::weight> values_orig;
    map<Xapian::docid, Xapian::weight> values_zero;
    for (Xapian::doccount i = 0; i < mset_zero.size(); ++i) {
	TEST_NOT_EQUAL(mset_orig[i].get_weight(), 0.0);
	TEST_EQUAL(mset_zero[i].get_weight(), 0.0);

	values_orig[*mset_orig[i]] = mset_orig[i].get_weight();
	values_zero[*mset_zero[i]] = mset_zero[i].get_weight();
    }

    for (map<Xapian::docid, Xapian::weight>::const_iterator
	 j = values_orig.begin();
	 j != values_orig.end(); ++j)
    {
	Xapian::docid did = j->first;
	// Check that all the results in the orig mset are in the zero mset.
	TEST(values_zero.find(did) != values_zero.end());
    }
    TEST_EQUAL(values_orig.size(), values_zero.size());

    return true;
}
