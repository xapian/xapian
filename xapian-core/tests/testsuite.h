/* testsuite.h - a test suite engine
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

#ifndef OM_HGUARD_TESTSUITE_H
#define OM_HGUARD_TESTSUITE_H

#include <iostream>
#include <string>
#include "om/omerror.h"

typedef bool (*test_func)();

struct test_desc {
  char *name;
  test_func run;
};

class test_driver {
    public:
	test_driver() : abort_on_error(false) {};


	struct result {
	    unsigned int succeeded;
	    unsigned int failed;
	};

	result run_tests(const test_desc *tests);

	void set_abort_on_error(bool aoe_);
    private:
	bool runtest(const test_desc *test);
	
	// abort tests at the first failure
	bool abort_on_error;
};

inline void test_driver::set_abort_on_error(bool aoe_)
{
    abort_on_error = aoe_;
}

#endif  // OM_HGUARD_TESTSUITE_H
