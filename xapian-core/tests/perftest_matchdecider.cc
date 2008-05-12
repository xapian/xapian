/* perftest_matchdecider.cc: performance tests for match decider
 *
 * Copyright 2008 Lemur Consulting Ltd
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

#include "perftest_matchdecider.h"

#include <xapian.h>

#include "backendmanager.h"
#include "perftest.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

using namespace std;

// Test the performance of a ValueSetMatchDecider
DEFINE_TESTCASE(valuesetmatchdecider1, writable && !remote) {
    logger.testcase_begin("valuesetmatchdecider1");

    std::string dbname("1");
    Xapian::WritableDatabase dbw = backendmanager->get_writable_database(dbname, "");
    logger.indexing_begin(dbname);
    unsigned int runsize = 1000000;
    unsigned int i;
    for (i = 0; i < runsize; ++i) {
	unsigned int v = i % 100;
	Xapian::Document doc;
	doc.set_data("test document " + om_tostring(i));
	doc.add_term("foo");
	doc.add_value(0, om_tostring(v));
	dbw.add_document(doc);
	logger.indexing_add();
    }
    dbw.flush();
    logger.indexing_end();

    Xapian::Enquire enquire(dbw);
    Xapian::Query query("foo");
    enquire.set_query(query);

    logger.searching_start("No match decider");
    logger.search_start();
    Xapian::MSet mset = enquire.get_mset(0, 10, 0, NULL, NULL, NULL);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    TEST(mset.get_matches_upper_bound() <= runsize);

    logger.search_start();
    mset = enquire.get_mset(0, 10, 0, NULL, NULL, NULL);
    logger.search_end(query, mset);
    TEST(mset.size() == 10);
    TEST(mset.get_matches_lower_bound() <= runsize);
    logger.searching_end();

    Xapian::ValueSetMatchDecider md(0, true);

    for (i = 0; i < 100; ++i) {
	md.add_value(om_tostring(i));
	logger.searching_start("Match decider accepting " + om_tostring(i + 1) + "%");
	logger.search_start();
	mset = enquire.get_mset(0, 10, 0, NULL, &md, NULL);
	logger.search_end(query, mset);
	TEST(mset.size() == 10);
	TEST_LESSER_OR_EQUAL(mset.get_matches_lower_bound(), runsize * (i + 1) / 100);

	logger.search_start();
	mset = enquire.get_mset(0, 10, 0, NULL, &md, NULL);
	logger.search_end(query, mset);
	TEST(mset.size() == 10);
	TEST_LESSER_OR_EQUAL(mset.get_matches_lower_bound(), runsize * (i + 1) / 100);
	logger.searching_end();
    }

    logger.testcase_end();
    return true;
}
