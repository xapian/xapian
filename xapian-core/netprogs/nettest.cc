/* nettest.cc: tests for the network matching code.
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

#include "progclient.h"
#include "tcpclient.h"
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include <om/omenquire.h>
#include <unistd.h>
#include <string>

// Directory which the data is stored in.
std::string datadir;

// #######################################################################
// # Start of test cases.

// Test a simple network match
bool test_netmatch1()
{
    OmDatabaseGroup databases;
    BackendManager backendmanager;
    backendmanager.set_dbtype("net");
    backendmanager.set_datadir(datadir);
    std::vector<std::string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);
    databases.add_database(db);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	std::cout << mset;
    }

    return true;
}

// test a network match with two databases
bool test_netmatch2()
{
    OmDatabaseGroup databases;
    BackendManager backendmanager;
    backendmanager.set_dbtype("net");
    backendmanager.set_datadir(datadir);
    std::vector<std::string> paths;

    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);
    databases.add_database(db);

    paths[0] = "apitest_simpledata2";
    db = backendmanager.get_database(paths);
    databases.add_database(db);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	std::cout << mset;
    }

    return true;
}

// test a simple network expand
bool test_netexpand1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("net");
    backendmanager.set_datadir(datadir);
    std::vector<std::string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);

    OmDatabaseGroup databases;
    databases.add_database(db);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	std::cout << mset;
    }

    Assert(mset.items.size() > 0);

    OmRSet rset;
    rset.add_document(mset.items[0].did);

    OmESet eset(enq.get_eset(10, rset));

    return true;
}

// test a tcp connection
bool test_tcpclient1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("sleepycat");
    backendmanager.set_datadir(datadir);
    std::vector<std::string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);

    std::string command =
	    std::string("./omtcpsrv --one-shot --sleepycat ") +
	    ".sleepy/db=apitest_simpledata" +
	    " --port 1235 &";

    system(command.c_str());

    sleep(1);
    TcpClient tc("localhost", 1235);

    return true;
}

// test a tcp match
bool test_tcpmatch1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("sleepycat");
    backendmanager.set_datadir(datadir);
    std::vector<std::string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);

    std::string command =
	    std::string("./omtcpsrv --one-shot --sleepycat ") +
	    ".sleepy/db=apitest_simpledata" +
	    " --port 1236 &";
    system(command.c_str());
    sleep(1);

    OmDatabaseGroup databases;
    std::vector<std::string> params;
    params.push_back("tcp");
    params.push_back("localhost");
    params.push_back("1236");
    databases.add_database("net", params);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	std::cout << mset;
    }

    return true;
}

// #######################################################################
// # End of test cases.

test_desc tests[] = {
    {"netmatch1",	test_netmatch1},
    {"netmatch2",	test_netmatch2},
    {"netexpand1",      test_netexpand1},
#ifdef MUS_BUILD_BACKEND_SLEEPY
    {"tcpclient1",	test_tcpclient1},
    {"tcpmatch1",	test_tcpmatch1},
#endif
    {0,			0},
};

int main(int argc, char *argv[])
{
    char *srcdir = getenv("srcdir");
    if (srcdir == NULL) {
        std::cout << "Error: $srcdir must be in the environment!" << std::endl;
	return(1);
    }
    datadir = std::string(srcdir) + "/../tests/testdata/";

    return test_driver::main(argc, argv, tests);
}
