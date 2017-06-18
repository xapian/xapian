/** @file api_cluster.cc
 *  @brief Cluster API tests
 */
/* Copyright (C) 2016 Richhiey Thomas
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

#include "api_cluster.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

/** Test for cosine distance
 *  Cosine distance = 1 - (cosine of the angle between two vectors).
 *  Thus, if two vectors are equal, the distance between them will be zero
 *  and if two vectors are unequal, the distance will be 1 >= dist >= 0.
 */
DEFINE_TESTCASE(cosine_distance1, backend)
{
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster"));

    Xapian::MSet matches = enquire.get_mset(0, 4);
    Xapian::TermListGroup tlg(matches);
    Xapian::Document doc1 = matches[0].get_document();
    Xapian::Document doc2 = matches[1].get_document();
    Xapian::Point x1(tlg, doc1);
    Xapian::Point x2(tlg, doc2);

    // Check whether same vector gives zero distance
    Xapian::CosineDistance d;
    double distance = d.similarity(x1, x1);
    TEST_EQUAL(distance, 0);

    // Check whether two different vectors gives a distance such that
    // 0 <= distance <= 1
    distance = d.similarity(x1, x2);
    TEST_REL(distance, >, 0);
    TEST_REL(distance, <=, 1);

    return true;
}

/** Round Robin Test
 *  Test that none of the returned clusters are empty
 */
DEFINE_TESTCASE(round_robin1, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("cluster"));
    Xapian::MSet matches = enq.get_mset(0, 4);

    int num_clusters = 3;
    Xapian::RoundRobin rr(num_clusters);
    Xapian::ClusterSet cset = rr.cluster(matches);
    int size = cset.size();
    for (int i = 0; i < size; ++i) {
	Xapian::DocumentSet d = cset[i].get_documents();
	TEST(d.size() != 0);
    }
    return true;
}
