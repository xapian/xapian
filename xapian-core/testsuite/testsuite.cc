/* testsuite.cc: a test suite engine
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#define COL_RED "\x1b[1m\x1b[31m"
#define COL_GREEN "\x1b[1m\x1b[32m"
#define COL_YELLOW "\x1b[1m\x1b[33m"
#define COL_RESET "\x1b[0m"

#include "config.h"
#include <iostream>

#ifdef HAVE_STREAMBUF
#include <streambuf>
#else // HAVE_STREAMBUF
#include <streambuf.h>
#endif // HAVE_STREAMBUF

#include <set>

#include <string>
#include <cstdio>
#include <dlfcn.h>

#include <stdlib.h>

#include "getopt.h"

#include <unistd.h> // for chdir

#include <setjmp.h>
#include <signal.h>

#include <exception>

#include "om/omerror.h"
#include "testsuite.h"
#include "omdebug.h"

#ifdef HAVE_STREAMBUF
class null_streambuf : public std::streambuf {
};
#else // HAVE_STREAMBUF
class null_streambuf : public streambuf {
};
#endif // HAVE_STREAMBUF

/// A null stream buffer which we can redirect output to.
static null_streambuf nullsb;

/// The global verbose flag.
bool verbose;

/// The debug printing stream
om_ostringstream tout;

int test_driver::runs = 0;
test_driver::result test_driver::total = {0, 0, 0};
std::string test_driver::argv0 = "";

void
test_driver::set_quiet(bool quiet_)
{
    if (quiet_) {
	out.rdbuf(&nullsb);
    } else {
	out.rdbuf(std::cout.rdbuf());
    }
}

std::string
test_driver::get_srcdir(const std::string & argv0)
{
    char *p = getenv("srcdir");
    if (p != NULL) return std::string(p);

    std::string srcdir = argv0;
    // default srcdir to everything leading up to the last "/" on argv0
    std::string::size_type i = srcdir.find_last_of('/');
    std::string srcfile;
    if (i != std::string::npos) {
	srcfile = srcdir.substr(i + 1);
	srcdir.erase(i);
    } else {
	// default to current directory - probably won't work if libtool
	// is involved as the true executable is usually in .libs
	srcfile = srcdir;
	srcdir = ".";
    }
    srcfile += ".cc";
    // deal with libtool
    if (srcfile[0] == 'l' && srcfile[1] == 't' && srcfile[2] == '-')
        srcfile.erase(0, 3);
    // sanity check
    if (!file_exists(srcdir + "/" + srcfile)) {
	// try the likely subdirectories and chdir to them if we find one
	// with a likely looking source file in - some tests need to run
	// from the current directory
	srcdir = '.';
	if (file_exists("tests/" + srcfile)) {
	    chdir("tests");
	} else if (file_exists("netprogs/" + srcfile)) {
	    chdir("netprogs");
	} else {
	    std::cout << argv0
		<< ": srcdir not in the environment and I can't guess it!"
		<< std::endl;
	    exit(1);
	}
    }
    return srcdir;
}

test_driver::test_driver(const test_desc *tests_)
	: abort_on_error(false),
	  out(std::cout.rdbuf()),
	  tests(tests_)
{
}

static jmp_buf jb;
static int signum = 0;

static void
handle_sig(int signum_)
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
#ifdef SIGSTKFLT
    signal(SIGSTKFLT, SIG_DFL);
#endif
    signum = signum_;
    longjmp(jb, 1);
}

//  A wrapper around the tests to trap exceptions,
//  and avoid having to catch them in every test function.
//  If this test driver is used for anything other than
//  Xapian tests, then this ought to be provided by
//  the client, really.
bool
test_driver::runtest(const test_desc *test)
{
    bool success = true;

    // This is used to make a note of how many times we've run the test
    volatile int runcount = 0;

    while (true) {
	runcount++;
	tout.str("");
	// FIXME get snapshot with valgrind
	if (!setjmp(jb)) {
	    signal(SIGSEGV, handle_sig);
	    signal(SIGFPE, handle_sig);
	    signal(SIGILL, handle_sig);
	    signal(SIGBUS, handle_sig);
#ifdef SIGSTKFLT
	    signal(SIGSTKFLT, handle_sig);
#endif
	    try {
		success = test->run();
		if (!success) {
		    out << tout.str();
		    out << " " << COL_RED << "FAILED" << COL_RESET;
		}
	    } catch (TestFailure &fail) {
		success = false;
		out << tout.str();
		out << " " << COL_RED << "FAILED" << COL_RESET;
		if (verbose) {
		    out << fail.message << std::endl;
		}
	    } catch (TestSkip &skip) {
		out << " " << COL_YELLOW << "SKIPPED" << COL_RESET;
		if (verbose) {
		    out << skip.message << std::endl;
		}
		// Rethrow the exception to avoid success/fail
		// (caught in do_run_tests())
		throw;
	    } catch (OmError &err) {
		out << tout.str();
		out << " " << COL_RED << "OMEXCEPT" << COL_RESET;
		if (verbose) {
		    out << err.get_type() << " exception: " << err.get_msg();
		    if (!err.get_context().empty())
			out << " (context:" << err.get_context() << ")";
		    if (err.get_errno())
			out << " (errno:" << strerror(err.get_errno()) << ")";
		    out << std::endl;
		}
		success = false;
	    } catch (...) {
		out << tout.str();
		out << " " << COL_RED << "EXCEPT" << COL_RESET;
		if (verbose) {
		    out << "Unknown exception!" << std::endl;
		}
		success = false;
	    }
	    signal(SIGSEGV, SIG_DFL);
	    signal(SIGFPE, SIG_DFL);
	    signal(SIGILL, SIG_DFL);
	    signal(SIGBUS, SIG_DFL);
#ifdef SIGSTKFLT
	    signal(SIGSTKFLT, SIG_DFL);
#endif
	} else {
	    // caught signal
	    out << tout.str();
	    const char *sig = "SIGNAL";
	    switch (signum) {
		case SIGSEGV: sig = "SIGSEGV"; break;
		case SIGFPE: sig = "SIGFPE"; break;
		case SIGILL: sig = "SIGILL"; break;
		case SIGBUS: sig = "SIGBUS"; break;
#ifdef SIGSTKFLT
		case SIGSTKFLT: sig = "SIGSTKFLT"; break;
#endif
	    }
    	    out << " " << COL_RED << sig << COL_RESET;
	    success = false;
	}

	// check snapshot with valgrind
	return success; // all new allocations freed
#if 0
	if (!success || runcount >= 2) {
	    if (verbose) {
		print_alloc_differences(before, after, out);
	    }
	    out << " " << COL_RED << "LEAK" << COL_RESET;
	    return false;
	}
#endif
    }
}

test_driver::result
test_driver::run_tests(std::vector<std::string>::const_iterator b,
		       std::vector<std::string>::const_iterator e)
{
    return do_run_tests(b, e);
}

test_driver::result
test_driver::run_tests()
{
    const std::vector<std::string> blank;
    return do_run_tests(blank.begin(), blank.end());
}

test_driver::result
test_driver::do_run_tests(std::vector<std::string>::const_iterator b,
			  std::vector<std::string>::const_iterator e)
{
    std::set<std::string> m(b, e);
    bool check_name = !m.empty();

    test_driver::result result = {0, 0, 0};

    for (const test_desc *test = tests; test->name; test++) {
	bool do_this_test = !check_name;
	if (!do_this_test && m.find(test->name) != m.end())
	    do_this_test = true;
	if (!do_this_test) {
	    // if this test is "foo123" see if "foo" was listed
	    // this way "./testprog foo" can run foo1, foo2, etc.
	    std::string t = test->name;
	    std::string::size_type i;
	    i = t.find_last_not_of("0123456789") + 1;
	    if (i < t.length()) {
		t = t.substr(0, i);
		if (m.find(t) != m.end()) do_this_test = true;
	    }
	}
	if (do_this_test) {
	    out << "Running test: " << test->name << "...";
	    out.flush();
	    try {
		bool succeeded = runtest(test);
		if (succeeded) {
		    ++result.succeeded;
//		    out << " ok." << std::endl;
		    out << "\r                                                                               \r";
		} else {
		    ++result.failed;
		    out << std::endl;
		    if (abort_on_error) {
			out << "Test failed - aborting further tests." << std::endl;
			break;
		    }
		}
	    } catch (const TestSkip &e) {
		out << std::endl;
		// ignore the result of this test.
		++result.skipped;
	    }
	}
    }
    return result;
}

static void usage(char *progname)
{
    std::cerr << "Usage: " << progname
              << " [-v] [-o] [TESTNAME]..." << std::endl;
    exit(1);
}

void
test_driver::report(const test_driver::result &r, const std::string &desc)
{
    if (r.succeeded != 0 || r.failed != 0) {
	std::cout << argv0 << " " << desc << ": ";

	if (r.failed == 0)
	    std::cout << "All ";

	std::cout << COL_GREEN << r.succeeded << COL_RESET << " tests passed";

	if (r.failed != 0)
	    std::cout << ", " << COL_RED << r.failed << COL_RESET << " failed";

	if (r.skipped) {
	    std::cout << ", " << COL_YELLOW << r.skipped << COL_RESET
		<< " skipped." << std::endl;
	} else {
	    std::cout << "." << std::endl;
	}
    }
}

// call via atexit if there's more than one test run
void
report_totals()
{
    test_driver::report(test_driver::total, "total");
}

int
test_driver::main(int argc, char *argv[], const test_desc *tests)
{
    if (runs == 0) argv0 = argv[0];
    if (runs == 1) atexit(report_totals);
    runs++;

    test_driver driver(tests);

    std::vector<std::string> test_names;

    int c;
    while ((c = getopt(argc, argv, "vo")) != EOF) {
	switch (c) {
	    case 'v':
		verbose = true;
		break;
	    case 'o':
		driver.set_abort_on_error(true);
		break;
	    default:
	    	usage(argv[0]);
		return 1;
	}
    }

    while (argv[optind]) {
	test_names.push_back(std::string(argv[optind]));
	optind++;
    }

    // We need to display something before we start, or the allocation
    // made when the first debug message is displayed is (wrongly) picked
    // up on as a memory leak.
    DEBUGLINE(UNKNOWN, "Starting testsuite run.");
#ifdef MUS_DEBUG_VERBOSE
    om_debug.initialise();
#endif /* MUS_DEBUG_VERBOSE */

    test_driver::result myresult;
    myresult = driver.run_tests(test_names.begin(), test_names.end());

    report(myresult, "completed test run");

    total.succeeded += myresult.succeeded;
    total.failed += myresult.failed;
    total.skipped += myresult.skipped;

    return (bool)myresult.failed; // if 0, then everything passed
}
