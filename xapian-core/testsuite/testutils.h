/* testutils.h: utilities used when writing test suites for om.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Olly Betts
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
#include "om/om.h"

#include <typeinfo>

// ######################################################################
// Useful display operators

std::ostream &operator<<(std::ostream &os,
                         const std::vector<unsigned int> &ints);


// ######################################################################
// Useful comparison operators

bool
mset_range_is_same(const OmMSet &mset1, unsigned int first1,
		   const OmMSet &mset2, unsigned int first2,
		   unsigned int count);

bool
mset_range_is_same_weights(const OmMSet &mset1, unsigned int first1,
			   const OmMSet &mset2, unsigned int first2,
			   unsigned int count);

bool operator==(const OmMSet &first, const OmMSet &second);

inline bool operator!=(const OmMSet &first, const OmMSet &second)
{
    return !(first == second);
}


void
mset_expect_order(const OmMSet &A,
		  om_docid d1 = 0, om_docid d2 = 0, om_docid d3 = 0,
		  om_docid d4 = 0, om_docid d5 = 0, om_docid d6 = 0,
		  om_docid d7 = 0, om_docid d8 = 0, om_docid d9 = 0,
		  om_docid d10 = 0, om_docid d11 = 0, om_docid d12 = 0);

void
mset_expect_order_begins(const OmMSet &A,
			 om_docid d1 = 0, om_docid d2 = 0, om_docid d3 = 0,
			 om_docid d4 = 0, om_docid d5 = 0, om_docid d6 = 0,
			 om_docid d7 = 0, om_docid d8 = 0, om_docid d9 = 0,
			 om_docid d10 = 0, om_docid d11 = 0, om_docid d12 = 0);

bool doubles_are_equal_enough(double a, double b);

void weights_are_equal_enough(double a, double b);

void test_mset_order_equal(const OmMSet &mset1, const OmMSet &mset2);

// ######################################################################
// Useful test macros

/// Check size of mset is as expected
#define TEST_MSET_SIZE(a, b) TEST_AND_EXPLAIN(((a).size() == (b)), \
	"MSet `"STRINGIZE(a)"' is not of expected size: was `" << \
	(a).size() << "' expected `" << (b) << "':\n" << \
	"Full mset was:\n" << (a))

/// Check that a piece of code throws an expected exception
#define TEST_EXCEPTION(a,b) do {\
	try{\
	if (verbose) tout << "Expecting exception "STRINGIZE(a) << endl;\
	try {b;FAIL_TEST(TESTCASE_LOCN(Expected #a));}\
	catch(const a &e){\
	if (verbose) tout << "Caught expected "STRINGIZE(a)" exception: " << e.get_msg() << endl;\
	}}\
	catch(OmError &e){\
	const char *name = typeid(e).name();\
	if (strcmp(name, STRINGIZE(a)) == 0){\
	tout << "COMPILER EXCEPTION HANDLING IS BROKEN!!!" << endl;\
	}else{\
	if (verbose) tout << "Expected exception "STRINGIZE(a)", but got " << name << ": " << e.get_msg() << endl;\
	throw;\
	}}\
	catch(...){\
	if (verbose) tout << "Expected exception "STRINGIZE(a)", got unknown exception" << endl;\
	throw;\
	}} while (0)

#endif  // OM_HGUARD_TESTUTILS_H
