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

/** Test that none of the returned clusters should be empty
 *  Note that the DocumentSet can be iterated through using DocumentSetIterator
 */
DEFINE_TESTCASE(round_robin1, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("cluster"));
    Xapian::MSet matches = enq.get_mset(0, 10);

    int num_clusters = 3;
    Xapian::RoundRobin rr(num_clusters);
    Xapian::ClusterSet cset = rr.cluster(matches);
    int size = cset.size();
    for (int i = 0; i < size; i++) {
	Xapian::DocumentSet d = cset[i].get_documents();
	TEST (d.size() != 0);
    }
    return true;
}

/// Test that none of the clusters returned by KMeans are empty
DEFINE_TESTCASE(kmeans, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("cluster"));
    Xapian::MSet matches = enq.get_mset(0, 10);

    int k = 3;
    Xapian::KMeans kmeans(k);
    Xapian::ClusterSet cset = kmeans.cluster(matches);
    int size = cset.size();
    for (int i = 0; i < size; i++) {
	Xapian::DocumentSet d = cset[i].get_documents();
	TEST (d.size() != 0);
    }
    return true;
}

/** Test that none of the clusters returned by KMeans while using
 *  KMeans++ as initialization are empty
 */
DEFINE_TESTCASE(kmeanspp, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("cluster"));
    Xapian::MSet matches = enq.get_mset(0, 10);

    int k = 3;
    string mode = "kmeanspp";
    Xapian::KMeans kmeanspp(k, mode);
    Xapian::ClusterSet cset = kmeanspp.cluster(matches);
    int size = cset.size();
    for (int i = 0; i < size; i++) {
	Xapian::DocumentSet d = cset[i].get_documents();
	TEST (d.size() != 0);
    }
    return true;
}
