/* testsuite.h - a test suite engine
 *
 * ----START-LICENCE----
 * Copyright 2000 Dialog Corporation
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

#ifndef OM_HGUARD_TESTSUITE_H
#define OM_HGUARD_TESTSUITE_H

#include <iostream>
#include <string>
#include "om/omerror.h"

typedef bool (*test_func)();

struct test_desc {
  char *name;
  test_func run;
};

class test_driver {
    public:
	test_driver();

	struct result {
	    unsigned int succeeded;
	    unsigned int failed;
	};

	/** Run all the tests supplied and return the results
	 *
	 *  @param tests The zero-terminated array of tests to run.
	 */
	result run_tests(const test_desc *tests);

	/** Similar to run_tests() but only run the named test.
	 *
	 *  @param tests The zero-terminated array of tests to run
	 *  @param testname The name of the test(s) to run.
	 */
	result run_test(const test_desc *tests, const string &testname);

	void set_abort_on_error(bool aoe_);
	void set_quiet(bool quiet_);
    private:
	/** Runs the test function and returns its result.  It will
	 *  also trap exceptions and some memory leaks and force a
	 *  failure in those cases.
	 *
	 *  @param test A description of the test to run.
	 */
	bool runtest(const test_desc *test);

	/** The implementation used by both run_test and run_tests.
	 *  it runs test(s) (with runtest()), prints out messages for
	 *  the user, and tracks the successes and failures.
	 *
	 *  @param tests The list of tests
	 *  @param name  If non-empty, the name of the test(s) to run.
	 *               If empty, all tests will be run.
	 */
	result do_run_tests(const test_desc *tests, const string &name);
	
	// abort tests at the first failure
	bool abort_on_error;

	// the default stream to output to
	ostream out;
};

inline void test_driver::set_abort_on_error(bool aoe_)
{
    abort_on_error = aoe_;
}

#endif  // OM_HGUARD_TESTSUITE_H
