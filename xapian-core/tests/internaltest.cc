/* internaltest.cc: test of the Xapian internals
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include <string>
#include <dlfcn.h>

using namespace std;

#include "om/om.h"
#include "testsuite.h"
#include "refcnt.h"
#include "omstringstream.h"

// always succeeds
static bool test_trivial();
// always fails (for testing the framework)
static bool test_alwaysfail();
// test the test framework
static bool test_testsuite1();
static bool test_testsuite2();
static bool test_testsuite3();
static bool test_testsuite4();

static bool test_trivial()
{
    return true;
}

static bool test_alwaysfail()
{
    return false;
}

static bool test_skip()
{
    SKIP_TEST("skip test");
}

static bool test_except1()
{
    try {
	throw 1;
    } catch (int) {
    }
    return true;
}

class duffnew_test {
   duffnew_test() { *foo = 0; }
   char foo[7];
};

duffnew_test *duff_allocation = 0;
duffnew_test *duff_allocation_2 = 0;

static bool test_duffnew()
{
    // make an unfreed allocation
    duff_allocation_2 = duff_allocation;
    duff_allocation = new duffnew_test; // char[7];
    return true;
}

char *duff_malloc_allocation = 0;
char *duff_malloc_allocation_2 = 0;

static bool test_duffmalloc()
{
    // make an unfreed allocation
    duff_malloc_allocation_2 = duff_malloc_allocation;
    duff_malloc_allocation = (char *)malloc(7);
    return true;
}

static bool test_testsuite1()
{
    test_desc mytests[] = {
	{"test0", test_skip},
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver(mytests);
    driver.set_abort_on_error(false);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 1 && res.failed == 1 && res.skipped == 1,
		     res.succeeded << " succeeded, "
		     << res.failed << " failed,"
		     << res.skipped << " skipped.");

    return true;
}

static bool test_testsuite2()
{
    test_desc mytests[] = {
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver(mytests);
    driver.set_abort_on_error(true);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     res.succeeded << " succeeded, "
		     << res.failed << " failed.");

    return true;
}

/* In some systems <dlfcn.h> doesn't define RTLD_DEFAULT */
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT (void *)0
#endif

// test the memory leak tests
static bool test_testsuite3()
{
    // Note that duffnew leaks (deliberately), so it'll get run twice.
    // Bear this in mind if you're trying to debug stuff round here...
    test_desc mytests[] = {
	{"duff_new", test_duffnew},
	{0, 0}
    };

    test_driver driver(mytests);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     "Memory leak checking with new/delete doesn't work");

    // clean up after test_duffnew()
    delete duff_allocation;
    duff_allocation = 0;
    delete duff_allocation_2;
    duff_allocation_2 = 0;

    return true;
}

// test the malloc() memory leak tests
static bool test_testsuite4()
{
    test_desc mytests[] = {
	{"duff_malloc", test_duffmalloc},
	{0, 0}
    };

#if 0
    if (!dlsym(RTLD_DEFAULT, "malloc_allocdata")) {
#endif
	SKIP_TEST("malloc tracking library not installed");
#if 0
    }

    test_driver driver(mytests);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     "Memory leak checking with malloc()/free() doesn't work");

    // clean up after test_duffmalloc()
    if (duff_malloc_allocation) {
	free(duff_malloc_allocation);
	duff_malloc_allocation = 0;
    }
    if (duff_malloc_allocation_2) {
	free(duff_malloc_allocation_2);
	duff_malloc_allocation_2 = 0;
    }

    return true;
#endif
}

class Test_Exception {
    public:
	int value;
	Test_Exception(int value_) : value(value_) {}
};

static bool test_exception1()
{
    try {
	try {
	    throw Test_Exception(1);
	} catch (...) {
	    try {
		throw Test_Exception(2);
	    } catch (...) {
	    }
	}
    } catch (Test_Exception & e) {
	TEST_EQUAL(e.value, 1);
    }
    return true;
}

// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

#ifdef HAVE_NO_ACCESS_CONTROL
class test_refcnt : public RefCntBase {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    if (verbose) {
	        cout << " constructor ";
	    }
	}
	RefCntPtr<const test_refcnt> test() {
	    return RefCntPtr<const test_refcnt>(RefCntPtrToThis(), this);
	}
	~test_refcnt() {
	    deleted = true;
	    if (verbose) {
		cout << " destructor ";
	    }
	}
};
#endif

static bool test_refcnt1()
{
#ifdef HAVE_NO_ACCESS_CONTROL
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    TEST_EQUAL(p->ref_count, 0);

    {
	RefCntPtr<test_refcnt> rcp(p);

	TEST_EQUAL(rcp->ref_count, 1);
	
	{
	    RefCntPtr<test_refcnt> rcp2;
	    rcp2 = rcp;
	    TEST_EQUAL(rcp->ref_count, 2);
	    // rcp2 goes out of scope here
	}
	
	TEST_AND_EXPLAIN(!deleted, "Object prematurely deleted!");
	TEST_EQUAL(rcp->ref_count, 1);
	// rcp goes out of scope here
    }
    
    TEST_AND_EXPLAIN(deleted, "Object not properly deleted");

    return true;
#else
    SKIP_TEST("Unable to disable class member access checking in C++ compiler");
#endif
}

// This is a regression test - a RefCntPtr used to delete the object pointed
// to if it was the reference count was 1 and you assigned it to itself.
static bool test_refcnt2()
{
#ifdef HAVE_NO_ACCESS_CONTROL
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    RefCntPtr<test_refcnt> rcp(p);
    
    rcp = rcp;
    
    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
#else
    SKIP_TEST("Unable to disable class member access checking in C++ compiler");
#endif
}

// test string comparisions
static bool test_stringcomp1()
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

static bool test_omstringstream1()
{
    om_ostringstream oss;
    oss << "foo" << 4 << "bar";
    TEST_EQUAL(oss.str(), "foo4bar");

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"except1",			test_except1},
    {"testsuite1",		test_testsuite1},
    {"testsuite2",		test_testsuite2},
//FIXME: disabled until we have leak checking again...
//    {"testsuite3",		test_testsuite3},
    {"testsuite4",		test_testsuite4},
    {"exception1",              test_exception1},
    {"refcnt1",			test_refcnt1},
    {"refcnt2",			test_refcnt2},
    {"stringcomp1",		test_stringcomp1},
    {"omstringstream1",		test_omstringstream1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
