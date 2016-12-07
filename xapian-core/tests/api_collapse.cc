/** @file api_collapse.cc
 * @brief Test collapsing during the match.
 */
/* Copyright (C) 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_collapse.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

/// Simple test of collapsing with collapse_max > 1.
DEFINE_TESTCASE(collapsekey5, backend) {
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    // "this" matches all documents.
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet full_mset = enquire.get_mset(0, db.get_doccount());

    for (Xapian::valueno slot = 0; slot < 10; ++slot) {
	map<string, Xapian::doccount> tally;
	for (Xapian::docid did = 1; did <= db.get_doccount(); ++did) {
	    ++tally[db.get_document(did).get_value(slot)];
	}

	for (Xapian::doccount cmax = db.get_doccount() + 1; cmax > 0; --cmax) {
	    tout << "Collapsing on slot " << slot << " max " << cmax << endl;
	    enquire.set_collapse_key(slot, cmax);
	    Xapian::MSet mset = enquire.get_mset(0, full_mset.size());

	    // Check the collapse MSet size is as expected.
	    Xapian::doccount expect_size = 0;
	    map<string, Xapian::doccount>::const_iterator i;
	    for (i = tally.begin(); i != tally.end(); ++i) {
		if (i->first.empty() || i->second <= cmax) {
		    expect_size += i->second;
		} else {
		    expect_size += cmax;
		}
	    }
	    TEST_EQUAL(mset.size(), expect_size);

	    // Check that the right number of documents with each collapse key
	    // value are left after collapsing.
	    map<string, Xapian::doccount> seen;
	    for (Xapian::MSetIterator j = mset.begin(); j != mset.end(); ++j) {
		const string & key = j.get_collapse_key();
		TEST(tally.find(key) != tally.end());
		++seen[key];
	    }
	    for (i = tally.begin(); i != tally.end(); ++i) {
		if (i->first.empty() || i->second <= cmax) {
		    TEST_EQUAL(seen[i->first], i->second);
		} else {
		    TEST_EQUAL(seen[i->first], cmax);
		}
	    }
	}
    }

    return true;
}
