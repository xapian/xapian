/* testutils.h: utilities used when writing test suites for om.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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

#ifndef OM_HGUARD_TESTUTILS_H
#define OM_HGUARD_TESTUTILS_H

#include "testsuite.h"
#include <xapian.h>

// ######################################################################
// Useful display operators

std::ostream &operator<<(std::ostream &os,
                         const std::vector<unsigned int> &ints);


// ######################################################################
// Useful comparison operators

bool
mset_range_is_same(const Xapian::MSet &mset1, unsigned int first1,
		   const Xapian::MSet &mset2, unsigned int first2,
		   unsigned int count);

bool
mset_range_is_same_weights(const Xapian::MSet &mset1, unsigned int first1,
			   const Xapian::MSet &mset2, unsigned int first2,
			   unsigned int count);

bool operator==(const Xapian::MSet &first, const Xapian::MSet &second);

inline bool operator!=(const Xapian::MSet &first, const Xapian::MSet &second)
{
    return !(first == second);
}


void
mset_expect_order(const Xapian::MSet &A,
		  Xapian::docid d1 = 0, Xapian::docid d2 = 0, Xapian::docid d3 = 0,
		  Xapian::docid d4 = 0, Xapian::docid d5 = 0, Xapian::docid d6 = 0,
		  Xapian::docid d7 = 0, Xapian::docid d8 = 0, Xapian::docid d9 = 0,
		  Xapian::docid d10 = 0, Xapian::docid d11 = 0, Xapian::docid d12 = 0);

void
mset_expect_order_begins(const Xapian::MSet &A,
			 Xapian::docid d1 = 0, Xapian::docid d2 = 0, Xapian::docid d3 = 0,
			 Xapian::docid d4 = 0, Xapian::docid d5 = 0, Xapian::docid d6 = 0,
			 Xapian::docid d7 = 0, Xapian::docid d8 = 0, Xapian::docid d9 = 0,
			 Xapian::docid d10 = 0, Xapian::docid d11 = 0, Xapian::docid d12 = 0);

bool doubles_are_equal_enough(double a, double b);

void weights_are_equal_enough(double a, double b);

void test_mset_order_equal(const Xapian::MSet &mset1, const Xapian::MSet &mset2);

// ######################################################################
// Useful test macros

/// Check size of mset is as expected
#define TEST_MSET_SIZE(a, b) TEST_AND_EXPLAIN(((a).size() == (b)), \
	"MSet `"STRINGIZE(a)"' is not of expected size: was `" << \
	(a).size() << "' expected `" << (b) << "':\n" << \
	"Full mset was:\n" << (a))

/// Check that a piece of code throws an expected exception
#define TEST_EXCEPTION(a,b) do {\
    	expected_exception = STRINGIZE(a);\
	if (strncmp(expected_exception, "Xapian::", 8) == 0)\
	    expected_exception += 8;\
	if (verbose)\
	    tout << "Expecting exception " << expected_exception << endl;\
	try {b;FAIL_TEST(TESTCASE_LOCN(Expected #a));}\
	catch(const a &e){\
	if (verbose)\
	    tout << "Caught expected " << expected_exception << " exception: "\
		 << e.get_msg() << endl;\
	}\
	expected_exception = NULL;\
	} while (0)

#endif  // OM_HGUARD_TESTUTILS_H
