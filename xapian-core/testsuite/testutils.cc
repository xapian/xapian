/* testutils.cc: utilities used when writing test suites for om.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "testutils.h"

#include <vector>

using std::cout;
using std::endl;

std::ostream &
operator<<(std::ostream &os, const std::vector<unsigned int> &ints)
{
    copy(ints.begin(), ints.end(),
	 std::ostream_iterator<unsigned int>(os, ", "));
    return os;
}

// ######################################################################
// Useful comparison operators

bool
mset_range_is_same(const OmMSet &mset1, unsigned int first1,
		   const OmMSet &mset2, unsigned int first2,
		   unsigned int count)
{
    TEST_AND_EXPLAIN(mset1.items.size() >= first1 + count - 1,
		     "mset1 is too small: expected at least " <<
		     (first1 + count - 1) << " items." << endl);

    TEST_AND_EXPLAIN(mset2.items.size() >= first2 + count - 1,
		     "mset2 is too small: expected at least " <<
		     (first2 + count - 1) << " items." << endl);

    for (unsigned int i=0; i<count; ++i) {
	if ((mset1.items[first1+i].wt != mset2.items[first2+i].wt) ||
	    (mset1.items[first1+i].did != mset2.items[first2+i].did)) {
	    return false;
	}
    }
    return true;
}

bool
mset_range_is_same_weights(const OmMSet &mset1, unsigned int first1,
			   const OmMSet &mset2, unsigned int first2,
			   unsigned int count)
{
    TEST_AND_EXPLAIN(mset1.items.size() >= first1 + count - 1,
		     "mset1 is too small: expected at least " <<
		     (first1 + count - 1) << " items." << endl);

    TEST_AND_EXPLAIN(mset2.items.size() >= first2 + count - 1,
		     "mset2 is too small: expected at least " <<
		     (first2 + count - 1) << " items." << endl);


    for (unsigned int i=0; i<count; ++i) {
	if (mset1.items[first1+i].wt != mset2.items[first2+i].wt) {
	    return false;
	}
    }
    return true;
}

bool operator==(const OmMSet &first, const OmMSet &second)
{
    if ((first.mbound != second.mbound) ||
	(first.max_possible != second.max_possible) ||
	(first.items.size() != second.items.size())) {
	return false;
    }
    if(first.items.size() == 0) return true;
    return mset_range_is_same(first, 0, second, 0, first.items.size());
}

static void
mset_expect_order_(const OmMSet &A, bool beginning,
		   om_docid d1, om_docid d2, om_docid d3, om_docid d4,
		   om_docid d5, om_docid d6, om_docid d7, om_docid d8,
		   om_docid d9, om_docid d10, om_docid d11, om_docid d12)
{
    vector<om_docid> expect;
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
	TEST_AND_EXPLAIN(A.items.size() >= expect.size(),
			 "Mset is of wrong size (" << A.items.size()
			 << " < " << expect.size() << "):\n"
			 << "Full mset was: " << A << endl
			 << "Expected order to start: {" << expect << "}\n");
    } else {
	TEST_AND_EXPLAIN(A.items.size() == expect.size(),
			 "Mset is of wrong size (" << A.items.size()
			 << " != " << expect.size() << "):\n"
			 << "Full mset was: " << A << endl
			 << "Expected order: {" << expect << "}\n");
    }

    for (size_t i = 0; i < expect.size(); i++) {
	TEST_AND_EXPLAIN(A.items[i].did == expect[i],
			 "Mset didn't contain expected result:\n"
			 << "Item " << i << " was " << A.items[i].did
			 << ", expected " << expect[i] << endl
			 << "Full mset was: " << A << endl
			 << "Expected: {" << expect << "}\n");
    }
}

void
mset_expect_order_begins(const OmMSet &A,
			 om_docid d1, om_docid d2, om_docid d3, om_docid d4,
			 om_docid d5, om_docid d6, om_docid d7, om_docid d8,
			 om_docid d9, om_docid d10, om_docid d11, om_docid d12)
{
    mset_expect_order_(A, true, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12);
}

void
mset_expect_order(const OmMSet &A,
		  om_docid d1, om_docid d2, om_docid d3, om_docid d4,
		  om_docid d5, om_docid d6, om_docid d7, om_docid d8,
		  om_docid d9, om_docid d10, om_docid d11, om_docid d12)
{
    mset_expect_order_(A, false, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12);
}
