/** @file api_valuestream.cc
 * @brief Tests of valuestream functionality.
 */
/* Copyright (C) 2008 Olly Betts
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

#include "api_valuestream.h"

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

/// Feature test simple valuestream iteration.
DEFINE_TESTCASE(valuestream1, backend) {
    Xapian::Database db = get_database("apitest_simpledata");

    for (Xapian::valueno slot = 0; slot < 15; ++slot) {
	tout << "testing valuestream iteration for slot " << slot << endl;
	Xapian::ValueIterator it = db.valuestream_begin(slot);
	while (it != db.valuestream_end(slot)) {
	    TEST_EQUAL(it.get_valueno(), slot);
	    string value = *it;
	    Xapian::docid did = it.get_docid();

	    Xapian::Document doc = db.get_document(did);
	    TEST_EQUAL(doc.get_value(slot), value);

	    ++it;
	}
    }
 
    return true;
}
