/* docsimtest.cc
 *
 * Copyright 2007 Yung-chung Lin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <config.h>
#include <iostream>
#include <string>
#include <xapian.h>

#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include "utils.h"
#include "apitest.h"

using namespace std;

static BackendManager backendmanager;

static bool test_cosine()
{
    try {
        Xapian::Database db(backendmanager.get_database("etext"));
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
            tout << "Similarity: "
                 << doc_sim.calculate_similarity(matches[i].get_document(),
                                                 matches[i+1].get_document())
                 << endl;
        }
                
    }
    catch (const Xapian::Error &e) {
        cerr << e.get_msg() << endl;
        return false;
    }
    catch (...) {
        cerr << "Unexpected error occurred" << endl;
        return false;
    }

    return true;
}

test_desc tests[] = {
    {"cosinetest", test_cosine},
    {0, 0}
};

int main(void)
{
    try {
        string srcdir = test_driver::get_srcdir();
        backendmanager.set_datadir(srcdir + "/testdata/");
        backendmanager.set_dbtype("inmemory");
        test_driver::run(tests);
    }
    catch (const Xapian::Error &e) {
        cerr << e.get_msg() << endl;
    }
    catch (...) {
        cerr << "Unexpected error occurred" << endl;
    }
    
    return 0;
}
