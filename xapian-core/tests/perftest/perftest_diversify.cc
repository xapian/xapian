/** @file
 * @brief performance tests for diversification
 */
/* Copyright 2018 Uppinder Chugh
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

#include "perftest/perftest_diversify.h"

#include <xapian.h>

#include "backendmanager.h"
#include "perftest.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;

// Test that diversified document set is not empty
DEFINE_TESTCASE(perfdiversify1, writable && !remote && !inmemory)
{
    Xapian::Database db;
    db = backendmanager->get_database("apitest_diversify");

    logger.testcase_begin("perfdiversify1");
    Xapian::Enquire enq(db);

    Xapian::Query query("java");

    logger.searching_start("Diversfication");
    logger.search_start();
    enq.set_query(query);
    Xapian::MSet matches = enq.get_mset(0, 10);
    logger.search_end(query, matches);
    logger.searching_end();

    logger.diversifying_start("Diversification");
    unsigned int k = 4, r = 2;
    Xapian::Diversify d(k, r);
    logger.diversify_start();
    Xapian::DocumentSet dset = d.get_dmset(matches);
    logger.diversify_end(k, r, dset);

    TEST(dset.size() != 0);
    logger.diversifying_end();

    logger.testcase_end();
}
