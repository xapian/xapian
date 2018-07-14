/** @file api_diversify.cc
 *  @brief Diversification API tests
 */
/* Copyright (C) 2018 Uppinder Chugh
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

#include "api_diversify.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

// Test that diversified document set is not empty
DEFINE_TESTCASE(diversify1, generated)
{
    Xapian::Database db = get_database("apitest_diversify");
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("java"));
    Xapian::MSet matches = enq.get_mset(0, 10);

    unsigned int k = 4, r = 2;
    Xapian::Diversify d(k, r);
    Xapian::DocumentSet dset = d.get_dmset(matches);

    TEST(dset.size() != 0);
    return true;
}

/** LC cluster Test
 *  Test that none of the returned clusters are empty
 */
DEFINE_TESTCASE(lc1, generated)
{
    Xapian::Database db = get_database("apitest_diversify");
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("java"));
    Xapian::MSet matches = enq.get_mset(0, 10);

    int num_clusters = 4;
    Xapian::LC lc(num_clusters);
    Xapian::ClusterSet cset = lc.cluster(matches);
    int size = cset.size();
    for (int i = 0; i < size; ++i) {
	Xapian::DocumentSet d = cset[i].get_documents();
	TEST(d.size() != 0);
    }
    return true;
}
