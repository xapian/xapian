/** @file
 * @brief a generic test suite engine
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005,2006,2007,2008,2009,2013,2015,2016,2018 Olly Betts
 * Copyright 2007 Richard Boulton
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

#ifndef OM_HGUARD_TESTSUITE_H
#define OM_HGUARD_TESTSUITE_H

#include "noreturn.h"

#ifndef XAPIAN_UNITTEST
# include "output.h"
# define UNITTEST_CHECK_EXCEPTION
#endif

#include "stringutils.h" // For STRINGIZE().

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <cfloat> // For DBL_DIG.

/** Class which is thrown when a test case fails.
 */
class TestFail { };

/** Class which is thrown when a test case is to be skipped.
 *
 *  This happens when something can't be tested for some reason, but
 *  that reason isn't grounds for causing the test to fail.
 */
class TestSkip { };

/// Helper macro.
#define THROW_TEST_(EXCEPTION, MSG) \
    do { \
	if (verbose) { \
	    tout << __FILE__ ":" STRINGIZE(__LINE__) ": " << MSG << '\n'; \
	} \
	throw EXCEPTION(); \
    } while (0)

/** Fail the current testcase with message MSG.
 *
 *  MSG is written to an std::ostream and so can contain <<.
 */
#define FAIL_TEST(MSG) THROW_TEST_(TestFail, MSG)

/** Skip the current testcase with message MSG.
 *
 *  MSG is written to an std::ostream and so can contain <<.
 */
#define SKIP_TEST(MSG) THROW_TEST_(TestSkip, MSG)

/// Structure holding a description of a test.
struct test_desc {
    /// The name of the test.
    const char *name;

    /// The function to run to perform the test.
    void (*run)();
};

/// The global verbose flag.
//
//  If verbose is non-zero, then the test harness will display diagnostic output
//  for tests which fail or skip.  If it is > 1, then the diagnostic output will
//  also be displayed for tests which pass.  Individual tests may use this flag
//  to avoid needless generation of diagnostic output in cases when it's
//  expensive.
extern int verbose;

/** Set to a string explanation for testcases expected to fail. */
extern const char* expected_failure;

/// The exception type we were expecting in TEST_EXCEPTION.
//  Used to detect if such an exception was mishandled by the
//  compiler/runtime.
extern const char * expected_exception;

/** The output stream.  Data written to this stream will only appear
 *  when a test fails.
 */
extern std::ostringstream tout;

/// The test driver.  This class takes care of running the tests.
class test_driver {
    /// Write out anything in tout and clear it.
    void write_and_clear_tout();

  public:
    /** A structure used to report the summary of tests passed
     *  and failed.
     */
    struct result {
	/// The number of tests which succeeded.
	unsigned int succeeded = 0;

	/// The number of tests which failed.
	unsigned int failed = 0;

	/// The number of tests which were skipped.
	unsigned int skipped = 0;

	/** Number of tests with result XFAIL.
	 *
	 *  I.e. tests which were expected to fail and did.
	 */
	unsigned int xfailed = 0;

	/** Number of tests with result XFAIL.
	 *
	 *  I.e. tests which were expected to fail but passed.
	 */
	unsigned int xpassed = 0;

	result & operator+=(const result & o) {
	    succeeded += o.succeeded;
	    failed += o.failed;
	    skipped += o.skipped;
	    xfailed += o.xfailed;
	    xpassed += o.xpassed;
	    return *this;
	}

	void reset() {
	    succeeded = 0;
	    failed = 0;
	    skipped = 0;
	    xfailed = 0;
	    xpassed = 0;
	}
    };

    /** Add a test-specific command line option.
     *
     *  The recognised option will be described as:
     *
     *   -<s> <l>
     *
     *  And any value set will be put into arg.
     */
    static void add_command_line_option(const std::string &l, char s,
					std::string * arg);

    /** Parse the command line arguments.
     *
     *  @param  argc	The argument count passed into ::main()
     *  @param  argv	The argument list passed into ::main()
     */
    static void parse_command_line(int argc, char **argv);

    XAPIAN_NORETURN(static void usage());

    static int run(const test_desc *tests);

    /** The constructor, which sets up the test driver.
     *
     *  @param tests The zero-terminated array of tests to run.
     */
    test_driver(const test_desc *tests_);

    /** Run all the tests supplied and return the results
     */
    result run_tests();

    /** Run the tests in the list and return the results
     */
    result run_tests(std::vector<std::string>::const_iterator b,
		     std::vector<std::string>::const_iterator e);

    /** Read srcdir from environment and if not present, make a valiant
     *  attempt to guess a value
     */
    static std::string get_srcdir();

    // Running subtotal for current backend.
    static result subtotal;

    // Running total for the whole test run.
    static result total;

    /// Print summary of tests passed, failed, and skipped.
    static void report(const test_driver::result &r, const std::string &desc);

  private:
    /** Prevent copying */
    test_driver(const test_driver &);
    test_driver & operator=(const test_driver &);

    enum test_result {
	XPASS = 3, XFAIL = 2, PASS = 1, FAIL = 0, SKIP = -1
    };

    static std::map<int, std::string *> short_opts;

    static std::string opt_help;

    static std::vector<std::string> test_names;

    /** Runs the test function and returns its result.  It will
     *  also trap exceptions and some memory leaks and force a
     *  failure in those cases.
     *
     *  @param test A description of the test to run.
     */
    test_result runtest(const test_desc *test);

    /** The implementation used by run_tests.
     *  it runs test(s) (with runtest()), prints out messages for
     *  the user, and tracks the successes and failures.
     *
     *  @param b, e  If b != e, a vector of the test(s) to run.
     *               If b == e, all tests will be run.
     */
    result do_run_tests(std::vector<std::string>::const_iterator b,
			std::vector<std::string>::const_iterator e);

    // abort tests at the first failure
    static bool abort_on_error;

    // the default stream to output to
    std::ostream out;

    // the list of tests to run.
    const test_desc *tests;

    // how many test runs we've done - no summary if just one run
    static int runs;

    // program name
    static std::string argv0;

    // strings to use for colouring - empty if output isn't a tty
    static std::string col_red, col_green, col_yellow, col_reset;

    // use \r to not advance a line when a test passes (this only
    // really makes sense if the output is a tty)
    static bool use_cr;
};

/** Test a condition, and display the test with an extra explanation if
 *  the condition fails.
 *  NB: wrapped in do { ... } while (0) so a trailing ';' works correctly.
 */
#define TEST_AND_EXPLAIN(a, b) do {\
	bool test_and_explain_fail_ = !(a);\
	UNITTEST_CHECK_EXCEPTION\
	if (test_and_explain_fail_)\
	    FAIL_TEST(STRINGIZE(a) << '\n' << b << '\n');\
    } while (0)

/// Test a condition, without an additional explanation for failure.
#define TEST(a) TEST_AND_EXPLAIN(a, "")

/// Test for equality of two things.
#define TEST_EQUAL(a, b) TEST_AND_EXPLAIN(((a) == (b)), \
	"Expected '" STRINGIZE(a) "' and '" STRINGIZE(b) "' to be equal:" \
	" were " << (a) << " and " << (b))

/** Test for equality of two strings.
 *
 *  If they aren't equal, show each on a separate line so the difference can
 *  be seen clearly.
 */
#define TEST_STRINGS_EQUAL(a, b) TEST_AND_EXPLAIN(((a) == (b)), \
	"Expected " STRINGIZE(a) " and " STRINGIZE(b) " to be equal, were:\n\"" \
	<< (a) << "\"\n\"" << (b) << '"')

/// Helper function for TEST_EQUAL_DOUBLE macro.
extern bool TEST_EQUAL_DOUBLE_(double a, double b);

/// Test two doubles for near equality.
#define TEST_EQUAL_DOUBLE(a, b) TEST_AND_EXPLAIN(TEST_EQUAL_DOUBLE_((a), (b)), \
	"Expected '" STRINGIZE(a) "' and '" STRINGIZE(b) "' to be (nearly) equal:" \
	" were " << setprecision(DBL_DIG) << (a) << " and " << (b) << ")" << setprecision(6))

/// Test two doubles for non-near-equality.
#define TEST_NOT_EQUAL_DOUBLE(a, b) TEST_AND_EXPLAIN(!TEST_EQUAL_DOUBLE_((a), (b)), \
	"Expected '" STRINGIZE(a) "' and '" STRINGIZE(b) "' not to be (nearly) equal:" \
	" were " << setprecision(DBL_DIG) << (a) << " and " << (b) << ")" << setprecision(6))

/// Test for non-equality of two things.
#define TEST_NOT_EQUAL(a, b) TEST_AND_EXPLAIN(((a) != (b)), \
	"Expected '" STRINGIZE(a) "' and '" STRINGIZE(b) "' not to be equal:" \
	" were " << (a) << " and " << (b))

#define DEFINE_TESTCASE(S,COND) void test_##S()

// Newer test macros:
#include "testmacros.h"

/** Mark a testcase as expected to fail.
 *
 *  @param msg	An static string explaining why the testcase is expected to
 *		fail.  Must not be NULL.
 *
 *  This is intended to be used temporarily to mark tests for known bugs before
 *  the bugs are fixed.  If the test fails, the result will be shown as "XFAIL"
 *  and this won't cause the test run to fail.  However, if a test marked in
 *  this way actually passed, the result will be shown as "XPASS" and the test
 *  run *will* fail.  (So XFAIL is explicitly not suitable for marking "flaky"
 *  testcases - please fix flaky testcases rather than trying to find a way to
 *  mark them as flaky!)
 *
 *  This macro should be used inside the testcase code.  It can be used inside
 *  a conditional if the testcase is only expected to fail in certain situations
 *  (for example, only for some backends) - it only has an effect if it is
 *  actually executed.
 */
inline void XFAIL(const char* msg) {
    expected_failure = msg;
}

#endif // OM_HGUARD_TESTSUITE_H
