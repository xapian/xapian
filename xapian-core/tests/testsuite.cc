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
#include <string>
#include "om/omerror.h"
#include "testsuite.h"


//  A wrapper around the tests to trap exceptions,
//  and avoid having to catch them in every test function.
bool test_driver::runtest(const test_desc *test)
{
    bool success;
    try {
        success = test->run();
    } catch (OmError &err) {
	cout << "OmError exception: " << err.get_msg();
	success = false;
    } catch (...) {
	cout << "Unknown exception! ";
	success = false;
    }
    return success;
}

test_driver::result test_driver::run_tests(const test_desc *tests)
{
    test_desc *test = &tests[0];
    test_driver::result result = {0, 0};

    while ((test->name) != 0) {
    	cout << "Running test: " << test->name << "...";
	cout.flush();
	bool succeeded = runtest(test);
	if (succeeded) {
	    ++result.succeeded;
	    cout << " ok." << endl;
	} else {
	    ++result.failed;
	    cout << " FAILED" << endl;
	    if (abort_on_error) {
	        cout << "Test failed - aborting further tests." << endl;
		break;
	    }
	}
	++test;
    }
    return result;
}
