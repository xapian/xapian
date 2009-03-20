/** @file api_query.cc
 * @brief Query-related tests which don't need a backend.
 */
/* Copyright (C) 2009 Olly Betts
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

#include "api_query.h"

#include <xapian.h>

#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include "apitest.h"

using namespace std;

/// Regression test - in 1.0.10 and earlier "" was included in the list.
DEFINE_TESTCASE(queryterms1, !backend) {
    Xapian::Query query = Xapian::Query::MatchAll;
    TEST(query.get_terms_begin() == query.get_terms_end());
    query = Xapian::Query(query.OP_AND_NOT, query, Xapian::Query("fair"));
    TEST_EQUAL(*query.get_terms_begin(), "fair");
    return true;
}

DEFINE_TESTCASE(matchnothing1, !backend) {
    vector<Xapian::Query> subqs;
    subqs.push_back(Xapian::Query("foo"));
    subqs.push_back(Xapian::Query::MatchNothing);
    Xapian::Query q(Xapian::Query::OP_AND, subqs.begin(), subqs.end());
    TEST_STRINGS_EQUAL(q.get_description(), "Xapian::Query()");

    Xapian::Query q2(Xapian::Query::OP_AND,
		     Xapian::Query("foo"), Xapian::Query::MatchNothing);
    TEST_STRINGS_EQUAL(q2.get_description(), "Xapian::Query()");
    return true;
}
