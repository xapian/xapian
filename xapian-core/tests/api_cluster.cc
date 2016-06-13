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

// Test for distance between the same document should be zero
DEFINE_TESTCASE(euclidian_distance1, backend) {
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster") );

    Xapian::MSet matches = enquire.get_mset(0, 5);
    Xapian::MSetDocumentSource docs(matches);
    Xapian::TermListGroup tlg;
    tlg.add_documents(docs);
    Xapian::MSetIterator i = matches.begin();
    Xapian::Document doc1 = i.get_document();
    Xapian::EuclidianDistance d;
    double sim = d.similarity(tlg, doc1.termlist_begin(), doc1.termlist_end(), doc1.termlist_begin(), doc1.termlist_end());
    TEST_EQUAL (sim, 0);
    return true;
}

// Test for distance between different documents should be greater than zero
DEFINE_TESTCASE(euclidian_distance2, backend) {
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster") );

    Xapian::MSet matches = enquire.get_mset(0, 5);
    Xapian::MSetDocumentSource docs(matches);
    Xapian::TermListGroup tlg;
    tlg.add_documents(docs);
    Xapian::MSetIterator i = matches.begin();
    Xapian::Document doc1 = i.get_document();
    i++;
    Xapian::Document doc2 = i.get_document();
    Xapian::EuclidianDistance d;
    double sim = d.similarity(tlg, doc1.termlist_begin(), doc1.termlist_end(), doc2.termlist_begin(), doc2.termlist_end());
    TEST_NOT_EQUAL (sim, 0);
    return true;
}

/** Test that none of the returned clusters should be empty
 *  Note that the DocumentSet can be iterated through using DocumentSetIterator
 */
DEFINE_TESTCASE(round_robin1, backend)
{
    Xapian::Database db(get_database("apitest_cluster"));
    Xapian::Enquire enq(db);
    enq.set_query(Xapian::Query("computer"));
    Xapian::MSet matches = enq.get_mset(0, 10);
    Xapian::RoundRobin rr;
    int k = 3;
    Xapian::ClusterSet cset = rr.cluster(matches, k);
    for (Xapian::ClusterSetIterator i=cset.begin(); i!=cset.end(); i++) {
	Xapian::DocumentSet d = i.get_cluster();
	TEST (d.size() != 0);
    }
    return true;
}
