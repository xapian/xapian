/* testsuite.cc: a test suite engine
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>

#include <iostream>

#ifdef HAVE_STREAMBUF
#include <streambuf>
#else // HAVE_STREAMBUF
#include <streambuf.h>
#endif // HAVE_STREAMBUF

#include <set>

#include <stdlib.h>

#include "getopt.h"

#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#include <exception>

#include <xapian/error.h>
#include "testsuite.h"
#include "omdebug.h"
#include "utils.h"

#ifdef HAVE_VALGRIND
# include <valgrind/memcheck.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
#endif

// fd that we elsewhere tell valgrind to log to (with --logfile-fd=N)
#define LOG_FD_FOR_VG 255

using namespace std;

/// The global verbose flag.
bool verbose;

/// The exception type we were expecting in TEST_EXCEPTION
//  We use this to attempt to diagnose when the code fails to catch an
//  exception when it should (due to a compiler or runtime fault in
//  GCC 2.95 it seems)
const char * expected_exception = NULL;

/// The debug printing stream
om_ostringstream tout;

int test_driver::runs = 0;
test_driver::result test_driver::total = {0, 0, 0};
string test_driver::argv0 = "";
string test_driver::opt_help = "";
map<int, string *> test_driver::short_opts;
vector<string> test_driver::test_names;
bool test_driver::abort_on_error = false;
string test_driver::col_red, test_driver::col_green;
string test_driver::col_yellow, test_driver::col_reset;
bool test_driver::use_cr = false;

string
test_driver::get_srcdir()
{
    char *p = getenv("srcdir");
    if (p != NULL) return string(p);

    string srcdir = argv0;
    // default srcdir to everything leading up to the last "/" on argv0
    string::size_type i = srcdir.find_last_of('/');
    string srcfile;
    if (i != string::npos) {
	srcfile = srcdir.substr(i + 1);
	srcdir.erase(i);
    } else {
	// default to current directory - probably won't work if libtool
	// is involved as the true executable is usually in .libs
	srcfile = srcdir;
	srcdir = ".";
    }
    // libtool puts the real executable in .libs...
    i = srcdir.find_last_of('/');
    if (srcdir.substr(i + 1) == ".libs") {
	srcdir.erase(i);
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
	} else {
	    cout << argv0
		 << ": srcdir not in the environment and I can't guess it!"
		 << endl;
	    exit(1);
	}
    }
    return srcdir;
}

test_driver::test_driver(const test_desc *tests_)
	: out(cout.rdbuf()), tests(tests_)
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
#ifdef SIGBUS
    signal(SIGBUS, SIG_DFL);
#endif
#ifdef SIGSTKFLT
    signal(SIGSTKFLT, SIG_DFL);
#endif
    signum = signum_;
    longjmp(jb, 1);
}

class SignalRedirector {
  private:
    bool active;
  public:
    SignalRedirector() : active(false) { }
    void activate() {
	active = true;
	signal(SIGSEGV, handle_sig);
	signal(SIGFPE, handle_sig);
	signal(SIGILL, handle_sig);
#ifdef SIGBUS
	signal(SIGBUS, handle_sig);
#endif
#ifdef SIGSTKFLT
	signal(SIGSTKFLT, handle_sig);
#endif
    }
    ~SignalRedirector() {
	if (active) {
	    signal(SIGSEGV, SIG_DFL);
	    signal(SIGFPE, SIG_DFL);
	    signal(SIGILL, SIG_DFL);
#ifdef SIGBUS
	    signal(SIGBUS, SIG_DFL);
#endif
#ifdef SIGSTKFLT
	    signal(SIGSTKFLT, SIG_DFL);
#endif
	}
    }
};

//  A wrapper around the tests to trap exceptions,
//  and avoid having to catch them in every test function.
//  If this test driver is used for anything other than
//  Xapian tests, then this ought to be provided by
//  the client, really.
//  return: test_driver::PASS, test_driver::FAIL, or test_driver::SKIP
test_driver::test_result
test_driver::runtest(const test_desc *test)
{
#ifdef HAVE_VALGRIND
    // This is used to make a note of how many times we've run the test
    volatile int runcount = 0;
#endif

    while (true) {
	tout.str("");
	SignalRedirector sig; // use object so signal handlers are reset
	if (!setjmp(jb)) {
	    static bool catch_signals = (getenv("XAPIAN_SIG_DFL") == NULL);
	    if (catch_signals) sig.activate();
	    try {
		expected_exception = NULL;
#ifdef HAVE_VALGRIND
		VALGRIND_DO_LEAK_CHECK;
		int vg_errs = VALGRIND_COUNT_ERRORS;
		int vg_leaks = 0, vg_dubious = 0, vg_reachable = 0, dummy;
		VALGRIND_COUNT_LEAKS(vg_leaks, vg_dubious, vg_reachable, dummy);
		ftruncate(LOG_FD_FOR_VG, 0);
#endif
		if (!test->run()) {
		    string s = tout.str();
		    if (!s.empty()) {
			out << '\n' << tout.str();
			if (s[s.size() - 1] != '\n') out << endl;
			tout.str("");
		    }
		    out << " " << col_red << "FAILED" << col_reset;
		    return FAIL;
		}
#ifdef HAVE_VALGRIND
		// We must empty tout before asking valgrind to perform its
		// leak checks, otherwise the buffers holding the output may
		// be identified as a memory leak (especially if >1K of output
		// has been buffered it appears...)
	        tout.str("");
#define REPORT_FAIL_VG(M) do { \
    if (verbose) { \
	lseek(LOG_FD_FOR_VG, 0, SEEK_SET); \
	char buf[1024]; \
	while (true) { \
	    ssize_t c = read(LOG_FD_FOR_VG, buf, sizeof(buf)); \
	    if (c == 0 || (c < 0 && errno != EINTR)) break; \
	    if (c > 0) out << string(buf, c); \
	} \
	ftruncate(LOG_FD_FOR_VG, 0); \
    } \
    out << " " << col_red << M << col_reset; \
} while (0)
		VALGRIND_DO_LEAK_CHECK;
		int vg_errs2 = VALGRIND_COUNT_ERRORS;
		vg_errs = vg_errs2 - vg_errs;
		int vg_leaks2 = 0, vg_dubious2 = 0, vg_reachable2 = 0;
		VALGRIND_COUNT_LEAKS(vg_leaks2, vg_dubious2, vg_reachable2,
			dummy);
		vg_leaks = vg_leaks2 - vg_leaks;
		vg_dubious = vg_dubious2 - vg_dubious;
		vg_reachable = vg_reachable2 - vg_reachable;
		if (vg_errs) {
		    REPORT_FAIL_VG("USED UNINITIALISED DATA");
		    return FAIL;
		}
		if (vg_leaks > 0) {
		    REPORT_FAIL_VG("LEAKED " << vg_leaks << " BYTES");
		    return FAIL;
		}
		if (vg_dubious > 0) {
		    // If code deliberately holds onto blocks by a pointer not
		    // to the start (e.g. languages/utilities.c does) then we
		    // need to rerun the test to see if the leak is real...
		    if (runcount == 0) {
			out << " " << col_yellow << "PROBABLY LEAKED MEMORY - RETRYING TEST" << col_reset;
			++runcount;
			continue;
		    }
		    REPORT_FAIL_VG("PROBABLY LEAKED " << vg_dubious << " BYTES");
		    return FAIL;
		}
		if (vg_reachable > 0) {
		    // FIXME:
		    // C++ STL implementations often "horde" released
		    // memory - perhaps we can supply our own allocator
		    // so we can tell the difference?
		    // Also see Q14 here:
// http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/*checkout*/valgrind/valgrind/FAQ.txt?rev=HEAD&content-type=text/plain 
		    //
		    // For now, just use runcount to rerun the test and see
		    // if more is leaked - hopefully this shouldn't give
		    // false positives.
		    if (runcount == 0) {
			out << " " << col_yellow << "POSSIBLE UNRELEASED MEMORY - RETRYING TEST" << col_reset;
			++runcount;
			continue;
		    }
		    REPORT_FAIL_VG("FAILED TO RELEASE " << vg_reachable << " BYTES");
		    return FAIL;
		}
#endif
	    } catch (const TestFail &) {
		string s = tout.str();
		if (!s.empty()) {
		    out << '\n' << tout.str();
		    if (s[s.size() - 1] != '\n') out << endl;
		    tout.str("");
		}
		out << " " << col_red << "FAILED" << col_reset;
		return FAIL;
	    } catch (const TestSkip &) {
		string s = tout.str();
		if (!s.empty()) {
		    out << '\n' << tout.str();
		    if (s[s.size() - 1] != '\n') out << endl;
		    tout.str("");
		}
		out << " " << col_yellow << "SKIPPED" << col_reset;
		return SKIP;
	    } catch (const Xapian::Error &err) {
		string errclass = err.get_type();
		if (expected_exception && expected_exception == errclass) {
		    out << " " << col_yellow << "C++ FAILED TO CATCH " << errclass << col_reset;
		    return SKIP;
		}
		string s = tout.str();
		if (!s.empty()) {
		    out << '\n' << tout.str();
		    if (s[s.size() - 1] != '\n') out << endl;
		    tout.str("");
		}
		out << " " << col_red << errclass << col_reset;
		if (verbose) {
		    out << err.get_type() << " exception: " << err.get_msg();
		    if (!err.get_context().empty())
			out << " (context:" << err.get_context() << ")";
		    if (err.get_errno())
			out << " (errno:" << strerror(err.get_errno()) << ")";
		    out << endl;
		}
		return FAIL;
	    } catch (...) {
		string s = tout.str();
		if (!s.empty()) {
		    out << '\n' << tout.str();
		    if (s[s.size() - 1] != '\n') out << endl;
		    tout.str("");
		}
		out << " " << col_red << "EXCEPT" << col_reset;
		if (verbose) {
		    out << "Unknown exception!" << endl;
		}
		return FAIL;
	    }
	} else {
	    // caught signal
	    string s = tout.str();
	    if (!s.empty()) {
		out << '\n' << tout.str();
		if (s[s.size() - 1] != '\n') out << endl;
		tout.str("");
	    }
	    const char *sig = "SIGNAL";
	    switch (signum) {
		case SIGSEGV: sig = "SIGSEGV"; break;
		case SIGFPE: sig = "SIGFPE"; break;
		case SIGILL: sig = "SIGILL"; break;
#ifdef SIGBUS
		case SIGBUS: sig = "SIGBUS"; break;
#endif
#ifdef SIGSTKFLT
		case SIGSTKFLT: sig = "SIGSTKFLT"; break;
#endif
	    }
    	    out << " " << col_red << sig << col_reset;
	    return FAIL;
	}
	return PASS;
    }
}

test_driver::result
test_driver::run_tests(vector<string>::const_iterator b,
		       vector<string>::const_iterator e)
{
    return do_run_tests(b, e);
}

test_driver::result
test_driver::run_tests()
{
    const vector<string> blank;
    return do_run_tests(blank.begin(), blank.end());
}

test_driver::result
test_driver::do_run_tests(vector<string>::const_iterator b,
			  vector<string>::const_iterator e)
{
    set<string> m(b, e);
    bool check_name = !m.empty();

    test_driver::result result = {0, 0, 0};

    for (const test_desc *test = tests; test->name; test++) {
	bool do_this_test = !check_name;
	if (!do_this_test && m.find(test->name) != m.end())
	    do_this_test = true;
	if (!do_this_test) {
	    // if this test is "foo123" see if "foo" was listed
	    // this way "./testprog foo" can run foo1, foo2, etc.
	    string t = test->name;
	    string::size_type i;
	    i = t.find_last_not_of("0123456789") + 1;
	    if (i < t.length()) {
		t = t.substr(0, i);
		if (m.find(t) != m.end()) do_this_test = true;
	    }
	}
	if (do_this_test) {
	    out << "Running test: " << test->name << "...";
	    out.flush();
	    switch (runtest(test)) {
		case PASS:
		    ++result.succeeded;
		    if (verbose || !use_cr) {
			out << col_green << " ok" << col_reset << endl;
		    } else {
			out << "\r                                                                               \r";
		    }
		    break;
		case FAIL:
		    ++result.failed;
		    out << endl;
		    if (abort_on_error) {
			out << "Test failed - aborting further tests." << endl;
			return result;
		    }
		    break;
		case SKIP:
		    ++result.skipped;
		    out << endl;
		    // ignore the result of this test.
		    break;
	    }
	}
    }
    return result;
}

void
test_driver::usage()
{
    cout << "Usage: " << argv0 << " [-v|--verbose] [-o|--abort-on-error] " << opt_help
         << "[TESTNAME]..." << endl;
    cout << "       " << argv0 << " [-h|--help]" << endl;
    exit(1);
}

void
test_driver::report(const test_driver::result &r, const string &desc)
{
    if (r.succeeded != 0 || r.failed != 0) {
	cout << argv0 << " " << desc << ": ";

	if (r.failed == 0)
	    cout << "All ";

	cout << col_green << r.succeeded << col_reset << " tests passed";

	if (r.failed != 0)
	    cout << ", " << col_red << r.failed << col_reset << " failed";

	if (r.skipped) {
	    cout << ", " << col_yellow << r.skipped << col_reset
		 << " skipped." << endl;
	} else {
	    cout << "." << endl;
	}
    }
}

// call via atexit if there's more than one test run
void
report_totals()
{
    test_driver::report(test_driver::total, "total");
}

void
test_driver::add_command_line_option(const string &l, char s, string * arg)
{
    short_opts.insert(make_pair<int, string *>(int(s), arg));
    opt_help += "[-";
    opt_help += s;
    opt_help += '=';
    opt_help += l;
    opt_help += "] ";
}

void
test_driver::parse_command_line(int argc, char **argv)
{
    argv0 = argv[0];

#ifndef __WIN32__
    if (isatty(1)) {
	col_red = "\x1b[1m\x1b[31m";
	col_green = "\x1b[1m\x1b[32m";
	col_yellow = "\x1b[1m\x1b[33m";
	col_reset = "\x1b[0m";
	use_cr = true;
    }
#endif

    struct option long_opts[] = {
	{"verbose",		no_argument, 0, 'v'},
	{"abort-on-error",	no_argument, 0, 'o'},
	{"help",		no_argument, 0, 'h'},
	{NULL,			0, 0, 0}
    };

    string opts = "voh";
    map<int, string *>::const_iterator i;
    for (i = short_opts.begin(); i != short_opts.end(); ++i) {
	opts += char(i->first);
	opts += ':';
    }

    int c;
    while ((c = getopt_long(argc, argv, opts.c_str(), long_opts, 0)) != EOF) {
	switch (c) {
	    case 'v':
		verbose = true;
		break;
	    case 'o':
		abort_on_error = true;
		break;
	    default: {
		map<int, string *>::const_iterator i;
		i = short_opts.find(c);
		if (i != short_opts.end()) {
		    *(i->second) = string(optarg);
		    break;
		}
		// -h or unrecognised option
	    	usage();
		return; // usage() doesn't return ...
	    }
	}
    }

    while (argv[optind]) {
	test_names.push_back(string(argv[optind]));
	optind++;
    }

#ifdef HAVE_VALGRIND
    if (verbose && RUNNING_ON_VALGRIND) {
	// Open a temporary file for valgrind to log to.
	FILE * f = tmpfile();
	if (f) {
	    int fd = fileno(f);
	    if (fd != -1) {
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_APPEND);
		if (fd != LOG_FD_FOR_VG) {
		    dup2(fd, LOG_FD_FOR_VG);
		    fclose(f);
		}
	    }
	}
    }
#endif

    // We need to display something before we start, or the allocation
    // made when the first debug message is displayed is (wrongly) picked
    // up on as a memory leak.
    DEBUGLINE(UNKNOWN, "Starting testsuite run.");
#ifdef MUS_DEBUG_VERBOSE
    om_debug.initialise();
#endif /* MUS_DEBUG_VERBOSE */
}

int
test_driver::run(const test_desc *tests)
{
    // If we're running more than one batch of tests, report the totals
    // at the end.
    if (runs == 1) atexit(report_totals);
    ++runs;

    test_driver driver(tests);

    test_driver::result myresult;
    myresult = driver.run_tests(test_names.begin(), test_names.end());

    report(myresult, "completed test run");

    total.succeeded += myresult.succeeded;
    total.failed += myresult.failed;
    total.skipped += myresult.skipped;

    return (bool)myresult.failed; // if 0, then everything passed
}
