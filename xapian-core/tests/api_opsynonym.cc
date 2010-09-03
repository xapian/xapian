/** @file api_opsynonym.cc
 * @brief tests of OP_SYNONYM.
 */
/* Copyright 2009 Olly Betts
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
#include <set>
#include <vector>

#include <xapian.h>

#include "backendmanager.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

// Check a synonym search
DEFINE_TESTCASE(synonym1, backend) {
    Xapian::Database db(get_database("etext"));

    TEST_REL(db.get_doclength_upper_bound(), >, 0);

    Xapian::doccount lots = 214;

    // Make a list of lists of subqueries, which are going to be joined
    // together as a synonym.
    vector<vector<Xapian::Query> > subqueries_list;

    // For each set of subqueries, keep a list of the number of results for
    // which the weight should be the same when combined with OP_SYNONYM as
    // when combined with OP_OR.
    vector<int> subqueries_sameweight_count;
    vector<int> subqueries_diffweight_count;

    vector<Xapian::Query> subqueries;
    subqueries.push_back(Xapian::Query("date"));
    subqueries_list.push_back(subqueries);
    // Single term - all 33 results should be same weight.
    subqueries_sameweight_count.push_back(33);
    subqueries_diffweight_count.push_back(0);

    // Two terms, which co-occur in some documents.
    subqueries.clear();
    subqueries.push_back(Xapian::Query("sky"));
    subqueries.push_back(Xapian::Query("date"));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    // Two terms which are entirely disjoint, and where the maximum weight
    // doesn't occur in the first or second match.
    subqueries.clear();
    subqueries.push_back(Xapian::Query("gutenberg"));
    subqueries.push_back(Xapian::Query("blockhead"));
    subqueries_list.push_back(subqueries);
    // All 18 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(18);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_OR,
				       Xapian::Query("sky"),
				       Xapian::Query("glove")));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_OR,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_AND_MAYBE,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_AND_NOT,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_AND,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // The AND only matches 1 document, so the estimated termfreq for the whole
    // synonym works out as 33 (due to rounding), which is the same as the
    // termfreq for "date".  Therefore most of the weights are the same as just
    // for the pure "date" search, and the only document which gets a different
    // weight is the one also matched by "sky" (because it has a wdf boost).
    subqueries_sameweight_count.push_back(32);
    subqueries_diffweight_count.push_back(1);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_XOR,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // All 34 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(34);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_SYNONYM,
				       Xapian::Query("sky"),
				       Xapian::Query("date")));
    subqueries_list.push_back(subqueries);
    // When the top-level operator is OR, the synonym part has an estimated
    // termfreq of 35.  When the top-level operator is SYNONYM, the whole query
    // has an estimated termfreq of 35, and is in fact the same as the synonym
    // part in the OR query, except that the wqf of "date" is 2.  We're
    // currently not using the wqfs of components of synonyms, so this
    // difference has no effect on the weightings.  Therefore, for the 1
    // document which does not contain "data", we get the same result with
    // SYNONYM as with OR.
    subqueries_sameweight_count.push_back(1);
    subqueries_diffweight_count.push_back(33);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("sky"));
    subqueries.push_back(Xapian::Query("date"));
    subqueries.push_back(Xapian::Query("stein"));
    subqueries.push_back(Xapian::Query("ally"));
    subqueries_list.push_back(subqueries);
    // All 35 results should be different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(35);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("attitud"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_PHRASE,
				       Xapian::Query("german"),
				       Xapian::Query("adventur")));
    subqueries_list.push_back(subqueries);
    // The estimated term frequency for the synoynm is 2 (because the estimate
    // for the phrase is 0), which is the same as the term frequency of
    // "attitud".  Thus, the synonym gets the same weight as "attitud", so
    // documents with only "attitud" (but not the phrase) in them get the same
    // wdf, and have the same total weight.  There turns out to be exactly one
    // such document.
    subqueries_sameweight_count.push_back(1);
    subqueries_diffweight_count.push_back(3);

    subqueries.clear();
    subqueries.push_back(Xapian::Query("attitud"));
    subqueries.push_back(Xapian::Query(Xapian::Query::OP_OR,
				       Xapian::Query("german"),
				       Xapian::Query(Xapian::Query::OP_SYNONYM,
						     Xapian::Query("sky"),
						     Xapian::Query("date"))));
    subqueries_list.push_back(subqueries);
    // All 54 results are different.
    subqueries_sameweight_count.push_back(0);
    subqueries_diffweight_count.push_back(54);

    for (vector<vector<Xapian::Query> >::size_type subqgroup = 0;
	 subqgroup != subqueries_list.size(); ++subqgroup)
    {
	vector<Xapian::Query> * qlist = &(subqueries_list[subqgroup]);
	// Run two queries, one joining the subqueries with OR and one joining
	// them with SYNONYM.
	Xapian::Enquire enquire(db);

	// Do the search with OR
	Xapian::Query orquery(Xapian::Query::OP_OR, qlist->begin(), qlist->end());
	enquire.set_query(orquery);
	Xapian::MSet ormset = enquire.get_mset(0, lots);

	// Do the search with synonym, getting all the results.
	Xapian::Query synquery(Xapian::Query::OP_SYNONYM, qlist->begin(), qlist->end());
	enquire.set_query(synquery);
	Xapian::MSet synmset = enquire.get_mset(0, lots);

	tout << "Comparing " << orquery << " with " << synquery << '\n';

	// Check that the queries return some results.
	TEST_NOT_EQUAL(synmset.size(), 0);
	// Check that the queries return the same number of results.
	TEST_EQUAL(synmset.size(), ormset.size());
	map<Xapian::docid, Xapian::weight> values_or;
	map<Xapian::docid, Xapian::weight> values_synonym;
	for (Xapian::doccount i = 0; i < synmset.size(); ++i) {
	    values_or[*ormset[i]] = ormset[i].get_weight();
	    values_synonym[*synmset[i]] = synmset[i].get_weight();
	}
	TEST_EQUAL(values_or.size(), values_synonym.size());

	/* Check that the most of the weights for items in the "or" mset are
	 * different from those in the "synonym" mset. */
	int same_weight = 0;
	int different_weight = 0;
	for (map<Xapian::docid, Xapian::weight>::const_iterator
	     j = values_or.begin(); j != values_or.end(); ++j) {
	    Xapian::docid did = j->first;
	    // Check that all the results in the or tree make it to the synonym
	    // tree.
	    TEST(values_synonym.find(did) != values_synonym.end());
	    if (values_or[did] == values_synonym[did]) {
		++same_weight;
	    } else {
		++different_weight;
	    }
	}

	int expected_same = subqueries_sameweight_count[subqgroup];
	int expected_diff = subqueries_diffweight_count[subqgroup];

	TEST_EQUAL(different_weight, expected_diff);
	TEST_EQUAL(same_weight, expected_same);

	// Do the search with synonym, but just get the top result.
	// (Regression test - the OR subquery in the synonym postlist tree used
	// to shortcut incorrectly, and return the wrong result here).
	Xapian::MSet mset_top = enquire.get_mset(0, 1);
	TEST_EQUAL(mset_top.size(), 1);
	TEST(mset_range_is_same(mset_top, 0, synmset, 0, 1));
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

    tout << query << '\n';

    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    tout << mset << '\n';

    // Regression test that OP_SCALE_WEIGHT works with OP_SYNONYM
    double maxposs = mset.get_max_possible();
    query = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 10.0);
    enquire.set_query(query);
    mset = enquire.get_mset(0, 10);
    double maxposs2 = mset.get_max_possible();

    TEST_EQUAL_DOUBLE(maxposs * 10.0, maxposs2);

    return true;
}

static void
check_msets_contain_same_docs(const Xapian::MSet & mset1,
			      const Xapian::MSet & mset2)
{
    TEST_EQUAL(mset1.size(), mset2.size());

    set<Xapian::docid> docids;
    for (Xapian::doccount i = 0; i < mset1.size(); ++i) {
	docids.insert(*mset1[i]);
    }

    // Check that all the results in mset1 are in mset2.
    for (Xapian::doccount j = 0; j < mset2.size(); ++j) {
	// Check that we can erase each entry from mset2 element.  Since mset1
	// and mset2 are the same size this means we can be sure that there
	// were no repeated docids in either (it would be a bug if there were).
	TEST(docids.erase(*mset2[j]));
    }
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

    tout << query << '\n';
    tout << mset_orig << '\n';

    // Test that OP_SCALE_WEIGHT with a factor of 0.0 works with OP_SYNONYM
    // (this has a special codepath to avoid doing the synonym calculation).
    query = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, query, 0.0);
    enquire.set_query(query);
    Xapian::MSet mset_zero = enquire.get_mset(0, db.get_doccount());

    tout << query << '\n';
    tout << mset_zero << '\n';

    // Check that the queries return some results.
    TEST_NOT_EQUAL(mset_zero.size(), 0);
    // Check that the queries return the same document IDs, and the zero
    // one has zero weight.
    check_msets_contain_same_docs(mset_orig, mset_zero);
    for (Xapian::doccount i = 0; i < mset_orig.size(); ++i) {
	TEST_NOT_EQUAL(mset_orig[i].get_weight(), 0.0);
	TEST_EQUAL(mset_zero[i].get_weight(), 0.0);
    }

    return true;
}

// Test synonym searches combined with various operators.
DEFINE_TESTCASE(synonym4, backend) {
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    Xapian::Query syn_query = Xapian::Query(Xapian::Query::OP_SYNONYM,
					    Xapian::Query("gutenberg"),
					    Xapian::Query("blockhead"));
    Xapian::Query or_query = Xapian::Query(Xapian::Query::OP_OR,
					   Xapian::Query("gutenberg"),
					   Xapian::Query("blockhead"));
    Xapian::Query date_query = Xapian::Query("date");

    // Check some queries.
    static const Xapian::Query::op operators[] = {
	Xapian::Query::OP_AND_MAYBE,
	Xapian::Query::OP_AND_NOT,
	Xapian::Query::OP_AND,
	Xapian::Query::OP_XOR,
	Xapian::Query::OP_OR,
	Xapian::Query::OP_SYNONYM
    };
    const Xapian::Query::op * end;
    end = operators + sizeof(operators) / sizeof(operators[0]);
    for (const Xapian::Query::op * i = operators; i != end; ++i) {
	tout.str(string());
	Xapian::Query query1(*i, syn_query, date_query);
	Xapian::Query query2(*i, or_query, date_query);

	enquire.set_query(query1);
	tout << "query1:" << query1 << '\n';
	Xapian::MSet mset1 = enquire.get_mset(0, db.get_doccount());
	tout << "mset1:" << mset1 << '\n';
	enquire.set_query(query2);
	tout << "query2:" << query2 << '\n';
	Xapian::MSet mset2 = enquire.get_mset(0, db.get_doccount());
	tout << "mset2:" << mset2 << '\n';

	TEST_NOT_EQUAL(mset1.size(), 0);
	if (*i != Xapian::Query::OP_XOR) {
	    TEST_EQUAL(mset1[0].get_percent(), 100);
	} else {
	    TEST(mset1[0].get_percent() != 100);
	}
	check_msets_contain_same_docs(mset1, mset2);
    }

    return true;
}
