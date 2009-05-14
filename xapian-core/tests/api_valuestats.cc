/* api_valuestats.cc: tests related to the 1.1.x value statistics functions
 *
 * Copyright 2008 Lemur Consulting Ltd
 * Copyright 2008,2009 Olly Betts
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

#include "api_valuestats.h"

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

// #######################################################################
// # Tests start here

/// Regression test for bug fixed in 1.1.1 and backported for 1.0.13.
DEFINE_TESTCASE(valuestats5, !backend) {
    Xapian::Document doc;
    doc.add_value(0, "zero");
    doc.add_value(1, "one");
    doc.add_value(2, "two");
    doc.add_value(3, "three");
    doc.add_value(4, "");
    doc.add_value(5, "five");
    doc.remove_value(3);
    doc.add_value(1, "");

    // Check that we don't have any empty values reported.
    size_t c = 0;
    Xapian::ValueIterator v = doc.values_begin();
    while (v != doc.values_end()) {
	TEST(!(*v).empty());
	++c;
	++v;
    }
    TEST_EQUAL(c, 3); // 0, 2, 5

    return true;
}
