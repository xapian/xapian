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
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

#include <xapian.h>
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include "utils.h"

#include "apitest.h"

#include "api_nodb.h"
#include "api_posdb.h"
#include "api_db.h"
#include "api_wrdb.h"
#include "api_anydb.h"
#include "api_transdb.h"

static BackendManager backendmanager;

const std::string & get_dbtype()
{
    return backendmanager.get_dbtype();
}

Xapian::Database
get_database(const string &dbname)
{
    return backendmanager.get_database(dbname);
}

Xapian::Database
get_database(const string &dbname, const string &dbname2)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    dbnames.push_back(dbname2);
    return backendmanager.get_database(dbnames);
}

Xapian::Database
get_network_database(const string &dbname,
		     unsigned int timeout)
{
    vector<string> params;
    params.push_back("#TIMEOUT#");
    params.push_back(om_tostring(timeout));
    params.push_back(dbname);
    return backendmanager.get_database(params);
}

Xapian::WritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager.get_writable_database(dbname);
}

#define RUNTESTS(B, T) if (backend.empty() || backend == (B)) {\
    backendmanager.set_dbtype((B));\
    cout << "Running " << #T << " tests with " << (B) << " backend..." << endl;\
    result = max(result, test_driver::run(T##_tests));\
    } else (void)0

int main(int argc, char **argv)
{
    string backend;
    test_driver::add_command_line_option("backend", 'b', &backend);

    test_driver::parse_command_line(argc, argv);

    string srcdir = test_driver::get_srcdir();

    int result = 0;

    backendmanager.set_datadir(srcdir + "/testdata/");

    RUNTESTS("void", nodb);

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
    RUNTESTS("inmemory", anydb);
    RUNTESTS("inmemory", specchar);
    RUNTESTS("inmemory", writabledb);
    RUNTESTS("inmemory", localdb);
    RUNTESTS("inmemory", positionaldb);
    RUNTESTS("inmemory", localpositionaldb);
    RUNTESTS("inmemory", doclendb);
    RUNTESTS("inmemory", collfreq);
    RUNTESTS("inmemory", allterms);
    RUNTESTS("inmemory", multivalue);
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
    RUNTESTS("flint", anydb);
    RUNTESTS("flint", specchar);
    RUNTESTS("flint", writabledb);
    RUNTESTS("flint", localdb);
    RUNTESTS("flint", positionaldb);
    RUNTESTS("flint", localpositionaldb);
    RUNTESTS("flint", doclendb);
    RUNTESTS("flint", collfreq);
    RUNTESTS("flint", allterms);
    RUNTESTS("flint", multivalue);
    RUNTESTS("flint", transactiondb);
    RUNTESTS("flint", flint);
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
    RUNTESTS("quartz", anydb);
    RUNTESTS("quartz", specchar);
    RUNTESTS("quartz", writabledb);
    RUNTESTS("quartz", localdb);
    RUNTESTS("quartz", positionaldb);
    RUNTESTS("quartz", localpositionaldb);
    RUNTESTS("quartz", doclendb);
    RUNTESTS("quartz", collfreq);
    RUNTESTS("quartz", allterms);
    RUNTESTS("quartz", multivalue);
    RUNTESTS("quartz", transactiondb);
    RUNTESTS("quartz", quartz);
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
    RUNTESTS("remote", anydb);
    RUNTESTS("remote", specchar);
    RUNTESTS("remote", writabledb);
    RUNTESTS("remote", remotedb);
    RUNTESTS("remote", positionaldb);
    RUNTESTS("remote", doclendb);
    RUNTESTS("remote", multivalue);
    RUNTESTS("remote", transactiondb);

    RUNTESTS("remotetcp", anydb);
    RUNTESTS("remotetcp", specchar);
    RUNTESTS("remotetcp", writabledb);
    RUNTESTS("remotetcp", remotedb);
    RUNTESTS("remotetcp", positionaldb);
    RUNTESTS("remotetcp", doclendb);
    RUNTESTS("remotetcp", multivalue);
    RUNTESTS("remotetcp", transactiondb);
#endif

    return result;
}
