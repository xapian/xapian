/* testsuite.cc: a test suite engine
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2013 Olly Betts
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

#include <config.h>

#include "testsuite.h"

#include "backendmanager.h"
#include "fdtracker.h"
#include "testrunner.h"
#include "safeunistd.h"

#ifdef HAVE_VALGRIND
# include "safeerrno.h"
# include <valgrind/memcheck.h>
# include <sys/types.h>
# include "safefcntl.h"
#endif

#include <algorithm>
#include <iostream>
#include <set>

#include <cfloat> // For DBL_DIG.
#include <cmath> // For ceil, fabs, log10.
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gnu_getopt.h"

#include <setjmp.h>
#include <signal.h>

#include <exception>
#ifdef USE_RTTI
# include <typeinfo>
# ifdef __GNUC__
#  include <cxxabi.h> // Added in GCC 3.1 which is now required.
# endif
#endif

#include <xapian/error.h>
#include "noreturn.h"
#include "stringutils.h"
#include "utils.h"

using namespace std;

/// The global verbose flag.
int verbose;

#ifdef HAVE_VALGRIND
static int vg_log_fd = -1;
#endif

#ifdef HAVE_SIGSETJMP
# define SIGSETJMP(ENV, SAVESIGS) sigsetjmp(ENV, SAVESIGS)
# define SIGLONGJMP(ENV, VAL) siglongjmp(ENV, VAL)
# define SIGJMP_BUF sigjmp_buf
#else
# define SIGSETJMP(ENV, SAVESIGS) setjmp(ENV)
# define SIGLONGJMP(ENV, VAL) longjmp(ENV, VAL)
# define SIGJMP_BUF jmp_buf
#endif

/// The exception type we were expecting in TEST_EXCEPTION
//  We use this to attempt to diagnose when the code fails to catch an
//  exception when it should (due to a compiler or runtime fault in
//  GCC 2.95 it seems)
const char * expected_exception = NULL;

/// The debug printing stream
std::ostringstream tout;

int test_driver::runs = 0;
test_driver::result test_driver::subtotal;
test_driver::result test_driver::total;
string test_driver::argv0;
string test_driver::opt_help;
map<int, string *> test_driver::short_opts;
vector<string> test_driver::test_names;
bool test_driver::abort_on_error = false;
string test_driver::col_red, test_driver::col_green;
string test_driver::col_yellow, test_driver::col_reset;
bool test_driver::use_cr = false;

void
test_driver::write_and_clear_tout()
{
    const string & s = tout.str();
    if (!s.empty()) {
	out << '\n' << s;
	tout.str(string());
    }
}

string
test_driver::get_srcdir()
{
    char *p = getenv("srcdir");
    if (p != NULL) return string(p);

#ifdef __WIN32__
    // The path on argv[0] will always use \ for the directory separator.
    const char ARGV0_SEP = '\\';
#else
    const char ARGV0_SEP = '/';
#endif
    // Default srcdir to the pathname of argv[0].
    string srcdir(argv0);
    string::size_type i = srcdir.find_last_of(ARGV0_SEP);
    string srcfile;
    if (i != string::npos) {
	srcfile = srcdir.substr(i + 1);
	srcdir.erase(i);
	// libtool may put the real executable in .libs.
	i = srcdir.find_last_of(ARGV0_SEP);
	if (srcdir.substr(i + 1) == ".libs") {
	    srcdir.erase(i);
	    // And it may have an "lt-" prefix.
	    if (startswith(srcfile, "lt-")) srcfile.erase(0, 3);
	}
    } else {
	// No path of argv[0], so default srcdir to the current directory.
	// This may not work if libtool is involved as the true executable is
	// sometimes in ".libs".
	srcfile = srcdir;
	srcdir = ".";
    }

    // Remove any trailing ".exe" suffix, since some platforms add this.
    if (endswith(srcfile, ".exe")) srcfile.resize(srcfile.size() - 4);

    // Sanity check.
    if (!file_exists(srcdir + '/' + srcfile + ".cc")) {
	cout << argv0
	     << ": srcdir is not in the environment and I can't guess it!\n"
		"Run test programs using the runtest script - see HACKING for details"
	     << endl;
	exit(1);
    }
    return srcdir;
}

test_driver::test_driver(const test_desc *tests_)
	: out(cout.rdbuf()), tests(tests_)
{
}

static SIGJMP_BUF jb;
static int signum = 0;
static void * sigaddr = NULL;

// Needs C linkage so we can pass it to sigaction()/signal() without problems.
extern "C" {

#if defined HAVE_SIGACTION && defined SA_SIGINFO
XAPIAN_NORETURN(static void handle_sig(int signum_, siginfo_t *si, void *));
static void handle_sig(int signum_, siginfo_t *si, void *)
{
    // Disable all our signal handlers to avoid problems if the signal
    // handling code causes a signal.
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    // We set the handlers with SA_RESETHAND, but that will only reset the
    // handler for the signal which fired.
    if (signum_ != SIGSEGV) sigaction(SIGSEGV, &sa, NULL);
    if (signum_ != SIGFPE) sigaction(SIGFPE, &sa, NULL);
    if (signum_ != SIGILL) sigaction(SIGILL, &sa, NULL);
# ifdef SIGBUS
    if (signum_ != SIGBUS) sigaction(SIGBUS, &sa, NULL);
# endif
# ifdef SIGSTKFLT
    if (signum_ != SIGSTKFLT) sigaction(SIGSTKFLT, &sa, NULL);
# endif
    signum = signum_;
    sigaddr = si->si_addr;
    SIGLONGJMP(jb, 1);
}

#else

XAPIAN_NORETURN(static void handle_sig(int signum_));
static void handle_sig(int signum_)
{
    // Disable all our signal handlers to avoid problems if the signal
    // handling code causes a signal.
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
    SIGLONGJMP(jb, 1);
}
#endif

}

class SignalRedirector {
  private:
    bool active;
  public:
    SignalRedirector() : active(false) { }
    void activate() {
	active = true;
	signum = 0;
	sigaddr = NULL;
	// SA_SIGINFO not universal (e.g. not present on Linux < 2.2 and Hurd).
#if defined HAVE_SIGACTION && defined SA_SIGINFO
	struct sigaction sa;
	sa.sa_sigaction = handle_sig;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND|SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);
# ifdef SIGBUS
	sigaction(SIGBUS, &sa, NULL);
# endif
# ifdef SIGSTKFLT
	sigaction(SIGSTKFLT, &sa, NULL);
# endif
#else
	signal(SIGSEGV, handle_sig);
	signal(SIGFPE, handle_sig);
	signal(SIGILL, handle_sig);
# ifdef SIGBUS
	signal(SIGBUS, handle_sig);
# endif
# ifdef SIGSTKFLT
	signal(SIGSTKFLT, handle_sig);
# endif
#endif
    }
    ~SignalRedirector() {
	if (active) {
#if defined HAVE_SIGACTION && defined SA_SIGINFO
	    struct sigaction sa;
	    sa.sa_handler = SIG_DFL;
	    sigemptyset(&sa.sa_mask);
	    sa.sa_flags = 0;
	    sigaction(SIGSEGV, &sa, NULL);
	    sigaction(SIGFPE, &sa, NULL);
	    sigaction(SIGILL, &sa, NULL);
# ifdef SIGBUS
	    sigaction(SIGBUS, &sa, NULL);
# endif
# ifdef SIGSTKFLT
	    sigaction(SIGSTKFLT, &sa, NULL);
# endif
#else
	    signal(SIGSEGV, SIG_DFL);
	    signal(SIGFPE, SIG_DFL);
	    signal(SIGILL, SIG_DFL);
# ifdef SIGBUS
	    signal(SIGBUS, SIG_DFL);
# endif
# ifdef SIGSTKFLT
	    signal(SIGSTKFLT, SIG_DFL);
# endif
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
    // This is used to make a note of how many times we've run the test
    volatile int runcount = 0;

    FDTracker fdtracker;
    fdtracker.init();

    while (true) {
	tout.str(string());
	if (SIGSETJMP(jb, 1) == 0) {
	    SignalRedirector sig; // use object so signal handlers are reset
	    static bool catch_signals =
		(getenv("XAPIAN_TESTSUITE_SIG_DFL") == NULL);
	    if (catch_signals) sig.activate();
	    try {
		expected_exception = NULL;
#ifdef HAVE_VALGRIND
		int vg_errs = 0;
		long vg_leaks = 0, vg_dubious = 0, vg_reachable = 0;
		if (vg_log_fd != -1) {
		    VALGRIND_DO_LEAK_CHECK;
		    vg_errs = VALGRIND_COUNT_ERRORS;
		    long dummy;
		    VALGRIND_COUNT_LEAKS(vg_leaks, vg_dubious, vg_reachable, dummy);
		    (void)dummy;
		    // Skip past any unread log output.
		    lseek(vg_log_fd, 0, SEEK_END);
		}
#endif
		if (!test->run()) {
		    out << col_red << " FAILED" << col_reset;
		    write_and_clear_tout();
		    return FAIL;
		}
		if (verbose > 1)
		    write_and_clear_tout();
		if (backendmanager)
		    backendmanager->clean_up();
#ifdef HAVE_VALGRIND
		if (vg_log_fd != -1) {
		    // We must empty tout before asking valgrind to perform its
		    // leak checks, otherwise the buffers holding the output
		    // may be identified as a memory leak (especially if >1K of
		    // output has been buffered it appears...)
		    tout.str(string());
#define REPORT_FAIL_VG(M) do { \
    if (verbose) { \
	while (true) { \
	    ssize_t c = read(vg_log_fd, buf, sizeof(buf)); \
	    if (c == 0 || (c < 0 && errno != EINTR)) break; \
	    if (c > 0) out << string(buf, c); \
	} \
    } \
    out << " " << col_red << M << col_reset; \
} while (0)
		    // Record the current position so we can restore it so
		    // REPORT_FAIL_VG() gets the whole output.
		    off_t curpos = lseek(vg_log_fd, 0, SEEK_CUR);
		    char buf[4096];
		    while (true) {
			ssize_t c = read(vg_log_fd, buf, sizeof(buf));
			if (c == 0 || (c < 0 && errno != EINTR)) {
			    buf[0] = 0;
			    break;
			}
			if (c > 0) {
			    // Valgrind output has "==<pid>== \n" between
			    // report "records", so skip any lines like that,
			    // and also any warnings and continuation lines.
			    ssize_t i = 0;
			    while (true) {
				const char * spc;
				spc = static_cast<const char *>(
					memchr(buf + i, ' ', c - i));
				if (!spc) {
				    i = c;
				    break;
				}
				i = spc - buf;
				if (++i >= c) break;
				if (buf[i] == '\n')
				    continue;
				if (c - i >= 8 &&
				    (memcmp(buf + i, "Warning:", 8) == 0 ||
				     memcmp(buf + i, "   ", 3) == 0)) {
				    // Skip this line.
				    i += 3;
				    const char * nl;
				    nl = static_cast<const char *>(
					    memchr(buf + i, '\n', c - i));
				    if (!nl) {
					i = c;
					break;
				    }
				    i = nl - buf;
				    continue;
				}
				break;
			    }

			    char *start = buf + i;
			    c -= i;
			    if (c > 128) c = 128;

			    {
				const char *p;
				p = static_cast<const char*>(
					memchr(start, '\n', c));
				if (p != NULL) c = p - start;
			    }

			    memmove(buf, start, c);
			    buf[c] = '\0';
			    break;
			}
		    }
		    lseek(vg_log_fd, curpos, SEEK_SET);

		    int vg_errs2 = VALGRIND_COUNT_ERRORS;
		    vg_errs = vg_errs2 - vg_errs;
		    VALGRIND_DO_LEAK_CHECK;
		    long vg_leaks2 = 0, vg_dubious2 = 0, vg_reachable2 = 0;
		    long dummy;
		    VALGRIND_COUNT_LEAKS(vg_leaks2, vg_dubious2, vg_reachable2,
					 dummy);
		    (void)dummy;
		    vg_leaks = vg_leaks2 - vg_leaks;
		    vg_dubious = vg_dubious2 - vg_dubious;
		    vg_reachable = vg_reachable2 - vg_reachable;
		    if (vg_errs) {
			string fail_msg(buf);
			if (fail_msg.empty())
			    fail_msg = "VALGRIND DETECTED A PROBLEM";
			REPORT_FAIL_VG(fail_msg);
			return FAIL;
		    }
		    if (vg_leaks > 0) {
			REPORT_FAIL_VG("LEAKED " << vg_leaks << " BYTES");
			return FAIL;
		    }
		    if (vg_dubious > 0) {
			// If code deliberately holds onto blocks by a pointer
			// not to the start (e.g. languages/utilities.c does)
			// then we need to rerun the test to see if the leak is
			// real...
			if (runcount == 0) {
			    out << col_yellow << " PROBABLY LEAKED MEMORY - RETRYING TEST" << col_reset;
			    ++runcount;
			    // Ensure that any cached memory from fd tracking
			    // is allocated before we rerun the test.
			    (void)fdtracker.check();
			    continue;
			}
			REPORT_FAIL_VG("PROBABLY LEAKED " << vg_dubious << " BYTES");
			return FAIL;
		    }
		    if (vg_reachable > 0) {
			// C++ STL implementations often "horde" released
			// memory - for GCC 3.4 and newer the runtest script
			// sets GLIBCXX_FORCE_NEW=1 which will disable this
			// behaviour so we avoid this issue, but for older
			// GCC and other compilers this may be an issue.
			//
			// See also:
			// http://valgrind.org/docs/FAQ/#faq.reports
			//
			// For now, just use runcount to rerun the test and see
			// if more is leaked - hopefully this shouldn't give
			// false positives.
			if (runcount == 0) {
			    out << col_yellow << " POSSIBLE UNRELEASED MEMORY - RETRYING TEST" << col_reset;
			    ++runcount;
			    // Ensure that any cached memory from fd tracking
			    // is allocated before we rerun the test.
			    (void)fdtracker.check();
			    continue;
			}
			REPORT_FAIL_VG("FAILED TO RELEASE " << vg_reachable << " BYTES");
			return FAIL;
		    }
		}
#endif
		if (!fdtracker.check()) {
		    if (runcount == 0) {
			out << col_yellow << " POSSIBLE FDLEAK:" << fdtracker.get_message() << col_reset;
			++runcount;
			continue;
		    }
		    out << col_red << " FDLEAK:" << fdtracker.get_message() << col_reset;
		    return FAIL;
		}
	    } catch (const TestFail &) {
		out << col_red << " FAILED" << col_reset;
		write_and_clear_tout();
		return FAIL;
	    } catch (const TestSkip &) {
		out << col_yellow << " SKIPPED" << col_reset;
		write_and_clear_tout();
		return SKIP;
	    } catch (const Xapian::Error &err) {
		string errclass = err.get_type();
		if (expected_exception && expected_exception == errclass) {
		    out << col_yellow << " C++ FAILED TO CATCH " << errclass << col_reset;
		    return SKIP;
		}
		out << " " << col_red << err.get_description() << col_reset;
		write_and_clear_tout();
		return FAIL;
	    } catch (const string & msg) {
		out << col_red << " EXCEPTION std::string " << msg << col_reset;
		write_and_clear_tout();
		return FAIL;
	    } catch (const std::exception & e) {
		out << " " << col_red;
#ifndef USE_RTTI
		out << "std::exception";
#else
		const char * name = typeid(e).name();
# ifdef __GNUC__
		// __cxa_demangle() apparently requires GCC >= 3.1.
		// Demangle the name which GCC returns for type_info::name().
		int status;
		char * realname = abi::__cxa_demangle(name, NULL, 0, &status);
		if (realname) {
		    out << realname;
		    free(realname);
		} else {
		    out << name;
		}
# else
		out << name;
# endif
#endif
		out << ": " << e.what() << col_reset;
		write_and_clear_tout();
		return FAIL;
	    } catch (const char * msg) {
		out << col_red;
		if (msg) {
		    out << " EXCEPTION char * " << msg;
		} else {
		    out << " EXCEPTION (char*)NULL";
		}
		out << col_reset;
		write_and_clear_tout();
		return FAIL;
	    } catch (...) {
		out << col_red << " UNKNOWN EXCEPTION" << col_reset;
		write_and_clear_tout();
		return FAIL;
	    }
	    return PASS;
	}

	// Caught a signal.
	const char *signame = "SIGNAL";
	bool show_addr = true;
	switch (signum) {
	    case SIGSEGV: signame = "SIGSEGV"; break;
	    case SIGFPE: signame = "SIGFPE"; break;
	    case SIGILL: signame = "SIGILL"; break;
#ifdef SIGBUS
	    case SIGBUS: signame = "SIGBUS"; break;
#endif
#ifdef SIGSTKFLT
	    case SIGSTKFLT:
		signame = "SIGSTKFLT";
		show_addr = false;
		break;
#endif
	}
	out << " " << col_red << signame;
	if (show_addr) {
	    char buf[40];
	    sprintf(buf, " at %p", sigaddr);
	    out << buf;
	}
	out << col_reset;
	write_and_clear_tout();
	return FAIL;
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

    test_driver::result res;

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
	    if (i != string::npos) {
		t.resize(i);
		if (m.find(t) != m.end()) do_this_test = true;
	    }
	}
	if (do_this_test) {
	    out << "Running test: " << test->name << "...";
	    out.flush();
	    test_driver::test_result test_res = runtest(test);
	    if (backendmanager)
		backendmanager->clean_up();
	    switch (test_res) {
		case PASS:
		    ++res.succeeded;
		    if (verbose || !use_cr) {
			out << col_green << " ok" << col_reset << endl;
		    } else {
			out << "\r                                                                               \r";
		    }
		    break;
		case FAIL:
		    ++res.failed;
		    out << endl;
		    if (abort_on_error) {
			throw "Test failed - aborting further tests";
		    }
		    break;
		case SKIP:
		    ++res.skipped;
		    out << endl;
		    // ignore the result of this test.
		    break;
	    }
	}
    }
    return res;
}

void
test_driver::usage()
{
    cout << "Usage: " << argv0 << " [-v|--verbose] [-o|--abort-on-error] " << opt_help
	 << "[TESTNAME]..." << endl;
    cout << "       " << argv0 << " [-h|--help]" << endl;
    exit(1);
}

/* Needs C linkage so we can pass it to atexit() without problems. */
extern "C" {
// Call upon program exit if there's more than one test run.
static void
report_totals(void)
{
    test_driver::report(test_driver::total, "total");
}
}

void
test_driver::report(const test_driver::result &r, const string &desc)
{
    // Report totals at the end if we reported two or more subtotals.
    if (++runs == 2) atexit(report_totals);

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

void
test_driver::add_command_line_option(const string &l, char s, string * arg)
{
    short_opts.insert(make_pair(int(s), arg));
    opt_help += "[-";
    opt_help += s;
    opt_help += ' ';
    opt_help += l;
    opt_help += "] ";
}

void
test_driver::parse_command_line(int argc, char **argv)
{
    argv0 = argv[0];

#ifndef __WIN32__
    {
	bool colourise = true;
	const char *p = getenv("XAPIAN_TESTSUITE_OUTPUT");
	if (p == NULL || !*p || strcmp(p, "auto") == 0) {
	    colourise = isatty(1);
	} else if (strcmp(p, "plain") == 0) {
	    colourise = false;
	}
	if (colourise) {
	    col_red = "\x1b[1m\x1b[31m";
	    col_green = "\x1b[1m\x1b[32m";
	    col_yellow = "\x1b[1m\x1b[33m";
	    col_reset = "\x1b[0m";
	    use_cr = true;
	}
    }
#endif

    const struct option long_opts[] = {
	{"verbose",		no_argument, 0, 'v'},
	{"abort-on-error",	no_argument, 0, 'o'},
	{"help",		no_argument, 0, 'h'},
	{NULL,			0, 0, 0}
    };

    string short_opts_string = "voh";
    map<int, string *>::const_iterator i;
    for (i = short_opts.begin(); i != short_opts.end(); ++i) {
	short_opts_string += char(i->first);
	short_opts_string += ':';
    }
    const char * opts = short_opts_string.c_str();

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'v':
		++verbose;
		break;
	    case 'o':
		abort_on_error = true;
		break;
	    default: {
		i = short_opts.find(c);
		if (i != short_opts.end()) {
		    i->second->assign(optarg);
		    break;
		}
		// -h or unrecognised option
		usage();
		return; // usage() doesn't return ...
	    }
	}
    }

    if (verbose == 0) {
	const char *p = getenv("VERBOSE");
	if (p != NULL) {
	    verbose = atoi(p);
	}
    }

    while (argv[optind]) {
	test_names.push_back(string(argv[optind]));
	optind++;
    }

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	if (getenv("XAPIAN_TESTSUITE_VALGRIND") != NULL) {
	    // Open the valgrind log file, and unlink it.
	    char fname[64];
	    sprintf(fname, ".valgrind.log.%lu", (unsigned long)getpid());
	    vg_log_fd = open(fname, O_RDONLY|O_NONBLOCK);
	    if (vg_log_fd != -1) unlink(fname);
	}
    }
#endif
}

int
test_driver::run(const test_desc *tests)
{
    test_driver driver(tests);

    test_driver::result myresult;
    myresult = driver.run_tests(test_names.begin(), test_names.end());

    subtotal += myresult;

    return bool(myresult.failed); // if 0, then everything passed
}

bool
TEST_EQUAL_DOUBLE_(double a, double b)
{
    if (a == b) return true;
    return (ceil(log10(max(fabs(a), fabs(b)))) - log10(fabs(a - b)) > DBL_DIG);
}
