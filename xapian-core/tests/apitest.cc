/* apitest.cc: tests the OpenMuscat API
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
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ostream_iterator;
using std::map;
using std::auto_ptr;
using std::max;
using std::ostream;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "textfile_indexer.h"
#include "../indexer/index_utils.h"
#include "backendmanager.h"
#include "utils.h"

#include "apitest.h"

#include "api_nodb.h"
#include "api_posdb.h"
#include "api_db.h"

OmDatabase
make_dbgrp(OmDatabase * db1, OmDatabase * db2,
	   OmDatabase * db3, OmDatabase * db4)
{
    OmDatabase result;

    if(db1 != 0) result.add_database(*db1);
    if(db2 != 0) result.add_database(*db2);
    if(db3 != 0) result.add_database(*db3);
    if(db4 != 0) result.add_database(*db4);

    return result;
}

static BackendManager backendmanager;

OmDatabase
get_database(const string &dbname, const string &dbname2)
{
    return backendmanager.get_database(dbname, dbname2);
}

OmWritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager.get_writable_database(dbname);
}

#define RUNTESTS(B, T) if (backend.empty() || backend == (B)) {\
    test_driver::result sum_temp;\
    backendmanager.set_dbtype((B));\
    cout << "Running " << #T << " tests with " << (B) << " backend..." << endl;\
    result = max(result, test_driver::main(argc, argv, T##_tests, &sum_temp));\
    summary.succeeded += sum_temp.succeeded;\
    summary.failed += sum_temp.failed; } else (void)0

int main(int argc, char *argv[])
{
    string srcdir = test_driver::get_srcdir(argv[0]);
    string backend;
    const char *p = getenv("OM_TEST_BACKEND");
    if (p) backend = p;

    int result = 0;
    test_driver::result summary = {0, 0};

    backendmanager.set_datadir(srcdir + "/testdata/");

    RUNTESTS("void", nodb);

#if 1 && defined(MUS_BUILD_BACKEND_INMEMORY)
    RUNTESTS("inmemory", db);
    RUNTESTS("inmemory", writabledb);
    RUNTESTS("inmemory", localdb);
    RUNTESTS("inmemory", positionaldb);
    RUNTESTS("inmemory", doclendb);
#endif

#if 1 && defined(MUS_BUILD_BACKEND_QUARTZ)
    RUNTESTS("quartz", db);
    RUNTESTS("quartz", writabledb);
    RUNTESTS("quartz", localdb);
    RUNTESTS("quartz", positionaldb);
    RUNTESTS("quartz", doclendb);
#endif

#if 1 && defined(MUS_BUILD_BACKEND_SLEEPYCAT)
    RUNTESTS("sleepycat", db);
    RUNTESTS("sleepycat", writabledb);
    RUNTESTS("sleepycat", localdb);
    RUNTESTS("sleepycat", positionaldb);
    RUNTESTS("sleepycat", doclendb);
#endif

#if 1 && defined(MUS_BUILD_BACKEND_REMOTE)
    RUNTESTS("remote", db);
    RUNTESTS("remote", positionaldb);
    RUNTESTS("remote", doclendb);
#endif

#if 1 && defined(MUS_BUILD_BACKEND_MUSCAT36)
    // need makeDA tool to build da databases
    if (file_exists("../../makeda/makeDA")) {
	RUNTESTS("da", db);
    }
#endif

    cout << argv[0] << " total: " << summary.succeeded << " passed, "
	 << summary.failed << " failed." << endl;

    return result;
}
