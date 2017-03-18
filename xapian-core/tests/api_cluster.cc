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

/** Test for cosine distance
 *  Cosine distance = 1 - (cosine of the angle between two vectors).
 *  Thus, if two vectors are equal, the distance between them will be zero
 *  and if two vectors are unequal, the distance will be 1<=dist<=0.
 */
DEFINE_TESTCASE(cosine_distance1, backend) {
    Xapian::Enquire enquire(get_database("apitest_cluster"));
    enquire.set_query(Xapian::Query("cluster"));

    Xapian::MSet matches = enquire.get_mset(0, 4);
    Xapian::TermListGroup tlg;
    tlg.add_documents(matches);
    Xapian::Document doc1 = matches[0].get_document();
    Xapian::Document doc2 = matches[1].get_document();
    Xapian::Point x1, x2;
    x1.initialize(tlg, doc1);
    x2.initialize(tlg, doc2);

    // Check whether same vector gives zero distance
    Xapian::CosineDistance d;
    double distance = d.similarity(x1, x1);
    TEST_EQUAL (distance, 0);

    // Check whether different vector gives non-zero distance
    distance = d.similarity(x1, x2);
    TEST_REL (distance, >=, 0);
    TEST_REL (distance, <=, 1);

    return true;
}
