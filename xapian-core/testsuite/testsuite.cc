/* testsuite.cc: a test suite engine
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

#include "config.h"
#include <iostream>

#ifdef HAVE_STREAMBUF
#include <streambuf>
#else // HAVE_STREAMBUF
#include <streambuf.h>
#endif // HAVE_STREAMBUF

#include <string>
#include <new>
#include <cstdio>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else // HAVE_GETOPT_H
#include <stdlib.h>
#endif // HAVE_GETOPT_H

#include <unistd.h> // for chdir

#include <exception>

#include "om/omerror.h"
#include "testsuite.h"
#include "omdebug.h"

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>

pthread_mutex_t test_driver_mutex;
#endif // HAVE_LIBPTHREAD

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

void
test_driver::set_quiet(bool quiet_)
{
    if (quiet_) {
	out.rdbuf(&nullsb);
    } else {
	out.rdbuf(std::cout.rdbuf());
    }
}

string
test_driver::get_srcdir(const string & argv0)
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
	    cout << argv0
		<< ": srcdir not in the environment and I can't guess it!"
		<< endl;
	    exit(1);
	}
    }
    return srcdir;
}

/* Global data used by the overridden new and delete
 * operators.
 */

// The maximum number of allocations which can be tracked
static const int max_allocations = 1000000;

// The number of currently unfreed allocations
static long num_new_allocations = 0;
// One past the highest used array position
static long new_allocations_bound = 0;
// The array of current allocations.
struct allocation_info {
    void *p;
    size_t size;
};
static allocation_info new_allocations[max_allocations];

/* To help in tracking memory leaks, we can set a trap on a particular
 * memory address being allocated (which would be from the leak reporting
 * also provided by our operator new)
 */
/// The address to trap (or 0 for none)
static void *new_trap_address = 0;
/// Which one to trap (eg 1st, 2nd, etc.)
static unsigned long new_trap_count = 0;

/** Our overridden new and delete operators, which
 *  allow us to check for leaks.
 *
 *  FIXME: add handling of new[] and delete[]
 *  FIXME: add malloc() handling (trickier, since can't use
 *  the built-in malloc so easily in implementation.)
 */
void *operator new(size_t size) throw(std::bad_alloc) {
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_lock(&test_driver_mutex);
#endif // HAVE_LIBPTHREAD
    size_t real_size = (size > 0)? size : 1;

    void *result = malloc(real_size);

    if (new_trap_address != 0 &&
        new_trap_address == result &&
        new_trap_count != 0) {
        --new_trap_count;
	if (new_trap_count == 0) {
            abort();
        }
    }

    if (!result) {
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&test_driver_mutex);
#endif // HAVE_LIBPTHREAD
	throw std::bad_alloc();
    }

    if (new_allocations_bound >= max_allocations) {
	// our array is too small - panic!
	fprintf(stderr, "Ran out of room for malloc tracking!\n");
	abort();
    } else {
	new_allocations[new_allocations_bound].p = result;
	new_allocations[new_allocations_bound].size = real_size;
	++new_allocations_bound;

	++num_new_allocations;
    }

#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&test_driver_mutex);
#endif // HAVE_LIBPTHREAD
    return result;
}

/// This method is simply here to be an easy place to set a break point
void memory_weirdness() {
}

void operator delete(void *p) throw() {
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_lock(&test_driver_mutex);
#endif // HAVE_LIBPTHREAD
    if (p) {
	bool found_it = false;
	for (int i = new_allocations_bound - 1;
	     i >= 0;
	     --i) {
	    if (new_allocations[i].p == p) {
		new_allocations[i].p = 0;
		found_it = true;

		// lower new_allocations_bound if possible
		if (i == (new_allocations_bound - 1)) {
		    while (new_allocations_bound > 0 &&
			   new_allocations[new_allocations_bound-1].p == 0) {
			new_allocations_bound--;
		    }
		}
	    }
	}
	if (!found_it) {
	    // note: we can use C-style I/O, but nothing C++ish in
	    // case new is needed.
	    fprintf(stderr,
		    "Trying to delete %p which wasn't allocated with new\n",
		    p);
	    memory_weirdness();
	}

	--num_new_allocations;
	free(p);
    }
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&test_driver_mutex);
#endif // HAVE_LIBPTHREAD
}

test_driver::test_driver(const test_desc *tests_)
	: abort_on_error(false),
	  out(std::cout.rdbuf()),
	  tests(tests_)
{
    // set up special handling to check for a particular allocation
    const char *addr = getenv("OM_NEW_TRAP");
    if (addr) {
        new_trap_address = (void *)strtol(addr, 0, 16);
        const char *count = getenv("OM_NEW_TRAP_COUNT");
        if (count) {
            new_trap_count = atol(count);
	} else {
  	    new_trap_count = 1;
	}
        DEBUGLINE(UNKNOWN, "new trap address set to " << new_trap_address);
        DEBUGLINE(UNKNOWN, "new trap count set to " << new_trap_count);
    }
}

//  A wrapper around the tests to trap exceptions,
//  and avoid having to catch them in every test function.
//  If this test driver is used for anything other than
//  Open Muscat tests, then this ought to be provided by
//  the client, really.
bool
test_driver::runtest(const test_desc *test)
{
    bool success = true;

    int old_allocations = num_new_allocations;
    int old_bound = new_allocations_bound;

    // This is used to make a note of how many times we've run the test
    int runcount = 0;
    bool repeat;

    do {
	runcount++;
	repeat = false;
	try {
	    success = test->run();
	    if (!success) {
		out << " FAILED";
	    }
	} catch (TestFailure &fail) {
	    success = false;
	    out << " FAILED";
	    if (verbose) {
		out << fail.message << std::endl;
	    }
	} catch (OmError &err) {
	    out << " OMEXCEPT";
	    if (verbose) {
		out << err.get_type() << " exception: " << err.get_msg() << std::endl;
	    }
	    success = false;
	} catch (...) {
	    out << " EXCEPT";
	    if (verbose) {
		out << "Unknown exception!" << std::endl;
	    }
	    success = false;
	}

	int after_allocations = num_new_allocations;
	int after_bound = new_allocations_bound;
	if (after_allocations != old_allocations) {
	    if (verbose) {
		if (after_allocations > old_allocations) {
		    out << after_allocations - old_allocations
			    << " extra allocations not freed: ";
		    for (int i=old_bound; i<after_bound; ++i) {
			if (new_allocations[i].p != 0) {
			    out << hex;
			    out << new_allocations[i].p << "("
				    << new_allocations[i].size << ") ";
			    out << dec;
			}
		    }
		    out << std::endl;
		} else {
		    out << old_allocations - after_allocations
			    << " extra frees not allocated!" << std::endl;
		}
	    }
	    if(runcount < 2 && success) {
		out << " repeating...";
		repeat = true;
		old_allocations = num_new_allocations;
		old_bound = new_allocations_bound;
	    } else {
		out << " LEAK";
		success = false;
	    }
	}
    } while(repeat);
    return success;
}

test_driver::result test_driver::run_tests()
{
    const std::string blank;
    return do_run_tests(blank);
}

test_driver::result test_driver::run_test(const std::string &test_name)
{
    return do_run_tests(test_name);
}

test_driver::result test_driver::do_run_tests(const std::string &testname)
{
    const test_desc *test = tests;
    test_driver::result result = {0, 0};

    bool check_name = (testname.length() > 0);
    while ((test->name) != 0) {
	if ((check_name == false) || (testname == test->name)) {
	    out << "Running test: " << test->name << "...";
	    out.flush();
	    bool succeeded = runtest(test);
	    if (succeeded) {
		++result.succeeded;
		out << " ok." << std::endl;
	    } else {
		++result.failed;
                out << std::endl;
		if (abort_on_error) {
		    out << "Test failed - aborting further tests." << std::endl;
		    break;
		}
	    }
	}
	++test;
    }
    return result;
}

static void usage(char *progname)
{
    std::cerr << "Usage: " << progname
              << " [-v] [-o] [testname]" << std::endl;
    exit(1);
}

int test_driver::main(int argc,
		      char *argv[],
		      const test_desc *tests,
		      test_driver::result *summary)
{
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_init(&test_driver_mutex, 0);
#endif // HAVE_LIBPTHREAD

    int c;

    test_driver driver(tests);

    std::string one_test_name;
    bool one_test = false;

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

    if (optind == (argc-1)) {
	one_test = true;
	one_test_name = argv[argc-1];
    } else if (optind != (argc)) {
    	usage(argv[0]);
	return 1;
    }

    // We need to display something before we start, or the allocation
    // made when the first debug message is displayed is (wrongly) picked
    // up on as a memory leak.
    DEBUGLINE(UNKNOWN, "Starting testsuite run.");
#ifdef MUS_DEBUG_VERBOSE
    om_debug.initialise();
#endif /* MUS_DEBUG_VERBOSE */

    test_driver::result myresult;
    if (one_test) {
	myresult = driver.run_test(one_test_name);
    } else {
	myresult = driver.run_tests();
    }

    if (summary) {
	*summary = myresult;
    }

    std::cout << argv[0] << " completed test run: "
         << myresult.succeeded << " tests passed, "
	 << myresult.failed << " failed."
	 << std::endl;

    return (bool)myresult.failed; // if 0, then everything passed
}
