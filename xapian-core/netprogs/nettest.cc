/* nettest.cc: tests for the network matching code.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include "progclient.h"
#include "tcpclient.h"
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include <xapian/enquire.h>
#include "utils.h"
#include <unistd.h>
#include <string>

#include <sys/types.h>
#include <signal.h> // for kill()
#include <sys/wait.h>

using namespace std;

// Directory which the data is stored in.
string datadir;

// #######################################################################
// # Start of test cases.

// Test a simple network match
static bool test_netmatch1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("remote");
    backendmanager.set_datadir(datadir);
    vector<string> paths;
    paths.push_back("apitest_simpledata");

    OmEnquire enq(backendmanager.get_database(paths));

    enq.set_query(Xapian::Query("word"));

    Xapian::MSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

// test a network match with two databases
static bool test_netmatch2()
{
    OmDatabase databases;
    BackendManager backendmanager;
    backendmanager.set_dbtype("remote");
    backendmanager.set_datadir(datadir);
    vector<string> paths;

    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);
    databases.add_database(db);

    paths[0] = "apitest_simpledata2";
    db = backendmanager.get_database(paths);
    databases.add_database(db);

    OmEnquire enq(databases);

    enq.set_query(Xapian::Query("word"));

    Xapian::MSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

// test a simple network expand
static bool test_netexpand1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("remote");
    backendmanager.set_datadir(datadir);
    vector<string> paths;
    paths.push_back("apitest_simpledata");

    OmEnquire enq(backendmanager.get_database(paths));

    enq.set_query(Xapian::Query("word"));

    Xapian::MSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    Assert(mset.size() > 0);

    OmRSet rset;
    rset.add_document(*mset.begin());

    Xapian::ESet eset(enq.get_eset(10, rset));

    return true;
}

// test a tcp connection
static bool test_tcpclient1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("quartz");
    backendmanager.set_datadir(datadir);
    vector<string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase db = backendmanager.get_database(paths);

    string command = "./omtcpsrv --one-shot --quiet --port 1236 "
	                  ".quartz/db=apitest_simpledata &";
    system(command);

    sleep(3);
    TcpClient tc("localhost", 1236, 10000, 10000);

    return true;
}

// test a tcp match
static bool test_tcpmatch1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("quartz");
    backendmanager.set_datadir(datadir);
    vector<string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase dbremote = backendmanager.get_database(paths);

    string command = "./omtcpsrv --one-shot --quiet --port 1235 "
	                  ".quartz/db=apitest_simpledata &";
    system(command);
    sleep(3);

    OmDatabase db = OmRemote__open("localhost", 1235);

    OmEnquire enq(db);

    enq.set_query(Xapian::Query("word"));

    Xapian::MSet mset(enq.get_mset(0, 10));

    if (verbose) {
	cout << mset;
    }

    return true;
}

#if 0
// test a tcp match when the remote end dies
static bool test_tcpdead1()
{
    BackendManager backendmanager;
    backendmanager.set_dbtype("quartz");
    backendmanager.set_datadir(datadir);
    vector<string> paths;
    paths.push_back("apitest_simpledata");
    OmDatabase dbremote = backendmanager.get_database(paths);

    int pid = fork();
    if (pid == 0) {
	// child code
	char *args[] = {
	    "./omtcpsrv",
	    "--one-shot",
	    "--quiet",
	    "--port",
	    "1237",
	    ".quartz/db=apitest_simpledata",
	    NULL
	};
	// FIXME: we run this directly so we know the pid of the omtcpsrv
	// parent - below we assume the child is the next pid (which isn't
	// necessarily true)
	execv(".libs/lt-omtcpsrv", args);
	// execv only returns if it couldn't start omtcpsrv
	exit(1);
    } else if (pid < 0) {
	// fork() failed
	FAIL_TEST("fork() failed");
    }

    sleep(3);

    // parent code:
    OmDatabase db(OmRemote__open("localhost", 1237);

    OmEnquire enq(db);

    // FIXME: this assumes fork-ed child omtcpsrv process is the pid after
    // the parent
    if (kill(pid + 1, SIGTERM) == -1) {
	FAIL_TEST("Couldn't send signal to child");
    }

    sleep(3);

//    cout << pid << endl;
//    system("ps x | grep omtcp");
    
    time_t t = time(NULL);
    try {
	enq.set_query(Xapian::Query("word"));	
	Xapian::MSet mset(enq.get_mset(0, 10));
    }
    catch (const Xapian::NetworkError &e) {
	time_t t2 = time(NULL) - t;
	if (t2 > 1) {
	    FAIL_TEST("Client took too long to notice server died (" +
		      om_tostring(t2) + " secs)");
	}
	return true;
    }
    FAIL_TEST("Client didn't get exception when server died");
    return false;
}
#endif

// #######################################################################
// # End of test cases.

test_desc tests[] = {
    {"netmatch1",	test_netmatch1},
    {"netmatch2",	test_netmatch2},
    {"netexpand1",      test_netexpand1},
    {"tcpclient1",	test_tcpclient1},
    {"tcpmatch1",	test_tcpmatch1},
// disable until we can work out how to kill the right process cleanly
    //{"tcpdead1",	test_tcpdead1},
    {0,			0},
};

int main(int argc, char **argv)
{
    test_driver::parse_command_line(argc, argv);
    string srcdir = test_driver::get_srcdir();

    datadir = srcdir + "/../tests/testdata/";

    return test_driver::run(tests);
}
