/* api_snipper.cc: tests Snipper class
 *
 * Copyright 2012 Mihai Bivol
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

#include "api_snipper.h"

#include <string>

#include <xapian.h>
#include "backendmanager_local.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

#include <iostream>

using namespace std;

// tests that the number of indexed documents is the size of the one set
DEFINE_TESTCASE(snipper1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
	Xapian::Query query = Xapian::Query("this");
    enquire.set_query(query);
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // MSet size should be 6.
    TEST_MSET_SIZE(mymset, 6);

    Xapian::Snipper snipper;
    snipper.set_query(query);
    snipper.set_mset(mymset, 4);
    TEST(snipper.get_description().find("rm_doccount=4,") != string::npos);
    TEST(snipper.get_description().find("query=this)") != string::npos);
    return true;
}

// tests that the relevance model is reset when setting another mset
DEFINE_TESTCASE(snipper2, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(Xapian::Query("word"));
    Xapian::MSet mymset2 = enquire.get_mset(0, 10);

    // MSet sizes for the two queries should be 6 and 2.
    TEST_MSET_SIZE(mymset1, 6);
    TEST_MSET_SIZE(mymset2, 2);

    Xapian::Snipper snipper;
    snipper.set_mset(mymset1);
    // Should add to relevance model 6 documents.
    TEST(snipper.get_description().find("rm_doccount=6,") != string::npos);

    snipper.set_mset(mymset2);
    // Should add to relevance model 2 documents.
    TEST(snipper.get_description().find("rm_doccount=2,") != string::npos);
    return true;
}


/* tests that the number of indexed documents is the size of the one set and check if query was not set in snipper,
 *  it doesn't come in description */
DEFINE_TESTCASE(snipper3, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 10);

    // MSet size should be 6.
    TEST_MSET_SIZE(mymset, 6);

    Xapian::Snipper snipper;
    snipper.set_mset(mymset, 4);
    TEST(snipper.get_description().find("rm_doccount=4,") != string::npos);
    TEST(snipper.get_description().find("query=)") == string::npos);
    return true;
}

