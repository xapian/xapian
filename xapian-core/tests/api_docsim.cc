/* api_anydb.cc: tests which work with any backend
 *
 * Copyright 2007 Yung-chung Lin
 * Copyright 2007 Lemur Consulting Ltd
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

#include "api_docsim.h"

#include <string>

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

/// Test for document similarity
DEFINE_TESTCASE(docsim1, backend) {
    Xapian::Database db(get_database("etext"));
    string query = "Time";
    Xapian::Stem stemmer("en");

    Xapian::QueryParser qp;
    qp.set_database(db);
    qp.set_stemmer(stemmer);

    Xapian::Enquire enq(db);
    enq.set_query(qp.parse_query(query));
    Xapian::MSet matches = enq.get_mset(0, 30);

    Xapian::DocSimCosine doc_sim;
    doc_sim.set_database(db);
    for (Xapian::doccount i = 0; i < matches.size(); i+=2) {
        double sim
            = doc_sim.calculate_similarity(matches[i].get_document(),
                                           matches[i+1].get_document());
        TEST(sim >= 0 && sim <= 1);
    }

    return true;
}
