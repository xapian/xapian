/* nettest.cc: tests for the network matching code.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
#include <om/omenquire.h>
#include <unistd.h>

// Directory which the data is stored in.
string datadir;

// #######################################################################
// # Start of test cases.

// Test a simple network match
bool test_netmatch1()
{
    OmDatabaseGroup databases;
    vector<string> params;
    params.push_back("prog");
    params.push_back("./omprogsrv");
    params.push_back(datadir + "apitest_simpledata.txt");
    databases.add_database("net", params);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

// test a network match with two databases
bool test_netmatch2()
{
    OmDatabaseGroup databases;
    vector<string> params;
    params.push_back("prog");
    params.push_back("./omprogsrv");
    params.push_back(datadir + "apitest_simpledata.txt");
    databases.add_database("net", params);

    params.pop_back();
    params.push_back(datadir + "apitest_simpledata2.txt");
    databases.add_database("net", params);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

// test a simple network expand
bool test_netexpand1()
{
    OmDatabaseGroup databases;
    vector<string> params;
    params.push_back("prog");
    params.push_back("./omprogsrv");
    params.push_back(datadir + "apitest_simpledata.txt");
    databases.add_database("net", params);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
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
    string command =
	    string("./omtcpsrv --im ") +
	    datadir +
	    "apitest_simpledata.txt --port 1235 &";

    system(command.c_str());

    sleep(3);
    TcpClient tc("localhost", 1235);

    return true;
}

// test a tcp match
bool test_tcpmatch1()
{
    string command =
	    string("./omtcpsrv --im ") +
	    datadir +
	    "apitest_simpledata.txt --port 1236 &";
    system(command.c_str());
    sleep(3);

    OmDatabaseGroup databases;
    vector<string> params;
    params.push_back("tcp");
    params.push_back("localhost");
    params.push_back("1236");
    databases.add_database("net", params);

    OmEnquire enq(databases);

    enq.set_query(OmQuery("word"));

    OmMSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

// #######################################################################
// # End of test cases.

test_desc tests[] = {
    {"netmatch1",	test_netmatch1},
    {"netmatch2",	test_netmatch2},
    {"netexpand1",      test_netexpand1},
    {"tcpclient1",	test_tcpclient1},
    {"tcpmatch",	test_tcpmatch1},
    {0,			0},
};

int main(int argc, char *argv[])
{
    char *srcdir = getenv("srcdir");
    if (srcdir == NULL) {
        cout << "Error: $srcdir must be in the environment!" << endl;
	return(1);
    }
    datadir = std::string(srcdir) + "/../tests/testdata/";

    return test_driver::main(argc, argv, tests);
}
