/* internaltest.cc - test of the OpenMuscat internals
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
#include <string>
#include <memory>
using std::cout;
using std::endl;

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else // HAVE_GETOPT_H
#include <stdlib.h>
#endif // HAVE_GETOPT_H

#include "om/om.h"
#include "testsuite.h"
#include "omrefcnt.h"
#include "omstringstream.h"

#ifdef MUS_BUILD_BACKEND_SLEEPY
#include "../backends/sleepy/sleepy_list.h"
#endif

// always succeeds
bool test_trivial();
// always fails (for testing the framework)
bool test_alwaysfail();
// test the test framework
bool test_testsuite1();
bool test_testsuite2();
bool test_testsuite3();


bool test_trivial()
{
    return true;
}

bool test_alwaysfail()
{
    return false;
}

bool test_except1()
{
    bool success = false;
    try {
	throw 1;
    } catch (int) {
	success = true;
    }
    return success;
}

char *duff_allocation = 0;

bool test_duffnew()
{
    // make an unfreed allocation
    duff_allocation = new char[7];
    return true;
}

bool test_testsuite1()
{
    bool success = true;

    test_desc mytests[] = {
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

    test_driver driver(mytests);
    driver.set_abort_on_error(true);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
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

// test the memory leak tests
bool test_testsuite3()
{
    test_desc mytests[] = {
	{"duff_new", test_duffnew},
	{0, 0}
    };

    test_driver driver(mytests);
    if (!verbose) {
	driver.set_quiet(true);
    }

    bool success = true;

    test_driver::result res = driver.run_tests();
    if (res.succeeded != 0 ||
	res.failed != 1) {
	if (verbose) {
	    cout << "Memory leak checking with new/delete doesn't work"
		 << endl;
	}
	success = false;
    }

    // clean up after test_duffnew()
    delete duff_allocation;
    duff_allocation = 0;

    return success;
}

#ifdef HAVE_NO_ACCESS_CONTROL
// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

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

#endif /* HAVE_NO_ACCESS_CONTROL */

// test string comparisions
bool test_stringcomp1()
{
    bool success = true;

    std::string s1;
    std::string s2;

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

bool test_omstringstream1()
{
    bool success = true;

    om_ostringstream oss;
    oss << "foo" << 4 << "bar";

    if (oss.str() != "foo4bar") {
	success = false;
	if (verbose) {
	    cout << "oss.str() returned `" << oss.str()
		 << "' instead of foo4bar" << endl;
	}
    }

    return success;
}

bool test_omstringstream2()
{
    bool success = true;

    om_ostringstream oss;

    oss << "a" << std::endl;

    std::string foo = oss.str();
    cout << oss.str() << endl;

    return success;
}

#ifdef MUS_BUILD_BACKEND_SLEEPY
// test whether a SleepyList packs and unpacks correctly
bool test_sleepypack1()
{
    bool success = true;

    SleepyListItem::id_type id = 7;
    om_doccount termfreq = 92;
    om_termcount wdf = 81;
    om_doclength doclen = 75;
    vector<om_termpos> positions;
    positions.push_back(6u);
    positions.push_back(16u);

    SleepyListItem item1(id, wdf, positions, termfreq, doclen);
    std::string packed1 = item1.pack(true);
    SleepyListItem item2(packed1, true);
    std::string packed2 = item2.pack(true);

    if(packed1 != packed2) {
	success = false;
	if(verbose) {
	    cout << "Packed items were not equal ('" << packed1 <<
		    "' and '" << packed2 << "'" << endl;
	}
    }
    if(item1.id != item2.id) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (ids '" << item1.id <<
		    "' and '" << item2.id << "')" << endl;
	}
    }
    if(item1.termfreq != item2.termfreq) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (termfreqs '" <<
		    item1.termfreq << "' and '" << item2.termfreq << "')" <<
		    endl;
	}
    }
    if(item1.wdf != item2.wdf) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (wdfs '" << item1.wdf <<
		    "' and '" << item2.wdf << "')" << endl;
	}
    }
    if(item1.positions != item2.positions) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (positions '";
	    copy(item1.positions.begin(), item1.positions.end(),
		 ostream_iterator<om_termpos>(cout, " "));
	    cout << "' and '";
	    copy(item2.positions.begin(), item2.positions.end(),
		 ostream_iterator<om_termpos>(cout, " "));
	    cout << "')" << endl;
	}
    }
    if(item1.doclength != item2.doclength) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (doclengths '" <<
		    item1.doclength << "' and '" << item2.doclength <<
		    "')" << endl;
	}
    }

    return success;
}
#endif

// ####################################
// # test the behaviour of OmSettings #
// ####################################
#if 0
bool
test_omsettings1()
{
    bool success = true;

    OmSettings settings;

    settings.set_value("K1", "V1");
    settings.set_value("K2", "V2");
    settings.set_value("K1", "V3");

    if (settings.get_value("K1") != "V3" ||
	settings.get_value("K2") != "V2") {
	success = false;
    }
    return success;
}

bool
test_omsettings2()
{
    bool success = false;

    OmSettings settings;
    try {
	settings.get_value("nonexistant");

	if (verbose) {
	    cout << "get_value() didn't throw with invalid key" << endl;
	}
    } catch (OmRangeError &e) {
	success = true;
    }

    return success;
}

bool
test_omsettings3()
{
    bool success = true;

    // test copy-on-write behaviour.
    OmSettings settings1;

    settings1.set_value("FOO", "BAR");
    settings1.set_value("MOO", "COW");

    OmSettings settings2(settings1);

    if (settings2.get_value("FOO") != "BAR" ||
	settings2.get_value("MOO") != "COW") {
	success = false;
	if (verbose) {
	    cout << "settings weren't copied properly." << endl;
	}
    }

    if (settings1.get_value("FOO") != "BAR" ||
	settings1.get_value("MOO") != "COW") {
	success = false;
	if (verbose) {
	    cout << "settings destroyed when copied." << endl;
	}
    }

    settings2.set_value("BOO", "AAH");

    try {
	settings1.get_value("BOO");
	// should throw

	success = false;
	if (verbose) {
	    cout << "Changes leaked to original" << endl;
	}
    } catch (OmRangeError &) {
    }

    settings1.set_value("FOO", "RAB");

    if (settings2.get_value("FOO") != "BAR") {
	success = false;

	if (verbose) {
	    cout << "Changes leaked to copy" << endl;
	}
    }

    return success;
}
#endif

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"except1",			test_except1},
    {"testsuite1",		test_testsuite1},
    {"testsuite2",		test_testsuite2},
    {"testsuite3",		test_testsuite3},
#ifdef HAVE_NO_ACCESS_CONTROL
    {"refcnt1",			test_refcnt1},
#endif HAVE_NO_ACCESS_CONTROL
    {"stringcomp1",		test_stringcomp1},
#ifdef MUS_BUILD_BACKEND_SLEEPY
    {"sleepypack1",		test_sleepypack1},
#endif
    {"omstringstream1",		test_omstringstream1},
    {"omstringstream2",		test_omstringstream2},
#if 0  // OmSettings currently disabled, and not used.  CME 20/6/2000
    {"omsettings1",		test_omsettings1},
    {"omsettings2",		test_omsettings2},
    {"omsettings3",		test_omsettings3},
#endif
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
