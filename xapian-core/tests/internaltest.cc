/* internaltest.cc - test of the OpenMuscat internals
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
#include <string>
#include <memory>
#include <getopt.h>
#include "om/om.h"
#include "testsuite.h"

// always succeeds
bool test_trivial();
// always fails (for testing the framework)
bool test_alwaysfail();
// test the test framework
bool test_testsuite1();
bool test_testsuite2();

test_desc tests[] = {
    {"testsuite1",		test_testsuite1},
    {"testsuite2",		test_testsuite2},
    {0, 0}
};

bool verbose = false;

void usage(char *progname)
{
    cerr << "Usage: " << progname << " [-v] [-o] [-f]" << endl;
}

int main(int argc, char *argv[])
{
    bool fussy = true;

    int c;

    test_driver driver;

    while ((c = getopt(argc, argv, "vof")) != EOF) {
	switch (c) {
	    case 'v':
		verbose = true;
		break;
	    case 'o':
		driver.set_abort_on_error(true);
		break;
	    case 'f':
	    	fussy = true;
		break;
	    default:
	    	usage(argv[0]);
		exit(1);
	}
    }

    if (optind != (argc)) {
    	usage(argv[0]);
	return 1;
    }

    test_driver::result myresult = driver.run_tests(tests);

    cout << "internaltest finished: "
         << myresult.succeeded << " tests passed, "
	 << myresult.failed << " failed."
	 << endl;
	
    // FIXME: fussy should be the default, but for the moment
    // we want distcheck to succeed even though the tests don't
    // all pass, so that we can get nightly snapshots.
    if (fussy) {
	return (bool)myresult.failed; // if 0, then everything passed
    } else {
	return 0;
    }
}

bool test_trivial()
{
    return true;
}

bool test_alwaysfail()
{
    return false;
}

bool test_testsuite1()
{
    bool success = true;

    test_desc mytests[] = {
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver;
    driver.set_abort_on_error(false);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests(mytests);
    if (res.succeeded != 1 ||
	res.failed != 1) {
	if (verbose) {
	    cout << res.succeeded << " succeeded, "
		 << res.failed << " failed." << endl;
	}
	success = false;
    }

    return success;
}

bool test_testsuite2()
{
    bool success = true;

    test_desc mytests[] = {
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver;
    driver.set_abort_on_error(true);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests(mytests);
    if (res.succeeded != 0 ||
	res.failed != 1) {
	if (verbose) {
	    cout << res.succeeded << " succeeded, "
		 << res.failed << " failed." << endl;
	}
	success = false;
    }

    return success;
}
