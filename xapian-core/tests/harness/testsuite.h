/* testsuite.h: a generic test suite engine
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005,2006,2007,2008,2009,2013,2015 Olly Betts
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

#include "output.h"

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

/** Macro used to build a TestFail object and throw it.
 */
// Don't bracket a, because it may have <<'s in it
#define FAIL_TEST(a) do { if (verbose) { tout << a << '\n'; } \
			  throw TestFail(); } while (0)

/** Macro used to build a TestSkip object and throw it.
 */
// Don't bracket a, because it may have <<'s in it
#define SKIP_TEST(a) do { if (verbose) { tout << a << '\n'; } \
			  throw TestSkip(); } while (0)

/// Type for a test function.
typedef bool (*test_func)();

/// Structure holding a description of a test.
struct test_desc {
    /// The name of the test.
    const char *name;

    /// The function to run to perform the test.
    test_func run;
};

/// The global verbose flag.
//
//  If verbose is non-zero, then the test harness will display diagnostic output
//  for tests which fail or skip.  If it is > 1, then the diagnostic output will
//  also be displayed for tests which pass.  Individual tests may use this flag
//  to avoid needless generation of diagnostic output in cases when it's
//  expensive.
extern int verbose;

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
	    unsigned int succeeded;

	    /// The number of tests which failed.
	    unsigned int failed;

	    /// The number of tests which were skipped
	    unsigned int skipped;

	    result() : succeeded(0), failed(0), skipped(0) { }

	    result & operator+=(const result & o) {
		succeeded += o.succeeded;
		failed += o.failed;
		skipped += o.skipped;
		return *this;
	    }

	    void reset() {
		succeeded = 0;
		failed = 0;
		skipped = 0;
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
	test_driver & operator = (const test_driver &);

	typedef enum { PASS = 1, FAIL = 0, SKIP = -1 } test_result;

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

/// Display the location at which a testcase occurred, with an explanation.
#define TESTCASE_LOCN(a) __FILE__":" STRINGIZE(__LINE__) ": " STRINGIZE(a)

/** Test a condition, and display the test with an extra explanation if
 *  the condition fails.
 *  NB: wrapped in do { ... } while (0) so a trailing ';' works correctly.
 */
#define TEST_AND_EXPLAIN(a, b) do {\
	if (!(a)) FAIL_TEST(TESTCASE_LOCN(a) << std::endl << b << std::endl);\
    } while (0)

/// Test a condition, without an additional explanation for failure.
#define TEST(a) TEST_AND_EXPLAIN(a, "")

/// Test for equality of two things.
#define TEST_EQUAL(a, b) TEST_AND_EXPLAIN(((a) == (b)), \
	"Expected `" STRINGIZE(a) "' and `" STRINGIZE(b) "' to be equal:" \
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
	"Expected `" STRINGIZE(a) "' and `" STRINGIZE(b) "' to be (nearly) equal:" \
	" were " << setprecision(DBL_DIG) << (a) << " and " << (b) << ")" << setprecision(6))

/// Test two doubles for non-near-equality.
#define TEST_NOT_EQUAL_DOUBLE(a, b) TEST_AND_EXPLAIN(!TEST_EQUAL_DOUBLE_((a), (b)), \
	"Expected `" STRINGIZE(a) "' and `" STRINGIZE(b) "' not to be (nearly) equal:" \
	" were " << setprecision(DBL_DIG) << (a) << " and " << (b) << ")" << setprecision(6))

/// Test for non-equality of two things.
#define TEST_NOT_EQUAL(a, b) TEST_AND_EXPLAIN(((a) != (b)), \
	"Expected `" STRINGIZE(a) "' and `" STRINGIZE(b) "' not to be equal:" \
	" were " << (a) << " and " << (b))

#define DEFINE_TESTCASE(S,COND) bool test_##S()

// Newer test macros:
#include "testmacros.h"

#endif // OM_HGUARD_TESTSUITE_H
