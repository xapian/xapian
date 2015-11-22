/* perftest_matchdecider.cc: performance tests for match decider
 *
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2015 Olly Betts
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

#include "perftest/perftest_matchdecider.h"

#include <xapian.h>

#include "backendmanager.h"
#include "perftest.h"
#include "str.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;

static void
builddb_valuestest1(Xapian::WritableDatabase &db, const string & dbname)
{
    logger.testcase_begin(dbname);
    unsigned int runsize = 1000000;

    // Rebuild the database.
    std::map<std::string, std::string> params;
    params["runsize"] = str(runsize);
    logger.indexing_begin(dbname, params);
    for (unsigned int i = 0; i < runsize; ++i) {
	unsigned int v = i % 100;
	Xapian::Document doc;
	doc.set_data("test document " + str(i));
	doc.add_term("foo");
	string vs = str(v);
	if (vs.size() == 1) vs = "0" + vs;
	doc.add_value(0, vs);
	doc.add_term("F" + vs);
	doc.add_term("Q" + str(i));
	for (int j = 0; j != 100; ++j)
	    doc.add_term("J" + str(j));
	db.replace_document(i + 10, doc);
	logger.indexing_add();
    }
    db.commit();
    logger.indexing_end();
    logger.testcase_end();
}

// Test the performance of a ValueSetMatchDecider, compared to a Value range operator.
DEFINE_TESTCASE(valuesetmatchdecider1, writable && !remote && !inmemory) {
    Xapian::Database db;
    db = backendmanager->get_database("valuestest1", builddb_valuestest1,
				      "valuestest1");

    logger.testcase_begin("valuesetmatchdecider1");
    Xapian::Enquire enquire(db);
    Xapian::doccount runsize = db.get_doccount();

    Xapian::Query query("foo");
    logger.searching_start("No match decider");
    logger.search_start();
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(0, 10);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    TEST(mset.get_matches_upper_bound() <= runsize);

    logger.search_start();
    mset = enquire.get_mset(0, 10);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    logger.searching_end();

    Xapian::ValueSetMatchDecider md(0, true);

    for (unsigned int i = 0; i < 100; ++i) {
	string vs = str(i);
	if (vs.size() == 1) vs = "0" + vs;
	md.add_value(vs);

	logger.searching_start("Match decider accepting " + str(i + 1) + "%");
	logger.search_start();
	enquire.set_query(query);
	mset = enquire.get_mset(0, 10, 0, NULL, &md);
	logger.search_end(query, mset);
	TEST_EQUAL(mset.size(), 10);
	TEST_REL(mset.get_matches_lower_bound(),<=,runsize * (i + 1) / 100);
	logger.searching_end();

	Xapian::Query query2(Xapian::Query::OP_FILTER, query,
			     Xapian::Query(Xapian::Query::OP_VALUE_LE, 0, vs));
	logger.searching_start("Value range LE accepting " + str(i + 1) + "%");
	Xapian::MSet mset2;
	logger.search_start();
	enquire.set_query(query2);
	mset2 = enquire.get_mset(0, 10);
	logger.search_end(query2, mset2);
	TEST_EQUAL(mset2.size(), 10);
	TEST_REL(mset2.get_matches_lower_bound(),<=,runsize * (i + 1) / 100);
	test_mset_order_equal(mset, mset2);
	logger.searching_end();
    }

    logger.testcase_end();
    return true;
}

// Test the performance of an AllDocsIterator.
DEFINE_TESTCASE(alldocsiter1, writable && !remote && !inmemory) {
    Xapian::Database db;
    db = backendmanager->get_database("valuestest1", builddb_valuestest1,
				      "valuestest1");

    logger.testcase_begin("alldocsiter1");

    logger.searching_start("AllDocsPostingIterator, full iteration");
    logger.search_start();
    Xapian::PostingIterator begin(db.postlist_begin(""));
    Xapian::PostingIterator end(db.postlist_end(""));
    while (begin != end) {
	++begin;
    }
    logger.search_end(Xapian::Query(), Xapian::MSet());

    logger.searching_end();

    logger.testcase_end();
    return true;
}
