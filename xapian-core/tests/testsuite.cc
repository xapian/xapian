/* testsuite.cc - a test suite engine
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

#include <iostream>
#include <streambuf.h>
#include <string>
#include <new>
#include <cstdio>
#include "om/omerror.h"
#include "testsuite.h"

class null_streambuf : public streambuf {
};

/// A null stream buffer which we can redirect output to.
static null_streambuf nullsb;

test_driver::test_driver()
	: abort_on_error(false),
	  out(cout.rdbuf())
{}

void
test_driver::set_quiet(bool quiet_)
{
    if (quiet_) {
	out.rdbuf(&nullsb);
    } else {
	out.rdbuf(cout.rdbuf());
    }
}

/* Global data used by the overridden new and delete
 * operators.
 */

// The maximum number of allocations which can be tracked
static const int max_allocations = 10000;

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

/** Our overridden new and delete operators, which 
 *  allow us to check for leaks.
 *
 *  FIXME: add handling of new[] and delete[]
 *  FIXME: add malloc() handling (trickier, since can't use
 *  the built-in malloc so easily in implementation.)
 */
void *operator new(size_t size) throw(bad_alloc) {
    size_t real_size = (size > 0)? size : 1;

    void *result = malloc(real_size);

    if (!result) {
	throw bad_alloc();
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

    return result;
}

void operator delete(void *p) throw() {
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
	}

	--num_new_allocations;
	free(p);
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
    bool success;

    int old_allocations = num_new_allocations;
    int old_bound = new_allocations_bound;

    try {
        success = test->run();
    } catch (OmError &err) {
	out << "OmError exception: " << err.get_msg();
	success = false;
    } catch (...) {
	out << "Unknown exception! ";
	success = false;
    }
    int after_allocations = num_new_allocations;
    int after_bound = new_allocations_bound;

    if (after_allocations != old_allocations) {
	if (after_allocations > old_allocations) {
	    out << after_allocations - old_allocations
		    << " extra allocations not freed: ";
	    for (int i=old_bound; i<after_bound; ++i) {
		if (new_allocations[i].p != 0) {
		    out << new_allocations[i].p << "("
			<< new_allocations[i].size << ") ";
		}
	    }
	    out << endl;
	} else {
	    out << old_allocations - after_allocations
		    << " extra frees not allocated!";
	}
	success = false;
    }
    return success;
}

test_driver::result test_driver::run_tests(const test_desc *tests)
{
    const string blank;
    return do_run_tests(tests, blank);
}

test_driver::result test_driver::run_test(const test_desc *tests,
					  const string &test_name)
{
    return do_run_tests(tests, test_name);
}

test_driver::result test_driver::do_run_tests(const test_desc *tests,
					      const string &testname)
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
		out << " ok." << endl;
	    } else {
		++result.failed;
		out << " FAILED" << endl;
		if (abort_on_error) {
		    out << "Test failed - aborting further tests." << endl;
		    break;
		}
	    }
	}
	++test;
    }
    return result;
}
