/* apitest.cc: tests the Xapian API
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006,2007 Olly Betts
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

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include "backendmanager_flint.h"
#include "backendmanager_inmemory.h"
#include "backendmanager_multi.h"
#include "backendmanager_quartz.h"
#include "backendmanager_remoteprog.h"
#include "backendmanager_remotetcp.h"
#include "utils.h"

#include "api_anydb.h"
#include "api_nodb.h"
#include "api_posdb.h"
#include "api_sorting.h"
#include "api_transdb.h"
#include "api_unicode.h"
#include "api_wrdb.h"

static BackendManager * backendmanager;

const char * get_dbtype()
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

Xapian::WritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager->get_writable_database(dbname);
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

#define RUNTESTS(B, T) if (backend_name.empty() || backend_name == (B)) {\
    cout << "Running " << #T << " tests with " << (B) << " backend..." << endl;\
    result = max(result, test_driver::run(T##_tests));\
    } else (void)0

#define USE_BACKEND(B, S) ((B).empty() || (B) == (S))

int main(int argc, char **argv)
{
    string backend_name;
    test_driver::add_command_line_option("backend", 'b', &backend_name);

    test_driver::parse_command_line(argc, argv);

    string srcdir = test_driver::get_srcdir();

    int result = 0;

    if (USE_BACKEND(backend_name, "none")) {
	backendmanager = new BackendManager;
	backendmanager->set_datadir(srcdir + "/testdata/");

	bool backend = false, remote = false, transactions = false;
	bool positional = false, writable = false, multi = false;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
    if (USE_BACKEND(backend_name, "inmemory")) {
	backendmanager = new BackendManagerInMemory;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("inmemory", specchar);
	RUNTESTS("inmemory", localdb);
	RUNTESTS("inmemory", doclendb);
	RUNTESTS("inmemory", collfreq);
	RUNTESTS("inmemory", allterms);
	RUNTESTS("inmemory", multivalue);

	bool backend = true, remote = false, transactions = false;
	bool positional = true, writable = true, multi = false;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (USE_BACKEND(backend_name, "flint")) {
	backendmanager = new BackendManagerFlint;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("flint", specchar);
	RUNTESTS("flint", localdb);
	RUNTESTS("flint", doclendb);
	RUNTESTS("flint", collfreq);
	RUNTESTS("flint", allterms);
	RUNTESTS("flint", multivalue);
	RUNTESTS("flint", flint);

	bool backend = true, remote = false, transactions = true;
	bool positional = true, writable = true, multi = false;
	bool spelling = true, metadata = true;
#include "api_collated.h"

	delete backendmanager;
    }
#endif

#if defined(XAPIAN_HAS_FLINT_BACKEND) || defined(XAPIAN_HAS_QUARTZ_BACKEND)
    if (USE_BACKEND(backend_name, "multi")) {
	backendmanager = new BackendManagerMulti;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("multi", specchar);
	RUNTESTS("multi", localdb);
	RUNTESTS("multi", doclendb);
	RUNTESTS("multi", collfreq);
	RUNTESTS("multi", allterms);
	RUNTESTS("multi", multivalue);
	RUNTESTS("multi", flint);

	bool backend = true, remote = false, transactions = false;
	bool positional = true, writable = false, multi = true;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
    if (USE_BACKEND(backend_name, "quartz")) {
	backendmanager = new BackendManagerQuartz;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("quartz", specchar);
	RUNTESTS("quartz", localdb);
	RUNTESTS("quartz", doclendb);
	RUNTESTS("quartz", collfreq);
	RUNTESTS("quartz", allterms);
	RUNTESTS("quartz", multivalue);
	RUNTESTS("quartz", quartz);

	bool backend = true, remote = false, transactions = true;
	bool positional = true, writable = true, multi = false;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    if (USE_BACKEND(backend_name, "remoteprog")) {
	backendmanager = new BackendManagerRemoteProg;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("remoteprog", specchar);
	RUNTESTS("remoteprog", remotedb);
	RUNTESTS("remoteprog", doclendb);
	RUNTESTS("remoteprog", collfreq);
	RUNTESTS("remoteprog", allterms);
	RUNTESTS("remoteprog", multivalue);

	bool backend = true, remote = true, transactions = true;
	bool positional = true, writable = true, multi = false;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }

    if (USE_BACKEND(backend_name, "remotetcp")) {
	backendmanager = new BackendManagerRemoteTcp;
	backendmanager->set_datadir(srcdir + "/testdata/");

	RUNTESTS("remotetcp", specchar);
	RUNTESTS("remotetcp", remotedb);
	RUNTESTS("remotetcp", doclendb);
	RUNTESTS("remotetcp", collfreq);
	RUNTESTS("remotetcp", allterms);
	RUNTESTS("remotetcp", multivalue);

	bool backend = true, remote = true, transactions = true;
	bool positional = true, writable = true, multi = false;
	bool spelling = false, metadata = false;
#include "api_collated.h"

	delete backendmanager;
    }
#endif

    return result;
}
