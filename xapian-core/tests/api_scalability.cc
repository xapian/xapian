/** @file api_scalability.cc
 * @brief Tests of scalability.
 */
/* Copyright (C) 2008,2009 Olly Betts
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

#include "api_scalability.h"

#include "apitest.h"
#include "cputimer.h"
#include "scalability.h"
#include "testsuite.h"
#include "testutils.h"

#include <xapian.h>

using namespace std;

static double
bigoaddvalue1_helper(unsigned num_values)
{
    Xapian::WritableDatabase db = get_writable_database();

    Xapian::Document doc;
    for (unsigned i = 0; i < num_values; ++i) {
	doc.add_value(i, "moo");
    }

    CPUTimer timer;

    db.add_document(doc);
    db.commit();

    return timer.get_time();
}

DEFINE_TESTCASE(bigoaddvalue1, writable) {
    // O(n*n) is bad, but O(n*log(n)) is acceptable.
    test_scalability(bigoaddvalue1_helper, 5000, O_N_LOG_N);
    return true;
}
