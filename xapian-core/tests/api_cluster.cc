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
#include "testutils.h"
#include "testsuite.h"

using namespace std;

// Test for euclidian distance between the same document should be zero
DEFINE_TESTCASE(euclidian_distance1, backend) {
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster") );
    Xapian::MSet matches = enquire.get_mset(0, 5);
    Xapian::TermListGroup tlg;
    tlg.add_documents(matches);
    Xapian::Document doc = matches[0].get_document();
    Xapian::Point x;
    x.initialize(tlg, doc);
    Xapian::EuclidianDistance d;
    double sim = d.similarity(x, x);
    TEST_EQUAL (sim, 0);
    return true;
}

// Test for euclidian distance between different documents should be greater than zero
DEFINE_TESTCASE(euclidian_distance2, backend) {
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster") );

    Xapian::MSet matches = enquire.get_mset(0, 5);
    Xapian::TermListGroup tlg;
    tlg.add_documents(matches);
    Xapian::Document doc1 = matches[0].get_document();
    Xapian::Document doc2 = matches[1].get_document();
    Xapian::EuclidianDistance d;
    Xapian::Point x1, x2;
    x1.initialize(tlg, doc1);
    x2.initialize(tlg, doc2);
    double sim = d.similarity(x1, x2);
    TEST_REL (sim,>,0);
    return true;
}

/** Test that none of the returned clusters should be empty
 *  Note that the DocumentSet can be iterated through using DocumentSetIterator
 */
DEFINE_TESTCASE(round_robin1, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("cluster"));
    Xapian::MSet matches = enq.get_mset(0, 10);
    unsigned int k = 3;
    Xapian::RoundRobin rr(k);
    Xapian::ClusterSet cset = rr.cluster(matches);
    unsigned int size = cset.size();
    TEST_EQUAL (size, k);
    return true;
}
