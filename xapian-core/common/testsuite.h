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

/** Class which is thrown when a test case fails.
 *  This class contains a message, which is displayed to the user if
 *  the verbose flag is set, which should give helpful information as
 *  to the cause of the failure.
 */
class TestFailure {
    public:
	TestFailure(string message_ = "") : message(message_) {}
	~TestFailure() {}
	string message;

//	ostream & operator<<(ostream &os, const OmMSetItem &mitem);
};

/** Macro used to build a TestFailure object and throw it.
 */
// Don't bracket a, because it may have <<'s in it
//#define FAIL_TEST(a) { TestFailure testfail; testfail << a; throw testfail; }
#define FAIL_TEST(a) { TestFailure testfail; \
                       if (verbose) { cout << a;} \
		       throw testfail; }

/// Type for a test function.
typedef bool (*test_func)();

/// Structure holding a description of a test.
struct test_desc {
    /// The name of the test.
    const char *name;

    /// The function to run to perform the test.
    test_func run;
};

// The global verbose flag.  Individual tests may need to get at it.
extern bool verbose;

/// The test driver.  This class takes care of running the tests.
class test_driver {
    public:
	/** A structure used to report the summary of tests passed
	 *  and failed.
	 */
	struct result {
	    /// The number of tests which succeeded.
	    unsigned int succeeded;

	    /// The number of tests which failed.
	    unsigned int failed;
	};

	/** main() replacement.  Standard OM test suite programs
	 *  can have a one-liner main() which calls test_driver::main()
	 *  with argc, argv, and the test array.  This function handles
	 *  all the standard argument parsing and running of the tests.
	 *  main() should return the result of test_driver::main() to
	 *  the system.
	 *
	 *  @param  argc	The argument count passed into ::main()
	 *  @param  argv	The argument list passed into ::main()
	 *  @param  tests	The array of tests to run.
	 *  @param  results	An optional pointer used for returning
	 *                      the results summary.
	 */
	static int main(int argc,
			char *argv[],
			const test_desc *tests,
			test_driver::result *summary = 0);

	/** The constructor, which sets up the test driver.
	 *
	 *  @param tests The zero-terminated array of tests to run.
	 */
	test_driver(const test_desc *tests_);

	/** Run all the tests supplied and return the results
	 */
	result run_tests();

	/** Similar to run_tests() but only run the named test.
	 *
	 *  @param testname The name of the test(s) to run.
	 */
	result run_test(const string &testname);

	/** If set, this will cause the testsuite to stop executing further
	 *  tests if any fail.
	 */
	void set_abort_on_error(bool aoe_);

	/** If set to true, the testsuite will produce no output whatsoever.
	 */
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
	 *  @param name  If non-empty, the name of the test(s) to run.
	 *               If empty, all tests will be run.
	 */
	result do_run_tests(const string &name);
	
	// abort tests at the first failure
	bool abort_on_error;

	// the default stream to output to
	ostream out;

	// the list of tests to run.
	const test_desc *tests;
};

inline void test_driver::set_abort_on_error(bool aoe_)
{
    abort_on_error = aoe_;
}

#ifndef STRINGIZE
/** STRINGIZE converts a piece of code to a string, so it can be displayed.
 * 
 *  The 2nd level of the stringize definition here is not needed for the use we
 *  put this to in this file (since we always use it within a macro here) but
 *  is required in general  (#N doesn't work outside a macro definition)
 */
#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N
#endif

#ifndef TESTCASE_LOCN
/// Display the location at which a testcase occured, with an explanation
#define TESTCASE_LOCN(a) __FILE__":"STRINGIZE(__LINE__)": "STRINGIZE(a)
#endif

/** Test a condition, and display the test with an extra explanation if
 *  the condition fails.
 *  NB: uses an else clause to avoid dangling else damage
 */
#define TEST_AND_EXPLAIN(a, b) if (a) { } else \
                             FAIL_TEST(TESTCASE_LOCN(a) << endl << \
				       b << endl)

/// Test a condition, without an additional explanation for failure.
#define TEST(a) TEST_AND_EXPLAIN(a, "")

/// Test for equality of two things.
#define TEST_EQUAL(a, b) TEST_AND_EXPLAIN(((a) == (b)), \
	"Expected `"STRINGIZE(a)"' and `"STRINGIZE(b)"' to be equal:" \
	" were " << (a) << " and " << (b) << endl)

/// Test for non-equality of two things.
#define TEST_NOT_EQUAL(a, b) TEST_AND_EXPLAIN(((a) != (b)), \
	"Expected `"STRINGIZE(a)"' and `"STRINGIZE(b)"' not to be equal:" \
	" were " << (a) << " and " << (b) << endl)


#endif  // OM_HGUARD_TESTSUITE_H
