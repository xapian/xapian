/* testutils.cc: Xapian-specific test helper functions.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2007,2008,2009,2015 Olly Betts
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

#include "testutils.h"

#include "testsuite.h"

#include <fstream>
#include <vector>

using namespace std;

ostream &
operator<<(ostream &os, const vector<Xapian::docid> &ints)
{
    copy(ints.begin(), ints.end(),
	 ostream_iterator<Xapian::docid>(os, ", "));
    return os;
}

// ######################################################################
// Useful comparison operators

bool
mset_range_is_same(const Xapian::MSet &mset1, unsigned int first1,
		   const Xapian::MSet &mset2, unsigned int first2,
		   unsigned int count)
{
    TEST_AND_EXPLAIN(mset1.size() >= first1 + count - 1,
		     "mset1 is too small: expected at least " <<
		     (first1 + count - 1) << " items, got " <<
		     mset1.size() << ".");

    TEST_AND_EXPLAIN(mset2.size() >= first2 + count - 1,
		     "mset2 is too small: expected at least " <<
		     (first2 + count - 1) << " items, got " <<
		     mset2.size() << ".");

    Xapian::MSetIterator i = mset1[first1];
    Xapian::MSetIterator j = mset2[first2];

    for (unsigned int l = 0; l < count; ++l) {
	if (*i != *j) {
	    tout << "docids differ at item " << (l + 1) << " in range: "
		    << *i << " != " << *j << "\n";
	    return false;
	}
	// FIXME: don't use internal macro here...
	if (!TEST_EQUAL_DOUBLE_(i.get_weight(), j.get_weight())) {
	    tout << "weights differ at item " << (l + 1) << " in range: "
		    << i.get_weight() << " != " << j.get_weight() << "\n";
	    return false;
	}
	++i;
	++j;
    }
    return true;
}

bool
mset_range_is_same_weights(const Xapian::MSet &mset1, unsigned int first1,
			   const Xapian::MSet &mset2, unsigned int first2,
			   unsigned int count)
{
    TEST_AND_EXPLAIN(mset1.size() >= first1 + count - 1,
		     "mset1 is too small: expected at least " <<
		     (first1 + count - 1) << " items, got " <<
		     mset1.size() << ".");

    TEST_AND_EXPLAIN(mset2.size() >= first2 + count - 1,
		     "mset2 is too small: expected at least " <<
		     (first2 + count - 1) << " items, got " <<
		     mset2.size() << ".");

    Xapian::MSetIterator i = mset1[first1];
    Xapian::MSetIterator j = mset2[first2];

    for (unsigned int l = 0; l < count; ++l) {
	// FIXME: don't use internal macro here...
	if (!TEST_EQUAL_DOUBLE_(i.get_weight(), j.get_weight())) {
	    tout << "weights differ at item " << (l + 1) << " in range: "
		    << i.get_weight() << " != " << j.get_weight() << "\n";
	    return false;
	}
	++i;
	++j;
    }
    return true;
}

bool operator==(const Xapian::MSet &first, const Xapian::MSet &second)
{
    if ((first.get_matches_lower_bound() != second.get_matches_lower_bound()) ||
	(first.get_matches_upper_bound() != second.get_matches_upper_bound()) ||
	(first.get_matches_estimated() != second.get_matches_estimated()) ||
	(first.get_max_possible() != second.get_max_possible()) ||
	(first.size() != second.size())) {
	return false;
    }
    if (first.empty()) return true;
    return mset_range_is_same(first, 0, second, 0, first.size());
}

static void
mset_expect_order_(const Xapian::MSet &A, bool beginning,
		   Xapian::docid d1, Xapian::docid d2, Xapian::docid d3, Xapian::docid d4,
		   Xapian::docid d5, Xapian::docid d6, Xapian::docid d7, Xapian::docid d8,
		   Xapian::docid d9, Xapian::docid d10, Xapian::docid d11, Xapian::docid d12)
{
    vector<Xapian::docid> expect;
    if (d1) {
	expect.push_back(d1);
	if (d2) {
	    expect.push_back(d2);
	    if (d3) {
		expect.push_back(d3);
		if (d4) {
		    expect.push_back(d4);
		    if (d5) {
			expect.push_back(d5);
			if (d6) {
			    expect.push_back(d6);
			    if (d7) {
				expect.push_back(d7);
				if (d8) {
				    expect.push_back(d8);
				    if (d9) {
					expect.push_back(d9);
					if (d10) {
					    expect.push_back(d10);
					    if (d11) {
						expect.push_back(d11);
						if (d12) {
						    expect.push_back(d12);
						}
					    }
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
    // Wheeee!

    if (beginning) {
	TEST_AND_EXPLAIN(A.size() >= expect.size(),
			 "Mset is of wrong size (" << A.size()
			 << " < " << expect.size() << "):\n"
			 << "Full mset was: " << A << endl
			 << "Expected order to start: {" << expect << "}");
    } else {
	TEST_AND_EXPLAIN(A.size() == expect.size(),
			 "Mset is of wrong size (" << A.size()
			 << " != " << expect.size() << "):\n"
			 << "Full mset was: " << A << endl
			 << "Expected order: {" << expect << "}");
    }

    Xapian::MSetIterator j = A.begin();
    for (size_t i = 0; i < expect.size(); i++, j++) {
	TEST_AND_EXPLAIN(*j == expect[i],
			 "Mset didn't contain expected result:\n"
			 << "Item " << i << " was " << *j
			 << ", expected " << expect[i] << endl
			 << "Full mset was: " << A << endl
			 << "Expected: {" << expect << "}");	
    }
}

void
mset_expect_order(const Xapian::MSet &A,
		  Xapian::docid d1, Xapian::docid d2, Xapian::docid d3, Xapian::docid d4,
		  Xapian::docid d5, Xapian::docid d6, Xapian::docid d7, Xapian::docid d8,
		  Xapian::docid d9, Xapian::docid d10, Xapian::docid d11, Xapian::docid d12)
{
    mset_expect_order_(A, false, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12);
}

void
test_mset_order_equal(const Xapian::MSet &mset1, const Xapian::MSet &mset2)
{
    TEST_AND_EXPLAIN(mset1.size() == mset2.size(),
		     "Msets not the same size - "
		     << mset1.size() << " != " << mset2.size());
    Xapian::MSetIterator i = mset1.begin();
    Xapian::MSetIterator j = mset2.begin();
    for (; i != mset1.end(); i++, j++) {
	TEST_AND_EXPLAIN(*i == *j,
			 "Msets have different contents -\n" <<
			 mset1 << "\n !=\n" << mset2);
    }
}
