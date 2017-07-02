/* apitest.cc: tests the Xapian API
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006,2007,2008,2009 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "apitest.h"

#include "api_all.h"
#include "backendmanager.h"
#include "stringutils.h"
#include "testrunner.h"
#include "testsuite.h"

#include <xapian.h>

#include <string>
#include <vector>

using namespace std;

Xapian::Database
get_database(const string &dbname)
{
    return backendmanager->get_database(dbname);
}

Xapian::Database
get_database(const string &dbname, const string &dbname2)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    dbnames.push_back(dbname2);
    return backendmanager->get_database(dbnames);
}

string
get_database_path(const string &dbname)
{
    return backendmanager->get_database_path(dbname);
}

Xapian::Database
get_database(const std::string &dbname,
	     void (*gen)(Xapian::WritableDatabase&,
			 const std::string &),
	     const std::string &arg)
{
    return backendmanager->get_database(dbname, gen, arg);
}

class ApiTestRunner : public TestRunner
{
  public:
    int run() const {
	int result = 0;
#include "api_collated.h"
	test_driver::report(test_driver::subtotal,
			    "backend " + backendmanager->get_dbtype());
	test_driver::total += test_driver::subtotal;
	test_driver::subtotal.reset();
	return result;
    }
};

int main(int argc, char **argv)
{
    ApiTestRunner runner;
    return runner.run_tests(argc, argv);
}
