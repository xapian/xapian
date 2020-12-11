/** @file
 * @brief tests the Xapian API
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006,2007,2008,2009,2018 Olly Betts
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

std::string get_dbtype()
{
    return backendmanager->get_dbtype();
}

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

Xapian::Database
get_database(const std::string &dbname,
	     void (*gen)(Xapian::WritableDatabase&,
			 const std::string &),
	     const std::string &arg)
{
    return backendmanager->get_database(dbname, gen, arg);
}

string
get_database_path(const string &dbname)
{
    return backendmanager->get_database_path(dbname);
}

string
get_database_path(const std::string &dbname,
		  void (*gen)(Xapian::WritableDatabase&,
			      const std::string &),
		  const std::string &arg)
{
    return backendmanager->get_database_path(dbname, gen, arg);
}

Xapian::WritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager->get_writable_database("dbw", dbname);
}

Xapian::WritableDatabase
get_named_writable_database(const std::string &name, const std::string &source)
{
    return backendmanager->get_writable_database("dbw__" + name, source);
}

std::string
get_named_writable_database_path(const std::string &name)
{
    return backendmanager->get_writable_database_path("dbw__" + name);
}

std::string
get_compaction_output_path(const std::string& name)
{
    return backendmanager->get_compaction_output_path(name);
}

Xapian::Database
get_remote_database(const string &dbname, unsigned int timeout)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return backendmanager->get_remote_database(dbnames, timeout);
}

Xapian::Database
get_writable_database_as_database()
{
    return backendmanager->get_writable_database_as_database();
}

Xapian::WritableDatabase
get_writable_database_again()
{
    return backendmanager->get_writable_database_again();
}

void
skip_test_unless_backend(const std::string & backend_prefix)
{
    if (!startswith(get_dbtype(), backend_prefix)) {
	SKIP_TEST("Test only supported for " << backend_prefix << " backend");
    }
}

void
skip_test_for_backend(const std::string & backend_prefix)
{
    if (startswith(get_dbtype(), backend_prefix)) {
	SKIP_TEST("Test not supported for " << backend_prefix << " backend");
    }
}

void
XFAIL_FOR_BACKEND(const std::string& backend_prefix,
		  const char* msg)
{
    if (startswith(get_dbtype(), backend_prefix)) {
	XFAIL(msg);
    }
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
