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
#include "omrefcnt.h"

// always succeeds
bool test_trivial();
// always fails (for testing the framework)
bool test_alwaysfail();
// test the test framework
bool test_testsuite1();
bool test_testsuite2();
// test the reference counted pointers
bool test_refcnt1();
// test string comparisions
bool test_stringcomp1();

test_desc tests[] = {
    {"testsuite1",		test_testsuite1},
    {"testsuite2",		test_testsuite2},
    {"refcnt1",			test_refcnt1},
    {"stringcomp1",		test_stringcomp1},
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

class test_refcnt : public OmRefCntBase {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    if (verbose) {
	        cout << " constructor ";
	    }
	}
	~test_refcnt() {
	    deleted = true;
	    if (verbose) {
		cout << " destructor ";
	    }
	}
};

bool test_refcnt1()
{
    bool success = true;

    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    if (p->ref_count != 0) {
	success = false;
	if (verbose) {
	    cout << "Plain ref-counted class has ref count "
		 << p->ref_count << endl;
	}
	delete p;
    } else {
	OmRefCntPtr<test_refcnt> rcp(p);

	if (rcp->ref_count != 1) {
	    success = false;
	    if (verbose) {
		cout << "Reference count not 1: "
	             << rcp->ref_count << endl;
	    }
	}

	{
	    OmRefCntPtr<test_refcnt> rcp2;
	    rcp2 = rcp;
	    if (rcp->ref_count != 2) {
		success = false;
		if (verbose) {
		    cout << "Refcount not 2: "
			 << rcp->ref_count << endl;
		}
	    }
	    // rcp2 goes out of scope here
	}

	if (deleted) {
	    success = false;
	    if (verbose) {
		cout << "Object prematurely deleted!" << endl;
	    }
	} else if (rcp->ref_count != 1) {
	    success = false;
	    if (verbose) {
		cout << "Reference count not 1 again: "
	             << rcp->ref_count << endl;
	    }
	}
    }

    if (deleted != true) {
	success = false;
	if (verbose) {
	    cout << "Object not properly deleted"
		 << endl;
	}
    }

    return success;
}

bool test_stringcomp1()
{
    bool success = true;

    string s1;
    string s2;

    s1 = "foo";
    s2 = "foo";

    if ((s1 != s2) || (s1 > s2)) {
	success = false;
	if (verbose) {
	    cout << "String comparisons BADLY wrong" << endl;
	}
    }

    s1 += '\0';

    if ((s1 == s2) || (s1 < s2)) {
	success = false;
	if (verbose) {
	    cout << "String comparisions don't cope with extra nulls" << endl;
	}
    }

    s2 += '\0';

    s1 += 'a';
    s2 += 'z';

    if ((s1.length() != 5) || (s2.length() != 5)) {
	success = false;
	if (verbose) {
	    cout << "Lengths with added nulls wrong" << endl;
	}
    }

    if ((s1 == s2) || !(s1 < s2)) {
	success = false;
	if (verbose) {
	    cout << "Characters after a null ignored in comparisons" << endl;
	}
    }

    return success;
}
