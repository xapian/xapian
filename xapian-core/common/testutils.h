/* testutils.h - utilities used when writing test suites for om.
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

#ifndef OM_HGUARD_TESTUTILS_H
#define OM_HGUARD_TESTUTILS_H

#include "testsuite.h"
#include <om/om.h>

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


// ######################################################################
// Useful test macros

/// Check size of mset is as expected
#define TEST_MSET_SIZE(a, b) TEST_AND_EXPLAIN(((a).items.size() == (b)), \
	"MSet `"STRINGIZE(a)"' is not of expected size: was `" << \
	(a).items.size() << "' expected `" << (b) << "': " << endl << \
	"Full mset was: " << endl << (a) << endl)

/// Check that a piece of code throws an expected exception
#define TEST_EXCEPTION(a,b) try {b;FAIL_TEST("Expected "#a);}catch(a &e){}

#endif  // OM_HGUARD_TESTUTILS_H
